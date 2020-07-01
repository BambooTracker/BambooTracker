#include "pattern_editor.hpp"
#include "ui_pattern_editor.h"

PatternEditor::PatternEditor(QWidget *parent) :
	QFrame(parent),
	ui(new Ui::PatternEditor),
	freezed_(false),
	songLoaded_(false),
	hScrollCellMove_(true)
{
	ui->setupUi(this);

	installEventFilter(this);
	ui->panel->installEventFilter(this);
	ui->verticalScrollBar->installEventFilter(this);

	ui->panel->setFocus();
	QObject::connect(ui->panel, &PatternEditorPanel::hScrollBarChangeRequested,
					 ui->horizontalScrollBar, &QScrollBar::setValue);
	QObject::connect(ui->panel, &PatternEditorPanel::vScrollBarChangeRequested,
					 this, [&](int num, int max) {
		if (ui->verticalScrollBar->maximum() < num) {
			ui->verticalScrollBar->setMaximum(max);
			ui->verticalScrollBar->setValue(num);
		}
		else {
			ui->verticalScrollBar->setValue(num);
			ui->verticalScrollBar->setMaximum(max);
		}
	});
	QObject::connect(ui->panel, &PatternEditorPanel::currentTrackChanged,
					 this, [&](int idx) { emit currentTrackChanged(idx); });
	QObject::connect(ui->panel, &PatternEditorPanel::currentOrderChanged,
					 this, [&](int num, int max) { emit currentOrderChanged(num, max); });

	QObject::connect(ui->panel, &PatternEditorPanel::effectColsCompanded,
					 this, [&](int num, int max) {
		if (ui->horizontalScrollBar->maximum() < num) {
			ui->horizontalScrollBar->setMaximum(max);
			ui->horizontalScrollBar->setValue(num);
		}
		else {
			ui->horizontalScrollBar->setValue(num);
			ui->horizontalScrollBar->setMaximum(max);
		}
	});
	QObject::connect(ui->panel, &PatternEditorPanel::selected,
					 this, [&](bool isSelected) { emit selected(isSelected); });
	QObject::connect(ui->panel, &PatternEditorPanel::instrumentEntered,
					 this, [&](int num) { emit instrumentEntered(num); });
	QObject::connect(ui->panel, &PatternEditorPanel::volumeEntered,
					 this, [&](int volume) { emit volumeEntered(volume); });
	QObject::connect(ui->panel, &PatternEditorPanel::effectEntered,
					 this, [&](QString text) { emit effectEntered(text); });

	auto focusSlot = [&] { ui->panel->setFocus(); };

	QObject::connect(ui->horizontalScrollBar, &QScrollBar::valueChanged,
					 ui->panel, &PatternEditorPanel::onHScrollBarChanged);
	QObject::connect(ui->horizontalScrollBar, &QScrollBar::sliderPressed, this, focusSlot);

	QObject::connect(ui->verticalScrollBar, &QScrollBar::valueChanged,
					 ui->panel, &PatternEditorPanel::onVScrollBarChanged);
	QObject::connect(ui->verticalScrollBar, &QScrollBar::sliderPressed, this, focusSlot);
}

PatternEditor::~PatternEditor()
{
	delete ui;
}

void PatternEditor::setCore(std::shared_ptr<BambooTracker> core)
{
	bt_ = core;
	ui->panel->setCore(core);
}

void PatternEditor::setCommandStack(std::weak_ptr<QUndoStack> stack)
{
	ui->panel->setCommandStack(stack);
}

void PatternEditor::setConfiguration(std::shared_ptr<Configuration> config)
{
	ui->panel->setConfiguration(config);
}

void PatternEditor::setColorPallete(std::shared_ptr<ColorPalette> palette)
{
	ui->panel->setColorPallete(palette);
}

void PatternEditor::addActionToPanel(QAction* action)
{
	ui->panel->addAction(action);
}

void PatternEditor::changeEditable()
{
	ui->panel->changeEditable();
}

void PatternEditor::updatePositionByStepUpdate(bool isFirstUpdate)
{
	ui->panel->updatePositionByStepUpdate(isFirstUpdate);
}

void PatternEditor::updatepositionByPositionJump(bool trackChanged)
{
	ui->panel->updatePositionByStepUpdate(false, true, trackChanged);
}

void PatternEditor::changeMarker()
{
	ui->panel->changeMarker();
}

void PatternEditor::copySelectedCells()
{
	ui->panel->copySelectedCells();
}

void PatternEditor::cutSelectedCells()
{
	ui->panel->cutSelectedCells();
}

void PatternEditor::freeze()
{
	setUpdatesEnabled(false);
	freezed_ = true;
	ui->panel->waitPaintFinish();
}

void PatternEditor::unfreeze()
{
	freezed_ = false;
	setUpdatesEnabled(true);
}

QString PatternEditor::getHeaderFont() const
{
	return ui->panel->getHeaderFont();
}

int PatternEditor::getHeaderFontSize() const
{
	return ui->panel->getHeaderFontSize();
}

QString PatternEditor::getRowsFont() const
{
	return ui->panel->getRowsFont();
}

int PatternEditor::getRowsFontSize() const
{
	return ui->panel->getRowsFontSize();
}

void PatternEditor::setFonts(QString headerFont, int headerSize, QString rowsFont, int rowsSize)
{
	ui->panel->setFonts(headerFont, headerSize, rowsFont, rowsSize);
}

void PatternEditor::setHorizontalScrollMode(bool cellBased, bool refresh)
{
	hScrollCellMove_ = cellBased;
	if (refresh) updateHorizontalSliderMaximum();
}

void PatternEditor::setVisibleTracks(std::vector<int> tracks)
{
	ui->horizontalScrollBar->setMaximum(200);	// Dummy
	ui->panel->setVisibleTracks(tracks);
	updateHorizontalSliderMaximum();
}

std::vector<int> PatternEditor::getVisibleTracks() const
{
	return ui->panel->getVisibleTracks();
}

bool PatternEditor::eventFilter(QObject *watched, QEvent *event)
{
	Q_UNUSED(watched)

	if (watched == this) {
		if (event->type() == QEvent::FocusIn) {
			ui->panel->setFocus();
		}
		return false;
	}

	if (watched == ui->panel) {
		if (freezed_ && event->type() != QEvent::Paint) return true;

		switch (event->type()) {
		case QEvent::FocusIn:
			ui->panel->redrawByFocusChanged();
			emit focusIn();
			return false;
		case QEvent::FocusOut:
			ui->panel->redrawByFocusChanged();
			emit focusOut();
			return false;
		case QEvent::HoverEnter:
		case QEvent::HoverLeave:
			ui->panel->redrawByHoverChanged();
			return false;
		default:
			return false;
		}
	}

	if (watched == ui->verticalScrollBar) {
		if (freezed_) return true;	// Ignore every events

		switch (event->type()) {
		case QEvent::MouseButtonPress:
		case QEvent::MouseButtonRelease:
		case QEvent::MouseButtonDblClick:
		case QEvent::DragMove:
		case QEvent::Wheel:
			return (bt_->isPlaySong() && bt_->isFollowPlay());
		default:
			return false;
		}
	}

	return false;
}

void PatternEditor::resizeEvent(QResizeEvent* event)
{
	Q_UNUSED(event)

	// For view-based scroll
	updateHorizontalSliderMaximum();
}

/********** Slots **********/
void PatternEditor::onOrderListCurrentTrackChanged(int idx)
{
	ui->panel->onOrderListCurrentTrackChanged(idx);
}

void PatternEditor::onOrderListCrrentOrderChanged(int num)
{
	ui->panel->onOrderListCurrentOrderChanged(num);
}

void PatternEditor::onOrderListEdited()
{
	ui->panel->onOrderListEdited();
}

void PatternEditor::onDefaultPatternSizeChanged()
{
	ui->panel->onDefaultPatternSizeChanged();
}

void PatternEditor::onShortcutUpdated()
{
	ui->panel->onShortcutUpdated();
}

void PatternEditor::onPatternDataGlobalChanged()
{
	ui->panel->redrawByPatternChanged();
}

void PatternEditor::setPatternHighlight1Count(int count)
{
	ui->panel->setPatternHighlight1Count(count);
}

void PatternEditor::setPatternHighlight2Count(int count)
{
	ui->panel->setPatternHighlight2Count(count);
}

void PatternEditor::setEditableStep(int n)
{
	ui->panel->setEditableStep(n);
}

void PatternEditor::onSongLoaded()
{
	ui->horizontalScrollBar->setValue(0);
	ui->panel->onSongLoaded();
	songLoaded_ = true;
	updateHorizontalSliderMaximum();
	ui->verticalScrollBar->setMaximum(static_cast<int>(bt_->getPatternSizeFromOrderNumber(
														   bt_->getCurrentSongNumber(),
														   bt_->getCurrentOrderNumber())) - 1);
	ui->verticalScrollBar->setValue(0);
}

void PatternEditor::onDeletePressed()
{
	ui->panel->onDeletePressed();
}

void PatternEditor::onPastePressed()
{
	ui->panel->onPastePressed();
}

void PatternEditor::onPasteMixPressed()
{
	ui->panel->onPasteMixPressed();
}

void PatternEditor::onPasteOverwritePressed()
{
	ui->panel->onPasteOverwritePressed();
}

void PatternEditor::onPasteInsertPressed()
{
	ui->panel->onPasteInsertPressed();
}

void PatternEditor::onSelectPressed(int type)
{
	ui->panel->onSelectPressed(type);
}

void PatternEditor::onTransposePressed(bool isOctave, bool isIncreased)
{
	int seminote = isOctave ? (isIncreased ? 12 : -12) : (isIncreased ? 1 : -1);
	ui->panel->onNoteTransposePressed(seminote);
}

void PatternEditor::onChangeValuesPressed(bool isCoarse, bool isIncreased)
{
	int val = isCoarse ? (isIncreased ? 16 : -16) : (isIncreased ? 1 : -1);
	ui->panel->onChangeValuesPressed(val);
}

void PatternEditor::onToggleTrackPressed()
{
	ui->panel->onToggleTrackPressed();
}

void PatternEditor::onSoloTrackPressed()
{
	ui->panel->onSoloTrackPressed();
}

void PatternEditor::onExpandPressed()
{
	ui->panel->onExpandPressed();
}

void PatternEditor::onShrinkPressed()
{
	ui->panel->onShrinkPressed();
}

void PatternEditor::onInterpolatePressed()
{
	ui->panel->onInterpolatePressed();
}

void PatternEditor::onReversePressed()
{
	ui->panel->onReversePressed();
}

void PatternEditor::onReplaceInstrumentPressed()
{
	ui->panel->onReplaceInstrumentPressed();
}

void PatternEditor::onFollowModeChanged()
{
	ui->panel->onFollowModeChanged();
}

void PatternEditor::onStoppedPlaySong()
{
	ui->panel->redrawPatterns();
}

void PatternEditor::onDuplicateInstrumentsRemoved()
{
	ui->panel->redrawByPatternChanged();
}

void PatternEditor::onPlayStepPressed()
{
	ui->panel->onPlayStepPressed();
}

void PatternEditor::updateHorizontalSliderMaximum()
{
	if (!ui->panel->isReadyCore() || !songLoaded_) return;
	int max = hScrollCellMove_ ? ui->panel->getFullColmunSize() : ui->panel->getScrollableCountByTrack();
	ui->horizontalScrollBar->setMaximum(max);
}
