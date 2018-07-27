#include "instrument_editor_psg_form.hpp"
#include "ui_instrument_editor_psg_form.h"
#include "misc.hpp"

InstrumentEditorPSGForm::InstrumentEditorPSGForm(int num, QWidget *parent) :
	QWidget(parent),
	ui(new Ui::InstrumentEditorPSGForm),
	instNum_(num)
{
	ui->setupUi(this);
}

InstrumentEditorPSGForm::~InstrumentEditorPSGForm()
{
	delete ui;
}
