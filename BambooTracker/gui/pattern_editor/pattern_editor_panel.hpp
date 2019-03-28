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
#include <memory>
#include <vector>
#include "bamboo_tracker.hpp"
#include "configuration.hpp"
#include "song.hpp"
#include "gui/pattern_editor/pattern_position.hpp"
#include "gui/color_palette.hpp"
#include "misc.hpp"

class PatternEditorPanel : public QWidget
{
	Q_OBJECT
public:
	explicit PatternEditorPanel(QWidget *parent = nullptr);
	~PatternEditorPanel() override;
	void setCore(std::shared_ptr<BambooTracker> core);
	void setCommandStack(std::weak_ptr<QUndoStack> stack);
	void setConfiguration(std::weak_ptr<Configuration> config);
	void setColorPallete(std::weak_ptr<ColorPalette> palette);

	void changeEditable();
	int getFullColmunSize() const;
	void updatePosition();

	void copySelectedCells();
	void cutSelectedCells();

	int getCurrentTrack() const;

public slots:
	void setCurrentCellInRow(int num);
	void setCurrentStep(int num);
	void setCurrentTrack(int num);
	void setCurrentOrder(int num);

	void onOrderListEdited();
	void onDefaultPatternSizeChanged();

	void setPatternHighlight1Count(int count);
	void setPatternHighlight2Count(int count);
	void setEditableStep(int n);

	void onSongLoaded();

	void onDeletePressed();
	void onPastePressed();
	void onPasteMixPressed();
	void onPasteOverwritePressed();
	/// 0: None
	/// 1: All
	/// 2: Row
	/// 3: Column
	/// 4: Pattern
	/// 5: Order
	void onSelectPressed(int type);
	void onTransposePressed(bool isOctave, bool isIncreased);
	void onToggleTrackPressed(int track);
	void onSoloTrackPressed(int track);
	void onUnmuteAllPressed();
	void onExpandPressed();
	void onShrinkPressed();
	void onInterpolatePressed();
	void onReversePressed();
	void onReplaceInstrumentPressed();

signals:
	void currentCellInRowChanged(int num);
	void currentStepChanged(int num, int max);
	void currentTrackChanged(int num);
	void currentOrderChanged(int num, int max);
	void effectColsCompanded(int num, int max);

	void selected(bool isSelected);
	void returnPressed();
	void instrumentEntered(int num);
	void effectEntered(QString text);

protected:
	virtual bool event(QEvent *event) override;
	bool keyPressed(QKeyEvent* event);
	bool keyReleased(QKeyEvent* event);
	virtual void paintEvent(QPaintEvent* event) override;
	virtual void resizeEvent(QResizeEvent* event) override;
	virtual void mousePressEvent(QMouseEvent* event) override;
	virtual void mouseMoveEvent(QMouseEvent* event) override;
	virtual void mouseReleaseEvent(QMouseEvent* event) override;
	virtual void mouseDoubleClickEvent(QMouseEvent* event) override;
	bool mouseHoverd(QHoverEvent* event);
	virtual void wheelEvent(QWheelEvent* event) override;
	virtual void leaveEvent(QEvent* event) override;

	// Midi
private:
	static void midiThreadReceivedEvent(double delay, const uint8_t *msg, size_t len, void *userData);
private slots:
	void midiKeyEvent(uchar status, uchar key, uchar velocity);

private:
	std::unique_ptr<QPixmap> pixmap_;
	std::shared_ptr<BambooTracker> bt_;
	std::weak_ptr<QUndoStack> comStack_;
	std::weak_ptr<Configuration> config_;
	std::weak_ptr<ColorPalette> palette_;

	QFont stepFont_, headerFont_;
	int stepFontWidth_, stepFontHeight_, stepFontAscend_, stepFontLeading_;

	int widthSpace_, widthSpaceDbl_;
	int stepNumWidth_;
	int baseTrackWidth_;
	int toneNameWidth_, instWidth_;
	int volWidth_;
	int effWidth_, effIDWidth_, effValWidth_;
	int TracksWidthFromLeftToEnd_;
	int hdMuteToggleWidth_, hdEffCompandButtonWidth_;
	int headerHeight_;
	int hdPlusY_, hdMinusY_;
	int curRowBaselineY_;
	int curRowY_;

	std::vector<int> rightEffn_;

	int leftTrackNum_;
	SongStyle songStyle_;

	int curSongNum_;
	PatternPosition curPos_, hovPos_, editPos_;
	PatternPosition mousePressPos_, mouseReleasePos_;
	PatternPosition selLeftAbovePos_, selRightBelowPos_;
	PatternPosition shiftPressedPos_;
	PatternPosition doubleClickPos_;

	bool isIgnoreToSlider_, isIgnoreToOrder_;

	bool isPressedPlus_, isPressedMinus_;

	int entryCnt_;

	int selectAllState_;
	bool isMuteElse_;

	int hl1Cnt_, hl2Cnt_;
	int editableStepCnt_;

	// Meta methods
	int midiKeyEventMethod_;

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
	int calculateColNumInRow(int trackNum, int colNumInTrack, bool isExpanded = false) const;
	int calculateColumnDistance(int beginTrack, int beginColumn, int endTrack, int endColumn, bool isExpanded = false) const;
	int calculateStepDistance(int beginOrder, int beginStep, int endOrder, int endStep) const;
	QPoint calculateCurrentCursorPosition() const;

	void moveCursorToRight(int n);
	void moveCursorToDown(int n);

	void expandEffect(int trackNum);
	void shrinkEffect(int trackNum);

	bool enterToneData(QKeyEvent* event);
	void setStepKeyOn(Note note, int octave);
	bool enterInstrumentData(int key);
	void setStepInstrument(int num);
	bool enterVolumeData(int key);
	void setStepVolume(int volume);
	bool enterEffectID(int key);
	void setStepEffectID(QString str);
	bool enterEffectValue(int key);
	void setStepEffectValue(int value);

	void insertStep();
	void deletePreviousStep();

	void eraseSelectedCells();
	void pasteCopiedCells(const PatternPosition& startPos);
	void pasteMixCopiedCells(const PatternPosition& startPos);
	void pasteOverwriteCopiedCells(const PatternPosition& startPos);
	std::vector<std::vector<std::string> > instantiateCellsFromString(QString str, int& startCol);

	void increaseNoteKey(const PatternPosition& startPos, const PatternPosition& endPos);
	void decreaseNoteKey(const PatternPosition& startPos, const PatternPosition& endPos);
	void increaseNoteOctave(const PatternPosition& startPos, const PatternPosition& endPos);
	void decreaseNoteOctave(const PatternPosition& startPos, const PatternPosition& endPos);

	void setSelectedRectangle(const PatternPosition& start, const PatternPosition& end);
	bool isSelectedCell(int trackNum, int colNum, int orderNum, int stepNum);

	void showPatternContextMenu(const PatternPosition& pos, const QPoint& point);

	// Layout decypherer
	JamKey getJamKeyFromLayoutMapping(Qt::Key key);
};

#endif // PATTERN_EDITOR_PANEL_HPP
