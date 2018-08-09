#ifndef ORDER_LIST_PANEL_HPP
#define ORDER_LIST_PANEL_HPP

#include <QWidget>
#include <QPixmap>
#include <QFont>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QHoverEvent>
#include <QRect>
#include <QColor>
#include <memory>
#include "bamboo_tracker.hpp"
#include "module.hpp"

class OrderListPanel : public QWidget
{
	Q_OBJECT
public:
	explicit OrderListPanel(QWidget *parent = nullptr);
	void setCore(std::shared_ptr<BambooTracker> core);

	void changeEditable();

public slots:
	void setCurrentTrackForSlider(int num);
	void setCurrentTrack(int num);

signals:
	void currentTrackChangedForSlider(int num);
	void currentTrackChanged(int num);

protected:
	virtual bool event(QEvent *event) override;
	bool KeyPressed(QKeyEvent* event);
	virtual void paintEvent(QPaintEvent* event) override;
	virtual void resizeEvent(QResizeEvent* event) override;
	virtual void mousePressEvent(QMouseEvent* event) override;
	bool mouseHoverd(QHoverEvent* event);

private:
	std::unique_ptr<QPixmap> pixmap_;
	std::shared_ptr<BambooTracker> bt_;

	QFont rowFont_, headerFont_;
	int rowFontWidth_, rowFontHeight_, rowFontAscend_, rowFontLeading_;

	int widthSpace_;
	int rowNumWidth_;
	int trackWidth_;
	int columnsWidthFromLeftToEnd_;
	int headerHeight_;
	int curRowBaselineY_;
	int curRowY_;

	QColor defTextColor_, defRowColor_;
	QColor curTextColor_, curRowColor_, curRowColorEditable_, curCellColor_;
	QColor selTextColor_, selCellColor_;
	QColor hovCellColor_;
	QColor rowNumColor_;
	QColor headerTextColor_, headerRowColor_;
	QColor borderColor_;

	int leftTrackNum_;
	ModuleStyle modStyle_;

	int curTrackNum_, curRowNum_;
	int hovTrackNum_, hovRowNum_;

	bool isIgnoreToSlider_, isIgnoreToPattern_;

	/**********/
	int songNum = 0;	// dummy set
	/**********/

	void initDisplay();

	void drawList(const QRect& rect);
	void drawRows(int maxWidth);
	void drawHeaders(int maxWidth);
	void drawBorders(int maxWidth);
	void drawShadow();

	int calculateColumnsWidthWithRowNum(int begin, int end) const;

	void moveCursorToRight(int n);
	void MoveCursorToDown(int n);
};

#endif // ORDER_LIST_PANEL_HPP
