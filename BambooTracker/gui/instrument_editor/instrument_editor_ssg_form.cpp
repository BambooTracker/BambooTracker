#include "instrument_editor_ssg_form.hpp"
#include "ui_instrument_editor_ssg_form.h"
#include "misc.hpp"

InstrumentEditorSSGForm::InstrumentEditorSSGForm(int num, QWidget *parent) :
	QWidget(parent),
	ui(new Ui::InstrumentEditorSSGForm),
	instNum_(num)
{
	ui->setupUi(this);
}

InstrumentEditorSSGForm::~InstrumentEditorSSGForm()
{
	delete ui;
}
