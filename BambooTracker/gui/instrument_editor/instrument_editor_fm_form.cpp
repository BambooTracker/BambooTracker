#include "instrument_editor_fm_form.hpp"
#include "ui_instrument_editor_fm_form.h"
#include "misc.hpp"

#include <QDebug>

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

void InstrumentEditorFMForm::setInstrumentParameters(const InstrumentFM inst)
{
	qDebug() << "nksl";
}
