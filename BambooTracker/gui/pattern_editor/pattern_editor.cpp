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
	QObject::connect(ui->panel, &PatternEditorPanel::currentTrackChanged,
					 this, [&](int num) { emit currentTrackChanged(num); });

	auto focusSlot = [&]() { ui->panel->setFocus(); };

	QObject::connect(ui->horizontalScrollBar, &QScrollBar::valueChanged,
					 ui->panel, &PatternEditorPanel::setCurrentCellInRow);
	QObject::connect(ui->horizontalScrollBar, &QScrollBar::sliderPressed, this, focusSlot);

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
}

void PatternEditor::changeEditable()
{
	ui->panel->changeEditable();
}

/********** Slots **********/
void PatternEditor::setCurrentTrack(int num)
{
	ui->panel->setCurrentTrack(num);
}
