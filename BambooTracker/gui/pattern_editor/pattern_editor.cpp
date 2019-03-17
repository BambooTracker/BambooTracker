#include "pattern_editor.hpp"
#include "ui_pattern_editor.h"

PatternEditor::PatternEditor(QWidget *parent) :
	QFrame(parent),
	ui(new Ui::PatternEditor)
{
	ui->setupUi(this);

	installEventFilter(this);
	ui->panel->installEventFilter(this);

	ui->panel->setFocus();
	QObject::connect(ui->panel, &PatternEditorPanel::currentCellInRowChanged,
					 ui->horizontalScrollBar, &QScrollBar::setValue);
	QObject::connect(ui->panel, &PatternEditorPanel::currentStepChanged,
					 this, [&](int num, int max) {
		ui->verticalScrollBar->setMaximum(max);
		ui->verticalScrollBar->setValue(num);
	});
	QObject::connect(ui->panel, &PatternEditorPanel::currentTrackChanged,
					 this, [&](int num) { emit currentTrackChanged(num); });
	QObject::connect(ui->panel, &PatternEditorPanel::currentOrderChanged,
					 this, [&](int num, int max) { emit currentOrderChanged(num, max); });

	QObject::connect(ui->panel, &PatternEditorPanel::effectColsCompanded,
					 this, [&](int num, int max) {
		ui->horizontalScrollBar->setMaximum(max);
		ui->horizontalScrollBar->setValue(num);
	});
	QObject::connect(ui->panel, &PatternEditorPanel::selected,
					 this, [&](bool isSelected) { emit selected(isSelected); });
	QObject::connect(ui->panel, &PatternEditorPanel::returnPressed,
					 this, [&] { emit returnPressed(); });
	QObject::connect(ui->panel, &PatternEditorPanel::instrumentEntered,
					 this, [&](int num) { emit instrumentEntered(num); });
	QObject::connect(ui->panel, &PatternEditorPanel::effectEntered,
					 this, [&](QString text) { emit effectEntered(text); });

	auto focusSlot = [&] { ui->panel->setFocus(); };

	QObject::connect(ui->horizontalScrollBar, &QScrollBar::valueChanged,
					 ui->panel, &PatternEditorPanel::setCurrentCellInRow);
	QObject::connect(ui->horizontalScrollBar, &QScrollBar::sliderPressed, this, focusSlot);

	QObject::connect(ui->verticalScrollBar, &QScrollBar::valueChanged,
					 ui->panel, &PatternEditorPanel::setCurrentStep);
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

void PatternEditor::setConfiguration(std::weak_ptr<Configuration> config)
{
	ui->panel->setConfiguration(config);
}

void PatternEditor::setColorPallete(std::weak_ptr<ColorPalette> palette)
{
	ui->panel->setColorPallete(palette);
}

void PatternEditor::changeEditable()
{
	ui->panel->changeEditable();
}

void PatternEditor::updatePosition()
{
	ui->panel->updatePosition();
}

void PatternEditor::copySelectedCells()
{
	ui->panel->copySelectedCells();
}

void PatternEditor::cutSelectedCells()
{
	ui->panel->cutSelectedCells();
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
		switch (event->type()) {
		case QEvent::FocusIn:	emit focusIn();		return false;
		case QEvent::FocusOut:	emit focusOut();	return false;
		default:	return false;
		}
	}

	return false;
}

/********** Slots **********/
void PatternEditor::setCurrentTrack(int num)
{
	ui->panel->setCurrentTrack(num);
}

void PatternEditor::setCurrentOrder(int num)
{
	ui->panel->setCurrentOrder(num);
}

void PatternEditor::onOrderListEdited()
{
	ui->panel->onOrderListEdited();
}

void PatternEditor::onDefaultPatternSizeChanged()
{
	ui->panel->onDefaultPatternSizeChanged();
}

void PatternEditor::setPatternHighlightCount(int count)
{
	ui->panel->setPatternHighlightCount(count);
}

void PatternEditor::setEditableStep(int n)
{
	ui->panel->setEditableStep(n);
}

void PatternEditor::onSongLoaded()
{
	ui->panel->onSongLoaded();
	ui->horizontalScrollBar->setMaximum(ui->panel->getFullColmunSize());
	ui->horizontalScrollBar->setValue(0);
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

void PatternEditor::onSelectPressed(int type)
{
	ui->panel->onSelectPressed(type);
}

void PatternEditor::onTransposePressed(bool isOctave, bool isIncreased)
{
	ui->panel->onTransposePressed(isOctave, isIncreased);
}

void PatternEditor::onToggleTrackPressed()
{
	ui->panel->onToggleTrackPressed(ui->panel->getCurrentTrack());
}

void PatternEditor::onSoloTrackPressed()
{
	ui->panel->onSoloTrackPressed(ui->panel->getCurrentTrack());
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
