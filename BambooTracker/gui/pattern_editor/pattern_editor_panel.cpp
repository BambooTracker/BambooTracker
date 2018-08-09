#include "pattern_editor_panel.hpp"
#include <QPainter>
#include <QFontMetrics>
#include <QPoint>
#include <QApplication>
#include <algorithm>
#include "gui/event_guard.hpp"

#include <QDebug>

PatternEditorPanel::PatternEditorPanel(QWidget *parent)
	: QWidget(parent),
	  leftTrackNum_(0),
	  curTrackNum_(0),
	  curCellNumInTrack_(0),
	  curRowNum_(0),
	  isIgnoreToSlider_(false),
	  isIgnoreToOrder_(false)
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
	toneNameWidth_ = rowFontWidth_ * 3;
	instWidth_ = rowFontWidth_ * 2;
	volWidth_ = rowFontWidth_ * 2;
	effWidth_ = rowFontWidth_ * 3;
	trackWidth_ = toneNameWidth_ + instWidth_ + volWidth_ + effWidth_ + rowFontWidth_ * 4;
	headerHeight_ = rowFontHeight_ * 2;

	/* Color */
	defTextColor_ = QColor::fromRgb(180, 180, 180);
	defRowColor_ = QColor::fromRgb(0, 0, 40);
	mkRowColor_ = QColor::fromRgb(40, 40, 80);
	curTextColor_ = QColor::fromRgb(255, 255, 255);
	curRowColor_ = QColor::fromRgb(110, 90, 140);
	curRowColorEditable_ = QColor::fromRgb(140, 90, 110);
	curCellColor_ = QColor::fromRgb(255, 255, 255, 127);
	selTextColor_ = defTextColor_;
	selCellColor_ = QColor::fromRgb(100, 100, 200);
	defRowNumColor_ = QColor::fromRgb(255, 200, 180);
	mkRowNumColor_ = QColor::fromRgb(255, 140, 160);
	headerTextColor_ = QColor::fromRgb(240, 240, 200);
	headerRowColor_ = QColor::fromRgb(60, 60, 60);
	borderColor_ = QColor::fromRgb(120, 120, 120);


	initDisplay();

	setAttribute(Qt::WA_Hover);
}

void PatternEditorPanel::initDisplay()
{
	pixmap_ = std::make_unique<QPixmap>(geometry().size());
}

void PatternEditorPanel::setCore(std::shared_ptr<BambooTracker> core)
{
	bt_ = core;
	modStyle_ = bt_->getModuleStyle();
	TracksWidthFromLeftToEnd_ = calculateTracksWidthWithRowNum(0, modStyle_.trackAttribs.size() - 1);
}

void PatternEditorPanel::drawPattern(const QRect &rect)
{
	int maxWidth = std::min(geometry().width(), TracksWidthFromLeftToEnd_);

	pixmap_->fill(Qt::black);
	drawRows(maxWidth);
	drawHeaders(maxWidth);
	drawBorders(maxWidth);
	if (!hasFocus()) drawShadow();

	QPainter painter(this);
	painter.drawPixmap(rect, *pixmap_.get());
}

void PatternEditorPanel::drawRows(int maxWidth)
{
	QPainter painter(pixmap_.get());
	painter.setFont(rowFont_);

	int x, trackNum;

	int curRowNum = 32;	// dummy set
	int mkCnt = 8;

	/* Current row */
	// Fill row
	painter.fillRect(0, curRowY_, maxWidth, rowFontHeight_,
					 (bt_->isJamMode())? curRowColor_ : curRowColorEditable_);
	// Row number
	painter.setPen((curRowNum % mkCnt)? defRowNumColor_ : mkRowNumColor_);
	painter.drawText(1, curRowBaselineY_, QString("%1").arg(curRowNum, 2, 16, QChar('0')).toUpper());
	// Step data
	painter.setPen(curTextColor_);
	for (x = rowNumWidth_ + widthSpace_, trackNum = leftTrackNum_; x < maxWidth; ) {
		int offset = x;
		switch (modStyle_.trackAttribs[trackNum].source) {
		case SoundSource::FM:
		case SoundSource::PSG:
			if (trackNum == curTrackNum_ && curCellNumInTrack_ == 0)
				painter.fillRect(offset - widthSpace_, curRowY_, toneNameWidth_ + rowFontWidth_, rowFontHeight_, curCellColor_);
			painter.drawText(offset, curRowBaselineY_, "---");
			offset += toneNameWidth_ +  rowFontWidth_;
			if (trackNum == curTrackNum_ && curCellNumInTrack_ == 1)
				painter.fillRect(offset - widthSpace_, curRowY_, instWidth_ + rowFontWidth_, rowFontHeight_, curCellColor_);
			painter.drawText(offset, curRowBaselineY_, "--");
			offset += instWidth_ +  rowFontWidth_;
			if (trackNum == curTrackNum_ && curCellNumInTrack_ == 2)
				painter.fillRect(offset - widthSpace_, curRowY_, volWidth_ + rowFontWidth_, rowFontHeight_, curCellColor_);
			painter.drawText(offset, curRowBaselineY_, "--");
			offset += volWidth_ +  rowFontWidth_;
			if (trackNum == curTrackNum_ && curCellNumInTrack_ == 3)
				painter.fillRect(offset - widthSpace_, curRowY_, effWidth_ + rowFontWidth_, rowFontHeight_, curCellColor_);
			painter.drawText(offset, curRowBaselineY_, "---");

			x += trackWidth_;
			break;
		}
		++trackNum;
	}

	int rowNum;
	int rowY, baseY;

	/* Previous rows */
	for (rowY = curRowY_ - rowFontHeight_, baseY = curRowBaselineY_ - rowFontHeight_, rowNum = curRowNum - 1;
		 rowY >= headerHeight_ - rowFontHeight_;
		 rowY -= rowFontHeight_, baseY -= rowFontHeight_, --rowNum) {
		// Fill row
		painter.fillRect(0, rowY, maxWidth, rowFontHeight_, (rowNum % mkCnt)? defRowColor_ : mkRowColor_);
		// Row number
		painter.setPen((rowNum % mkCnt)? defRowNumColor_ : mkRowNumColor_);
		painter.drawText(1, baseY, QString("%1").arg(rowNum, 2, 16, QChar('0')).toUpper());
		painter.setPen(defTextColor_);
		for (x = rowNumWidth_ + widthSpace_, trackNum = leftTrackNum_; x < maxWidth; ) {
			int offset = x;
			painter.drawText(offset, baseY, "---");
			offset += toneNameWidth_ +  rowFontWidth_;
			painter.drawText(offset, baseY, "--");
			offset += instWidth_ +  rowFontWidth_;
			painter.drawText(offset, baseY, "--");
			offset += volWidth_ +  rowFontWidth_;
			painter.drawText(offset, baseY, "---");

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
	for (rowY = curRowY_ + rowFontHeight_, baseY = curRowBaselineY_ + rowFontHeight_, rowNum = curRowNum + 1;
		 rowY <= geometry().height();
		 rowY += rowFontHeight_, baseY += rowFontHeight_, ++rowNum) {
		// Fill row
		painter.fillRect(0, rowY, maxWidth, rowFontHeight_, (rowNum % mkCnt)? defRowColor_ : mkRowColor_);
		// Row number
		painter.setPen((rowNum % mkCnt)? defRowNumColor_ : mkRowNumColor_);
		painter.drawText(1, baseY, QString("%1").arg(rowNum, 2, 16, QChar('0')).toUpper());
		painter.setPen(defTextColor_);
		for (x = rowNumWidth_ + widthSpace_, trackNum = leftTrackNum_; x < maxWidth; ) {
			int offset = x;
			painter.drawText(offset, baseY, "---");
			offset += toneNameWidth_ +  rowFontWidth_;
			painter.drawText(offset, baseY, "--");
			offset += instWidth_ +  rowFontWidth_;
			painter.drawText(offset, baseY, "--");
			offset += volWidth_ +  rowFontWidth_;
			painter.drawText(offset, baseY, "---");

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

void PatternEditorPanel::drawHeaders(int maxWidth)
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

void PatternEditorPanel::drawBorders(int maxWidth)
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

void PatternEditorPanel::drawShadow()
{
	QPainter painter(pixmap_.get());
	painter.fillRect(0, 0, geometry().width(), geometry().height(), QColor::fromRgb(0, 0, 0, 47));
}

int PatternEditorPanel::calculateTracksWidthWithRowNum(int begin, int end) const
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

void PatternEditorPanel::moveCursorToRight(int n)
{
	int oldTrackNum = curTrackNum_;

	curCellNumInTrack_ += n;
	if (n > 0) {
		for (bool flag = true; flag; ) {
			switch (modStyle_.trackAttribs[curTrackNum_].source) {
			case SoundSource::FM:
			case SoundSource::PSG:
				if (curCellNumInTrack_ < 4) {
					flag = false;
				}
				else if (curTrackNum_ == modStyle_.trackAttribs.size() - 1) {
					curCellNumInTrack_ = 3;
					flag = false;
				}
				else {
					curCellNumInTrack_ -= 4;
					++curTrackNum_;
				}
				break;
			}
		}
		while (calculateTracksWidthWithRowNum(leftTrackNum_, curTrackNum_) > geometry().width())
			++leftTrackNum_;
	}
	else {
		for (bool flag = true; flag; ) {
			if (curCellNumInTrack_ >= 0) {
				flag = false;
			}
			else if (!curTrackNum_) {
				curCellNumInTrack_ = 0;
				flag = false;
			}
			else {
				--curTrackNum_;
				switch (modStyle_.trackAttribs[curTrackNum_].source) {
				case SoundSource::FM:
				case SoundSource::PSG:
					curCellNumInTrack_ += 4;
					break;
				}
			}
		}
		if (curTrackNum_ < leftTrackNum_) leftTrackNum_ = curTrackNum_;
	}
	TracksWidthFromLeftToEnd_
			= calculateTracksWidthWithRowNum(leftTrackNum_, modStyle_.trackAttribs.size() - 1);

	if (!isIgnoreToSlider_)
		emit currentCellInRowChanged(calculateCellNumInRow(curTrackNum_, curCellNumInTrack_));

	if (!isIgnoreToOrder_ && curTrackNum_ != oldTrackNum)
		emit currentTrackChanged(curTrackNum_);	// Send to order list

	update();
}

int PatternEditorPanel::calculateCellNumInRow(int trackNum, int cellNumInTrack) const
{
	int i, cnt = 0;
	for (i = 0; i < trackNum; ++i) {
		switch (modStyle_.trackAttribs[i].source) {
		case SoundSource::FM:
		case SoundSource::PSG:
			cnt += 4;
			break;
		}
	}
	cnt += cellNumInTrack;

	return cnt;
}

void PatternEditorPanel::changeEditable()
{
	update();
}

int PatternEditorPanel::getFullColmunSize() const
{
	switch (modStyle_.trackAttribs.back().source) {
	case SoundSource::FM:
	case SoundSource::PSG:
		return calculateCellNumInRow(modStyle_.trackAttribs.size() - 1, 3);
	default: return 0;
	}
}

/********** Slots **********/
void PatternEditorPanel::setCurrentCellInRow(int num)
{
	Ui::EventGuard eg(isIgnoreToSlider_);

	if (int dif = num - calculateCellNumInRow(curTrackNum_, curCellNumInTrack_))
		moveCursorToRight(dif);
}

void PatternEditorPanel::setCurrentTrack(int num)
{
	Ui::EventGuard eg(isIgnoreToOrder_);

	int dif = calculateCellNumInRow(num, 0)
			- calculateCellNumInRow(curTrackNum_, curCellNumInTrack_);
	moveCursorToRight(dif);
}

/********** Events **********/
bool PatternEditorPanel::event(QEvent *event)
{
	switch (event->type()) {
	case QEvent::KeyPress:
		return keyPressed(dynamic_cast<QKeyEvent*>(event));
	case QEvent::HoverMove:
		return mouseHoverd(dynamic_cast<QHoverEvent*>(event));
	default:
		return QWidget::event(event);
	}
}

bool PatternEditorPanel::keyPressed(QKeyEvent *event)
{
	switch (event->key()) {
	case Qt::Key_Left:	moveCursorToRight(-1);	return true;
	case Qt::Key_Right:	moveCursorToRight(1);	return true;
	case Qt::Key_Up:	return true;
	case Qt::Key_Down:	return true;
	default: return false;
	}
}

void PatternEditorPanel::paintEvent(QPaintEvent *event)
{
	if (bt_ != nullptr) drawPattern(event->rect());
}

void PatternEditorPanel::resizeEvent(QResizeEvent *event)
{
	QWidget::resizeEvent(event);

	// Recalculate center row position
	curRowBaselineY_ = (geometry().height() - headerHeight_) / 2 + headerHeight_;
	curRowY_ = curRowBaselineY_ - (rowFontAscend_ + rowFontLeading_ / 2);

	initDisplay();
}

void PatternEditorPanel::mousePressEvent(QMouseEvent *event)
{
	setFocus();
}

bool PatternEditorPanel::mouseHoverd(QHoverEvent *event)
{
	QPoint pos = event->pos();
	int rowNum = 0;
	int colNum = 0;

	// Detect row
	if (pos.y() <= headerHeight_) {
		// Track header
		rowNum = -1;
	}
	else {
		int curRow = 32;	// Dummy

		int tmp = (geometry().height() - curRowY_) / rowFontHeight_;
		int num = curRow + tmp;
		int y = curRowY_ + rowFontHeight_ * tmp;
		for (; ; --num, y -= rowFontHeight_) {
			if (y <= pos.y()) break;
		}
		rowNum = num;
	}

	// Detect column
	if (pos.x() <= rowNumWidth_) {
		// Row number
		colNum = -1;
	}
	else {
		// TODO
	}

	return true;
}
