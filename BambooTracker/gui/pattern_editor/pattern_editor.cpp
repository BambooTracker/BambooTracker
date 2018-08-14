#include "pattern_editor.hpp"
#include "ui_pattern_editor.h"

PatternEditor::PatternEditor(QWidget *parent) :
	QFrame(parent),
	ui(new Ui::PatternEditor)
{
	ui->setupUi(this);

	ui->panel->setFocus();
	QObject::connect(ui->panel, &PatternEditorPanel::currentCellInRowChanged,
					 ui->horizontalScrollBar, &QScrollBar::setValue);
	QObject::connect(ui->panel, &PatternEditorPanel::currentStepChanged,
					 this, [&](int num, int max) {
		ui->verticalScrollBar->setValue(num);
		ui->verticalScrollBar->setMaximum(max);
	});
	QObject::connect(ui->panel, &PatternEditorPanel::currentTrackChanged,
					 this, [&](int num) { emit currentTrackChanged(num); });
	QObject::connect(ui->panel, &PatternEditorPanel::currentOrderChanged,
					 this, [&](int num) { emit currentOrderChanged(num); });

	auto focusSlot = [&]() { ui->panel->setFocus(); };

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
	ui->panel->setCore(core);
	ui->horizontalScrollBar->setMaximum(ui->panel->getFullColmunSize());
	ui->verticalScrollBar->setMaximum(core->getPatternSizeFromOrderNumber(
										  core->getCurrentSongNumber(),
										  core->getCurrentOrderNumber()) - 1);
}

void PatternEditor::setCommandStack(std::weak_ptr<QUndoStack> stack)
{
	ui->panel->setCommandStack(stack);
}

void PatternEditor::changeEditable()
{
	ui->panel->changeEditable();
}

void PatternEditor::updatePosition()
{
	ui->panel->updatePosition();
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
