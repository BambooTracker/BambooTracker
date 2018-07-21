#include "instrument_editor_fm_form.hpp"
#include "ui_instrument_editor_fm_form.h"
#include <QString>
#include "misc.hpp"

#include <QDebug>

InstrumentEditorFMForm::InstrumentEditorFMForm(int num, QWidget *parent) :
	QWidget(parent),
	ui(new Ui::InstrumentEditorFMForm),
	instNum_(num),
	isValidEmit(true)
{
	ui->setupUi(this);

	ui->alSlider->setText("AL");
	ui->alSlider->setMaximum(7);
	QObject::connect(ui->alSlider, &LabeledHorizontalSlider::valueChanged, this,
					 [&](int value) { emit parameterChanged(instNum_, FMParameter::AL, value); });
	ui->fbSlider->setText("FB");
	ui->fbSlider->setMaximum(7);
	QObject::connect(ui->fbSlider, &LabeledHorizontalSlider::valueChanged, this,
					 [&](int value) { emit parameterChanged(instNum_, FMParameter::FB, value); });

	ui->op1Table->setOperatorNumber(0);
	QObject::connect(ui->op1Table, &FMOperatorTable::operatorValueChanged,
					 this, [&](Ui::FMOperatorParameter opParam, int value) {
			FMParameter param;
			switch (opParam) {
			case Ui::FMOperatorParameter::AR:		param = FMParameter::AR1;		break;
			case Ui::FMOperatorParameter::DR:		param = FMParameter::DR1;		break;
			case Ui::FMOperatorParameter::SR:		param = FMParameter::SR1;		break;
			case Ui::FMOperatorParameter::RR:		param = FMParameter::RR1;		break;
			case Ui::FMOperatorParameter::SL:		param = FMParameter::SL1;		break;
			case Ui::FMOperatorParameter::TL:		param = FMParameter::TL1;		break;
			case Ui::FMOperatorParameter::KS:		param = FMParameter::KS1;		break;
			case Ui::FMOperatorParameter::ML:		param = FMParameter::ML1;		break;
			case Ui::FMOperatorParameter::DT:		param = FMParameter::DT1;		break;
			case Ui::FMOperatorParameter::SSGEG:	param = FMParameter::SSGEG1;	break;
			}
			emit parameterChanged(instNum_, param, value);
	});

	ui->op2Table->setOperatorNumber(1);
	QObject::connect(ui->op2Table, &FMOperatorTable::operatorValueChanged,
						 this, [&](Ui::FMOperatorParameter opParam, int value) {
			FMParameter param;
			switch (opParam) {
			case Ui::FMOperatorParameter::AR:		param = FMParameter::AR2;		break;
			case Ui::FMOperatorParameter::DR:		param = FMParameter::DR2;		break;
			case Ui::FMOperatorParameter::SR:		param = FMParameter::SR2;		break;
			case Ui::FMOperatorParameter::RR:		param = FMParameter::RR2;		break;
			case Ui::FMOperatorParameter::SL:		param = FMParameter::SL2;		break;
			case Ui::FMOperatorParameter::TL:		param = FMParameter::TL2;		break;
			case Ui::FMOperatorParameter::KS:		param = FMParameter::KS2;		break;
			case Ui::FMOperatorParameter::ML:		param = FMParameter::ML2;		break;
			case Ui::FMOperatorParameter::DT:		param = FMParameter::DT2;		break;
			case Ui::FMOperatorParameter::SSGEG:	param = FMParameter::SSGEG2;	break;
			}
			emit parameterChanged(instNum_, param, value);
	});

	ui->op3Table->setOperatorNumber(2);
	QObject::connect(ui->op3Table, &FMOperatorTable::operatorValueChanged,
						 this, [&](Ui::FMOperatorParameter opParam, int value) {
			FMParameter param;
			switch (opParam) {
			case Ui::FMOperatorParameter::AR:		param = FMParameter::AR3;		break;
			case Ui::FMOperatorParameter::DR:		param = FMParameter::DR3;		break;
			case Ui::FMOperatorParameter::SR:		param = FMParameter::SR3;		break;
			case Ui::FMOperatorParameter::RR:		param = FMParameter::RR3;		break;
			case Ui::FMOperatorParameter::SL:		param = FMParameter::SL3;		break;
			case Ui::FMOperatorParameter::TL:		param = FMParameter::TL3;		break;
			case Ui::FMOperatorParameter::KS:		param = FMParameter::KS3;		break;
			case Ui::FMOperatorParameter::ML:		param = FMParameter::ML3;		break;
			case Ui::FMOperatorParameter::DT:		param = FMParameter::DT3;		break;
			case Ui::FMOperatorParameter::SSGEG:	param = FMParameter::SSGEG3;	break;
			}
			emit parameterChanged(instNum_, param, value);
	});

	ui->op4Table->setOperatorNumber(3);
	QObject::connect(ui->op4Table, &FMOperatorTable::operatorValueChanged,
						 this, [&](Ui::FMOperatorParameter opParam, int value) {
			FMParameter param;
			switch (opParam) {
			case Ui::FMOperatorParameter::AR:		param = FMParameter::AR4;		break;
			case Ui::FMOperatorParameter::DR:		param = FMParameter::DR4;		break;
			case Ui::FMOperatorParameter::SR:		param = FMParameter::SR4;		break;
			case Ui::FMOperatorParameter::RR:		param = FMParameter::RR4;		break;
			case Ui::FMOperatorParameter::SL:		param = FMParameter::SL4;		break;
			case Ui::FMOperatorParameter::TL:		param = FMParameter::TL4;		break;
			case Ui::FMOperatorParameter::KS:		param = FMParameter::KS4;		break;
			case Ui::FMOperatorParameter::ML:		param = FMParameter::ML4;		break;
			case Ui::FMOperatorParameter::DT:		param = FMParameter::DT4;		break;
			case Ui::FMOperatorParameter::SSGEG:	param = FMParameter::SSGEG4;	break;
			}
			emit parameterChanged(instNum_, param, value);
	});
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
