#include "order_list_panel.hpp"
#include <QPainter>
#include <QFontMetrics>
#include <QApplication>
#include <algorithm>
#include "gui/event_guard.hpp"

#include <QDebug>

OrderListPanel::OrderListPanel(QWidget *parent)
	: QWidget(parent),
	  leftTrackNum_(0),
	  curTrackNum_(0),
	  curRowNum_(0),
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
	rowNumColor_ = QColor::fromRgb(255, 200, 180);
	headerTextColor_ = QColor::fromRgb(240, 240, 200);
	headerRowColor_ = QColor::fromRgb(60, 60, 60);
	borderColor_ = QColor::fromRgb(120, 120, 120);


	initDisplay();
}

void OrderListPanel::setCore(std::shared_ptr<BambooTracker> core)
{
	bt_ = core;
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
					 (bt_->isJamMode())? curRowColor_ : curRowColorEditable_);
	// Row number
	painter.setPen(rowNumColor_);
	painter.drawText(1, curRowBaselineY_, QString("%1").arg(curRowNum_, 2, 16, QChar('0')).toUpper());
	// Step data
	painter.setPen(curTextColor_);
	for (x = rowNumWidth_ + widthSpace_, trackNum = leftTrackNum_; x < maxWidth; ) {
		lists_.push_back(bt_->getOrderList(songNum, trackNum));
		if (trackNum == curTrackNum_) {	// Paint current cell
			int curCellWidth;
			switch (modStyle_.trackAttribs[trackNum].source) {
			case SoundSource::FM:
			case SoundSource::PSG:
				curCellWidth = trackWidth_;
				break;
			}
			painter.fillRect(x - widthSpace_, curRowY_, curCellWidth, rowFontHeight_, curCellColor_);
		}
		painter.drawText(
					x,
					curRowBaselineY_,
					QString(" %1")
					.arg(lists_[trackNum - leftTrackNum_].at(curRowNum_), 2, 16, QChar('0')).toUpper()
				);

		switch (modStyle_.trackAttribs[trackNum].source) {
		case SoundSource::FM:
		case SoundSource::PSG:
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
			painter.drawText(x, baseY, QString(" %1").arg(0, 2, 16, QChar('0')).toUpper());

			switch (modStyle_.trackAttribs[trackNum].source) {
			case SoundSource::FM:
			case SoundSource::PSG:
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
			painter.drawText(x, baseY, QString(" %1").arg(0, 2, 16, QChar('0')).toUpper());

			switch (modStyle_.trackAttribs[trackNum].source) {
			case SoundSource::FM:
			case SoundSource::PSG:
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
		case SoundSource::PSG:	str = " PSG";	break;
		}
		painter.drawText(x,
						 rowFontLeading_ + rowFontAscend_,
						 str + QString::number(modStyle_.trackAttribs[trackNum].channelInSource + 1));

		switch (modStyle_.trackAttribs[trackNum].source) {
		case SoundSource::FM:
		case SoundSource::PSG:
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
		case SoundSource::PSG:
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
		case SoundSource::PSG:
			width +=  trackWidth_;
			break;
		}
	}
	return width;
}

void OrderListPanel::MoveCursorToRight(int n)
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

	bt_->setCurrentTrack(curTrackNum_);

	if (!isIgnoreToPattern_) emit currentTrackChanged(curTrackNum_);	// Send to pattern editor

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

	if (int dif = num - curTrackNum_) MoveCursorToRight(dif);
}

void OrderListPanel::setCurrentTrack(int num)
{
	Ui::EventGuard eg(isIgnoreToPattern_);

	if (int dif = num - curTrackNum_) MoveCursorToRight(dif);
}

/********** Events **********/
bool OrderListPanel::event(QEvent *event)
{
	switch (event->type()) {
	case QEvent::KeyPress:	return KeyPressed(dynamic_cast<QKeyEvent*>(event));
	default:				return QWidget::event(event);
	}
}

bool OrderListPanel::KeyPressed(QKeyEvent *event)
{
	switch (event->key()) {
	case Qt::Key_Left:	MoveCursorToRight(-1);	return true;
	case Qt::Key_Right:	MoveCursorToRight(1);	return true;
	case Qt::Key_Up:	return true;
	case Qt::Key_Down:	return true;
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
	setFocus();
}
