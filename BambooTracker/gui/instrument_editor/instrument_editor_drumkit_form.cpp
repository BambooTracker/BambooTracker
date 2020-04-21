#include "instrument_editor_drumkit_form.hpp"
#include "ui_instrument_editor_drumkit_form.h"

InstrumentEditorDrumkitForm::InstrumentEditorDrumkitForm(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::InstrumentEditorDrumkitForm)
{
	ui->setupUi(this);
}

InstrumentEditorDrumkitForm::~InstrumentEditorDrumkitForm()
{
	delete ui;
}
