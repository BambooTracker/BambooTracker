#include "instrument_editor_fm_form.hpp"
#include "ui_instrument_editor_fm_form.h"
#include <QString>
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
	// Envelope tab
	auto name = QString::fromUtf8(inst.getName().c_str(), inst.getName().length());
	setWindowTitle(name);
	ui->nameLineEdit->setText(name);
	ui->alSlider->setValue(inst.getParameterValue(FMParameter::AL));
	ui->fbSlider->setValue(inst.getParameterValue(FMParameter::FB));
	ui->op1Table->setValue(Ui::FMOperatorParameter::AR, inst.getParameterValue(FMParameter::AR1));
	ui->op1Table->setValue(Ui::FMOperatorParameter::DR, inst.getParameterValue(FMParameter::DR1));
	ui->op1Table->setValue(Ui::FMOperatorParameter::SR, inst.getParameterValue(FMParameter::SR1));
	ui->op1Table->setValue(Ui::FMOperatorParameter::RR, inst.getParameterValue(FMParameter::RR1));
	ui->op1Table->setValue(Ui::FMOperatorParameter::SL, inst.getParameterValue(FMParameter::SL1));
	ui->op1Table->setValue(Ui::FMOperatorParameter::TL, inst.getParameterValue(FMParameter::TL1));
	ui->op1Table->setValue(Ui::FMOperatorParameter::KS, inst.getParameterValue(FMParameter::KS1));
	ui->op1Table->setValue(Ui::FMOperatorParameter::ML, inst.getParameterValue(FMParameter::ML1));
	ui->op1Table->setValue(Ui::FMOperatorParameter::DT, inst.getParameterValue(FMParameter::DT1));
	ui->op1Table->setValue(Ui::FMOperatorParameter::SSGEG, inst.getParameterValue(FMParameter::SSGEG1));
	ui->op2Table->setValue(Ui::FMOperatorParameter::AR, inst.getParameterValue(FMParameter::AR2));
	ui->op2Table->setValue(Ui::FMOperatorParameter::DR, inst.getParameterValue(FMParameter::DR2));
	ui->op2Table->setValue(Ui::FMOperatorParameter::SR, inst.getParameterValue(FMParameter::SR2));
	ui->op2Table->setValue(Ui::FMOperatorParameter::RR, inst.getParameterValue(FMParameter::RR2));
	ui->op2Table->setValue(Ui::FMOperatorParameter::SL, inst.getParameterValue(FMParameter::SL2));
	ui->op2Table->setValue(Ui::FMOperatorParameter::TL, inst.getParameterValue(FMParameter::TL2));
	ui->op2Table->setValue(Ui::FMOperatorParameter::KS, inst.getParameterValue(FMParameter::KS2));
	ui->op2Table->setValue(Ui::FMOperatorParameter::ML, inst.getParameterValue(FMParameter::ML2));
	ui->op2Table->setValue(Ui::FMOperatorParameter::DT, inst.getParameterValue(FMParameter::DT2));
	ui->op2Table->setValue(Ui::FMOperatorParameter::SSGEG, inst.getParameterValue(FMParameter::SSGEG2));
	ui->op3Table->setValue(Ui::FMOperatorParameter::AR, inst.getParameterValue(FMParameter::AR3));
	ui->op3Table->setValue(Ui::FMOperatorParameter::DR, inst.getParameterValue(FMParameter::DR3));
	ui->op3Table->setValue(Ui::FMOperatorParameter::SR, inst.getParameterValue(FMParameter::SR3));
	ui->op3Table->setValue(Ui::FMOperatorParameter::RR, inst.getParameterValue(FMParameter::RR3));
	ui->op3Table->setValue(Ui::FMOperatorParameter::SL, inst.getParameterValue(FMParameter::SL3));
	ui->op3Table->setValue(Ui::FMOperatorParameter::TL, inst.getParameterValue(FMParameter::TL3));
	ui->op3Table->setValue(Ui::FMOperatorParameter::KS, inst.getParameterValue(FMParameter::KS3));
	ui->op3Table->setValue(Ui::FMOperatorParameter::ML, inst.getParameterValue(FMParameter::ML3));
	ui->op3Table->setValue(Ui::FMOperatorParameter::DT, inst.getParameterValue(FMParameter::DT3));
	ui->op3Table->setValue(Ui::FMOperatorParameter::SSGEG, inst.getParameterValue(FMParameter::SSGEG3));
	ui->op4Table->setValue(Ui::FMOperatorParameter::AR, inst.getParameterValue(FMParameter::AR4));
	ui->op4Table->setValue(Ui::FMOperatorParameter::DR, inst.getParameterValue(FMParameter::DR4));
	ui->op4Table->setValue(Ui::FMOperatorParameter::SR, inst.getParameterValue(FMParameter::SR4));
	ui->op4Table->setValue(Ui::FMOperatorParameter::RR, inst.getParameterValue(FMParameter::RR4));
	ui->op4Table->setValue(Ui::FMOperatorParameter::SL, inst.getParameterValue(FMParameter::SL4));
	ui->op4Table->setValue(Ui::FMOperatorParameter::TL, inst.getParameterValue(FMParameter::TL4));
	ui->op4Table->setValue(Ui::FMOperatorParameter::KS, inst.getParameterValue(FMParameter::KS4));
	ui->op4Table->setValue(Ui::FMOperatorParameter::ML, inst.getParameterValue(FMParameter::ML4));
	ui->op4Table->setValue(Ui::FMOperatorParameter::DT, inst.getParameterValue(FMParameter::DT4));
	ui->op4Table->setValue(Ui::FMOperatorParameter::SSGEG, inst.getParameterValue(FMParameter::SSGEG4));
}
