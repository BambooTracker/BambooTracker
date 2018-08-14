#include "order_list_panel.hpp"
#include <QPainter>
#include <QFontMetrics>
#include <QApplication>
#include <QPoint>
#include <algorithm>
#include "gui/event_guard.hpp"

OrderListPanel::OrderListPanel(QWidget *parent)
	: QWidget(parent),
	  leftTrackNum_(0),
	  curSongNum_(0),
	  curTrackNum_(0),
	  curRowNum_(0),
	  hovTrackNum_(-1),
	  hovRowNum_(-1),
	  isIgnoreToSlider_(false),
	  isIgnoreToPattern_(false)
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
	rowFontHeight_ = metrics.height();
	rowFontLeading_ = metrics.leading();

	/* Width & height */
	widthSpace_ = rowFontWidth_ / 2;
	rowNumWidth_ = rowFontWidth_ * 2 + widthSpace_;
	trackWidth_ = rowFontWidth_ * 5;
	headerHeight_ = rowFontHeight_;

	/* Color */
	defTextColor_ = QColor::fromRgb(180, 180, 180);
	defRowColor_ = QColor::fromRgb(0, 0, 40);
	curTextColor_ = QColor::fromRgb(255, 255, 255);
	curRowColor_ = QColor::fromRgb(110, 90, 140);
	curRowColorEditable_ = QColor::fromRgb(140, 90, 110);
	curCellColor_ = QColor::fromRgb(255, 255, 255, 127);
	selTextColor_ = defTextColor_;
	selCellColor_ = QColor::fromRgb(100, 100, 200);
	hovCellColor_ = QColor::fromRgb(255, 255, 255, 64);
	rowNumColor_ = QColor::fromRgb(255, 200, 180);
	headerTextColor_ = QColor::fromRgb(240, 240, 200);
	headerRowColor_ = QColor::fromRgb(60, 60, 60);
	borderColor_ = QColor::fromRgb(120, 120, 120);


	initDisplay();

	setAttribute(Qt::WA_Hover);
}

void OrderListPanel::setCore(std::shared_ptr<BambooTracker> core)
{
	bt_ = core;
	curSongNum_ = bt_->getCurrentSongNumber();
	curTrackNum_ = bt_->getCurrentTrackAttribute().number;
	curRowNum_ = bt_->getCurrentOrderNumber();
	modStyle_ = bt_->getModuleStyle();
	columnsWidthFromLeftToEnd_ = calculateColumnsWidthWithRowNum(0, modStyle_.trackAttribs.size() - 1);
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

	std::vector<std::vector<int>> lists_;
	int x, trackNum;

	/* Current row */
	// Fill row
	painter.fillRect(0, curRowY_, maxWidth, rowFontHeight_,
					 bt_->isJamMode() ? curRowColor_ : curRowColorEditable_);
	// Row number
	painter.setPen(rowNumColor_);
	painter.drawText(1, curRowBaselineY_, QString("%1").arg(curRowNum_, 2, 16, QChar('0')).toUpper());
	// Step data
	painter.setPen(curTextColor_);
	for (x = rowNumWidth_ + widthSpace_, trackNum = leftTrackNum_; x < maxWidth; ) {
		lists_.push_back(bt_->getOrderList(curSongNum_, trackNum));
		if (trackNum == curTrackNum_) {	// Paint current cell
			int curCellWidth;
			switch (modStyle_.trackAttribs[trackNum].source) {
			case SoundSource::FM:
			case SoundSource::SSG:
				curCellWidth = trackWidth_;
				break;
			}
			painter.fillRect(x - widthSpace_, curRowY_, curCellWidth, rowFontHeight_, curCellColor_);
		}
		if (hovRowNum_ == curRowNum_ && hovTrackNum_ == trackNum) {	// Paint hover
			int curCellWidth;
			switch (modStyle_.trackAttribs[trackNum].source) {
			case SoundSource::FM:
			case SoundSource::SSG:
				curCellWidth = trackWidth_;
				break;
			}
			painter.fillRect(x - widthSpace_, curRowY_, curCellWidth, rowFontHeight_, hovCellColor_);
		}
		painter.drawText(
					x,
					curRowBaselineY_,
					QString(" %1")
					.arg(lists_[trackNum - leftTrackNum_].at(curRowNum_), 2, 16, QChar('0')).toUpper()
				);

		switch (modStyle_.trackAttribs[trackNum].source) {
		case SoundSource::FM:
		case SoundSource::SSG:
			x += trackWidth_;
			break;
		}
		++trackNum;
	}

	int rowNum;
	int rowY, baseY, endY;

	/* Previous rows */
	endY = std::max(headerHeight_ - rowFontHeight_, curRowY_ - rowFontHeight_ * curRowNum_);
	for (rowY = curRowY_ - rowFontHeight_, baseY = curRowBaselineY_ - rowFontHeight_, rowNum = curRowNum_ - 1;
		 rowY >= endY;
		 rowY -= rowFontHeight_, baseY -= rowFontHeight_, --rowNum) {
		// Fill row
		painter.fillRect(0, rowY, maxWidth, rowFontHeight_, defRowColor_);
		// Row number
		painter.setPen(rowNumColor_);
		painter.drawText(1, baseY, QString("%1").arg(rowNum, 2, 16, QChar('0')).toUpper());
		painter.setPen(defTextColor_);
		for (x = rowNumWidth_ + widthSpace_, trackNum = leftTrackNum_; x < maxWidth; ) {
			if (hovRowNum_ == rowNum && hovTrackNum_ == trackNum) {	// Paint hover
				int curCellWidth;
				switch (modStyle_.trackAttribs[trackNum].source) {
				case SoundSource::FM:
				case SoundSource::SSG:
					curCellWidth = trackWidth_;
					break;
				}
				painter.fillRect(x - widthSpace_, rowY, curCellWidth, rowFontHeight_, hovCellColor_);
			}
			painter.drawText(x, baseY, QString(" %1").arg(0, 2, 16, QChar('0')).toUpper());

			switch (modStyle_.trackAttribs[trackNum].source) {
			case SoundSource::FM:
			case SoundSource::SSG:
				x += trackWidth_;
				break;
			}
			++trackNum;
		}
	}

	/* Next rows */
	endY = std::min(geometry().height(),
					curRowY_ + rowFontHeight_ * (static_cast<int>(lists_[0].size()) - curRowNum_ - 1));
	for (rowY = curRowY_ + rowFontHeight_, baseY = curRowBaselineY_ + rowFontHeight_, rowNum = curRowNum_ + 1;
		 rowY <= endY;
		 rowY += rowFontHeight_, baseY += rowFontHeight_, ++rowNum) {
		// Fill row
		painter.fillRect(0, rowY, maxWidth, rowFontHeight_, defRowColor_);
		// Row number
		painter.setPen(rowNumColor_);
		painter.drawText(1, baseY, QString("%1").arg(rowNum, 2, 16, QChar('0')).toUpper());
		painter.setPen(defTextColor_);
		for (x = rowNumWidth_ + widthSpace_, trackNum = leftTrackNum_; x < maxWidth; ) {
			if (hovRowNum_ == rowNum && hovTrackNum_ == trackNum) {	// Paint hover
				int curCellWidth;
				switch (modStyle_.trackAttribs[trackNum].source) {
				case SoundSource::FM:
				case SoundSource::SSG:
					curCellWidth = trackWidth_;
					break;
				}
				painter.fillRect(x - widthSpace_, rowY, curCellWidth, rowFontHeight_, hovCellColor_);
			}
			painter.drawText(x, baseY, QString(" %1").arg(0, 2, 16, QChar('0')).toUpper());

			switch (modStyle_.trackAttribs[trackNum].source) {
			case SoundSource::FM:
			case SoundSource::SSG:
				x += trackWidth_;
				break;
			}
			++trackNum;
		}
	}
}

void OrderListPanel::drawHeaders(int maxWidth)
{
	QPainter painter(pixmap_.get());
	painter.setFont(headerFont_);

	painter.fillRect(0, 0, geometry().width(), headerHeight_, headerRowColor_);
	painter.setPen(headerTextColor_);
	int x, trackNum;
	for (x = rowNumWidth_ + widthSpace_, trackNum = leftTrackNum_; x < maxWidth; ) {
		QString str;
		switch (modStyle_.trackAttribs[trackNum].source) {
		case SoundSource::FM:	str = " FM";	break;
		case SoundSource::SSG:	str = " SSG";	break;
		}
		painter.drawText(x,
						 rowFontLeading_ + rowFontAscend_,
						 str + QString::number(modStyle_.trackAttribs[trackNum].channelInSource + 1));

		switch (modStyle_.trackAttribs[trackNum].source) {
		case SoundSource::FM:
		case SoundSource::SSG:
			x += trackWidth_;
			break;
		}
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

		switch (modStyle_.trackAttribs[trackNum].source) {
		case SoundSource::FM:
		case SoundSource::SSG:
			x += trackWidth_;
			break;
		}
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
		switch (modStyle_.trackAttribs.at(i).source) {
		case SoundSource::FM:
		case SoundSource::SSG:
			width +=  trackWidth_;
			break;
		}
	}
	return width;
}

void OrderListPanel::moveCursorToRight(int n)
{
	if (n > 0) {
		curTrackNum_ = std::min(curTrackNum_ + n, static_cast<int>(modStyle_.trackAttribs.size()) - 1);
		while (calculateColumnsWidthWithRowNum(leftTrackNum_, curTrackNum_) > geometry().width())
			++leftTrackNum_;
	}
	else {
		curTrackNum_ = std::max(curTrackNum_ + n, 0);
		if (curTrackNum_ < leftTrackNum_) leftTrackNum_ = curTrackNum_;
	}
	columnsWidthFromLeftToEnd_
			= calculateColumnsWidthWithRowNum(leftTrackNum_, modStyle_.trackAttribs.size() - 1);

	if (!isIgnoreToSlider_) emit currentTrackChangedForSlider(curTrackNum_);	// Send to slider

	if (!isIgnoreToPattern_) emit currentTrackChanged(curTrackNum_);	// Send to pattern editor

	update();
}

void OrderListPanel::moveCursorToDown(int n)
{
	int tmp = curRowNum_ + n;

	if (n > 0) {
		int endRow = bt_->getOrderList(curSongNum_, 0).size();
		if (tmp < endRow) {
			curRowNum_ = tmp;
		}
		else {
			curRowNum_ = endRow - 1;
		}
	}
	else {
		if (tmp < 0) {
			curRowNum_ = 0;
		}
		else {
			curRowNum_ = tmp;
		}
	}

	if (!isIgnoreToSlider_) emit currentOrderChangedForSlider(curRowNum_);	// Send to slider

	if (!isIgnoreToPattern_) emit currentOrderChanged(curRowNum_);	// Send to pattern editor

	update();
}

void OrderListPanel::changeEditable()
{
	update();
}

/********** Slots **********/
void OrderListPanel::setCurrentTrackForSlider(int num)
{
	Ui::EventGuard eg(isIgnoreToSlider_);

	if (int dif = num - curTrackNum_) moveCursorToRight(dif);
}

void OrderListPanel::setCurrentOrderForSlider(int num) {
	Ui::EventGuard eg(isIgnoreToSlider_);

	if (int dif = num - curRowNum_) moveCursorToDown(dif);
}

void OrderListPanel::setCurrentTrack(int num)
{
	Ui::EventGuard eg(isIgnoreToPattern_);

	if (int dif = num - curTrackNum_) moveCursorToRight(dif);
}

void OrderListPanel::setCurrentOrder(int num) {
	Ui::EventGuard eg(isIgnoreToPattern_);

	if (int dif = num - curRowNum_) moveCursorToDown(dif);
}

/********** Events **********/
bool OrderListPanel::event(QEvent *event)
{
	switch (event->type()) {
	case QEvent::KeyPress:
		return KeyPressed(dynamic_cast<QKeyEvent*>(event));
	case QEvent::HoverMove:
		return mouseHoverd(dynamic_cast<QHoverEvent*>(event));
	default:
		return QWidget::event(event);
	}
}

bool OrderListPanel::KeyPressed(QKeyEvent *event)
{
	/* General Keys (with Ctrl) */
	if (event->modifiers().testFlag(Qt::ControlModifier)) return false;

	/* General Keys */
	switch (event->key()) {
	case Qt::Key_Left:	moveCursorToRight(-1);	return true;
	case Qt::Key_Right:	moveCursorToRight(1);	return true;
	case Qt::Key_Up:	moveCursorToDown(-1);	return true;
	case Qt::Key_Down:	moveCursorToDown(1);	return true;
	default: return false;
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
	curRowBaselineY_ = (geometry().height() - headerHeight_) / 2 + headerHeight_;
	curRowY_ = curRowBaselineY_ - (rowFontAscend_ + rowFontLeading_ / 2);
	initDisplay();
}

void OrderListPanel::mousePressEvent(QMouseEvent *event)
{
	Q_UNUSED(event)

	setFocus();

	if (hovRowNum_ >= 0 && hovTrackNum_ >= 0) {
		int horDif = hovTrackNum_ - curTrackNum_;
		int verDif = hovRowNum_ - curRowNum_;

		moveCursorToRight(horDif);
		moveCursorToDown(verDif);
		update();
	}
}

bool OrderListPanel::mouseHoverd(QHoverEvent *event)
{
	QPoint pos = event->pos();

	int oldRow = hovRowNum_;
	int oldTrack = hovTrackNum_;

	// Detect row
	if (pos.y() <= headerHeight_) {
		 hovRowNum_ = -1;	// Header
	}
	else {
		if (pos.y() < curRowY_) {
			hovRowNum_ = curRowNum_ + (pos.y() - curRowY_) / rowFontHeight_ - 1;
		}
		else {
			hovRowNum_ = curRowNum_ + (pos.y() - curRowY_) / rowFontHeight_;
			if (hovRowNum_ >= bt_->getOrderList(curSongNum_, 0).size()) hovRowNum_ = -1;
		}
	}

	// Detect track
	if (pos.x() <= rowNumWidth_) {
		hovTrackNum_ = -1;	// Row number
	}
	else {
		int tmpWidth = rowNumWidth_;
		for (int i = leftTrackNum_; ; ++i) {
			switch (modStyle_.trackAttribs[i].source) {
			case SoundSource::FM:
			case SoundSource::SSG:
				tmpWidth += trackWidth_;
				break;
			}

			if (pos.x() <= tmpWidth) {
				hovTrackNum_ = i;
				break;
			}
		}
	}

	if (hovTrackNum_ != oldTrack || hovRowNum_ != oldRow) update();

	return true;
}

void OrderListPanel::wheelEvent(QWheelEvent *event)
{
	int degree = event->angleDelta().y() / 8;
	moveCursorToDown(-degree / 15);
}
