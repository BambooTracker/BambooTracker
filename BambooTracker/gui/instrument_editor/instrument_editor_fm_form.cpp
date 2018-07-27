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

	/******************** Envelope editor ********************/
	ui->alSlider->setText("AL");
	ui->alSlider->setMaximum(7);
	QObject::connect(ui->alSlider, &LabeledHorizontalSlider::valueChanged, this,
					 [&](int value) { bt_->setInstrumentFMParameter(instNum_, FMParameter::AL, value); });
	ui->fbSlider->setText("FB");
	ui->fbSlider->setMaximum(7);
	QObject::connect(ui->fbSlider, &LabeledHorizontalSlider::valueChanged, this,
					 [&](int value) { bt_->setInstrumentFMParameter(instNum_, FMParameter::FB, value); });

	ui->op1Table->setOperatorNumber(0);
	QObject::connect(ui->op1Table, &FMOperatorTable::operatorEnableChanged,
					 this, [&](bool enable) { bt_->setInstrumentFMOperatorEnable(instNum_, 0, enable); });
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
			bt_->setInstrumentFMParameter(instNum_, param, value);
	});

	ui->op2Table->setOperatorNumber(1);
	QObject::connect(ui->op2Table, &FMOperatorTable::operatorEnableChanged,
					 this, [&](bool enable) { bt_->setInstrumentFMOperatorEnable(instNum_, 1, enable); });
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
			bt_->setInstrumentFMParameter(instNum_, param, value);
	});

	ui->op3Table->setOperatorNumber(2);
	QObject::connect(ui->op3Table, &FMOperatorTable::operatorEnableChanged,
					 this, [&](bool enable) { bt_->setInstrumentFMOperatorEnable(instNum_, 2, enable); });
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
			bt_->setInstrumentFMParameter(instNum_, param, value);
	});

	ui->op4Table->setOperatorNumber(3);
	QObject::connect(ui->op4Table, &FMOperatorTable::operatorEnableChanged,
					 this, [&](bool enable) { bt_->setInstrumentFMOperatorEnable(instNum_, 3, enable); });
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
			bt_->setInstrumentFMParameter(instNum_, param, value);
	});
}

InstrumentEditorFMForm::~InstrumentEditorFMForm()
{
	delete ui;
}

void InstrumentEditorFMForm::setCore(std::shared_ptr<BambooTracker> core)
{
	bt_ = core;
	setInstrumentParameters();
}

void InstrumentEditorFMForm::setInstrumentParameters()
{	
	std::unique_ptr<AbstructInstrument> inst = bt_->getInstrument(instNum_);
	auto instFM = dynamic_cast<InstrumentFM*>(inst.get());
	auto name = QString::fromUtf8(instFM->getName().c_str(), instFM->getName().length());
	setWindowTitle(name);

	// Envelope tab
	ui->alSlider->setValue(instFM->getEnvelopeParameter(FMParameter::AL));
	ui->fbSlider->setValue(instFM->getEnvelopeParameter(FMParameter::FB));
	ui->op1Table->setValue(Ui::FMOperatorParameter::AR, instFM->getEnvelopeParameter(FMParameter::AR1));
	ui->op1Table->setValue(Ui::FMOperatorParameter::DR, instFM->getEnvelopeParameter(FMParameter::DR1));
	ui->op1Table->setValue(Ui::FMOperatorParameter::SR, instFM->getEnvelopeParameter(FMParameter::SR1));
	ui->op1Table->setValue(Ui::FMOperatorParameter::RR, instFM->getEnvelopeParameter(FMParameter::RR1));
	ui->op1Table->setValue(Ui::FMOperatorParameter::SL, instFM->getEnvelopeParameter(FMParameter::SL1));
	ui->op1Table->setValue(Ui::FMOperatorParameter::TL, instFM->getEnvelopeParameter(FMParameter::TL1));
	ui->op1Table->setValue(Ui::FMOperatorParameter::KS, instFM->getEnvelopeParameter(FMParameter::KS1));
	ui->op1Table->setValue(Ui::FMOperatorParameter::ML, instFM->getEnvelopeParameter(FMParameter::ML1));
	ui->op1Table->setValue(Ui::FMOperatorParameter::DT, instFM->getEnvelopeParameter(FMParameter::DT1));
	ui->op1Table->setValue(Ui::FMOperatorParameter::SSGEG, instFM->getEnvelopeParameter(FMParameter::SSGEG1));
	ui->op2Table->setValue(Ui::FMOperatorParameter::AR, instFM->getEnvelopeParameter(FMParameter::AR2));
	ui->op2Table->setValue(Ui::FMOperatorParameter::DR, instFM->getEnvelopeParameter(FMParameter::DR2));
	ui->op2Table->setValue(Ui::FMOperatorParameter::SR, instFM->getEnvelopeParameter(FMParameter::SR2));
	ui->op2Table->setValue(Ui::FMOperatorParameter::RR, instFM->getEnvelopeParameter(FMParameter::RR2));
	ui->op2Table->setValue(Ui::FMOperatorParameter::SL, instFM->getEnvelopeParameter(FMParameter::SL2));
	ui->op2Table->setValue(Ui::FMOperatorParameter::TL, instFM->getEnvelopeParameter(FMParameter::TL2));
	ui->op2Table->setValue(Ui::FMOperatorParameter::KS, instFM->getEnvelopeParameter(FMParameter::KS2));
	ui->op2Table->setValue(Ui::FMOperatorParameter::ML, instFM->getEnvelopeParameter(FMParameter::ML2));
	ui->op2Table->setValue(Ui::FMOperatorParameter::DT, instFM->getEnvelopeParameter(FMParameter::DT2));
	ui->op2Table->setValue(Ui::FMOperatorParameter::SSGEG, instFM->getEnvelopeParameter(FMParameter::SSGEG2));
	ui->op3Table->setValue(Ui::FMOperatorParameter::AR, instFM->getEnvelopeParameter(FMParameter::AR3));
	ui->op3Table->setValue(Ui::FMOperatorParameter::DR, instFM->getEnvelopeParameter(FMParameter::DR3));
	ui->op3Table->setValue(Ui::FMOperatorParameter::SR, instFM->getEnvelopeParameter(FMParameter::SR3));
	ui->op3Table->setValue(Ui::FMOperatorParameter::RR, instFM->getEnvelopeParameter(FMParameter::RR3));
	ui->op3Table->setValue(Ui::FMOperatorParameter::SL, instFM->getEnvelopeParameter(FMParameter::SL3));
	ui->op3Table->setValue(Ui::FMOperatorParameter::TL, instFM->getEnvelopeParameter(FMParameter::TL3));
	ui->op3Table->setValue(Ui::FMOperatorParameter::KS, instFM->getEnvelopeParameter(FMParameter::KS3));
	ui->op3Table->setValue(Ui::FMOperatorParameter::ML, instFM->getEnvelopeParameter(FMParameter::ML3));
	ui->op3Table->setValue(Ui::FMOperatorParameter::DT, instFM->getEnvelopeParameter(FMParameter::DT3));
	ui->op3Table->setValue(Ui::FMOperatorParameter::SSGEG, instFM->getEnvelopeParameter(FMParameter::SSGEG3));
	ui->op4Table->setValue(Ui::FMOperatorParameter::AR, instFM->getEnvelopeParameter(FMParameter::AR4));
	ui->op4Table->setValue(Ui::FMOperatorParameter::DR, instFM->getEnvelopeParameter(FMParameter::DR4));
	ui->op4Table->setValue(Ui::FMOperatorParameter::SR, instFM->getEnvelopeParameter(FMParameter::SR4));
	ui->op4Table->setValue(Ui::FMOperatorParameter::RR, instFM->getEnvelopeParameter(FMParameter::RR4));
	ui->op4Table->setValue(Ui::FMOperatorParameter::SL, instFM->getEnvelopeParameter(FMParameter::SL4));
	ui->op4Table->setValue(Ui::FMOperatorParameter::TL, instFM->getEnvelopeParameter(FMParameter::TL4));
	ui->op4Table->setValue(Ui::FMOperatorParameter::KS, instFM->getEnvelopeParameter(FMParameter::KS4));
	ui->op4Table->setValue(Ui::FMOperatorParameter::ML, instFM->getEnvelopeParameter(FMParameter::ML4));
	ui->op4Table->setValue(Ui::FMOperatorParameter::DT, instFM->getEnvelopeParameter(FMParameter::DT4));
	ui->op4Table->setValue(Ui::FMOperatorParameter::SSGEG, instFM->getEnvelopeParameter(FMParameter::SSGEG4));
}
