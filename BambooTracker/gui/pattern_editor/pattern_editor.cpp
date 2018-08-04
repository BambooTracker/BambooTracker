#include "pattern_editor.hpp"
#include <QPainter>
#include <QFontMetrics>
#include <QPoint>
#include <QApplication>

#include <QDebug>

PatternEditor::PatternEditor(QWidget *parent) : QWidget(parent)
{	
	/* Font */
	headerFont_ = QApplication::font();
	headerFont_.setPointSize(11);
	rowFont_ = QFont("Monospace", 11);
	rowFont_.setStyleHint(QFont::TypeWriter);
	rowFont_.setStyleStrategy(QFont::ForceIntegerMetrics);
	// Check font size
	QFontMetrics metrics(rowFont_);
	rowFontWidth_ = metrics.width('0');
	rowFontAscend_ = metrics.ascent();
	rowFontHeight_ = metrics.height();
	rowFontLeading_ = metrics.leading();

	/* Color */
	defTextColor_ = QColor::fromRgb(180, 180, 180);
	defRowColor_ = QColor::fromRgb(0, 0, 40);
	mkRowColor_ = QColor::fromRgb(40, 40, 80);
	curTextColor_ = QColor::fromRgb(255, 255, 255);
	curRowColor_ = QColor::fromRgb(110, 90, 140);
	curCellColor_ = QColor::fromRgb(130, 110, 160);
	selTextColor_ = defTextColor_;
	selCellColor_ = QColor::fromRgb(100, 100, 200);
	defRowNumColor_ = QColor::fromRgb(255, 200, 180);
	mkRowNumColor_ = QColor::fromRgb(255, 140, 160);
	headerTextColor_ = QColor::fromRgb(240, 240, 200);
	headerBackColor_ = QColor::fromRgb(60, 60, 60);
	borderColor_ = QColor::fromRgb(120, 120, 120);

	/* Width & height */
	widthSpace_ = rowFontWidth_ / 2;
	rowNumWidth_ = rowFontWidth_ * 2 + widthSpace_;
	toneNameWidth_ = rowFontWidth_ * 3;
	instWidth_ = rowFontWidth_ * 2;
	volWidth_ = rowFontWidth_ * 2;
	effWidth_ = rowFontWidth_ * 3;
	trackWidth_ = toneNameWidth_ + instWidth_ + volWidth_ + effWidth_ + rowFontWidth_ * 4;
	headerHeight_ = rowFontHeight_ * 2;


	initDisplay();

	setAttribute(Qt::WA_Hover);
}

void PatternEditor::initDisplay()
{
	/* Pixmap */
	pixmap_ = std::make_unique<QPixmap>(geometry().size());
	pixmap_->fill(Qt::transparent);
}

void PatternEditor::setCore(std::shared_ptr<BambooTracker> core)
{
	bt_ = core;
}

void PatternEditor::drawPattern(const QRect &rect)
{
	drawRows();
	drawHeaders();
	drawBorders();

	QPainter painter(this);
	painter.drawPixmap(rect, *pixmap_.get());
}

void PatternEditor::drawRows()
{
	QPainter painter(pixmap_.get());
	painter.setFont(rowFont_);

	int curRowNum = 32;	// dummy set
	int mkCnt = 6;

	/* Current row */
	// Fill row
	painter.fillRect(0, curRowY_, geometry().width(), rowFontHeight_, curRowColor_);
	// Row number
	painter.setPen((curRowNum % mkCnt)? defRowNumColor_ : mkRowNumColor_);
	painter.drawText(1, curRowBaselineY_, QString("%1").arg(curRowNum, 2, 16, QChar('0')).toUpper());
	// Step data
	painter.setPen(curTextColor_);
	for (int x = rowNumWidth_ + widthSpace_; x < geometry().width(); x += trackWidth_) {
		int offset = x;
		painter.drawText(offset, curRowBaselineY_, "---");
		offset += toneNameWidth_ +  rowFontWidth_;
		painter.drawText(offset, curRowBaselineY_, "--");
		offset += instWidth_ +  rowFontWidth_;
		painter.drawText(offset, curRowBaselineY_, "--");
		offset += volWidth_ +  rowFontWidth_;
		painter.drawText(offset, curRowBaselineY_, "---");
	}

	int rowNum;
	int rowY, baseY;

	/* Previous rows */
	for (rowY = curRowY_ - rowFontHeight_, baseY = curRowBaselineY_ - rowFontHeight_, rowNum = curRowNum - 1;
		 rowY >= headerHeight_ - rowFontHeight_;
		 rowY -= rowFontHeight_, baseY -= rowFontHeight_, --rowNum) {
		// Fill row
		painter.fillRect(0, rowY, geometry().width(), rowFontHeight_,
						 (rowNum % mkCnt)? defRowColor_ : mkRowColor_);
		// Row number
		painter.setPen((rowNum % mkCnt)? defRowNumColor_ : mkRowNumColor_);
		painter.drawText(1, baseY, QString("%1").arg(rowNum, 2, 16, QChar('0')).toUpper());
		painter.setPen(defTextColor_);
		for (int x = rowNumWidth_ + widthSpace_; x < geometry().width(); x += trackWidth_) {
			int offset = x;
			painter.drawText(offset, baseY, "---");
			offset += toneNameWidth_ +  rowFontWidth_;
			painter.drawText(offset, baseY, "--");
			offset += instWidth_ +  rowFontWidth_;
			painter.drawText(offset, baseY, "--");
			offset += volWidth_ +  rowFontWidth_;
			painter.drawText(offset, baseY, "---");
		}
	}

	/* Next rows */
	for (rowY = curRowY_ + rowFontHeight_, baseY = curRowBaselineY_ + rowFontHeight_, rowNum = curRowNum + 1;
		 rowY <= geometry().height();
		 rowY += rowFontHeight_, baseY += rowFontHeight_, ++rowNum) {
		// Fill row
		painter.fillRect(0, rowY, geometry().width(), rowFontHeight_,
						 (rowNum % mkCnt)? defRowColor_ : mkRowColor_);
		// Row number
		painter.setPen((rowNum % mkCnt)? defRowNumColor_ : mkRowNumColor_);
		painter.drawText(1, baseY, QString("%1").arg(rowNum, 2, 16, QChar('0')).toUpper());
		painter.setPen(defTextColor_);
		for (int x = rowNumWidth_ + widthSpace_; x < geometry().width(); x += trackWidth_) {
			int offset = x;
			painter.drawText(offset, baseY, "---");
			offset += toneNameWidth_ +  rowFontWidth_;
			painter.drawText(offset, baseY, "--");
			offset += instWidth_ +  rowFontWidth_;
			painter.drawText(offset, baseY, "--");
			offset += volWidth_ +  rowFontWidth_;
			painter.drawText(offset, baseY, "---");
		}
	}
}

void PatternEditor::drawHeaders()
{
	QPainter painter(pixmap_.get());
	painter.setFont(headerFont_);

	painter.fillRect(0, 0, geometry().width(), headerHeight_, headerBackColor_);
	painter.setPen(headerTextColor_);
	int x, num;
	for (x = rowNumWidth_ + widthSpace_, num = 1;
		 x < geometry().width();
		 x += trackWidth_, ++num) {
		painter.drawText(x, rowFontLeading_ + rowFontAscend_, " Channel " + QString::number(num));
	}
}

void PatternEditor::drawBorders()
{
	QPainter painter(pixmap_.get());

	painter.drawLine(0, headerHeight_, geometry().width(), headerHeight_);
	for (int x = rowNumWidth_; x <= geometry().width(); x += trackWidth_) {
		painter.drawLine(x, 0, x, geometry().height());
	}
}

/********** Events **********/
bool PatternEditor::event(QEvent *event)
{
	switch (event->type()) {
	case QEvent::HoverMove:
		mouseHoverd(dynamic_cast<QHoverEvent*>(event));
		return true;
	default:
		return QWidget::event(event);
	}
}

void PatternEditor::paintEvent(QPaintEvent *event)
{
	if (bt_ != nullptr) drawPattern(event->rect());
}

void PatternEditor::resizeEvent(QResizeEvent *event)
{
	QWidget::resizeEvent(event);

	// Recalculate center row position
	curRowBaselineY_ = (geometry().height() - headerHeight_) / 2 + headerHeight_;
	curRowY_ = curRowBaselineY_ - (rowFontAscend_ + rowFontLeading_ / 2);

	initDisplay();
}

void PatternEditor::mouseHoverd(QHoverEvent *event)
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
}
