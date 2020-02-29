#include "order_list_panel.hpp"
#include <QPainter>
#include <QFontMetrics>
#include <QFontInfo>
#include <QApplication>
#include <QClipboard>
#include <QMenu>
#include <QAction>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QPoint>
#include <QString>
#include <QIcon>
#include <algorithm>
#include <vector>
#include <utility>
#include <thread>
#include "gui/event_guard.hpp"
#include "gui/command/order/order_commands.hpp"
#include "playback.hpp"
#include "track.hpp"

OrderListPanel::OrderListPanel(QWidget *parent)
	: QWidget(parent),
	  config_(std::make_shared<Configuration>()),	// Dummy
	  rowFontWidth_(0),
	  rowFontHeight_(0),
	  rowFontAscent_(0),
	  rowFontLeading_(0),
	  headerFontAscent_(0),
	  widthSpace_(0),
	  rowNumWidthCnt_(0),
	  rowNumWidth_(0),
	  rowNumBase_(0),
	  trackWidth_(0),
	  columnsWidthFromLeftToEnd_(0),
	  headerHeight_(0),
	  curRowBaselineY_(0),
	  curRowY_(0),
	  leftTrackNum_(0),
	  curSongNum_(0),
	  curPos_{ 0, 0 },
	  hovPos_{ -1, -1 },
	  mousePressPos_{ -1, -1 },
	  mouseReleasePos_{ -1, -1 },
	  selLeftAbovePos_{ -1, -1 },
	  selRightBelowPos_{ -1, -1 },
	  shiftPressedPos_{ -1, -1 },
	  isIgnoreToSlider_(false),
	  isIgnoreToPattern_(false),
	  entryCnt_(0),
	  selectAllState_(-1),
	  viewedRowCnt_(1),
	  viewedRowsHeight_(0),
	  viewedRowOffset_(0),
	  viewedCenterY_(0),
	  viewedCenterBaseY_(0),
	  backChanged_(false),
	  textChanged_(false),
	  headerChanged_(false),
	  followModeChanged_(false),
	  hasFocussedBefore_(false),
	  orderDownCount_(0),
	  freezed_(false),
	  repaintable_(true),
	  repaintingCnt_(0),
	  playingRow_(-1)
{
	// Initialize font
	headerFont_ = QApplication::font();
	headerFont_.setPointSize(10);
	rowFont_ = QFont("Monospace", 10);
	rowFont_.setStyleHint(QFont::TypeWriter);
	rowFont_.setStyleStrategy(QFont::ForceIntegerMetrics);

	updateSizes();

	setAttribute(Qt::WA_Hover);
	setContextMenuPolicy(Qt::CustomContextMenu);
}

void OrderListPanel::setCore(std::shared_ptr<BambooTracker> core)
{
	bt_ = core;
}

void OrderListPanel::setCommandStack(std::weak_ptr<QUndoStack> stack)
{
	comStack_ = stack;
}

void OrderListPanel::setConfiguration(std::shared_ptr<Configuration> config)
{
	config_ = config;
}

void OrderListPanel::setColorPallete(std::shared_ptr<ColorPalette> palette)
{
	palette_ = palette;
}

void OrderListPanel::resetEntryCount()
{
	entryCnt_ = 0;
}

void OrderListPanel::freeze()
{
	freezed_ = true;
	while (true) {
		if (repaintingCnt_.load())
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		else {
			curPos_.row = 0;	// Init
			return;
		}
	}
}

void OrderListPanel::unfreeze()
{
	freezed_ = false;
}

QString OrderListPanel::getHeaderFont() const
{
	return QFontInfo(headerFont_).family();
}

int OrderListPanel::getHeaderFontSize() const
{
	return QFontInfo(headerFont_).pointSize();
}

QString OrderListPanel::getRowsFont() const
{
	return QFontInfo(rowFont_).family();
}
int OrderListPanel::getRowsFontSize() const
{
	return QFontInfo(rowFont_).pointSize();
}

void OrderListPanel::setFonts(QString headerFont, int headerSize, QString rowsFont, int rowsSize)
{
	headerFont_ = QFont(headerFont, headerSize);
	rowFont_ = QFont(rowsFont, rowsSize);

	updateSizes();
	updateTracksWidthFromLeftToEnd();
	setMaximumWidth(columnsWidthFromLeftToEnd_);

	redrawAll();
}

void OrderListPanel::updateSizes()
{
	QFontMetrics metrics(rowFont_);
#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)
	rowFontWidth_ = metrics.horizontalAdvance('0');
#else
	rowFontWidth_ = metrics.width('0');
#endif
#if QT_VERSION >= QT_VERSION_CHECK(5, 8, 0)
	rowFontAscent_ = metrics.capHeight();
#else
	rowFontAscent_ = metrics.boundingRect('X').height();
#endif
	rowFontLeading_ = metrics.ascent() - rowFontAscent_ + metrics.descent() / 2;
	rowFontHeight_ = rowFontAscent_ + rowFontLeading_;

	hdFontMets_ = std::make_unique<QFontMetrics>(headerFont_);
	headerHeight_ = hdFontMets_->height() + 4;
	headerFontAscent_ = hdFontMets_->ascent() + 2;

	/* Width & height */
	widthSpace_ = rowFontWidth_ / 4;
	trackWidth_ = rowFontWidth_ * 3 + widthSpace_ * 2;
	if (config_->getShowRowNumberInHex()) {
		rowNumWidthCnt_ = 2;
		rowNumBase_ = 16;
	}
	else {
		rowNumWidthCnt_ = 3;
		rowNumBase_ = 10;
	}
	rowNumWidth_ = rowFontWidth_ * rowNumWidthCnt_ + widthSpace_;

	initDisplay();
}

void OrderListPanel::initDisplay()
{
	completePixmap_ = std::make_unique<QPixmap>(geometry().size());

	int width = geometry().width();

	// Recalculate pixmap sizes
	viewedRegionHeight_ = std::max((geometry().height() - headerHeight_), rowFontHeight_);
	int cnt = viewedRegionHeight_ / rowFontHeight_;
	viewedRowCnt_ = (cnt % 2) ? (cnt + 2) : (cnt + 1);
	viewedRowsHeight_ = viewedRowCnt_ * rowFontHeight_;

	viewedRowOffset_ = (viewedRowsHeight_ - viewedRegionHeight_) >> 1;
	viewedCenterY_ = (viewedRowsHeight_ - rowFontHeight_) >> 1;
	viewedCenterBaseY_ = viewedCenterY_ + rowFontAscent_ + (rowFontLeading_ >> 1);

	backPixmap_ = std::make_unique<QPixmap>(width, viewedRowsHeight_);
	textPixmap_ = std::make_unique<QPixmap>(width, viewedRowsHeight_);
	headerPixmap_ = std::make_unique<QPixmap>(width, headerHeight_);
}

void OrderListPanel::drawList(const QRect &rect)
{
	if (!freezed_ && repaintable_.load()) {
		repaintable_.store(false);
		++repaintingCnt_;	// Use module data after this line

		if (backChanged_ || textChanged_ || headerChanged_ || orderDownCount_ || followModeChanged_) {

			int maxWidth = std::min(geometry().width(), columnsWidthFromLeftToEnd_);
			int trackSize = static_cast<int>(songStyle_.trackAttribs.size());

			completePixmap_->fill(palette_->odrBackColor);

			if (orderDownCount_ && !followModeChanged_) {
				quickDrawRows(maxWidth, trackSize);
			}
			else {
				backPixmap_->fill(Qt::transparent);
				if (textChanged_) textPixmap_->fill(Qt::transparent);
				drawRows(maxWidth, trackSize);
			}

			if (headerChanged_) {
				// headerPixmap_->fill(Qt::transparent);
				drawHeaders(maxWidth, trackSize);
			}

			{
				QPainter mergePainter(completePixmap_.get());
				QRect rowsRect(0, viewedRowOffset_, maxWidth, viewedRegionHeight_);
				QRect inViewRect(0, headerHeight_, maxWidth, viewedRegionHeight_);
				mergePainter.drawPixmap(inViewRect, *backPixmap_.get(), rowsRect);
				mergePainter.drawPixmap(inViewRect, *textPixmap_.get(), rowsRect);
				mergePainter.drawPixmap(headerPixmap_->rect(), *headerPixmap_.get());
			}

			drawBorders(maxWidth, trackSize);
			if (!hasFocus()) drawShadow();

			backChanged_ = false;
			textChanged_ = false;
			headerChanged_ = false;
			followModeChanged_ = false;
			orderDownCount_ = 0;

		}

		--repaintingCnt_;	// Used module data until this line
		repaintable_.store(true);
	}

	QPainter completePainter(this);
	completePainter.drawPixmap(rect, *completePixmap_.get());
}

void OrderListPanel::drawRows(int maxWidth, int trackSize)
{
	QPainter textPainter(textPixmap_.get());
	QPainter backPainter(backPixmap_.get());
	textPainter.setFont(rowFont_);

	std::vector<OrderData> orderRowData_;
	int x, trackNum;
	int textOffset = trackWidth_ / 2 - rowFontWidth_;

	/* Current row */
	// Fill row
	backPainter.fillRect(0, viewedCenterY_, maxWidth, rowFontHeight_,
						 hasFocus() ? palette_->odrCurEditRowColor : palette_->odrCurRowColor);
	if (textChanged_) {
		// Row number
		textPainter.setPen(palette_->odrRowNumColor);
		textPainter.drawText(1, viewedCenterBaseY_, QString("%1").arg(
								 curPos_.row, rowNumWidthCnt_, rowNumBase_, QChar('0')
								 ).toUpper());
	}
	// Order data
	orderRowData_ = bt_->getOrderData(curSongNum_, curPos_.row);
	textPainter.setPen(palette_->odrCurTextColor);
	for (x = rowNumWidth_, trackNum = leftTrackNum_; x < maxWidth && trackNum < trackSize; ) {
		if (trackNum == curPos_.track)	// Paint current cell
			backPainter.fillRect(x, viewedCenterY_, trackWidth_, rowFontHeight_, palette_->odrCurCellColor);
		if (((hovPos_.row == curPos_.row || hovPos_.row == -2) && hovPos_.track == trackNum)
				|| (hovPos_.track == -2 && hovPos_.row == curPos_.row))	// Paint hover
			backPainter.fillRect(x, viewedCenterY_, trackWidth_, rowFontHeight_, palette_->odrHovCellColor);
		if ((selLeftAbovePos_.track >= 0 && selLeftAbovePos_.row >= 0)
				&& isSelectedCell(trackNum, curPos_.row))	// Paint selected
			backPainter.fillRect(x, viewedCenterY_, trackWidth_, rowFontHeight_, palette_->odrSelCellColor);
		if (textChanged_) {
			textPainter.drawText(
						x + textOffset,
						viewedCenterBaseY_,
						QString("%1")
						.arg(orderRowData_.at(static_cast<size_t>(trackNum)).patten, 2, 16, QChar('0')).toUpper()
						);
		}

		x += trackWidth_;
		++trackNum;
	}
	viewedCenterPos_.row = curPos_.row;

	int rowNum;
	int rowY, baseY, endY;
	int playOdrNum = bt_->getPlayingOrderNumber();

	/* Previous rows */
	viewedFirstPos_.row = curPos_.row;
	endY = std::max(0, viewedCenterY_ - rowFontHeight_ * curPos_.row);
	for (rowY = viewedCenterY_ - rowFontHeight_, baseY = viewedCenterBaseY_ - rowFontHeight_, rowNum = curPos_.row - 1;
		 rowY >= endY;
		 rowY -= rowFontHeight_, baseY -= rowFontHeight_, --rowNum) {
		QColor rowColor;
		if (!config_->getFollowMode() && rowNum == playOdrNum) {
			rowColor = palette_->odrPlayRowColor;
		}
		else {
			rowColor = palette_->odrDefRowColor;
		}

		// Fill row
		backPainter.fillRect(0, rowY, maxWidth, rowFontHeight_, rowColor);
		if (textChanged_) {
			// Row number
			textPainter.setPen(palette_->odrRowNumColor);
			textPainter.drawText(1, baseY, QString("%1").arg(
									 rowNum, rowNumWidthCnt_, rowNumBase_, QChar('0')
									 ).toUpper());
		}
		// Order data
		orderRowData_ = bt_->getOrderData(curSongNum_, rowNum);
		textPainter.setPen(palette_->odrDefTextColor);
		for (x = rowNumWidth_, trackNum = leftTrackNum_; x < maxWidth && trackNum < trackSize; ) {
			if (((hovPos_.row == rowNum || hovPos_.row == -2) && hovPos_.track == trackNum)
					|| (hovPos_.track == -2 && hovPos_.row == rowNum))	// Paint hover
				backPainter.fillRect(x, rowY, trackWidth_, rowFontHeight_, palette_->odrHovCellColor);
			if ((selLeftAbovePos_.track >= 0 && selLeftAbovePos_.row >= 0)
					&& isSelectedCell(trackNum, rowNum))	// Paint selected
				backPainter.fillRect(x, rowY, trackWidth_, rowFontHeight_, palette_->odrSelCellColor);
			if (textChanged_) {
				textPainter.drawText(
							x + textOffset,
							baseY,
							QString("%1")
							.arg(orderRowData_.at(static_cast<size_t>(trackNum)).patten, 2, 16, QChar('0')).toUpper()
							);
			}

			x += trackWidth_;
			++trackNum;
		}
		viewedFirstPos_.row = rowNum;
	}

	/* Next rows */
	viewedLastPos_.row = curPos_.row;
	endY = std::min(viewedRowsHeight_ - viewedRowOffset_,
					viewedCenterY_ + rowFontHeight_ * (static_cast<int>(bt_->getOrderSize(curSongNum_)) - curPos_.row - 1));
	for (rowY = viewedCenterY_ + rowFontHeight_, baseY = viewedCenterBaseY_ + rowFontHeight_, rowNum = curPos_.row + 1;
		 rowY <= endY;
		 rowY += rowFontHeight_, baseY += rowFontHeight_, ++rowNum) {
		QColor rowColor;
		if (!config_->getFollowMode() && rowNum == playOdrNum)
			rowColor = palette_->odrPlayRowColor;
		else
			rowColor = palette_->odrDefRowColor;

		// Fill row
		backPainter.fillRect(0, rowY, maxWidth, rowFontHeight_, rowColor);
		if (textChanged_) {
			// Row number
			textPainter.setPen(palette_->odrRowNumColor);
			textPainter.drawText(1, baseY, QString("%1").arg(
									 rowNum, rowNumWidthCnt_, rowNumBase_, QChar('0')
									 ).toUpper());
		}
		// Order data
		orderRowData_ = bt_->getOrderData(curSongNum_, rowNum);
		textPainter.setPen(palette_->odrDefTextColor);
		for (x = rowNumWidth_, trackNum = leftTrackNum_; x < maxWidth && trackNum < trackSize; ) {
			if (((hovPos_.row == rowNum || hovPos_.row == -2) && hovPos_.track == trackNum)
					|| (hovPos_.track == -2 && hovPos_.row == rowNum))	// Paint hover
				backPainter.fillRect(x, rowY, trackWidth_, rowFontHeight_, palette_->odrHovCellColor);
			if ((selLeftAbovePos_.track >= 0 && selLeftAbovePos_.row >= 0)
					&& isSelectedCell(trackNum, rowNum))	// Paint selected
				backPainter.fillRect(x, rowY, trackWidth_, rowFontHeight_, palette_->odrSelCellColor);
			if (textChanged_) {
				textPainter.drawText(
							x + textOffset,
							baseY,
							QString("%1")
							.arg(orderRowData_.at(static_cast<size_t>(trackNum)).patten, 2, 16, QChar('0')).toUpper()
							);
			}

			x += trackWidth_;
			++trackNum;
		}
		viewedLastPos_.row = rowNum;
	}
}

void OrderListPanel::quickDrawRows(int maxWidth, int trackSize)
{
	int halfRowsCnt = viewedRowCnt_ >> 1;
	int shift = rowFontHeight_ * orderDownCount_;

	/* Move up by */
	QRect srcRect(0, 0, maxWidth, viewedRowsHeight_);
	textPixmap_->scroll(0, -shift, srcRect);
	backPixmap_->scroll(0, -shift, srcRect);
	{
		int fpos = viewedCenterPos_.row + orderDownCount_ - halfRowsCnt;
		if (fpos >= 0) viewedFirstPos_.row = fpos;
	}

	QPainter textPainter(textPixmap_.get());
	QPainter backPainter(backPixmap_.get());
	textPainter.setFont(rowFont_);

	std::vector<OrderData> orderRowData_;
	int x, trackNum;
	int textOffset = trackWidth_ / 2 - rowFontWidth_;

	/* Clear previous cursor row, current cursor row and last rows text */
	int prevY = viewedCenterY_ - shift;
	int lastY = viewedRowsHeight_ - shift;
	textPainter.setCompositionMode(QPainter::CompositionMode_Source);
	textPainter.fillRect(0, prevY, maxWidth, rowFontHeight_, Qt::transparent);
	textPainter.fillRect(0, viewedCenterY_, maxWidth, rowFontHeight_, Qt::transparent);
	textPainter.fillRect(0, lastY, maxWidth, shift, Qt::transparent);
	textPainter.setCompositionMode(QPainter::CompositionMode_SourceOver);

	/* Redraw previous cursor row */
	{
		int baseY = viewedCenterBaseY_ - shift;

		// Fill row
		backPainter.fillRect(0, prevY, maxWidth, rowFontHeight_, palette_->odrDefRowColor);
		// Row number
		textPainter.setPen(palette_->odrRowNumColor);
		textPainter.drawText(1, baseY, QString("%1").arg(
								 viewedCenterPos_.row, rowNumWidthCnt_, rowNumBase_, QChar('0')
								 ).toUpper());
		// Order data
		orderRowData_ = bt_->getOrderData(curSongNum_, viewedCenterPos_.row);
		textPainter.setPen(palette_->odrDefTextColor);
		for (x = rowNumWidth_, trackNum = leftTrackNum_; x < maxWidth && trackNum < trackSize; ) {
			if (((hovPos_.row == viewedCenterPos_.row || hovPos_.row == -2) && hovPos_.track == trackNum)
					|| (hovPos_.track == -2 && hovPos_.row == viewedCenterPos_.row))	// Paint hover
				backPainter.fillRect(x, prevY, trackWidth_, rowFontHeight_, palette_->odrHovCellColor);
			if ((selLeftAbovePos_.track >= 0 && selLeftAbovePos_.row >= 0)
					&& isSelectedCell(trackNum, viewedCenterPos_.row))	// Paint selected
				backPainter.fillRect(x, prevY, trackWidth_, rowFontHeight_, palette_->odrSelCellColor);
			textPainter.drawText(
						x + textOffset,
						baseY,
						QString("%1")
						.arg(orderRowData_.at(static_cast<size_t>(trackNum)).patten, 2, 16, QChar('0')).toUpper()
						);

			x += trackWidth_;
			++trackNum;
		}
	}

	/* Redraw current cursor row */
	// Fill row
	backPainter.fillRect(0, viewedCenterY_, maxWidth, rowFontHeight_,
						 hasFocus() ? palette_->odrCurEditRowColor : palette_->odrCurRowColor);
	// Row number
	textPainter.setPen(palette_->odrRowNumColor);
	textPainter.drawText(1, viewedCenterBaseY_, QString("%1").arg(
							 curPos_.row, rowNumWidthCnt_, rowNumBase_, QChar('0')
							 ).toUpper());
	// Order data
	orderRowData_ = bt_->getOrderData(curSongNum_, curPos_.row);
	textPainter.setPen(palette_->odrCurTextColor);
	for (x = rowNumWidth_, trackNum = leftTrackNum_; x < maxWidth && trackNum < trackSize; ) {
		if (trackNum == curPos_.track)	// Paint current cell
			backPainter.fillRect(x, viewedCenterY_, trackWidth_, rowFontHeight_, palette_->odrCurCellColor);
		if (((hovPos_.row == curPos_.row || hovPos_.row == -2) && hovPos_.track == trackNum)
				|| (hovPos_.track == -2 && hovPos_.row == curPos_.row))	// Paint hover
			backPainter.fillRect(x, viewedCenterY_, trackWidth_, rowFontHeight_, palette_->odrHovCellColor);
		if ((selLeftAbovePos_.track >= 0 && selLeftAbovePos_.row >= 0)
				&& isSelectedCell(trackNum, curPos_.row))	// Paint selected
			backPainter.fillRect(x, viewedCenterY_, trackWidth_, rowFontHeight_, palette_->odrSelCellColor);
		textPainter.drawText(
					x + textOffset,
					viewedCenterBaseY_,
					QString("%1")
					.arg(orderRowData_.at(static_cast<size_t>(trackNum)).patten, 2, 16, QChar('0')).toUpper()
					);

		x += trackWidth_;
		++trackNum;
	}
	viewedCenterPos_ = curPos_;

	/* Draw new rows at last if necessary */
	{
		int bpos = viewedCenterPos_.row + halfRowsCnt;
		int last = static_cast<int>(bt_->getOrderSize(curSongNum_)) - 1;
		bool needClear;
		if (bpos < last) {
			needClear = false;
			bpos = std::exchange(viewedLastPos_.row, bpos);
		}
		else {
			needClear = true;
			bpos = std::exchange(viewedLastPos_.row, last);
		}
		int baseY = lastY + (viewedCenterBaseY_ - viewedCenterY_);
		while (true) {
			if (bpos == viewedLastPos_.row) {
				if (needClear) {	// Clear row
					backPainter.setCompositionMode(QPainter::CompositionMode_Source);
					backPainter.fillRect(0, lastY, maxWidth, shift, Qt::transparent);
				}
				break;
			}
			++bpos;

			// Fill row
			backPainter.fillRect(0, lastY, maxWidth, rowFontHeight_, palette_->odrDefRowColor);
			// Row number
			textPainter.setPen(palette_->odrRowNumColor);
			textPainter.drawText(1, baseY, QString("%1").arg(
									 viewedLastPos_.row, rowNumWidthCnt_, rowNumBase_, QChar('0')
									 ).toUpper());
			// Order data
			orderRowData_ = bt_->getOrderData(curSongNum_, viewedLastPos_.row);
			textPainter.setPen(palette_->odrDefTextColor);
			for (x = rowNumWidth_, trackNum = leftTrackNum_; x < maxWidth && trackNum < trackSize; ) {
				if (((hovPos_.row == viewedLastPos_.row || hovPos_.row == -2) && hovPos_.track == trackNum)
						|| (hovPos_.track == -2 && hovPos_.row == viewedLastPos_.row))	// Paint hover
					backPainter.fillRect(x, lastY, trackWidth_, rowFontHeight_, palette_->odrHovCellColor);
				if ((selLeftAbovePos_.track >= 0 && selLeftAbovePos_.row >= 0)
						&& isSelectedCell(trackNum, viewedLastPos_.row))	// Paint selected
					backPainter.fillRect(x, lastY, trackWidth_, rowFontHeight_, palette_->odrSelCellColor);
				textPainter.drawText(
							x + textOffset,
							baseY,
							QString("%1")
							.arg(orderRowData_.at(static_cast<size_t>(trackNum)).patten, 2, 16, QChar('0')).toUpper()
							);

				x += trackWidth_;
				++trackNum;
			}

			baseY += rowFontHeight_;
			lastY += rowFontHeight_;
		}
	}
}

void OrderListPanel::drawHeaders(int maxWidth, int trackSize)
{
	QPainter painter(headerPixmap_.get());
	painter.setFont(headerFont_);

	painter.fillRect(0, 0, geometry().width(), headerHeight_, palette_->odrHeaderRowColor);
	painter.setPen(palette_->odrHeaderTextColor);
	int x, trackNum;
	for (x = rowNumWidth_, trackNum = leftTrackNum_; x < maxWidth && trackNum < trackSize; ) {
		QString str;
		auto& attrib = songStyle_.trackAttribs[static_cast<size_t>(trackNum)];
		switch (attrib.source) {
		case SoundSource::FM:
			switch (songStyle_.type) {
			case SongType::Standard:
				str = "FM" + QString::number(attrib.channelInSource + 1);
				break;
			case SongType::FM3chExpanded:
				switch (attrib.channelInSource) {
				case 2:
					str = "OP1";
					break;
				case 6:
					str = "OP2";
					break;
				case 7:
					str = "OP3";
					break;
				case 8:
					str = "OP4";
					break;
				default:
					str = "FM" + QString::number(attrib.channelInSource + 1);
					break;
				}
				break;
			}
			break;
		case SoundSource::SSG:
			str = "SG" + QString::number(attrib.channelInSource + 1);
			break;
		case SoundSource::DRUM:
			switch (attrib.channelInSource) {
			case 0:	str = "BD";		break;
			case 1:	str = "SD";		break;
			case 2:	str = "TOP";	break;
			case 3:	str = "HH";		break;
			case 4:	str = "TOM";	break;
			case 5:	str = "RIM";	break;
			}
			break;
		case SoundSource::ADPCM:
			str = "AP";
			break;
		}

#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)
		int rw = trackWidth_ - hdFontMets_->horizontalAdvance(str);
#else
		int rw = trackWidth_ - hdFontMets_->width(str);
#endif
		rw = (rw < 0) ? 0 : (rw / 2);
		painter.drawText(x + rw, headerFontAscent_, str);

		x += trackWidth_;
		++trackNum;
	}
}

void OrderListPanel::drawBorders(int maxWidth, int trackSize)
{
	QPainter painter(completePixmap_.get());

	painter.drawLine(0, headerHeight_, geometry().width(), headerHeight_);
	painter.drawLine(rowNumWidth_, 0, rowNumWidth_, geometry().height());
	int x, trackNum;
	for (x = rowNumWidth_ + trackWidth_, trackNum = leftTrackNum_; x <= maxWidth && trackNum < trackSize; ) {
		painter.drawLine(x, 0, x, geometry().height());
		x += trackWidth_;
		++trackNum;
	}
}

void OrderListPanel::drawShadow()
{
	QPainter painter(completePixmap_.get());
	painter.fillRect(0, 0, geometry().width(), geometry().height(), QColor::fromRgb(0, 0, 0, 47));
}

void OrderListPanel::moveCursorToRight(int n)
{
	int oldLeftTrack = leftTrackNum_;
	int prevTrack = curPos_.track;
	int tmp = curPos_.track + n;
	if (n > 0) {
		while (true) {
			int sub = tmp - static_cast<int>(songStyle_.trackAttribs.size());
			if (sub < 0) {
				curPos_.track = tmp;
				break;
			}
			else {
				if (config_->getWarpCursor()) {
					tmp = sub;
				}
				else {
					curPos_.track = static_cast<int>(songStyle_.trackAttribs.size()) - 1;
					break;
				}
			}
		}
	}
	else {
		while (true) {
			int add = tmp + static_cast<int>(songStyle_.trackAttribs.size());
			if (tmp < 0) {
				if (config_->getWarpCursor()) {
					tmp = add;
				}
				else {
					curPos_.track = 0;
					break;
				}
			}
			else {
				curPos_.track = tmp;
				break;
			}
		}
	}
	if (prevTrack < curPos_.track) {
		while (calculateColumnsWidthWithRowNum(leftTrackNum_, curPos_.track) > geometry().width())
			++leftTrackNum_;
	}
	else {
		if (curPos_.track < leftTrackNum_) leftTrackNum_ = curPos_.track;
	}

	updateTracksWidthFromLeftToEnd();
	entryCnt_ = 0;

	if (!isIgnoreToSlider_) {	// Send to slider
		if (config_->getMoveCursorByHorizontalScroll()) {
			emit hScrollBarChangeRequested(curPos_.track);
		}
		else {
			emit hScrollBarChangeRequested(leftTrackNum_);
		}
	}

	if (!isIgnoreToPattern_) emit currentTrackChanged(curPos_.track);	// Send to pattern editor

	// Request fore-background repaint if leftmost track is changed else request only background repaint
	if (leftTrackNum_ != oldLeftTrack) {
		headerChanged_ = true;
		textChanged_ = true;
	}
	backChanged_ = true;
	repaint();
}

void OrderListPanel::moveViewToRight(int n)
{
	leftTrackNum_ += n;
	updateTracksWidthFromLeftToEnd();

	// Move cursor and repaint all
	headerChanged_ = true;
	textChanged_ = true;
	moveCursorToRight(n);
}

void OrderListPanel::moveCursorToDown(int n)
{
	int tmp = curPos_.row + n;
	int endRow = static_cast<int>(bt_->getOrderSize(curSongNum_));
	if (n > 0) {
		while (true) {
			int sub = tmp - endRow;
			if (sub < 0) {
				curPos_.row = tmp;
				break;
			}
			else {
				tmp = sub;
			}
		}
	}
	else {
		while (true) {
			int add = tmp + endRow;
			if (tmp < 0) {
				tmp = add;
			}
			else {
				curPos_.row = tmp;
				break;
			}
		}
	}

	entryCnt_ = 0;

	if (!isIgnoreToSlider_)		// Send to slider
		emit vScrollBarChangeRequested(curPos_.row, static_cast<int>(bt_->getOrderSize(curSongNum_)) - 1);

	if (!isIgnoreToPattern_)	// Send to pattern editor
		emit currentOrderChanged(curPos_.row);

	backChanged_ = true;
	textChanged_ = true;
	repaint();
}

void OrderListPanel::changeEditable()
{
	backChanged_ = true;
	repaint();
}

void OrderListPanel::updatePositionByOrderUpdate(bool isFirstUpdate, bool forceJump)
{	
	int prev = std::exchange(playingRow_, bt_->getPlayingOrderNumber());
	if (!forceJump && !config_->getFollowMode() && prev != playingRow_) {	// Repaint only background
		backChanged_ = true;
		repaint();
		return;
	}

	int tmp = std::exchange(curPos_.row, bt_->getCurrentOrderNumber());
	int d = curPos_.row - tmp;
	if (!d) return;

	emit vScrollBarChangeRequested(curPos_.row, static_cast<int>(bt_->getOrderSize(curSongNum_)) - 1);

	// Redraw entire area in first update and jumping order
	orderDownCount_ = (isFirstUpdate || d < 0 || (viewedRowCnt_ >> 1) < d) ? 0 : d;

	textChanged_ = true;
	backChanged_ = true;
	repaint();
}

int OrderListPanel::getScrollableCountByTrack() const
{
	int width = rowNumWidth_;
	size_t i = songStyle_.trackAttribs.size();
	do {
		--i;
		width += trackWidth_;
		if (geometry().width() < width) {
			return static_cast<int>(i + 1);
		}
	} while (i);
	return 0;
}

void OrderListPanel::redrawByPatternChanged(bool ordersLengthChanged)
{
	textChanged_ = true;

	// When length of orders is changed, redraw all area
	if (ordersLengthChanged) backChanged_ = true;

	repaint();
}

void OrderListPanel::redrawByFocusChanged()
{
	if (hasFocussedBefore_) {
		backChanged_ = true;
		repaint();
	}
	else {
		redrawAll();
		hasFocussedBefore_ = true;
	}
}

void OrderListPanel::redrawByHoverChanged()
{
	headerChanged_ = true;
	backChanged_ = true;
	repaint();
}

void OrderListPanel::redrawAll()
{
	backChanged_ = true;
	textChanged_ = true;
	headerChanged_ = true;
	repaint();
}

bool OrderListPanel::enterOrder(int key)
{
	switch (key) {
	case Qt::Key_0:	setCellOrderNum(0x0);	return true;
	case Qt::Key_1:	setCellOrderNum(0x1);	return true;
	case Qt::Key_2:	setCellOrderNum(0x2);	return true;
	case Qt::Key_3:	setCellOrderNum(0x3);	return true;
	case Qt::Key_4:	setCellOrderNum(0x4);	return true;
	case Qt::Key_5:	setCellOrderNum(0x5);	return true;
	case Qt::Key_6:	setCellOrderNum(0x6);	return true;
	case Qt::Key_7:	setCellOrderNum(0x7);	return true;
	case Qt::Key_8:	setCellOrderNum(0x8);	return true;
	case Qt::Key_9:	setCellOrderNum(0x9);	return true;
	case Qt::Key_A:	setCellOrderNum(0xa);	return true;
	case Qt::Key_B:	setCellOrderNum(0xb);	return true;
	case Qt::Key_C:	setCellOrderNum(0xc);	return true;
	case Qt::Key_D:	setCellOrderNum(0xd);	return true;
	case Qt::Key_E:	setCellOrderNum(0xe);	return true;
	case Qt::Key_F:	setCellOrderNum(0xf);	return true;
	default:	return false;
	}
}

void OrderListPanel::setCellOrderNum(int n)
{
	bt_->setOrderPatternDigit(curSongNum_, curPos_.track, curPos_.row, n, (entryCnt_ == 1));
	comStack_.lock()->push(new SetPatternToOrderQtCommand(this, curPos_, (entryCnt_ == 1)));

	entryCnt_ = (entryCnt_ + 1) % 2;
	if ((!bt_->isPlaySong() || !bt_->isFollowPlay()) && !entryCnt_) moveCursorToDown(1);
}

void OrderListPanel::insertOrderBelow()
{
	if (!bt_->canAddNewOrder(curSongNum_)) return;

	bt_->insertOrderBelow(curSongNum_, curPos_.row);
	comStack_.lock()->push(new InsertOrderBelowQtCommand(this));
}

void OrderListPanel::deleteOrder()
{
	if (bt_->getOrderSize(curSongNum_) > 1) {
		bt_->deleteOrder(curSongNum_, curPos_.row);
		comStack_.lock()->push(new DeleteOrderQtCommand(this));
	}
}

void OrderListPanel::copySelectedCells()
{
	if (selLeftAbovePos_.row == -1) return;

	int w = selRightBelowPos_.track - selLeftAbovePos_.track + 1;
	int h = selRightBelowPos_.row - selLeftAbovePos_.row + 1;

	QString str = QString("ORDER_COPY:%1,%2,")
				  .arg(QString::number(w), QString::number(h));
	for (int i = 0; i < h; ++i) {
		std::vector<OrderData> odrs = bt_->getOrderData(curSongNum_, selLeftAbovePos_.row + i);
		for (int j = 0; j < w; ++j) {
			str += QString::number(odrs.at(static_cast<size_t>(selLeftAbovePos_.track + j)).patten);
			if (i < h - 1 || j < w - 1) str += ",";
		}
	}

	QApplication::clipboard()->setText(str);
}

void OrderListPanel::pasteCopiedCells(const OrderPosition& startPos)
{
	// Analyze text
	QString str = QApplication::clipboard()->text().remove(QRegularExpression("ORDER_COPY:"));
	QString hdRe = "^([0-9]+),([0-9]+),";
	QRegularExpression re(hdRe);
	QRegularExpressionMatch match = re.match(str);
	int w = match.captured(1).toInt();
	size_t h = match.captured(2).toUInt();
	str.remove(re);

	std::vector<std::vector<std::string>> cells;
	re = QRegularExpression("^([^,]+),");
	for (size_t i = 0; i < h; ++i) {
		cells.emplace_back();
		for (int j = 0; j < w; ++j) {
			match = re.match(str);
			if (match.hasMatch()) {
				cells.at(i).push_back(match.captured(1).toStdString());
				str.remove(re);
			}
			else {
				cells.at(i).push_back(str.toStdString());
				break;
			}
		}
	}

	// Send cells data
	bt_->pasteOrderCells(curSongNum_, startPos.track, startPos.row, std::move(cells));
	comStack_.lock()->push(new PasteCopiedDataToOrderQtCommand(this));
}

void OrderListPanel::setSelectedRectangle(const OrderPosition& start, const OrderPosition& end)
{
	if (start.track > end.track) {
		if (start.row > end.row) {
			selLeftAbovePos_ = end;
			selRightBelowPos_ = start;
		}
		else {
			selLeftAbovePos_ = { end.track, start.row };
			selRightBelowPos_ = { start.track, end.row };
		}
	}
	else {
		if (start.row > end.row) {
			selLeftAbovePos_ = { start.track, end.row };
			selRightBelowPos_ = { end.track, start.row };
		}
		else {
			selLeftAbovePos_ = start;
			selRightBelowPos_ = end;
		}
	}

	emit selected(true);

	backChanged_ = true;
	repaint();
}

bool OrderListPanel::isSelectedCell(int track, int row)
{
	OrderPosition pos{ track, row };
	return (selLeftAbovePos_.track <= pos.track && selRightBelowPos_.track >= pos.track
			&& selLeftAbovePos_.row <= pos.row && selRightBelowPos_.row >= pos.row);
}

void OrderListPanel::showContextMenu(const OrderPosition& pos, const QPoint& point)
{
	QMenu menu;
	// Leave Before Qt5.7.0 style due to windows xp
	QAction* insert = menu.addAction(tr("&Insert Order"));
	insert->setIcon(QIcon(":/icon/insert_order"));
	QObject::connect(insert, &QAction::triggered, this, [&]() { insertOrderBelow(); });
	QAction* remove = menu.addAction(tr("&Remove Order"));
	remove->setIcon(QIcon(":/icon/remove_order"));
	QObject::connect(remove, &QAction::triggered, this, [&]() { deleteOrder(); });
	QAction* duplicate = menu.addAction(tr("&Duplicate Order"));
	duplicate->setIcon(QIcon(":/icon/duplicate_order"));
	QObject::connect(duplicate, &QAction::triggered, this, &OrderListPanel::onDuplicatePressed);
	QAction* clonep = menu.addAction(tr("&Clone Patterns"));
	QAction::connect(clonep, &QAction::triggered, this, &OrderListPanel::onClonePatternsPressed);
	QAction* cloneo = menu.addAction(tr("Clone &Order"));
	QObject::connect(cloneo, &QAction::triggered, this, &OrderListPanel::onCloneOrderPressed);
	menu.addSeparator();
	QAction* moveUp = menu.addAction(tr("Move Order &Up"));
	moveUp->setIcon(QIcon(":/icon/order_up"));
	QObject::connect(moveUp, &QAction::triggered, this, [&]() { onMoveOrderPressed(true); });
	QAction* moveDown = menu.addAction(tr("Move Order Do&wn"));
	moveDown->setIcon(QIcon(":/icon/order_down"));
	QObject::connect(moveDown, &QAction::triggered, this, [&]() { onMoveOrderPressed(false); });
	menu.addSeparator();
	QAction* copy = menu.addAction(tr("Cop&y"));
	copy->setIcon(QIcon(":/icon/copy"));
	QObject::connect(copy, &QAction::triggered, this, &OrderListPanel::copySelectedCells);
	QAction* paste = menu.addAction(tr("&Paste"));
	paste->setIcon(QIcon(":/icon/paste"));
	QObject::connect(paste, &QAction::triggered, this, [&]() { pasteCopiedCells(pos); });
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
	duplicate->setShortcutVisibleInContextMenu(true);
	clonep->setShortcutVisibleInContextMenu(true);
	copy->setShortcutVisibleInContextMenu(true);
	paste->setShortcutVisibleInContextMenu(true);
#endif
	duplicate->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_D));
	clonep->setShortcut(QKeySequence(Qt::ALT + Qt::Key_D));
	copy->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_C));
	paste->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_V));

	if (pos.row < 0 || pos.track < 0) {
		remove->setEnabled(false);
		moveUp->setEnabled(false);
		moveDown->setEnabled(false);
		copy->setEnabled(false);
		paste->setEnabled(false);
	}
	if (!bt_->canAddNewOrder(curSongNum_)) {
		insert->setEnabled(false);
		duplicate->setEnabled(false);
		moveUp->setEnabled(false);
		moveDown->setEnabled(false);
		copy->setEnabled(false);
		paste->setEnabled(false);
	}
	QString clipText = QApplication::clipboard()->text();
	if (!clipText.startsWith("ORDER_COPY")) {
		paste->setEnabled(false);
	}
	if (bt_->getOrderSize(curSongNum_) == 1) {
		remove->setEnabled(false);
	}
	if (selRightBelowPos_.row < 0
			|| !isSelectedCell(pos.track, pos.row)) {
		clonep->setEnabled(false);
		copy->setEnabled(false);
	}
	if (pos.row == 0) {
		moveUp->setEnabled(false);
	}
	if (pos.row == static_cast<int>(bt_->getOrderSize(curSongNum_)) - 1) {
		moveDown->setEnabled(false);
	}

	menu.exec(mapToGlobal(point));
}

/********** Slots **********/
void OrderListPanel::onHScrollBarChanged(int num)
{
	Ui::EventGuard eg(isIgnoreToSlider_);

	// Skip if position has already changed in panel
	if (config_->getMoveCursorByHorizontalScroll()) {
		if (int dif = num - curPos_.track) moveCursorToRight(dif);
	}
	else {
		if (int dif = num - leftTrackNum_) moveViewToRight(dif);
	}
}

void OrderListPanel::onVScrollBarChanged(int num) {
	Ui::EventGuard eg(isIgnoreToSlider_);

	// Skip if position has already changed in panel
	if (int dif = num - curPos_.row) moveCursorToDown(dif);
}

void OrderListPanel::setCurrentTrack(int num)
{
	Ui::EventGuard eg(isIgnoreToPattern_);

	// Skip if position has already changed in panel
	if (int dif = num - curPos_.track) moveCursorToRight(dif);
}

void OrderListPanel::setCurrentOrder(int num) {
	Ui::EventGuard eg(isIgnoreToPattern_);

	// Skip if position has already changed in panel
	if (int dif = num - curPos_.row) moveCursorToDown(dif);
}

void OrderListPanel::onOrderEdited()
{
	// Move cursor
	int s = static_cast<int>(bt_->getOrderSize(curSongNum_));
	if (s <= curPos_.row) {
		curPos_.row = s - 1;
		bt_->setCurrentOrderNumber(curPos_.row);
	}

	emit orderEdited();
}

void OrderListPanel::onSongLoaded()
{
	curSongNum_ = bt_->getCurrentSongNumber();
	curPos_ = { bt_->getCurrentTrackAttribute().number, bt_->getCurrentOrderNumber() };
	songStyle_ = bt_->getSongStyle(curSongNum_);
	leftTrackNum_ = 0;
	updateTracksWidthFromLeftToEnd();
	setMaximumWidth(columnsWidthFromLeftToEnd_);

	hovPos_ = { -1, -1 };
	mousePressPos_ = { -1, -1 };
	mouseReleasePos_ = { -1, -1 };
	selLeftAbovePos_ = { -1, -1 };
	selRightBelowPos_ = { -1, -1 };
	shiftPressedPos_ = { -1, -1 };
	entryCnt_ = 0;
	selectAllState_ = -1;
	emit selected(false);

	redrawAll();
}

void OrderListPanel::onPastePressed()
{
	pasteCopiedCells(curPos_);
}

void OrderListPanel::onSelectPressed(int type)
{
	switch (type) {
	case 0:	// None
	{
		selLeftAbovePos_ = { -1, -1 };
		selRightBelowPos_ = { -1, -1 };
		selectAllState_ = -1;
		emit selected(false);
		backChanged_ = true;
		repaint();
		break;
	}
	case 1:	// All
	{
		int max = static_cast<int>(bt_->getOrderSize(curSongNum_)) - 1;
		selectAllState_ = (selectAllState_ + 1) % 2;
		if (!selectAllState_) {
			OrderPosition start = { curPos_.track, 0 };
			OrderPosition end = { curPos_.track, max };
			setSelectedRectangle(start, end);
		}
		else {
			OrderPosition start = { 0, 0 };
			OrderPosition end = { static_cast<int>(songStyle_.trackAttribs.size() - 1), max };
			setSelectedRectangle(start, end);
		}
		break;
	}
	case 2:	// Row
	{
		selectAllState_ = -1;
		OrderPosition start = { 0, curPos_.row };
		OrderPosition end = { static_cast<int>(songStyle_.trackAttribs.size() - 1), curPos_.row };
		setSelectedRectangle(start, end);
		break;
	}
	case 3:	// Column
	{
		selectAllState_ = -1;
		OrderPosition start = { curPos_.track, 0 };
		OrderPosition end = { curPos_.track, static_cast<int>(bt_->getOrderSize(curSongNum_) - 1) };
		setSelectedRectangle(start, end);
		break;
	}
	case 4:	// Pattern
	{
		selectAllState_ = -1;
		setSelectedRectangle(curPos_, curPos_);
		break;
	}
	case 5:	// Order
	{
		onSelectPressed(2);
		break;
	}
	}
}

void OrderListPanel::onDuplicatePressed()
{
	bt_->duplicateOrder(curSongNum_, curPos_.row);
	comStack_.lock()->push(new DuplicateOrderQtCommand(this));
}

void OrderListPanel::onMoveOrderPressed(bool isUp)
{
	if ((isUp && curPos_.row == 0)
			|| (!isUp && curPos_.row == static_cast<int>(bt_->getOrderSize(curSongNum_)) - 1))
		return;

	bt_->MoveOrder(curSongNum_, curPos_.row, isUp);
	comStack_.lock()->push(new MoveOrderQtCommand(this));
}

void OrderListPanel::onClonePatternsPressed()
{
	if (selLeftAbovePos_.row == -1) return;

	bt_->clonePatterns(curSongNum_, selLeftAbovePos_.row, selLeftAbovePos_.track,
					   selRightBelowPos_.row, selRightBelowPos_.track);
	comStack_.lock()->push(new ClonePatternsQtCommand(this));
}

void OrderListPanel::onCloneOrderPressed()
{
	bt_->cloneOrder(curSongNum_, curPos_.row);
	comStack_.lock()->push(new CloneOrderQtCommand(this));
}

void OrderListPanel::onFollowModeChanged()
{
	curPos_.row = bt_->getCurrentOrderNumber();
	emit vScrollBarChangeRequested(curPos_.row, static_cast<int>(bt_->getOrderSize(curSongNum_)) - 1);

	// Force redraw all area
	followModeChanged_ = true;
	textChanged_ = true;
	backChanged_ = true;
	repaint();
}

void OrderListPanel::onStoppedPlaySong()
{
	followModeChanged_ = true;
	textChanged_ = true;
	backChanged_ = true;
	repaint();
}

/********** Events **********/
bool OrderListPanel::event(QEvent *event)
{
	switch (event->type()) {
	case QEvent::KeyPress:
		return keyPressed(dynamic_cast<QKeyEvent*>(event));
	case QEvent::KeyRelease:
		return keyReleased(dynamic_cast<QKeyEvent*>(event));
	case QEvent::HoverMove:
		return mouseHoverd(dynamic_cast<QHoverEvent*>(event));
	default:
		return QWidget::event(event);
	}
}

bool OrderListPanel::keyPressed(QKeyEvent *event)
{	
	/* General Keys */
	switch (event->key()) {
	case Qt::Key_Return:
		emit returnPressed();
		return true;
	case Qt::Key_Shift:
		shiftPressedPos_ = curPos_;
		return true;
	case Qt::Key_Left:
		moveCursorToRight(-1);
		if (event->modifiers().testFlag(Qt::ShiftModifier)) setSelectedRectangle(shiftPressedPos_, curPos_);
		else onSelectPressed(0);
		return true;
	case Qt::Key_Right:
		moveCursorToRight(1);
		if (event->modifiers().testFlag(Qt::ShiftModifier)) setSelectedRectangle(shiftPressedPos_, curPos_);
		else onSelectPressed(0);
		return true;
	case Qt::Key_Up:
		if (bt_->isPlaySong() && bt_->isFollowPlay()) {
			return false;
		}
		else {
			moveCursorToDown(-1);
			if (event->modifiers().testFlag(Qt::ShiftModifier)) setSelectedRectangle(shiftPressedPos_, curPos_);
			else onSelectPressed(0);
			return true;
		}
	case Qt::Key_Down:
		if (bt_->isPlaySong() && bt_->isFollowPlay()) {
			return false;
		}
		else {
			moveCursorToDown(1);
			if (event->modifiers().testFlag(Qt::ShiftModifier)) setSelectedRectangle(shiftPressedPos_, curPos_);
			else onSelectPressed(0);
			return true;
		}
	case Qt::Key_Home:
		if (bt_->isPlaySong() && bt_->isFollowPlay()) {
			return false;
		}
		else {
			moveCursorToDown(-curPos_.row);
			if (event->modifiers().testFlag(Qt::ShiftModifier)) setSelectedRectangle(shiftPressedPos_, curPos_);
			else onSelectPressed(0);
			return true;
		}
	case Qt::Key_End:
		if (bt_->isPlaySong() && bt_->isFollowPlay()) {
			return false;
		}
		else {
			moveCursorToDown(
						static_cast<int>(bt_->getOrderSize(curSongNum_)) - curPos_.row - 1);
			if (event->modifiers().testFlag(Qt::ShiftModifier)) setSelectedRectangle(shiftPressedPos_, curPos_);
			else onSelectPressed(0);
			return true;
		}
	case Qt::Key_PageUp:
		if (bt_->isPlaySong() && bt_->isFollowPlay()) {
			return false;
		}
		else {
			moveCursorToDown(-static_cast<int>(config_->getPageJumpLength()));
			if (event->modifiers().testFlag(Qt::ShiftModifier)) setSelectedRectangle(shiftPressedPos_, curPos_);
			else onSelectPressed(0);
			return true;
		}
	case Qt::Key_PageDown:
		if (bt_->isPlaySong() && bt_->isFollowPlay()) {
			return false;
		}
		else {
			moveCursorToDown(static_cast<int>(config_->getPageJumpLength()));
			if (event->modifiers().testFlag(Qt::ShiftModifier)) setSelectedRectangle(shiftPressedPos_, curPos_);
			else onSelectPressed(0);
			return true;
		}
	case Qt::Key_Insert:
		insertOrderBelow();
		return true;
	case Qt::Key_Menu:
		showContextMenu(
					curPos_,
					QPoint(calculateColumnsWidthWithRowNum(leftTrackNum_, curPos_.track), curRowY_ - 8));
		return true;
	default:
		if (event->modifiers().testFlag(Qt::AltModifier)) {
			if (event->key() == Qt::Key_B) {
				insertOrderBelow();
				return true;
			}
		}
		if (event->modifiers().testFlag(Qt::NoModifier)) {
			return enterOrder(event->key());
		}
		return false;
	}
}

bool OrderListPanel::keyReleased(QKeyEvent* event)
{
	switch (event->key()) {
	case Qt::Key_Shift:
		shiftPressedPos_ = { -1, -1 };
		return true;
	default:
		return false;
	}
}

void OrderListPanel::paintEvent(QPaintEvent *event)
{
	if (bt_) {
		const QRect& area = event->rect();
		if (area.x() == 0 && area.y() == 0) {
			drawList(area);
		}
		else {
			drawList(rect());
		}
	}
}

void OrderListPanel::resizeEvent(QResizeEvent *event)
{
	QWidget::resizeEvent(event);

	// Recalculate center row position
	curRowBaselineY_ = (geometry().height() + headerHeight_) / 2;
	curRowY_ = curRowBaselineY_ + rowFontLeading_ / 2 - rowFontAscent_;

	initDisplay();

	redrawAll();
}

void OrderListPanel::mousePressEvent(QMouseEvent *event)
{
	Q_UNUSED(event)

	setFocus();

	mousePressPos_ = hovPos_;
	mouseReleasePos_ = { -1, -1 };

	if (event->button() == Qt::LeftButton) {
		selLeftAbovePos_ = { -1, -1 };
		selRightBelowPos_ = { -1, -1 };
		selectAllState_ = -1;
		emit selected(false);
	}
}

void OrderListPanel::mouseMoveEvent(QMouseEvent* event)
{
	if (event->buttons() & Qt::LeftButton) {
		if (mousePressPos_.track < 0 || mousePressPos_.row < 0) return;	// Start point is out of range

		if (hovPos_.track >= 0) {
			setSelectedRectangle(mousePressPos_, hovPos_);
		}

		if (event->x() < rowNumWidth_ && leftTrackNum_ > 0) {
			if (config_->getMoveCursorByHorizontalScroll())
				moveCursorToRight(-1);
			else
				moveViewToRight(-1);
		}
		else if (event->x() > geometry().width() - rowNumWidth_ && hovPos_.track != -1) {
			if (config_->getMoveCursorByHorizontalScroll())
				moveCursorToRight(1);
			else
				moveViewToRight(1);
		}
		if (event->pos().y() < headerHeight_ + rowFontHeight_) {
			if (!bt_->isPlaySong() || !bt_->isFollowPlay()) moveCursorToDown(-1);
		}
		else if (event->pos().y() > geometry().height() - rowFontHeight_) {
			if (!bt_->isPlaySong() || !bt_->isFollowPlay()) moveCursorToDown(1);
		}
	}
}

void OrderListPanel::mouseReleaseEvent(QMouseEvent* event)
{
	mouseReleasePos_ = hovPos_;

	switch (event->button()) {
	case Qt::LeftButton:
		if (mousePressPos_ == mouseReleasePos_) {	// Jump cell
			if (hovPos_.row >= 0 && hovPos_.track >= 0) {
				int horDif = hovPos_.track - curPos_.track;
				int verDif = hovPos_.row - curPos_.row;
				moveCursorToRight(horDif);
				if (!bt_->isPlaySong() || !bt_->isFollowPlay()) moveCursorToDown(verDif);
			}
			else if (hovPos_.row == -2 && hovPos_.track >= 0) {	// Header
				int horDif = hovPos_.track - curPos_.track;
				moveCursorToRight(horDif);
			}
			else if (hovPos_.track == -2 && hovPos_.row >= 0) {	// Row number
				if (!bt_->isPlaySong() || !bt_->isFollowPlay()) {
					int verDif = hovPos_.row - curPos_.row;
					moveCursorToDown(verDif);
				}
			}
		}
		break;

	case Qt::RightButton:
		showContextMenu(mousePressPos_, event->pos());
		break;

	case Qt::XButton1:
		if (!bt_->isPlaySong() || !bt_->isFollowPlay()) {
			moveCursorToDown(-1);
		}
		break;

	case Qt::XButton2:
		if (!bt_->isPlaySong() || !bt_->isFollowPlay()) {
			moveCursorToDown(1);
		}
		break;

	default:
		break;
	}

	mousePressPos_ = { -1, -1 };
	mouseReleasePos_ = { -1, -1 };
}

bool OrderListPanel::mouseHoverd(QHoverEvent *event)
{
	QPoint pos = event->pos();

	OrderPosition oldPos = hovPos_;

	// Detect row
	if (pos.y() <= headerHeight_) {
		hovPos_.row = -2;	// Header
	}
	else {
		if (pos.y() < curRowY_) {
			int tmp = curPos_.row + (pos.y() - curRowY_) / rowFontHeight_ - 1;
			hovPos_.row = (tmp < 0) ? -1 : tmp;
		}
		else {
			hovPos_.row = curPos_.row + (pos.y() - curRowY_) / rowFontHeight_;
			if (hovPos_.row >= static_cast<int>(bt_->getOrderSize(curSongNum_))) hovPos_.row = -1;
		}
	}

	// Detect track
	if (pos.x() <= rowNumWidth_) {
		hovPos_.track = -2;	// Row number
	}
	else {
		int tmpWidth = rowNumWidth_;
		for (int i = leftTrackNum_; ; ) {
			tmpWidth += trackWidth_;
			if (pos.x() <= tmpWidth) {
				hovPos_.track = i;
				break;
			}
			++i;

			if (i == static_cast<int>(songStyle_.trackAttribs.size())) {
				hovPos_.track = -1;
				break;
			}
		}
	}

	if (hovPos_ != oldPos) redrawByHoverChanged();

	return true;
}

void OrderListPanel::wheelEvent(QWheelEvent *event)
{
	if (bt_->isPlaySong() && bt_->isFollowPlay()) return;
	int degree = event->angleDelta().y() / 8;
	moveCursorToDown(-degree / 15);
}

void OrderListPanel::leaveEvent(QEvent* event)
{
	Q_UNUSED(event)
	// Clear mouse hover selection
	hovPos_ = { -1, -1 };
}
