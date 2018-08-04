#ifndef PATTERN_EDITOR_HPP
#define PATTERN_EDITOR_HPP

#include <QWidget>
#include <QPixmap>
#include <QFont>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QHoverEvent>
#include <QRect>
#include <QColor>
#include <memory>
#include "bamboo_tracker.hpp"

class PatternEditor : public QWidget
{
	Q_OBJECT
public:
	explicit PatternEditor(QWidget *parent = nullptr);
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
	int toneNameWidth_, instWidth_, volWidth_, effWidth_;
	int headerHeight_;
	int curRowBaselineY_;
	int curRowY_;

	QColor defTextColor_, defRowColor_, mkRowColor_;
	QColor curTextColor_, curRowColor_, curCellColor_;
	QColor selTextColor_, selCellColor_;
	QColor defRowNumColor_, mkRowNumColor_;
	QColor headerTextColor_, headerBackColor_;
	QColor borderColor_;

	void initDisplay();
	void drawPattern(const QRect& rect);
	void drawRows();
	void drawHeaders();
	void drawBorders();

	void mouseHoverd(QHoverEvent* event);
};

#endif // PATTERN_EDITOR_HPP
