#include "order_list_panel.hpp"
#include <QPainter>
#include <QFontMetrics>
#include <QApplication>
#include <QClipboard>
#include <QMenu>
#include <QAction>
#include <QFontMetrics>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QPoint>
#include <QString>
#include <algorithm>
#include <vector>
#include <utility>
#include "gui/event_guard.hpp"
#include "gui/command/order/order_commands.hpp"
#include "track.hpp"

OrderListPanel::OrderListPanel(QWidget *parent)
	: QWidget(parent),
	  leftTrackNum_(0),
	  curSongNum_(0),
	  curPos_{ 0, 0 },
	  hovPos_{ -1, -1 },
	  editPos_{ -1, -1 },
	  mousePressPos_{ -1, -1 },
	  mouseReleasePos_{ -1, -1 },
	  selLeftAbovePos_{ -1, -1 },
	  selRightBelowPos_{ -1, -1 },
	  shiftPressedPos_{ -1, -1 },
	  isIgnoreToSlider_(false),
	  isIgnoreToPattern_(false),
	  entryCnt_(0),
	  selectAllState_(-1)
{
	/* Font */
	headerFont_ = QApplication::font();
	headerFont_.setPointSize(10);
	rowFont_ = QFont("Monospace", 10);
	rowFont_.setStyleHint(QFont::TypeWriter);
	rowFont_.setStyleStrategy(QFont::ForceIntegerMetrics);
	// Check font size
	QFontMetrics metrics(rowFont_);
	rowFontWidth_ = metrics.width('0');
	rowFontAscend_ = metrics.ascent();
	rowFontLeading_ = metrics.descent() / 2;
	rowFontHeight_ = rowFontAscend_ + rowFontLeading_;

	/* Width & height */
	widthSpace_ = rowFontWidth_ / 2;
	rowNumWidth_ = rowFontWidth_ * 2 + widthSpace_;
	trackWidth_ = rowFontWidth_ * 5;
	headerHeight_ = rowFontHeight_;

	initDisplay();

	setAttribute(Qt::WA_Hover);
}

void OrderListPanel::setCore(std::shared_ptr<BambooTracker> core)
{
	bt_ = core;
}

void OrderListPanel::setCommandStack(std::weak_ptr<QUndoStack> stack)
{
	comStack_ = stack;
}

void OrderListPanel::setConfiguration(std::weak_ptr<Configuration> config)
{
	config_ = config;
}

void OrderListPanel::setColorPallete(std::weak_ptr<ColorPalette> palette)
{
	palette_ = palette;
}

void OrderListPanel::initDisplay()
{
	pixmap_ = std::make_unique<QPixmap>(geometry().size());
}

void OrderListPanel::drawList(const QRect &rect)
{
	int maxWidth = std::min(geometry().width(), columnsWidthFromLeftToEnd_);

	pixmap_->fill(Qt::black);
	drawRows(maxWidth);
	drawHeaders(maxWidth);
	drawBorders(maxWidth);
	if (!hasFocus()) drawShadow();

	QPainter painter(this);
	painter.drawPixmap(rect, *pixmap_.get());
}

void OrderListPanel::drawRows(int maxWidth)
{
	QPainter painter(pixmap_.get());
	painter.setFont(rowFont_);

	std::vector<OrderData> orderRowData_;
	int x, trackNum;
	int textOffset = trackWidth_ / 2 - rowFontWidth_;

	/* Current row */
	// Fill row
	painter.fillRect(0, curRowY_, maxWidth, rowFontHeight_,
					 bt_->isJamMode() ? palette_.lock()->odrCurRowColor : palette_.lock()->odrCurEditRowColor);
	// Row number
	painter.setPen(palette_.lock()->odrRowNumColor);
	painter.drawText(1, curRowBaselineY_, QString("%1").arg(
						 curPos_.row, 2, (config_.lock()->getShowRowNumberInHex() ? 16 : 10), QChar('0')
						 ).toUpper());
	// Order data
	orderRowData_ = bt_->getOrderData(curSongNum_, curPos_.row);
	painter.setPen(palette_.lock()->odrCurTextColor);
	for (x = rowNumWidth_, trackNum = leftTrackNum_; x < maxWidth; ) {
		if (trackNum == curPos_.track)	// Paint current cell
			painter.fillRect(x, curRowY_, trackWidth_, rowFontHeight_, palette_.lock()->odrCurCellColor);
		if (((hovPos_.row == curPos_.row || hovPos_.row == -2) && hovPos_.track == trackNum)
				|| (hovPos_.track == -2 && hovPos_.row == curPos_.row))	// Paint hover
			painter.fillRect(x, curRowY_, trackWidth_, rowFontHeight_, palette_.lock()->odrHovCellColor);
		if ((selLeftAbovePos_.track >= 0 && selLeftAbovePos_.row >= 0)
				&& isSelectedCell(trackNum, curPos_.row))	// Paint selected
			painter.fillRect(x, curRowY_, trackWidth_, rowFontHeight_, palette_.lock()->odrSelCellColor);
		painter.drawText(
					x + textOffset,
					curRowBaselineY_,
					QString("%1")
					.arg(orderRowData_.at(trackNum).patten, 2, 16, QChar('0')).toUpper()
				);

		x += trackWidth_;
		++trackNum;
	}

	int rowNum;
	int rowY, baseY, endY;
	int playOdrNum = bt_->getPlayingOrderNumber();

	/* Previous rows */
	endY = std::max(headerHeight_ - rowFontHeight_, curRowY_ - rowFontHeight_ * curPos_.row);
	for (rowY = curRowY_ - rowFontHeight_, baseY = curRowBaselineY_ - rowFontHeight_, rowNum = curPos_.row - 1;
		 rowY >= endY;
		 rowY -= rowFontHeight_, baseY -= rowFontHeight_, --rowNum) {
		QColor rowColor, textColor;
		if (rowNum == playOdrNum) {
			rowColor = palette_.lock()->odrPlayRowColor;
			textColor = palette_.lock()->odrPlayTextColor;
		}
		else {
			rowColor = palette_.lock()->odrDefRowColor;
			textColor = palette_.lock()->odrDefTextColor;
		}

		// Fill row
		painter.fillRect(0, rowY, maxWidth, rowFontHeight_, rowColor);
		// Row number
		painter.setPen(palette_.lock()->odrRowNumColor);
		painter.drawText(1, baseY, QString("%1").arg(rowNum, 2, 16, QChar('0')).toUpper());
		// Order data
		orderRowData_ = bt_->getOrderData(curSongNum_, rowNum);
		painter.setPen(textColor);
		for (x = rowNumWidth_, trackNum = leftTrackNum_; x < maxWidth; ) {
			if (((hovPos_.row == rowNum || hovPos_.row == -2) && hovPos_.track == trackNum)
					|| (hovPos_.track == -2 && hovPos_.row == rowNum))	// Paint hover
				painter.fillRect(x, rowY, trackWidth_, rowFontHeight_, palette_.lock()->odrHovCellColor);
			if ((selLeftAbovePos_.track >= 0 && selLeftAbovePos_.row >= 0)
					&& isSelectedCell(trackNum, rowNum))	// Paint selected
				painter.fillRect(x, rowY, trackWidth_, rowFontHeight_, palette_.lock()->odrSelCellColor);
			painter.drawText(
						x + textOffset,
						baseY,
						QString("%1").arg(orderRowData_.at(trackNum).patten, 2, 16, QChar('0')).toUpper()
					);

			x += trackWidth_;
			++trackNum;
		}
	}

	/* Next rows */
	endY = std::min(geometry().height(),
					curRowY_ + rowFontHeight_ * (static_cast<int>(bt_->getOrderSize(curSongNum_)) - curPos_.row - 1));
	for (rowY = curRowY_ + rowFontHeight_, baseY = curRowBaselineY_ + rowFontHeight_, rowNum = curPos_.row + 1;
		 rowY <= endY;
		 rowY += rowFontHeight_, baseY += rowFontHeight_, ++rowNum) {
		QColor rowColor, textColor;
		if (rowNum == playOdrNum) {
			rowColor = palette_.lock()->odrPlayRowColor;
			textColor = palette_.lock()->odrPlayTextColor;
		}
		else {
			rowColor = palette_.lock()->odrDefRowColor;
			textColor = palette_.lock()->odrDefTextColor;
		}

		// Fill row
		painter.fillRect(0, rowY, maxWidth, rowFontHeight_, rowColor);
		// Row number
		painter.setPen(palette_.lock()->odrRowNumColor);
		painter.drawText(1, baseY, QString("%1").arg(rowNum, 2, 16, QChar('0')).toUpper());
		// Order data
		orderRowData_ = bt_->getOrderData(curSongNum_, rowNum);
		painter.setPen(textColor);
		for (x = rowNumWidth_, trackNum = leftTrackNum_; x < maxWidth; ) {
			if (((hovPos_.row == rowNum || hovPos_.row == -2) && hovPos_.track == trackNum)
					|| (hovPos_.track == -2 && hovPos_.row == rowNum))	// Paint hover
				painter.fillRect(x, rowY, trackWidth_, rowFontHeight_, palette_.lock()->odrHovCellColor);
			if ((selLeftAbovePos_.track >= 0 && selLeftAbovePos_.row >= 0)
					&& isSelectedCell(trackNum, rowNum))	// Paint selected
				painter.fillRect(x, rowY, trackWidth_, rowFontHeight_, palette_.lock()->odrSelCellColor);
			painter.drawText(
						x + textOffset,
						baseY,
						QString("%1").arg(orderRowData_.at(trackNum).patten, 2, 16, QChar('0')).toUpper()
					);

			x += trackWidth_;
			++trackNum;
		}
	}
}

void OrderListPanel::drawHeaders(int maxWidth)
{
	QPainter painter(pixmap_.get());
	painter.setFont(headerFont_);

	painter.fillRect(0, 0, geometry().width(), headerHeight_, palette_.lock()->odrHeaderRowColor);
	painter.setPen(palette_.lock()->odrHeaderTextColor);
	int x, trackNum;
	for (x = rowNumWidth_, trackNum = leftTrackNum_; x < maxWidth; ) {
		QString str;
		switch (songStyle_.trackAttribs[trackNum].source) {
		case SoundSource::FM:
			str = "FM" + QString::number(songStyle_.trackAttribs[trackNum].channelInSource + 1);
			break;
		case SoundSource::SSG:
			str = "SSG" + QString::number(songStyle_.trackAttribs[trackNum].channelInSource + 1);
			break;
		case SoundSource::DRUM:
			switch (songStyle_.trackAttribs[trackNum].channelInSource) {
			case 0:	str = "BD";	break;
			case 1:	str = "SD";	break;
			case 2:	str = "TOP";	break;
			case 3:	str = "HH";	break;
			case 4:	str = "TOM";	break;
			case 5:	str = "RIM";	break;
			}
			break;
		}

		QFontMetrics metrics(headerFont_);
		int rw = trackWidth_ - metrics.width(str);
		rw = (rw < 0) ? 0 : (rw / 2);
		painter.drawText(x + rw, rowFontLeading_ + rowFontAscend_, str);

		x += trackWidth_;
		++trackNum;
	}
}

void OrderListPanel::drawBorders(int maxWidth)
{
	QPainter painter(pixmap_.get());

	painter.drawLine(0, headerHeight_, geometry().width(), headerHeight_);
	painter.drawLine(rowNumWidth_, 0, rowNumWidth_, geometry().height());
	int x, trackNum;
	for (x = rowNumWidth_ + trackWidth_, trackNum = leftTrackNum_; x <= maxWidth; ) {
		painter.drawLine(x, 0, x, geometry().height());
		x += trackWidth_;
		++trackNum;
	}
}

void OrderListPanel::drawShadow()
{
	QPainter painter(pixmap_.get());
	painter.fillRect(0, 0, geometry().width(), geometry().height(), QColor::fromRgb(0, 0, 0, 47));
}

int OrderListPanel::calculateColumnsWidthWithRowNum(int begin, int end) const
{
	int width = rowNumWidth_;
	for (int i = begin; i <= end; ++i) {
		width +=  trackWidth_;
	}
	return width;
}

void OrderListPanel::moveCursorToRight(int n)
{
	int prevTrack = curPos_.track;
	int tmp = curPos_.track + n;
	if (n > 0) {
		while (true) {
			int sub = tmp - songStyle_.trackAttribs.size();
			if (sub < 0) {
				curPos_.track = tmp;
				break;
			}
			else {
				if (config_.lock()->getWarpCursor()) {
					tmp = sub;
				}
				else {
					curPos_.track = songStyle_.trackAttribs.size() - 1;
					break;
				}
			}
		}
	}
	else {
		while (true) {
			int add = tmp + songStyle_.trackAttribs.size();
			if (tmp < 0) {
				if (config_.lock()->getWarpCursor()) {
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

	columnsWidthFromLeftToEnd_
			= calculateColumnsWidthWithRowNum(leftTrackNum_, songStyle_.trackAttribs.size() - 1);

	if (!isIgnoreToSlider_) emit currentTrackChangedForSlider(curPos_.track);	// Send to slider

	if (!isIgnoreToPattern_) emit currentTrackChanged(curPos_.track);	// Send to pattern editor

	update();
}

void OrderListPanel::moveCursorToDown(int n)
{
	int tmp = curPos_.row + n;
	int endRow = bt_->getOrderSize(curSongNum_);
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

	if (!isIgnoreToSlider_) emit currentOrderChangedForSlider(curPos_.row, bt_->getOrderSize(curSongNum_) - 1);	// Send to slider

	if (!isIgnoreToPattern_) emit currentOrderChanged(curPos_.row);	// Send to pattern editor

	update();
}

void OrderListPanel::changeEditable()
{
	update();
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
	entryCnt_ = (entryCnt_ == 1 && curPos_ == editPos_) ? 0 : 1;
	editPos_ = curPos_;
	bt_->setOrderPattern(curSongNum_, editPos_.track, editPos_.row, n);
	comStack_.lock()->push(new SetPatternToOrderQtCommand(this, editPos_));

	if (!bt_->isPlaySong() && !entryCnt_) moveCursorToDown(1);
}

void OrderListPanel::insertOrderBelow()
{
	if (bt_->isJamMode()) return;

	bt_->insertOrderBelow(curSongNum_, curPos_.row);
	comStack_.lock()->push(new InsertOrderBelowQtCommand(this));
	if (!bt_->isPlaySong()) moveCursorToDown(1);
}

void OrderListPanel::deleteOrder()
{
	if (bt_->isJamMode()) return;

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
			str += QString::number(odrs.at(selLeftAbovePos_.track + j).patten);
			if (i < h - 1 || j < w - 1) str += ",";
		}
	}

	QApplication::clipboard()->setText(str);
}

void OrderListPanel::pasteCopiedCells(OrderPosition& startPos)
{
	// Analyze text
	QString str = QApplication::clipboard()->text().remove(QRegularExpression("ORDER_COPY:"));
	QString hdRe = "^([0-9]+),([0-9]+),";
	QRegularExpression re(hdRe);
	QRegularExpressionMatch match = re.match(str);
	int w = match.captured(1).toInt();
	int h = match.captured(2).toInt();
	str.remove(re);

	std::vector<std::vector<std::string>> cells;
	re = QRegularExpression("^([^,]+),");
	for (int i = 0; i < h; ++i) {
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
}

bool OrderListPanel::isSelectedCell(int track, int row)
{
	OrderPosition pos{ track, row };
	return (selLeftAbovePos_.track <= pos.track && selRightBelowPos_.track >= pos.track
			&& selLeftAbovePos_.row <= pos.row && selRightBelowPos_.row >= pos.row);
}

/********** Slots **********/
void OrderListPanel::setCurrentTrackForSlider(int num)
{
	Ui::EventGuard eg(isIgnoreToSlider_);

	if (int dif = num - curPos_.track) moveCursorToRight(dif);
}

void OrderListPanel::setCurrentOrderForSlider(int num) {
	Ui::EventGuard eg(isIgnoreToSlider_);

	if (int dif = num - curPos_.row) moveCursorToDown(dif);
}

void OrderListPanel::setCurrentTrack(int num)
{
	Ui::EventGuard eg(isIgnoreToPattern_);

	if (int dif = num - curPos_.track) moveCursorToRight(dif);
}

void OrderListPanel::setCurrentOrder(int num) {
	Ui::EventGuard eg(isIgnoreToPattern_);

	if (int dif = num - curPos_.row) moveCursorToDown(dif);
}

void OrderListPanel::onOrderEdited()
{
	// Move cursor
	size_t s = bt_->getOrderSize(curSongNum_);
	if (s <= curPos_.row) curPos_.row = s - 1;

	emit orderEdited();
}

void OrderListPanel::onSongLoaded()
{
	curSongNum_ = bt_->getCurrentSongNumber();
	curPos_ = { bt_->getCurrentTrackAttribute().number, bt_->getCurrentOrderNumber() };
	songStyle_ = bt_->getSongStyle(curSongNum_);
	columnsWidthFromLeftToEnd_ = calculateColumnsWidthWithRowNum(0, songStyle_.trackAttribs.size() - 1);

	hovPos_ = { -1, -1 };
	editPos_ = { -1, -1 };
	mousePressPos_ = { -1, -1 };
	mouseReleasePos_ = { -1, -1 };
	selLeftAbovePos_ = { -1, -1 };
	selRightBelowPos_ = { -1, -1 };
	shiftPressedPos_ = { -1, -1 };
	entryCnt_ = 0;
	selectAllState_ = -1;
	emit selected(false);

	update();
}

void OrderListPanel::onPastePressed()
{
	if (!bt_->isPlaySong()) pasteCopiedCells(curPos_);
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
		update();
		break;
	}
	case 1:	// All
	{
		int max = bt_->getOrderSize(curSongNum_) - 1;
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

	update();
}

void OrderListPanel::onDuplicatePressed()
{
	bt_->duplicateOrder(curSongNum_, curPos_.row);
	comStack_.lock()->push(new DuplicateOrderQtCommand(this));
}

void OrderListPanel::onMoveOrderPressed(bool isUp)
{
	if ((isUp && curPos_.row == 0)
			|| (!isUp && curPos_.row == bt_->getOrderSize(curSongNum_) - 1))
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
	case Qt::Key_Shift:
		shiftPressedPos_ = curPos_;
		return true;
	case Qt::Key_Left:
		moveCursorToRight(-1);
		if (event->modifiers().testFlag(Qt::ShiftModifier))
			setSelectedRectangle(shiftPressedPos_, curPos_);
		return true;
	case Qt::Key_Right:
		moveCursorToRight(1);
		if (event->modifiers().testFlag(Qt::ShiftModifier))
			setSelectedRectangle(shiftPressedPos_, curPos_);
		return true;
	case Qt::Key_Up:
		if (bt_->isPlaySong()) {
			return false;
		}
		else {
			moveCursorToDown(-1);
			if (event->modifiers().testFlag(Qt::ShiftModifier)
					&& shiftPressedPos_.row == curPos_.row) {
				setSelectedRectangle(shiftPressedPos_, curPos_);
				return true;
			}
			return true;
		}
	case Qt::Key_Down:
		if (bt_->isPlaySong()) {
			return false;
		}
		else {
			moveCursorToDown(1);
			if (event->modifiers().testFlag(Qt::ShiftModifier)
					&& shiftPressedPos_.row == curPos_.row) {
				setSelectedRectangle(shiftPressedPos_, curPos_);
				return true;
			}
			return true;
		}
	case Qt::Key_Home:
		if (bt_->isPlaySong()) {
			return false;
		}
		else {
			moveCursorToDown(-curPos_.row);
			if (event->modifiers().testFlag(Qt::ShiftModifier)
					&& shiftPressedPos_.row == curPos_.row) {
				setSelectedRectangle(shiftPressedPos_, curPos_);
				return true;
			}
			return true;
		}
	case Qt::Key_End:
		if (bt_->isPlaySong()) {
			return false;
		}
		else {
			moveCursorToDown(
						bt_->getOrderSize(curSongNum_) - curPos_.row - 1);
			if (event->modifiers().testFlag(Qt::ShiftModifier)
					&& shiftPressedPos_.row == curPos_.row) {
				setSelectedRectangle(shiftPressedPos_, curPos_);
				return true;
			}
			return true;
		}
	case Qt::Key_PageUp:
		if (bt_->isPlaySong()) {
			return false;
		}
		else {
			moveCursorToDown(-config_.lock()->getPageJumpLength());
			if (event->modifiers().testFlag(Qt::ShiftModifier)
					&& shiftPressedPos_.row == curPos_.row) {
				setSelectedRectangle(shiftPressedPos_, curPos_);
				return true;
			}
			return true;
		}
	case Qt::Key_PageDown:
		if (bt_->isPlaySong()) {
			return false;
		}
		else {
			moveCursorToDown(config_.lock()->getPageJumpLength());
			if (event->modifiers().testFlag(Qt::ShiftModifier)
					&& shiftPressedPos_.row == curPos_.row) {
				setSelectedRectangle(shiftPressedPos_, curPos_);
				return true;
			}
			return true;
		}
	case Qt::Key_Insert:
		insertOrderBelow();
		return true;
	default:
		if (!bt_->isJamMode()) {
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
	if (bt_ != nullptr) drawList(event->rect());
}

void OrderListPanel::resizeEvent(QResizeEvent *event)
{
	QWidget::resizeEvent(event);

	// Recalculate center row position
	curRowBaselineY_ = (geometry().height() + headerHeight_) / 2;
	curRowY_ = curRowBaselineY_ + rowFontLeading_ / 2 - rowFontAscend_;
	initDisplay();
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
			update();
		}

		if (event->x() < rowNumWidth_ && leftTrackNum_ > 0) {
			moveCursorToRight(-1);
		}
		else if (event->x() > geometry().width() - rowNumWidth_ && hovPos_.track != -1) {
			moveCursorToRight(1);
		}
		if (event->pos().y() < headerHeight_ + rowFontHeight_) {
			moveCursorToDown(-1);
		}
		else if (event->pos().y() > geometry().height() - rowFontHeight_) {
			moveCursorToDown(1);
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
				moveCursorToDown(verDif);
				update();
			}
			else if (hovPos_.row == -2 && hovPos_.track >= 0) {	// Header
				int horDif = hovPos_.track - curPos_.track;
				moveCursorToRight(horDif);
				update();
			}
			else if (hovPos_.track == -2 && hovPos_.row >= 0) {	// Row number
				int verDif = hovPos_.row - curPos_.row;
				moveCursorToDown(verDif);
				update();
			}
		}
		break;

	case Qt::RightButton:
	{
		QMenu menu;
		QAction* insert = menu.addAction("Insert Order", this, [&]() { insertOrderBelow(); });
		QAction* remove = menu.addAction("Remove Order", this, [&]() { deleteOrder(); });
		QAction* duplicate = menu.addAction("Duplicate Order", this, &OrderListPanel::onDuplicatePressed);
		QAction* clonep = menu.addAction("Clone Patterns", this, &OrderListPanel::onClonePatternsPressed);
		QAction* cloneo = menu.addAction("Clone Order", this, &OrderListPanel::onCloneOrderPressed);
		menu.addSeparator();
		QAction* moveUp = menu.addAction("Move Order Up", this, [&]() { onMoveOrderPressed(true); });
		QAction* moveDown = menu.addAction("Move Order Down", this, [&]() { onMoveOrderPressed(false); });
		menu.addSeparator();
		QAction* copy = menu.addAction("Copy", this, &OrderListPanel::copySelectedCells);
		QAction* paste = menu.addAction("Paste", this, [&]() { pasteCopiedCells(mousePressPos_); });

		if (bt_->isJamMode() || mousePressPos_.row < 0 || mousePressPos_.track < 0) {
			insert->setEnabled(false);
			remove->setEnabled(false);
			duplicate->setEnabled(false);
			clonep->setEnabled(false);
			cloneo->setEnabled(false);
			moveUp->setEnabled(false);
			moveDown->setEnabled(false);
			copy->setEnabled(false);
			paste->setEnabled(false);
		}
		else {
			QString clipText = QApplication::clipboard()->text();
			if (!clipText.startsWith("ORDER_COPY")) {
					paste->setEnabled(false);
			}
			if (bt_->getOrderSize(curSongNum_) == 1) {
				remove->setEnabled(false);
			}
			if (selRightBelowPos_.row < 0
					|| !isSelectedCell(mousePressPos_.track, mousePressPos_.row)) {
				clonep->setEnabled(false);
				copy->setEnabled(false);
			}
			if (mousePressPos_.row == 0) {
				moveUp->setEnabled(false);
			}
			if (mousePressPos_.row == bt_->getOrderSize(curSongNum_) - 1) {
				moveDown->setEnabled(false);
			}
		}

		menu.exec(mapToGlobal(event->pos()));
		break;
	}

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
			if (hovPos_.row >= bt_->getOrderSize(curSongNum_)) hovPos_.row = -1;
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

			if (i == songStyle_.trackAttribs.size()) {
				hovPos_.track = -1;
				break;
			}
		}
	}

	if (hovPos_ != oldPos) update();

	return true;
}

void OrderListPanel::wheelEvent(QWheelEvent *event)
{
	int degree = event->angleDelta().y() / 8;
	moveCursorToDown(-degree / 15);
}

void OrderListPanel::leaveEvent(QEvent* event)
{
	Q_UNUSED(event)
	// Clear mouse hover selection
	hovPos_ = { -1, -1 };
}
