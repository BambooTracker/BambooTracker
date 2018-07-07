#include "instrument_editor_psg_form.hpp"
#include "ui_instrument_editor_psg_form.h"
#include "misc.hpp"

InstrumentEditorPSGForm::InstrumentEditorPSGForm(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::InstrumentEditorPSGForm)
{
	ui->setupUi(this);
}

InstrumentEditorPSGForm::~InstrumentEditorPSGForm()
{
	delete ui;
}

void InstrumentEditorPSGForm::setInstrumentParameters(const InstrumentPSG inst)
{

}
