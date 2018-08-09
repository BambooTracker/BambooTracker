#ifndef PATTERN_EDITOR_PANEL_HPP
#define PATTERN_EDITOR_PANEL_HPP

#include <QWidget>
#include <QPixmap>
#include <QFont>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QMouseEvent>
#include <QHoverEvent>
#include <QRect>
#include <QColor>
#include <memory>
#include "bamboo_tracker.hpp"
#include "module.hpp"

class PatternEditorPanel : public QWidget
{
	Q_OBJECT
public:
	explicit PatternEditorPanel(QWidget *parent = nullptr);
	void setCore(std::shared_ptr<BambooTracker> core);

	void changeEditable();
	int getFullColmunSize() const;

public slots:
	void setCurrentCellInRow(int num);
	void setCurrentTrack(int num);

signals:
	void currentCellInRowChanged(int num);
	void currentTrackChanged(int num);

protected:
	virtual bool event(QEvent *event) override;
	bool keyPressed(QKeyEvent* event);
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
	int toneNameWidth_, instWidth_, volWidth_, effWidth_;
	int TracksWidthFromLeftToEnd_;
	int headerHeight_;
	int curRowBaselineY_;
	int curRowY_;

	QColor defTextColor_, defRowColor_, mkRowColor_;
	QColor curTextColor_, curRowColor_, curRowColorEditable_, curCellColor_;
	QColor selTextColor_, selCellColor_;
	QColor defRowNumColor_, mkRowNumColor_;
	QColor headerTextColor_, headerRowColor_;
	QColor borderColor_;

	int leftTrackNum_;
	ModuleStyle modStyle_;

	int curTrackNum_, curCellNumInTrack_, curRowNum_;

	bool isIgnoreToSlider_, isIgnoreToOrder_;

	void initDisplay();
	void drawPattern(const QRect& rect);
	void drawRows(int maxWidth);
	void drawHeaders(int maxWidth);
	void drawBorders(int maxWidth);
	void drawShadow();

	int calculateTracksWidthWithRowNum(int begin, int end) const;
	int calculateCellNumInRow(int trackNum, int cellNumInTrack) const;

	void moveCursorToRight(int n);
};

#endif // PATTERN_EDITOR_PANEL_HPP
