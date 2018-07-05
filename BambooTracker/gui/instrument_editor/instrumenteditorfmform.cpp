#include "instrumenteditorfmform.hpp"
#include "ui_instrumenteditorfmform.h"

InstrumentEditorFMForm::InstrumentEditorFMForm(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::InstrumentEditorFMForm)
{
	ui->setupUi(this);
}

InstrumentEditorFMForm::~InstrumentEditorFMForm()
{
	delete ui;
}
