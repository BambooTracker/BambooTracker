#include "order_list.hpp"
#include <QPainter>
#include <QFontMetrics>
#include <QApplication>

OrderList::OrderList(QWidget *parent) : QWidget(parent)
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
	selTextColor_ = defTextColor_;
	selCellColor_ = QColor::fromRgb(100, 100, 200);
	rowNumColor_ = QColor::fromRgb(255, 200, 180);
	headerTextColor_ = QColor::fromRgb(240, 240, 200);
	headerRowColor_ = QColor::fromRgb(60, 60, 60);
	borderColor_ = QColor::fromRgb(120, 120, 120);


	initDisplay();
}

void OrderList::setCore(std::shared_ptr<BambooTracker> core)
{
	bt_ = core;
}

void OrderList::initDisplay()
{
	/* Pixmap */
	pixmap_ = std::make_unique<QPixmap>(geometry().size());
	pixmap_->fill(Qt::black);
}

void OrderList::drawList(const QRect &rect)
{
	drawRows();
	drawHeaders();
	drawBorders();

	QPainter painter(this);
	painter.drawPixmap(rect, *pixmap_.get());
}

void OrderList::drawRows()
{
	QPainter painter(pixmap_.get());
	painter.setFont(rowFont_);

	int curRowNum = 10;	// dummt set

	/* Current row */
	// Fill row
	painter.fillRect(0, curRowY_, geometry().width(), rowFontHeight_, curRowColor_);
	// Row number
	painter.setPen(rowNumColor_);
	painter.drawText(1, curRowBaselineY_, QString("%1").arg(curRowNum, 2, 16, QChar('0')).toUpper());
	// Step data
	painter.setPen(curTextColor_);
	for (int x = rowNumWidth_ + widthSpace_; x < geometry().width(); x += trackWidth_) {
		painter.drawText(x, curRowBaselineY_, QString(" %1").arg(0, 2, 16, QChar('0')).toUpper());
	}

	int rowNum;
	int rowY, baseY;

	/* Previous rows */
	for (rowY = curRowY_ - rowFontHeight_, baseY = curRowBaselineY_ - rowFontHeight_, rowNum = curRowNum - 1;
		 rowY >= headerHeight_ - rowFontHeight_;
		 rowY -= rowFontHeight_, baseY -= rowFontHeight_, --rowNum) {
		// Fill row
		painter.fillRect(0, rowY, geometry().width(), rowFontHeight_, defRowColor_);
		// Row number
		painter.setPen(rowNumColor_);
		painter.drawText(1, baseY, QString("%1").arg(rowNum, 2, 16, QChar('0')).toUpper());
		painter.setPen(defTextColor_);
		for (int x = rowNumWidth_ + widthSpace_; x < geometry().width(); x += trackWidth_) {
			painter.drawText(x, baseY, QString(" %1").arg(0, 2, 16, QChar('0')).toUpper());
		}
	}

	/* Next rows */
	for (rowY = curRowY_ + rowFontHeight_, baseY = curRowBaselineY_ + rowFontHeight_, rowNum = curRowNum + 1;
		 rowY <= geometry().height();
		 rowY += rowFontHeight_, baseY += rowFontHeight_, ++rowNum) {
		// Fill row
		painter.fillRect(0, rowY, geometry().width(), rowFontHeight_, defRowColor_);
		// Row number
		painter.setPen(rowNumColor_);
		painter.drawText(1, baseY, QString("%1").arg(rowNum, 2, 16, QChar('0')).toUpper());
		painter.setPen(defTextColor_);
		for (int x = rowNumWidth_ + widthSpace_; x < geometry().width(); x += trackWidth_) {
			painter.drawText(x, baseY, QString(" %1").arg(0, 2, 16, QChar('0')).toUpper());
		}
	}
}

void OrderList::drawHeaders()
{
	QPainter painter(pixmap_.get());
	painter.setFont(headerFont_);

	painter.fillRect(0, 0, geometry().width(), headerHeight_, headerRowColor_);
	painter.setPen(headerTextColor_);
	int x, num;
	for (x = rowNumWidth_ + widthSpace_, num = 1;
		 x < geometry().width();
		 x += trackWidth_, ++num) {
		painter.drawText(x, rowFontLeading_ + rowFontAscend_, " Ch" + QString::number(num));
	}
}

void OrderList::drawBorders()
{
	QPainter painter(pixmap_.get());

	painter.drawLine(0, headerHeight_, geometry().width(), headerHeight_);
	for (int x = rowNumWidth_; x <= geometry().width(); x += trackWidth_) {
		painter.drawLine(x, 0, x, geometry().height());
	}
}

/********** Events **********/
bool OrderList::event(QEvent *event)
{
	switch (event->type()) {
	default:
		return QWidget::event(event);
	}
}

void OrderList::paintEvent(QPaintEvent *event)
{
	if (bt_ != nullptr) drawList(event->rect());
}

void OrderList::resizeEvent(QResizeEvent *event)
{
	QWidget::resizeEvent(event);

	// Recalculate center row position
	curRowBaselineY_ = (geometry().height() - headerHeight_) / 2 + headerHeight_;
	curRowY_ = curRowBaselineY_ - (rowFontAscend_ + rowFontLeading_ / 2);

	initDisplay();
}
