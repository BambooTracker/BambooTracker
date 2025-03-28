/*
 * SPDX-FileCopyrightText: 2018 Rerrah
 * SPDX-License-Identifier: MIT
 */

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
#include <QEvent>
#include <QRect>
#include <QColor>
#include <QUndoStack>
#include <QString>
#include <QPoint>
#include <QShortcut>
#include <memory>
#include <vector>
#include <atomic>
#include "bamboo_tracker.hpp"
#include "configuration.hpp"
#include "song.hpp"
#include "vector_2d.hpp"
#include "gui/pattern_editor/pattern_position.hpp"
#include "gui/color_palette.hpp"

class PatternEditorPanel : public QWidget
{
	Q_OBJECT
public:
	explicit PatternEditorPanel(QWidget *parent = nullptr);
	~PatternEditorPanel() override;
	void setCore(std::shared_ptr<BambooTracker> core);
	bool isReadyCore() const;
	void setCommandStack(std::weak_ptr<QUndoStack> stack);
	void setConfiguration(std::shared_ptr<Configuration> config);
	void setColorPallete(std::shared_ptr<ColorPalette> palette);

	void changeEditable();
	int getFullColmunSize() const;
	void updatePositionByStepUpdate(bool isFirstUpdate, bool forceJump = false, bool trackChanged = false);
	int getScrollableCountByTrack() const;

	void changeMarker();

	void copySelectedCells();
	void cutSelectedCells();

	void redrawByPatternChanged(bool patternSizeChanged = false);
	void redrawByFocusChanged();
	void redrawByHoverChanged();
	void redrawByMaskChanged();
	void redrawPatterns();
	void redrawAll();

	void resetEntryCount();

	void waitPaintFinish();

	QFont getHeaderFont() const;
	QFont getRowsFont() const;
	QFont getDefaultHeaderFont() const;
	QFont getDefaultRowsFont() const;
	void setFonts(const QFont& headerFont, const QFont& rowsFont);

	void setVisibleTracks(std::vector<int> tracks);
	std::vector<int> getVisibleTracks() const;

public slots:
	void onHScrollBarChanged(int num);
	void onVScrollBarChanged(int num);
	void onOrderListCurrentTrackChanged(int idx);
	void onOrderListCurrentOrderChanged(int num);

	void onOrderListEdited();
	void onDefaultPatternSizeChanged();
	void onShortcutUpdated();

	void setPatternHighlight1Count(int count);
	void setPatternHighlight2Count(int count);
	void setEditableStep(int n);

	void onSongLoaded();

	void onDeletePressed();
	void onPastePressed();
	void onPasteMixPressed();
	void onPasteOverwritePressed();
	void onPasteInsertPressed();
	/// 0: None
	/// 1: All
	/// 2: Row
	/// 3: Column
	/// 4: Pattern
	/// 5: Order
	void onSelectPressed(int type);
	void onNoteTransposePressed(int semitone);
	void onToggleTrackPressed();
	void onSoloTrackPressed();
	void onUnmuteAllPressed();
	void onExpandPressed();
	void onShrinkPressed();
	void onInterpolatePressed();
	void onReversePressed();
	void onReplaceInstrumentPressed();
	void onExpandEffectColumnPressed(int trackVisIdx);
	void onShrinkEffectColumnPressed(int trackVisIdx);
	void onFollowModeChanged();
	void onChangeValuesPressed(int value);
	void onPlayStepPressed();

signals:
	void hScrollBarChangeRequested(int num);
	void vScrollBarChangeRequested(int num, int max);
	void currentTrackChanged(int num);
	void currentOrderChanged(int num, int max);
	void effectColsCompanded(int num, int max);

	void selected(bool isSelected);
	void instrumentEntered(int num);
	void volumeEntered(int volume);
	void effectEntered(QString text);

protected:
	bool event(QEvent *event) override;
	bool keyPressed(QKeyEvent* event);
	bool keyReleased(QKeyEvent* event);
	void paintEvent(QPaintEvent* event) override;
	void resizeEvent(QResizeEvent* event) override;
	void mousePressEvent(QMouseEvent* event) override;
	void mouseMoveEvent(QMouseEvent* event) override;
	void mouseReleaseEvent(QMouseEvent* event) override;
	void mouseDoubleClickEvent(QMouseEvent* event) override;
	bool mouseHoverd(QHoverEvent* event);
	void wheelEvent(QWheelEvent* event) override;
	void leaveEvent(QEvent*) override;

	// Midi
private:
	static void midiThreadReceivedEvent(double delay, const uint8_t *msg, size_t len, void *userData);
private slots:
	void midiKeyEvent(uchar status, uchar key, uchar velocity);

private:
	QPixmap completePixmap_, backPixmap_, textPixmap_, forePixmap_, headerPixmap_;
	std::shared_ptr<BambooTracker> bt_;
	std::weak_ptr<QUndoStack> comStack_;
	std::shared_ptr<Configuration> config_;
	std::shared_ptr<ColorPalette> palette_;

	QFont stepFont_, headerFont_;
	QFont stepFontDef_, headerFontDef_;
	int stepFontWidth_, stepFontHeight_, stepFontAscent_, stepFontLeading_;
	int headerFontAscent_;

	int widthSpace_, widthSpaceDbl_;
	int stepNumWidthCnt_, stepNumWidth_, stepNumBase_;
	int baseTrackWidth_;
	int toneNameWidth_, instWidth_;
	int volWidth_;
	int effWidth_, effIDWidth_, effValWidth_;
	int tracksWidthFromLeftToEnd_;
	int hdMuteToggleWidth_, hdEffCompandButtonWidth_;
	int headerHeight_;
	int hdPlusY_, hdMinusY_;
	int curRowBaselineY_;
	int curRowY_;

	std::vector<int> visTracks_, rightEffn_;

	int leftTrackVisIdx_;
	SongStyle songStyle_;

	int curSongNum_;
	PatternPosition curPos_, hovPos_;
	PatternPosition mousePressPos_, mouseReleasePos_;
	PatternPosition selLeftAbovePos_, selRightBelowPos_;
	PatternPosition shiftPressedPos_;
	PatternPosition doubleClickPos_;
	PatternPosition markerPos_;

	bool isIgnoreToSlider_, isIgnoreToOrder_;

	bool isPressedPlus_, isPressedMinus_;

	int entryCnt_;

	int selectAllState_;
	bool isMuteElse_;

	int hl1Cnt_, hl2Cnt_;
	int editableStepCnt_;

	int viewedRowCnt_;
	int viewedRegionHeight_;
	int viewedRowsHeight_, viewedRowOffset_, viewedCenterY_, viewedCenterBaseY_;
	PatternPosition viewedFirstPos_, viewedCenterPos_, viewedLastPos_;

	bool backChanged_, textChanged_, foreChanged_, headerChanged_, focusChanged_, followModeChanged_;
	bool hasFocussedBefore_;
	int stepDownCount_;

	std::atomic_bool repaintable_;	// Recurrensive repaint guard
	std::atomic_int repaintingCnt_;
	std::atomic_bool isInitedFirstMod_;

	// Shortcuts
	QShortcut upSc_, upWSSc_, dnSc_, dnWSSc_, pgUpSc_, pgUpWSSc_, pgDnSc_, pgDnWSSc_;
	QShortcut homeSc_, homeWSSc_, endSc_, endWSSc_, hlUpSc_, hlUpWSSc_, hlDnSc_, hlDnWSSc_;
	QShortcut ltSc_, ltWSSc_, rtSc_, rtWSSc_;
	QShortcut keyOffSc_, keyCutSc_, echoBufSc_, stepMvUpSc_, stepMvDnSc_, expandColSc_, shrinkColSc_;

	// Meta methods
	int midiKeyEventMethod_;

	void funcResize();
	void updateSizes();
	void initDisplay();
	void drawPattern(const QRect& rect);
	void drawRows(int maxWidth);
	void quickDrawRows(int maxWidth);
	/// Return:
	///		track width
	int drawStep(QPainter& forePainter, QPainter& textPainter, QPainter& backPainter, int trackVisIdx, int orderNum, int stepNum, int x, int baseY, int rowY);
	void drawHeaders(int maxWidth);
	void drawBorders(int maxWidth);
	void drawShadow();

	// NOTE: Calculated by visible tracks
	int calculateTracksWidthWithRowNum(int beginIdx, int endIdx) const;
	int calculateColNumInRow(int trackVisIdx, int colNumInTrack, bool isExpanded = false) const;
	int calculateColumnDistance(int beginTrackIdx, int beginColumn, int endTrackIdx, int endColumn, bool isExpanded = false) const;
	int calculateStepDistance(int beginOrder, int beginStep, int endOrder, int endStep) const;
	PatternPosition calculatePositionFrom(int order, int step, int by) const;
	QPoint calculateCurrentCursorPosition() const;

	inline void updateTracksWidthFromLeftToEnd()
	{
		tracksWidthFromLeftToEnd_ = calculateTracksWidthWithRowNum(
										leftTrackVisIdx_, static_cast<int>(visTracks_.size()) - 1);
	}

	void moveCursorToRight(int n);
	void moveViewToRight(int n);
	void moveCursorToDown(int n);

	inline void checkSelectionByCursorMove(bool isShift) {
		if (isShift) setSelectedRectangle(shiftPressedPos_, curPos_);
		else onSelectPressed(0);
	}

	bool enterToneData(QKeyEvent* event);
	void setStepKeyOn(const Note& note);
	bool enterInstrumentData(int key);
	void setStepInstrument(int num);
	bool enterVolumeData(int key);
	void setStepVolume(int volume);
	bool enterEffectID(int key);
	void setStepEffectID(QString str);
	bool enterEffectValue(int key);
	void setStepEffectValue(int value);

	inline int updateEntryCount()
	{
		entryCnt_ = (entryCnt_ + 1) % 2;
		return entryCnt_;
	}

	void insertStep();
	void deletePreviousStep();

	void eraseSelectedCells();
	void pasteCopiedCells(const PatternPosition& cursorPos);
	void pasteMixCopiedCells(const PatternPosition& cursorPos);
	void pasteOverwriteCopiedCells(const PatternPosition& cursorPos);
	void pasteInsertCopiedCells(const PatternPosition& cursorPos);
	PatternPosition getPasteLeftAbovePosition(
			int pasteCol, const PatternPosition& cursorPos, size_t cellW) const;
	Vector2d<std::string> makeCopiedCellsForPasteFull(const PatternPosition& laPos, const Vector2d<std::string>& cells);

	void transposeNote(const PatternPosition& startPos, const PatternPosition& endPos, int semitone);
	void changeValuesInPattern(const PatternPosition& startPos, const PatternPosition& endPos, int value);

	void toggleTrack(int trackIdx);
	void soloTrack(int trackIdx);

	void setSelectedRectangle(const PatternPosition& start, const PatternPosition& end);
	bool isSelectedCell(int trackVisIdx, int colNum, int orderNum, int stepNum);

	void showPatternContextMenu(const PatternPosition& pos, const QPoint& point);
};

#endif // PATTERN_EDITOR_PANEL_HPP
