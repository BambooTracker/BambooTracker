#ifndef PATTERN_EDITOR_PANEL_HPP
#define PATTERN_EDITOR_PANEL_HPP

#include <QWidget>
#include <QPixmap>
#include <QFont>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QMouseEvent>
#include <QHoverEvent>
#include <QWheelEvent>
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
	void updatePosition();

public slots:
	void setCurrentCellInRow(int num);
	void setCurrentStep(int num);
	void setCurrentTrack(int num);
	void setCurrentOrder(int num);

signals:
	void currentCellInRowChanged(int num);
	void currentStepChanged(int num, int max);
	void currentTrackChanged(int num);
	void currentOrderChanged(int num);

protected:
	virtual bool event(QEvent *event) override;
	bool keyPressed(QKeyEvent* event);
	virtual void paintEvent(QPaintEvent* event) override;
	virtual void resizeEvent(QResizeEvent* event) override;
	virtual void mousePressEvent(QMouseEvent* event) override;
	bool mouseHoverd(QHoverEvent* event);
	virtual void wheelEvent(QWheelEvent* event) override;

private:
	std::unique_ptr<QPixmap> pixmap_;
	std::shared_ptr<BambooTracker> bt_;

	QFont stepFont_, headerFont_;
	int stepFontWidth_, stepFontHeight_, stepFontAscend_, stepFontLeading_;

	int widthSpace_;
	int stepNumWidth_;
	int trackWidth_;
	int toneNameWidth_, instWidth_, volWidth_, effWidth_;
	int TracksWidthFromLeftToEnd_;
	int headerHeight_;
	int curRowBaselineY_;
	int curRowY_;

	QColor defTextColor_, defRowColor_, mkRowColor_;
	QColor curTextColor_, curRowColor_, curRowColorEditable_, curCellColor_;
	QColor selTextColor_, selCellColor_;
	QColor hovCellColor_;
	QColor defStepNumColor_, mkStepNumColor_;
	QColor toneColor_, instColor_, volColor_, effColor_;
	QColor headerTextColor_, headerRowColor_;
	QColor patternMaskColor_;
	QColor borderColor_;

	int leftTrackNum_;
	ModuleStyle modStyle_;

	int curSongNum_;
	int curTrackNum_, curCellNumInTrack_, curOrderNum_, curStepNum_;
	int hovTrackNum_, hovCellNumInTrack_, hovOrderNum_, hovStepNum_;

	bool isIgnoreToSlider_, isIgnoreToOrder_;

	void initDisplay();
	void drawPattern(const QRect& rect);
	void drawRows(int maxWidth);
	/// Return:
	///		track width
	int drawStep(QPainter& painter, int trackNum, int orderNum, int stepNum, int x, int baseY, int rowY);
	void drawHeaders(int maxWidth);
	void drawBorders(int maxWidth);
	void drawShadow();

	int calculateTracksWidthWithRowNum(int begin, int end) const;
	int calculateCellNumInRow(int trackNum, int cellNumInTrack) const;
	int calculateStepDistance(int beginOrder, int beginStep, int endOrder, int endStep) const;

	void moveCursorToRight(int n);
	void moveCursorToDown(int n);
};

#endif // PATTERN_EDITOR_PANEL_HPP
