#include "instrument_editor_fm_form.hpp"
#include "ui_instrument_editor_fm_form.h"
#include <QString>
#include <QPoint>
#include <QMenu>
#include <QApplication>
#include <QClipboard>
#include <QAction>
#include <QRegularExpression>
#include "gui/event_guard.hpp"
#include "misc.hpp"

#include <QDebug>

InstrumentEditorFMForm::InstrumentEditorFMForm(int num, QWidget *parent) :
	QWidget(parent),
	ui(new Ui::InstrumentEditorFMForm),
	instNum_(num),
	isIgnoreEvent_(false)
{
	ui->setupUi(this);

	installEventFilter(this);

	/******************** Envelope editor ********************/
	ui->envGroupBox->setContextMenuPolicy(Qt::CustomContextMenu);

	ui->alSlider->setText("AL");
	ui->alSlider->setMaximum(7);
	QObject::connect(ui->alSlider, &LabeledHorizontalSlider::valueChanged, this, [&](int value) {
		if (!isIgnoreEvent_) {
			bt_.lock()->setEnvelopeFMParameter(ui->envNumSpinBox->value(), FMEnvelopeParameter::AL, value);
			emit envelopeParameterChanged(ui->envNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	ui->fbSlider->setText("FB");
	ui->fbSlider->setMaximum(7);
	QObject::connect(ui->fbSlider, &LabeledHorizontalSlider::valueChanged, this, [&](int value) {
		if (!isIgnoreEvent_) {
			bt_.lock()->setEnvelopeFMParameter(ui->envNumSpinBox->value(), FMEnvelopeParameter::FB, value);
			emit envelopeParameterChanged(ui->envNumSpinBox->value(), instNum_);
			emit modified();
		}
	});

	ui->op1Table->setOperatorNumber(0);
	QObject::connect(ui->op1Table, &FMOperatorTable::operatorEnableChanged, this, [&](bool enable) {
		if (!isIgnoreEvent_) {
			bt_.lock()->setEnvelopeFMOperatorEnable(ui->envNumSpinBox->value(), 0, enable);
			emit envelopeParameterChanged(ui->envNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->op1Table, &FMOperatorTable::operatorValueChanged,
					 this, [&](Ui::FMOperatorParameter opParam, int value) {
		if (!isIgnoreEvent_) {
			FMEnvelopeParameter param;
			switch (opParam) {
			case Ui::FMOperatorParameter::AR:		param = FMEnvelopeParameter::AR1;		break;
			case Ui::FMOperatorParameter::DR:		param = FMEnvelopeParameter::DR1;		break;
			case Ui::FMOperatorParameter::SR:		param = FMEnvelopeParameter::SR1;		break;
			case Ui::FMOperatorParameter::RR:		param = FMEnvelopeParameter::RR1;		break;
			case Ui::FMOperatorParameter::SL:		param = FMEnvelopeParameter::SL1;		break;
			case Ui::FMOperatorParameter::TL:		param = FMEnvelopeParameter::TL1;		break;
			case Ui::FMOperatorParameter::KS:		param = FMEnvelopeParameter::KS1;		break;
			case Ui::FMOperatorParameter::ML:		param = FMEnvelopeParameter::ML1;		break;
			case Ui::FMOperatorParameter::DT:		param = FMEnvelopeParameter::DT1;		break;
			case Ui::FMOperatorParameter::SSGEG:	param = FMEnvelopeParameter::SSGEG1;	break;
			default:																break;
			}
			bt_.lock()->setEnvelopeFMParameter(ui->envNumSpinBox->value(), param, value);
			emit envelopeParameterChanged(ui->envNumSpinBox->value(), instNum_);
			emit modified();
		}
	});

	ui->op2Table->setOperatorNumber(1);
	QObject::connect(ui->op2Table, &FMOperatorTable::operatorEnableChanged, this, [&](bool enable) {
		if (!isIgnoreEvent_) {
			bt_.lock()->setEnvelopeFMOperatorEnable(ui->envNumSpinBox->value(), 1, enable);
			emit envelopeParameterChanged(ui->envNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->op2Table, &FMOperatorTable::operatorValueChanged,
						 this, [&](Ui::FMOperatorParameter opParam, int value) {
		if (!isIgnoreEvent_) {
			FMEnvelopeParameter param;
			switch (opParam) {
			case Ui::FMOperatorParameter::AR:		param = FMEnvelopeParameter::AR2;		break;
			case Ui::FMOperatorParameter::DR:		param = FMEnvelopeParameter::DR2;		break;
			case Ui::FMOperatorParameter::SR:		param = FMEnvelopeParameter::SR2;		break;
			case Ui::FMOperatorParameter::RR:		param = FMEnvelopeParameter::RR2;		break;
			case Ui::FMOperatorParameter::SL:		param = FMEnvelopeParameter::SL2;		break;
			case Ui::FMOperatorParameter::TL:		param = FMEnvelopeParameter::TL2;		break;
			case Ui::FMOperatorParameter::KS:		param = FMEnvelopeParameter::KS2;		break;
			case Ui::FMOperatorParameter::ML:		param = FMEnvelopeParameter::ML2;		break;
			case Ui::FMOperatorParameter::DT:		param = FMEnvelopeParameter::DT2;		break;
			case Ui::FMOperatorParameter::SSGEG:	param = FMEnvelopeParameter::SSGEG2;	break;
			default:																break;
			}
			bt_.lock()->setEnvelopeFMParameter(ui->envNumSpinBox->value(), param, value);
			emit envelopeParameterChanged(ui->envNumSpinBox->value(), instNum_);
			emit modified();
		}
	});

	ui->op3Table->setOperatorNumber(2);
	QObject::connect(ui->op3Table, &FMOperatorTable::operatorEnableChanged, this, [&](bool enable) {
		if (!isIgnoreEvent_) {
			bt_.lock()->setEnvelopeFMOperatorEnable(ui->envNumSpinBox->value(), 2, enable);
			emit envelopeParameterChanged(ui->envNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->op3Table, &FMOperatorTable::operatorValueChanged,
						 this, [&](Ui::FMOperatorParameter opParam, int value) {
		if (!isIgnoreEvent_) {
			FMEnvelopeParameter param;
			switch (opParam) {
			case Ui::FMOperatorParameter::AR:		param = FMEnvelopeParameter::AR3;		break;
			case Ui::FMOperatorParameter::DR:		param = FMEnvelopeParameter::DR3;		break;
			case Ui::FMOperatorParameter::SR:		param = FMEnvelopeParameter::SR3;		break;
			case Ui::FMOperatorParameter::RR:		param = FMEnvelopeParameter::RR3;		break;
			case Ui::FMOperatorParameter::SL:		param = FMEnvelopeParameter::SL3;		break;
			case Ui::FMOperatorParameter::TL:		param = FMEnvelopeParameter::TL3;		break;
			case Ui::FMOperatorParameter::KS:		param = FMEnvelopeParameter::KS3;		break;
			case Ui::FMOperatorParameter::ML:		param = FMEnvelopeParameter::ML3;		break;
			case Ui::FMOperatorParameter::DT:		param = FMEnvelopeParameter::DT3;		break;
			case Ui::FMOperatorParameter::SSGEG:	param = FMEnvelopeParameter::SSGEG3;	break;
			default:																break;
			}
			bt_.lock()->setEnvelopeFMParameter(ui->envNumSpinBox->value(), param, value);
			emit envelopeParameterChanged(ui->envNumSpinBox->value(), instNum_);
			emit modified();
		}
	});

	ui->op4Table->setOperatorNumber(3);
	QObject::connect(ui->op4Table, &FMOperatorTable::operatorEnableChanged, this, [&](bool enable) {
		if (!isIgnoreEvent_) {
			bt_.lock()->setEnvelopeFMOperatorEnable(ui->envNumSpinBox->value(), 3, enable);
			emit envelopeParameterChanged(ui->envNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->op4Table, &FMOperatorTable::operatorValueChanged,
						 this, [&](Ui::FMOperatorParameter opParam, int value) {
		if (!isIgnoreEvent_) {
			FMEnvelopeParameter param;
			switch (opParam) {
			case Ui::FMOperatorParameter::AR:		param = FMEnvelopeParameter::AR4;		break;
			case Ui::FMOperatorParameter::DR:		param = FMEnvelopeParameter::DR4;		break;
			case Ui::FMOperatorParameter::SR:		param = FMEnvelopeParameter::SR4;		break;
			case Ui::FMOperatorParameter::RR:		param = FMEnvelopeParameter::RR4;		break;
			case Ui::FMOperatorParameter::SL:		param = FMEnvelopeParameter::SL4;		break;
			case Ui::FMOperatorParameter::TL:		param = FMEnvelopeParameter::TL4;		break;
			case Ui::FMOperatorParameter::KS:		param = FMEnvelopeParameter::KS4;		break;
			case Ui::FMOperatorParameter::ML:		param = FMEnvelopeParameter::ML4;		break;
			case Ui::FMOperatorParameter::DT:		param = FMEnvelopeParameter::DT4;		break;
			case Ui::FMOperatorParameter::SSGEG:	param = FMEnvelopeParameter::SSGEG4;	break;
			default:																break;
			}
			bt_.lock()->setEnvelopeFMParameter(ui->envNumSpinBox->value(), param, value);
			emit envelopeParameterChanged(ui->envNumSpinBox->value(), instNum_);
			emit modified();
		}
	});

	/******************** LFO editor ********************/	
	ui->lfoGroupBox->setContextMenuPolicy(Qt::CustomContextMenu);

	ui->lfoFreqSlider->setText("Freq");
	ui->lfoFreqSlider->setMaximum(7);
	QObject::connect(ui->lfoFreqSlider, &LabeledVerticalSlider::valueChanged,
					 this, [&](int v) {
		if (!isIgnoreEvent_) {
			bt_.lock()->setLFOFMParameter(ui->lfoNumSpinBox->value(), FMLFOParamter::FREQ, v);
			emit lfoParameterChanged(ui->lfoNumSpinBox->value(), instNum_);
			emit modified();
		}
	});

	ui->pmsSlider->setText("PMS");
	ui->pmsSlider->setMaximum(7);
	QObject::connect(ui->pmsSlider, &LabeledVerticalSlider::valueChanged,
					 this, [&](int v) {
		if (!isIgnoreEvent_) {
			bt_.lock()->setLFOFMParameter(ui->lfoNumSpinBox->value(), FMLFOParamter::PMS, v);
			emit lfoParameterChanged(ui->lfoNumSpinBox->value(), instNum_);
			emit modified();
		}
	});

	ui->amsSlider->setText("AMS");
	ui->amsSlider->setMaximum(3);
	QObject::connect(ui->amsSlider, &LabeledVerticalSlider::valueChanged,
					 this, [&](int v) {
		if (!isIgnoreEvent_) {
			bt_.lock()->setLFOFMParameter(ui->lfoNumSpinBox->value(), FMLFOParamter::AMS, v);
			emit lfoParameterChanged(ui->lfoNumSpinBox->value(), instNum_);
			emit modified();
		}
	});

	QObject::connect(ui->amOp1CheckBox, &QCheckBox::stateChanged,
					 this, [&](int state) {
		if (!isIgnoreEvent_) {
			bt_.lock()->setLFOFMParameter(ui->lfoNumSpinBox->value(), FMLFOParamter::AM1,
										  (state == Qt::Checked) ? 1 : 0);
			emit lfoParameterChanged(ui->lfoNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->amOp2CheckBox, &QCheckBox::stateChanged,
					 this, [&](int state) {
		if (!isIgnoreEvent_) {
			bt_.lock()->setLFOFMParameter(ui->lfoNumSpinBox->value(), FMLFOParamter::AM2,
										  (state == Qt::Checked) ? 1 : 0);
			emit lfoParameterChanged(ui->lfoNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->amOp3CheckBox, &QCheckBox::stateChanged,
					 this, [&](int state) {
		if (!isIgnoreEvent_) {
			bt_.lock()->setLFOFMParameter(ui->lfoNumSpinBox->value(), FMLFOParamter::AM3,
										  (state == Qt::Checked) ? 1 : 0);
			emit lfoParameterChanged(ui->lfoNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->amOp4CheckBox, &QCheckBox::stateChanged,
					 this, [&](int state) {
		if (!isIgnoreEvent_) {
			bt_.lock()->setLFOFMParameter(ui->lfoNumSpinBox->value(), FMLFOParamter::AM4,
										  (state == Qt::Checked) ? 1 : 0);
			emit lfoParameterChanged(ui->lfoNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
}

InstrumentEditorFMForm::~InstrumentEditorFMForm()
{
	delete ui;
}

int InstrumentEditorFMForm::getInstrumentNumber() const
{
	return instNum_;
}

void InstrumentEditorFMForm::setCore(std::weak_ptr<BambooTracker> core)
{
	bt_ = core;
	updateInstrumentParameters();
}

void InstrumentEditorFMForm::updateInstrumentParameters()
{
	Ui::EventGuard eg(isIgnoreEvent_);

	std::unique_ptr<AbstructInstrument> inst = bt_.lock()->getInstrument(instNum_);
	auto instFM = dynamic_cast<InstrumentFM*>(inst.get());
	auto name = QString::fromUtf8(instFM->getName().c_str(), instFM->getName().length());
	setWindowTitle(QString("%1: %2").arg(instNum_, 2, 16, QChar('0')).toUpper().arg(name));

	setInstrumentEnvelopeParameters();
	setInstrumentLFOParameters();

	ui->envResetCheckBox->setChecked(instFM->getEnvelopeResetEnabled());
	ui->gateCountSpinBox->setValue(instFM->getGateCount());
}

/********** Events **********/
// MUST DIRECT CONNECTION
void InstrumentEditorFMForm::keyPressEvent(QKeyEvent *event)
{
	// For jam key on
	// General keys
	switch (event->key()) {
	case Qt::Key_Asterisk:	emit octaveChanged(true);		break;
	case Qt::Key_Slash:		emit octaveChanged(false);		break;
	default:
		if (!event->isAutoRepeat()) {
			// Musical keyboard
			switch (event->key()) {
			case Qt::Key_Z:
			case Qt::Key_S:
			case Qt::Key_X:
			case Qt::Key_D:
			case Qt::Key_C:
			case Qt::Key_V:
			case Qt::Key_G:
			case Qt::Key_B:
			case Qt::Key_H:
			case Qt::Key_N:
			case Qt::Key_J:
			case Qt::Key_M:
			case Qt::Key_Comma:
			case Qt::Key_L:
			case Qt::Key_Period:
			case Qt::Key_Q:
			case Qt::Key_2:
			case Qt::Key_W:
			case Qt::Key_3:
			case Qt::Key_E:
			case Qt::Key_R:
			case Qt::Key_5:
			case Qt::Key_T:
			case Qt::Key_6:
			case Qt::Key_Y:
			case Qt::Key_7:
			case Qt::Key_U:
			case Qt::Key_I:
			case Qt::Key_9:
			case Qt::Key_O:
				emit jamKeyOnEvent(event);
				break;
			default: break;
			}
		}
		break;
	}
}

// MUST DIRECT CONNECTION
void InstrumentEditorFMForm::keyReleaseEvent(QKeyEvent *event)
{
	// For jam key off
	if (!event->isAutoRepeat()) {
		switch (event->key()) {
		case Qt::Key_Z:
		case Qt::Key_S:
		case Qt::Key_X:
		case Qt::Key_D:
		case Qt::Key_C:
		case Qt::Key_V:
		case Qt::Key_G:
		case Qt::Key_B:
		case Qt::Key_H:
		case Qt::Key_N:
		case Qt::Key_J:
		case Qt::Key_M:
		case Qt::Key_Comma:
		case Qt::Key_L:
		case Qt::Key_Period:
		case Qt::Key_Q:
		case Qt::Key_2:
		case Qt::Key_W:
		case Qt::Key_3:
		case Qt::Key_E:
		case Qt::Key_R:
		case Qt::Key_5:
		case Qt::Key_T:
		case Qt::Key_6:
		case Qt::Key_Y:
		case Qt::Key_7:
		case Qt::Key_U:
		case Qt::Key_I:
		case Qt::Key_9:
		case Qt::Key_O:
			emit jamKeyOffEvent(event);
			break;
		default: break;
		}
	}
}

//========== Envelope ==========//
int InstrumentEditorFMForm::getEnvelopeNumber() const
{
	return ui->envNumSpinBox->value();
}

void InstrumentEditorFMForm::setInstrumentEnvelopeParameters()
{
	Ui::EventGuard eg(isIgnoreEvent_);

	std::unique_ptr<AbstructInstrument> inst = bt_.lock()->getInstrument(instNum_);
	auto instFM = dynamic_cast<InstrumentFM*>(inst.get());

	ui->envNumSpinBox->setValue(instFM->getEnvelopeNumber());
	onEnvelopeNumberChanged();
	ui->alSlider->setValue(instFM->getEnvelopeParameter(FMEnvelopeParameter::AL));
	ui->fbSlider->setValue(instFM->getEnvelopeParameter(FMEnvelopeParameter::FB));
	ui->op1Table->setValue(Ui::FMOperatorParameter::AR, instFM->getEnvelopeParameter(FMEnvelopeParameter::AR1));
	ui->op1Table->setValue(Ui::FMOperatorParameter::DR, instFM->getEnvelopeParameter(FMEnvelopeParameter::DR1));
	ui->op1Table->setValue(Ui::FMOperatorParameter::SR, instFM->getEnvelopeParameter(FMEnvelopeParameter::SR1));
	ui->op1Table->setValue(Ui::FMOperatorParameter::RR, instFM->getEnvelopeParameter(FMEnvelopeParameter::RR1));
	ui->op1Table->setValue(Ui::FMOperatorParameter::SL, instFM->getEnvelopeParameter(FMEnvelopeParameter::SL1));
	ui->op1Table->setValue(Ui::FMOperatorParameter::TL, instFM->getEnvelopeParameter(FMEnvelopeParameter::TL1));
	ui->op1Table->setValue(Ui::FMOperatorParameter::KS, instFM->getEnvelopeParameter(FMEnvelopeParameter::KS1));
	ui->op1Table->setValue(Ui::FMOperatorParameter::ML, instFM->getEnvelopeParameter(FMEnvelopeParameter::ML1));
	ui->op1Table->setValue(Ui::FMOperatorParameter::DT, instFM->getEnvelopeParameter(FMEnvelopeParameter::DT1));
	ui->op1Table->setValue(Ui::FMOperatorParameter::SSGEG, instFM->getEnvelopeParameter(FMEnvelopeParameter::SSGEG1));
	ui->op2Table->setValue(Ui::FMOperatorParameter::AR, instFM->getEnvelopeParameter(FMEnvelopeParameter::AR2));
	ui->op2Table->setValue(Ui::FMOperatorParameter::DR, instFM->getEnvelopeParameter(FMEnvelopeParameter::DR2));
	ui->op2Table->setValue(Ui::FMOperatorParameter::SR, instFM->getEnvelopeParameter(FMEnvelopeParameter::SR2));
	ui->op2Table->setValue(Ui::FMOperatorParameter::RR, instFM->getEnvelopeParameter(FMEnvelopeParameter::RR2));
	ui->op2Table->setValue(Ui::FMOperatorParameter::SL, instFM->getEnvelopeParameter(FMEnvelopeParameter::SL2));
	ui->op2Table->setValue(Ui::FMOperatorParameter::TL, instFM->getEnvelopeParameter(FMEnvelopeParameter::TL2));
	ui->op2Table->setValue(Ui::FMOperatorParameter::KS, instFM->getEnvelopeParameter(FMEnvelopeParameter::KS2));
	ui->op2Table->setValue(Ui::FMOperatorParameter::ML, instFM->getEnvelopeParameter(FMEnvelopeParameter::ML2));
	ui->op2Table->setValue(Ui::FMOperatorParameter::DT, instFM->getEnvelopeParameter(FMEnvelopeParameter::DT2));
	ui->op2Table->setValue(Ui::FMOperatorParameter::SSGEG, instFM->getEnvelopeParameter(FMEnvelopeParameter::SSGEG2));
	ui->op3Table->setValue(Ui::FMOperatorParameter::AR, instFM->getEnvelopeParameter(FMEnvelopeParameter::AR3));
	ui->op3Table->setValue(Ui::FMOperatorParameter::DR, instFM->getEnvelopeParameter(FMEnvelopeParameter::DR3));
	ui->op3Table->setValue(Ui::FMOperatorParameter::SR, instFM->getEnvelopeParameter(FMEnvelopeParameter::SR3));
	ui->op3Table->setValue(Ui::FMOperatorParameter::RR, instFM->getEnvelopeParameter(FMEnvelopeParameter::RR3));
	ui->op3Table->setValue(Ui::FMOperatorParameter::SL, instFM->getEnvelopeParameter(FMEnvelopeParameter::SL3));
	ui->op3Table->setValue(Ui::FMOperatorParameter::TL, instFM->getEnvelopeParameter(FMEnvelopeParameter::TL3));
	ui->op3Table->setValue(Ui::FMOperatorParameter::KS, instFM->getEnvelopeParameter(FMEnvelopeParameter::KS3));
	ui->op3Table->setValue(Ui::FMOperatorParameter::ML, instFM->getEnvelopeParameter(FMEnvelopeParameter::ML3));
	ui->op3Table->setValue(Ui::FMOperatorParameter::DT, instFM->getEnvelopeParameter(FMEnvelopeParameter::DT3));
	ui->op3Table->setValue(Ui::FMOperatorParameter::SSGEG, instFM->getEnvelopeParameter(FMEnvelopeParameter::SSGEG3));
	ui->op4Table->setValue(Ui::FMOperatorParameter::AR, instFM->getEnvelopeParameter(FMEnvelopeParameter::AR4));
	ui->op4Table->setValue(Ui::FMOperatorParameter::DR, instFM->getEnvelopeParameter(FMEnvelopeParameter::DR4));
	ui->op4Table->setValue(Ui::FMOperatorParameter::SR, instFM->getEnvelopeParameter(FMEnvelopeParameter::SR4));
	ui->op4Table->setValue(Ui::FMOperatorParameter::RR, instFM->getEnvelopeParameter(FMEnvelopeParameter::RR4));
	ui->op4Table->setValue(Ui::FMOperatorParameter::SL, instFM->getEnvelopeParameter(FMEnvelopeParameter::SL4));
	ui->op4Table->setValue(Ui::FMOperatorParameter::TL, instFM->getEnvelopeParameter(FMEnvelopeParameter::TL4));
	ui->op4Table->setValue(Ui::FMOperatorParameter::KS, instFM->getEnvelopeParameter(FMEnvelopeParameter::KS4));
	ui->op4Table->setValue(Ui::FMOperatorParameter::ML, instFM->getEnvelopeParameter(FMEnvelopeParameter::ML4));
	ui->op4Table->setValue(Ui::FMOperatorParameter::DT, instFM->getEnvelopeParameter(FMEnvelopeParameter::DT4));
	ui->op4Table->setValue(Ui::FMOperatorParameter::SSGEG, instFM->getEnvelopeParameter(FMEnvelopeParameter::SSGEG4));
}

void InstrumentEditorFMForm::setInstrumentEnvelopeParameters(QString data)
{
	QRegularExpression re("^(?<fb>\\d+),(?<al>\\d+),"
						  "(?<ar1>\\d+),(?<dr1>\\d+),(?<sr1>\\d+),(?<rr1>\\d+),(?<sl1>\\d+),"
						  "(?<tl1>\\d+),(?<ks1>\\d+),(?<ml1>\\d+),(?<dt1>\\d+),(?<ssgeg1>-?\\d+),"
						  "(?<ar2>\\d+),(?<dr2>\\d+),(?<sr2>\\d+),(?<rr2>\\d+),(?<sl2>\\d+),"
						  "(?<tl2>\\d+),(?<ks2>\\d+),(?<ml2>\\d+),(?<dt2>\\d+),(?<ssgeg2>-?\\d+),"
						  "(?<ar3>\\d+),(?<dr3>\\d+),(?<sr3>\\d+),(?<rr3>\\d+),(?<sl3>\\d+),"
						  "(?<tl3>\\d+),(?<ks3>\\d+),(?<ml3>\\d+),(?<dt3>\\d+),(?<ssgeg3>-?\\d+),"
						  "(?<ar4>\\d+),(?<dr4>\\d+),(?<sr4>\\d+),(?<rr4>\\d+),(?<sl4>\\d+),"
						  "(?<tl4>\\d+),(?<ks4>\\d+),(?<ml4>\\d+),(?<dt4>\\d+),(?<ssgeg4>-?\\d+)");
	QRegularExpressionMatch match = re.match(data);

	if (match.hasMatch()) {
		ui->fbSlider->setValue(match.captured("fb").toInt());
		ui->alSlider->setValue(match.captured("al").toInt());
		ui->op1Table->setValue(Ui::FMOperatorParameter::AR, match.captured("ar1").toInt());
		ui->op1Table->setValue(Ui::FMOperatorParameter::DR, match.captured("dr1").toInt());
		ui->op1Table->setValue(Ui::FMOperatorParameter::SR, match.captured("sr1").toInt());
		ui->op1Table->setValue(Ui::FMOperatorParameter::RR, match.captured("rr1").toInt());
		ui->op1Table->setValue(Ui::FMOperatorParameter::SL, match.captured("sl1").toInt());
		ui->op1Table->setValue(Ui::FMOperatorParameter::TL, match.captured("tl1").toInt());
		ui->op1Table->setValue(Ui::FMOperatorParameter::KS, match.captured("ks1").toInt());
		ui->op1Table->setValue(Ui::FMOperatorParameter::ML, match.captured("ml1").toInt());
		ui->op1Table->setValue(Ui::FMOperatorParameter::DT, match.captured("dt1").toInt());
		ui->op1Table->setValue(Ui::FMOperatorParameter::SSGEG, match.captured("ssgeg1").toInt());
		ui->op2Table->setValue(Ui::FMOperatorParameter::AR, match.captured("ar2").toInt());
		ui->op2Table->setValue(Ui::FMOperatorParameter::DR, match.captured("dr2").toInt());
		ui->op2Table->setValue(Ui::FMOperatorParameter::SR, match.captured("sr2").toInt());
		ui->op2Table->setValue(Ui::FMOperatorParameter::RR, match.captured("rr2").toInt());
		ui->op2Table->setValue(Ui::FMOperatorParameter::SL, match.captured("sl2").toInt());
		ui->op2Table->setValue(Ui::FMOperatorParameter::TL, match.captured("tl2").toInt());
		ui->op2Table->setValue(Ui::FMOperatorParameter::KS, match.captured("ks2").toInt());
		ui->op2Table->setValue(Ui::FMOperatorParameter::ML, match.captured("ml2").toInt());
		ui->op2Table->setValue(Ui::FMOperatorParameter::DT, match.captured("dt2").toInt());
		ui->op2Table->setValue(Ui::FMOperatorParameter::SSGEG, match.captured("ssgeg2").toInt());
		ui->op3Table->setValue(Ui::FMOperatorParameter::AR, match.captured("ar3").toInt());
		ui->op3Table->setValue(Ui::FMOperatorParameter::DR, match.captured("dr3").toInt());
		ui->op3Table->setValue(Ui::FMOperatorParameter::SR, match.captured("sr3").toInt());
		ui->op3Table->setValue(Ui::FMOperatorParameter::RR, match.captured("rr3").toInt());
		ui->op3Table->setValue(Ui::FMOperatorParameter::SL, match.captured("sl3").toInt());
		ui->op3Table->setValue(Ui::FMOperatorParameter::TL, match.captured("tl3").toInt());
		ui->op3Table->setValue(Ui::FMOperatorParameter::KS, match.captured("ks3").toInt());
		ui->op3Table->setValue(Ui::FMOperatorParameter::ML, match.captured("ml3").toInt());
		ui->op3Table->setValue(Ui::FMOperatorParameter::DT, match.captured("dt3").toInt());
		ui->op3Table->setValue(Ui::FMOperatorParameter::SSGEG, match.captured("ssgeg3").toInt());
		ui->op4Table->setValue(Ui::FMOperatorParameter::AR, match.captured("ar4").toInt());
		ui->op4Table->setValue(Ui::FMOperatorParameter::DR, match.captured("dr4").toInt());
		ui->op4Table->setValue(Ui::FMOperatorParameter::SR, match.captured("sr4").toInt());
		ui->op4Table->setValue(Ui::FMOperatorParameter::RR, match.captured("rr4").toInt());
		ui->op4Table->setValue(Ui::FMOperatorParameter::SL, match.captured("sl4").toInt());
		ui->op4Table->setValue(Ui::FMOperatorParameter::TL, match.captured("tl4").toInt());
		ui->op4Table->setValue(Ui::FMOperatorParameter::KS, match.captured("ks4").toInt());
		ui->op4Table->setValue(Ui::FMOperatorParameter::ML, match.captured("ml4").toInt());
		ui->op4Table->setValue(Ui::FMOperatorParameter::DT, match.captured("dt4").toInt());
		ui->op4Table->setValue(Ui::FMOperatorParameter::SSGEG, match.captured("ssgeg4").toInt());
	}
}

QString InstrumentEditorFMForm::toEnvelopeString() const
{
	auto str = QString("%1,%2,%3,%4,%5,%6")
			   .arg(QString::number(ui->fbSlider->value()))
			   .arg(QString::number(ui->alSlider->value()))
			   .arg(ui->op1Table->toString())
			   .arg(ui->op2Table->toString())
			   .arg(ui->op3Table->toString())
			   .arg(ui->op4Table->toString());
	return str;
}

/********** Slots **********/
void InstrumentEditorFMForm::on_envNumSpinBox_valueChanged(int arg1)
{
	if (!isIgnoreEvent_) {
		bt_.lock()->setInstrumentFMEnvelope(instNum_, arg1);
		setInstrumentEnvelopeParameters();
		emit envelopeNumberChanged();
		emit modified();
	}

	onEnvelopeNumberChanged();
}

void InstrumentEditorFMForm::on_envGroupBox_customContextMenuRequested(const QPoint &pos)
{
	QClipboard* clipboard = QApplication::clipboard();
	QPoint globalPos = ui->envGroupBox->mapToGlobal(pos);

	QMenu menu;
	menu.addAction("Copy envelope", this, [&, clipboard]() {
		clipboard->setText("FM_ENVELOPE:" + toEnvelopeString());
	});
	menu.addAction("Paste envelope", this, [&, clipboard]() {
		QString data = clipboard->text().remove("FM_ENVELOPE:");
		setInstrumentEnvelopeParameters(data);
	});
	if (!clipboard->text().startsWith("FM_ENVELOPE:")) menu.actions().at(1)->setEnabled(false);

	menu.exec(globalPos);
}

void InstrumentEditorFMForm::onEnvelopeParameterChanged(int envNum)
{
	if (ui->envNumSpinBox->value() == envNum) {
		Ui::EventGuard eg(isIgnoreEvent_);
		setInstrumentEnvelopeParameters();
	}
}

void InstrumentEditorFMForm::onEnvelopeNumberChanged()
{
	// Change users view
	QString str;
	std::vector<int> users = bt_.lock()->getEnvelopeFMUsers(ui->envNumSpinBox->value());
	for (auto& n : users) {
		str += (QString::number(n) + ",");
	}
	str.chop(1);
	ui->envUsersLineEdit->setText(str);
}

//========== LFO ==========//
int InstrumentEditorFMForm::getLFONumber() const
{
	return ui->lfoGroupBox->isChecked() ? ui->lfoNumSpinBox->value() : -1;
}

void InstrumentEditorFMForm::setInstrumentLFOParameters()
{
	Ui::EventGuard eg(isIgnoreEvent_);

	std::unique_ptr<AbstructInstrument> inst = bt_.lock()->getInstrument(instNum_);
	auto instFM = dynamic_cast<InstrumentFM*>(inst.get());

	int num = instFM->getLFONumber();
	if (num == -1) {
		ui->lfoGroupBox->setChecked(false);
	}
	else {
		ui->lfoGroupBox->setChecked(true);
		ui->lfoNumSpinBox->setValue(num);
		onLFONumberChanged();
		ui->lfoFreqSlider->setValue(instFM->getLFOParameter(FMLFOParamter::FREQ));
		ui->pmsSlider->setValue(instFM->getLFOParameter(FMLFOParamter::PMS));
		ui->amsSlider->setValue(instFM->getLFOParameter(FMLFOParamter::AMS));
		ui->amOp1CheckBox->setChecked(instFM->getLFOParameter(FMLFOParamter::AM1));
		ui->amOp2CheckBox->setChecked(instFM->getLFOParameter(FMLFOParamter::AM2));
		ui->amOp3CheckBox->setChecked(instFM->getLFOParameter(FMLFOParamter::AM3));
		ui->amOp4CheckBox->setChecked(instFM->getLFOParameter(FMLFOParamter::AM4));
	}
}

void InstrumentEditorFMForm::setInstrumentLFOParameters(QString data)
{
	QRegularExpression re("^(?<freq>\\d+),(?<pms>\\d+),(?<ams>\\d+),"
						  "(?<am1>\\d+),(?<am2>\\d+),(?<am3>\\d+),(?<am4>\\d+)");
	QRegularExpressionMatch match = re.match(data);

	if (match.hasMatch()) {
		ui->lfoFreqSlider->setValue(match.captured("freq").toInt());
		ui->pmsSlider->setValue(match.captured("pms").toInt());
		ui->amsSlider->setValue(match.captured("ams").toInt());
		ui->amOp1CheckBox->setChecked(match.captured("am1").toInt() == 1);
		ui->amOp2CheckBox->setChecked(match.captured("am2").toInt() == 1);
		ui->amOp3CheckBox->setChecked(match.captured("am3").toInt() == 1);
		ui->amOp4CheckBox->setChecked(match.captured("am4").toInt() == 1);
	}
}

QString InstrumentEditorFMForm::toLFOString() const
{
	auto str = QString("%1,%2,%3,%4,%5,%6,%7")
			   .arg(QString::number(ui->lfoFreqSlider->value()))
			   .arg(QString::number(ui->pmsSlider->value()))
			   .arg(QString::number(ui->amsSlider->value()))
			   .arg(QString::number(ui->amOp1CheckBox->isChecked() ? 1 : 0))
			   .arg(QString::number(ui->amOp2CheckBox->isChecked() ? 1 : 0))
			   .arg(QString::number(ui->amOp3CheckBox->isChecked() ? 1 : 0))
			   .arg(QString::number(ui->amOp4CheckBox->isChecked() ? 1 : 0));
	return str;
}

/********** Slots **********/
void InstrumentEditorFMForm::onLFOParameterChanged(int lfoNum)
{
	if (ui->lfoNumSpinBox->value() == lfoNum) {
		Ui::EventGuard eg(isIgnoreEvent_);
		setInstrumentLFOParameters();
	}
}

void InstrumentEditorFMForm::onLFONumberChanged()
{
	// Change users view
	QString str;
	std::vector<int> users = bt_.lock()->getLFOFMUsers(ui->lfoNumSpinBox->value());
	for (auto& n : users) {
		str += (QString::number(n) + ",");
	}
	str.chop(1);
	ui->lfoUsersLineEdit->setText(str);
}

void InstrumentEditorFMForm::on_lfoGroupBox_customContextMenuRequested(const QPoint &pos)
{
	QClipboard* clipboard = QApplication::clipboard();
	QPoint globalPos = ui->lfoGroupBox->mapToGlobal(pos);

	QMenu menu;
	QAction* copy = menu.addAction("Copy LFO parameters", this, [&, clipboard]() {
		clipboard->setText("FM_LFO:" + toLFOString());
	});
	QAction* paste = menu.addAction("Paste LFO parameters", this, [&, clipboard]() {
		QString data = clipboard->text().remove("FM_LFO:");
		setInstrumentLFOParameters(data);
	});
	if (!ui->lfoGroupBox->isChecked()) {
		copy->setEnabled(false);
		paste->setEnabled(false);
	}
	else if (!clipboard->text().startsWith("FM_LFO:")) {
		paste->setEnabled(false);
	}

	menu.exec(globalPos);
}

void InstrumentEditorFMForm::on_lfoNumSpinBox_valueChanged(int arg1)
{
	if (!isIgnoreEvent_) {
		bt_.lock()->setInstrumentFMLFO(instNum_, arg1);
		setInstrumentLFOParameters();
		emit lfoNumberChanged();
		emit modified();
	}

	onLFONumberChanged();
}

void InstrumentEditorFMForm::on_lfoGroupBox_toggled(bool arg1)
{
	if (!isIgnoreEvent_) {
		bt_.lock()->setInstrumentFMLFO(instNum_, arg1 ? ui->lfoNumSpinBox->value() : -1);
		setInstrumentLFOParameters();
		emit lfoNumberChanged();
		emit modified();
	}

	onLFONumberChanged();
}

//========== Else ==========//
/********** Slots **********/
void InstrumentEditorFMForm::on_envResetCheckBox_stateChanged(int arg1)
{
	if (arg1 == Qt::Checked) {
		bt_.lock()->setInstrumentFMEnvelopeResetEnabled(instNum_, true);
	}
	else {
		bt_.lock()->setInstrumentFMEnvelopeResetEnabled(instNum_, false);
	}
	emit modified();
}

void InstrumentEditorFMForm::on_gateCountSpinBox_valueChanged(int arg1)
{
	bt_.lock()->setInstrumentGateCount(instNum_, arg1);
	emit modified();
}
