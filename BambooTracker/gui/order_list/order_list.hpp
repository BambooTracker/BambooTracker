#ifndef ORDER_LIST_HPP
#define ORDER_LIST_HPP

#include <QWidget>
#include <QPixmap>
#include <QFont>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QRect>
#include <QColor>
#include <memory>
#include "bamboo_tracker.hpp"

class OrderList : public QWidget
{
	Q_OBJECT
public:
	explicit OrderList(QWidget *parent = nullptr);
	void setCore(std::shared_ptr<BambooTracker> core);

protected:
	virtual bool event(QEvent *event) override;
	virtual void paintEvent(QPaintEvent* event) override;
	virtual void resizeEvent(QResizeEvent* event) override;

private:
	std::unique_ptr<QPixmap> pixmap_;
	std::shared_ptr<BambooTracker> bt_;

	QFont rowFont_, headerFont_;
	int rowFontWidth_, rowFontHeight_, rowFontAscend_, rowFontLeading_;

	int widthSpace_;
	int rowNumWidth_;
	int trackWidth_;
	int headerHeight_;
	int curRowBaselineY_;
	int curRowY_;

	QColor defTextColor_, defRowColor_;
	QColor curTextColor_, curRowColor_;
	QColor selTextColor_, selCellColor_;
	QColor rowNumColor_;
	QColor headerTextColor_, headerRowColor_;
	QColor borderColor_;

	void initDisplay();
	void drawList(const QRect& rect);
	void drawRows();
	void drawHeaders();
	void drawBorders();
};

#endif // ORDER_LIST_HPP
