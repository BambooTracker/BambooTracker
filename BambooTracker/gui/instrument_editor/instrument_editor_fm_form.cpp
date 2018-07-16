#include "instrument_editor_fm_form.hpp"
#include "ui_instrument_editor_fm_form.h"
#include "misc.hpp"

#include <QDebug>

InstrumentEditorFMForm::InstrumentEditorFMForm(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::InstrumentEditorFMForm)
{
	ui->setupUi(this);

	ui->alSlider->setText("AL");
	ui->alSlider->setMaximum(7);
	ui->fbSlider->setText("FB");
	ui->fbSlider->setMaximum(7);

	ui->op1Table->setOperatorNumber(0);
	ui->op2Table->setOperatorNumber(1);
	ui->op3Table->setOperatorNumber(2);
	ui->op4Table->setOperatorNumber(3);
}

InstrumentEditorFMForm::~InstrumentEditorFMForm()
{
	delete ui;
}

void InstrumentEditorFMForm::setInstrumentParameters(const InstrumentFM inst)
{
	qDebug() << "nksl";
}
