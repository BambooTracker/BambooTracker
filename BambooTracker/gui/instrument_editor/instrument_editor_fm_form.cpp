#include "instrument_editor_fm_form.hpp"
#include "ui_instrument_editor_fm_form.h"
#include <QString>
#include <QPoint>
#include <QMenu>
#include <QApplication>
#include <QClipboard>
#include <QRegularExpression>
#include "gui/event_guard.hpp"
#include "misc.hpp"

InstrumentEditorFMForm::InstrumentEditorFMForm(int num, QWidget *parent) :
	QWidget(parent),
	ui(new Ui::InstrumentEditorFMForm),
	instNum_(num),
	isIgnoreEvent_(false)
{
	ui->setupUi(this);

	installEventFilter(this);

	/******************** Envelope editor ********************/
	ui->envelopeTab->setContextMenuPolicy(Qt::CustomContextMenu);

	ui->alSlider->setText("AL");
	ui->alSlider->setMaximum(7);
	QObject::connect(ui->alSlider, &LabeledHorizontalSlider::valueChanged, this, [&](int value) {
		if (!isIgnoreEvent_) {
			bt_.lock()->setEnvelopeFMParameter(ui->envNumSpinBox->value(), FMParameter::AL, value);
			emit envelopeParameterChanged(ui->envNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	ui->fbSlider->setText("FB");
	ui->fbSlider->setMaximum(7);
	QObject::connect(ui->fbSlider, &LabeledHorizontalSlider::valueChanged, this, [&](int value) {
		if (!isIgnoreEvent_) {
			bt_.lock()->setEnvelopeFMParameter(ui->envNumSpinBox->value(), FMParameter::FB, value);
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
			default:																break;
			}
			bt_.lock()->setEnvelopeFMParameter(ui->envNumSpinBox->value(), param, value);
			emit envelopeParameterChanged(ui->envNumSpinBox->value(), instNum_);
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

int InstrumentEditorFMForm::getEnvelopeNumber() const
{
	return ui->envNumSpinBox->value();
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
void InstrumentEditorFMForm::setInstrumentEnvelopeParameters()
{
	Ui::EventGuard eg(isIgnoreEvent_);

	std::unique_ptr<AbstructInstrument> inst = bt_.lock()->getInstrument(instNum_);
	auto instFM = dynamic_cast<InstrumentFM*>(inst.get());

	ui->envNumSpinBox->setValue(instFM->getEnvelopeNumber());
	onEnvelopeNumberChanged(instFM->getEnvelopeNumber());
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
		emit envelopeNumberChanged(arg1);
		emit modified();
	}

	onEnvelopeNumberChanged(arg1);
}

void InstrumentEditorFMForm::on_envelopeTab_customContextMenuRequested(const QPoint &pos)
{
	QClipboard* clipboard = QApplication::clipboard();
	QPoint globalPos = ui->envelopeTab->mapToGlobal(pos);

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

void InstrumentEditorFMForm::onEnvelopeNumberChanged(int n)
{
	if (ui->envNumSpinBox->value() == n) {
		// Change users view
		QString str;
		std::vector<int> users = bt_.lock()->getEnvelopeFMUsers(ui->envNumSpinBox->value());
		for (auto& n : users) {
			str += (QString::number(n) + ",");
		}
		str.chop(1);
		ui->envUsersLineEdit->setText(str);
	}
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
