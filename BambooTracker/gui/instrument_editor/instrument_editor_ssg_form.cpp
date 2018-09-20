#include "instrument_editor_ssg_form.hpp"
#include "ui_instrument_editor_ssg_form.h"
#include <vector>
#include <utility>
#include "gui/event_guard.hpp"
#include "misc.hpp"

#include <QDebug>

InstrumentEditorSSGForm::InstrumentEditorSSGForm(int num, QWidget *parent) :
	QWidget(parent),
	ui(new Ui::InstrumentEditorSSGForm),
	instNum_(num)
{
	ui->setupUi(this);

	//========== Wave form ==========//
	ui->waveEditor->setMaximumDisplayedRowCount(5);
	ui->waveEditor->setDefaultRow(0);
	ui->waveEditor->AddRow("Sq");
	ui->waveEditor->AddRow("Tri");
	ui->waveEditor->AddRow("Saw");
	ui->waveEditor->AddRow("Tri w");
	ui->waveEditor->AddRow("Saw w");

	QString tn[] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
	for (int i = 0; i < 8; ++i) {
		for (int j = 0; j < 12; ++j) {
			for (int k = 0; k < 32; ++k) {
				ui->squareMaskComboBox->addItem(
							QString("%1%2+%3").arg(tn[j]).arg(i).arg(k), 12 * i + 32 * j + k);
			}
		}
	}

	QObject::connect(ui->waveEditor, &VisualizedInstrumentMacroEditor::sequenceCommandAdded,
					 this, [&](int row, int col) {
		if (!isIgnoreEvent_) {
			if (row >= 3) {	// Set square mask frequency
				ui->waveEditor->setText(col, ui->squareMaskComboBox->currentText());
				ui->waveEditor->setData(col, ui->squareMaskComboBox->currentData().toInt());
			}
			bt_.lock()->addWaveFormSSGSequenceCommand(
						ui->waveNumSpinBox->value(), row, ui->waveEditor->getSequenceDataAt(col));
			emit waveFormParameterChanged(ui->waveNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->waveEditor, &VisualizedInstrumentMacroEditor::sequenceCommandRemoved,
					 this, [&]() {
		if (!isIgnoreEvent_) {
			bt_.lock()->removeWaveFormSSGSequenceCommand(ui->waveNumSpinBox->value());
			emit waveFormParameterChanged(ui->waveNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->waveEditor, &VisualizedInstrumentMacroEditor::sequenceCommandChanged,
					 this, [&](int row, int col) {
		if (!isIgnoreEvent_) {
			if (row >= 3) {	// Set square mask frequency
				ui->waveEditor->setText(col, ui->squareMaskComboBox->currentText());
				ui->waveEditor->setData(col, ui->squareMaskComboBox->currentData().toInt());
			}
			bt_.lock()->setWaveFormSSGSequenceCommand(
						ui->waveNumSpinBox->value(), col, row, ui->waveEditor->getSequenceDataAt(col));
			emit waveFormParameterChanged(ui->waveNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->waveEditor, &VisualizedInstrumentMacroEditor::loopChanged,
					 this, [&](std::vector<int> begins, std::vector<int> ends, std::vector<int> times) {
		if (!isIgnoreEvent_) {
			bt_.lock()->setWaveFormSSGLoops(
						ui->waveNumSpinBox->value(), std::move(begins), std::move(ends), std::move(times));
			emit waveFormParameterChanged(ui->waveNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->waveEditor, &VisualizedInstrumentMacroEditor::releaseChanged,
					 this, [&](VisualizedInstrumentMacroEditor::ReleaseType type, int point) {
		if (!isIgnoreEvent_) {
			ReleaseType t = convertReleaseTypeForData(type);
			bt_.lock()->setWaveFormSSGRelease(ui->waveNumSpinBox->value(), t, point);
			emit waveFormParameterChanged(ui->waveNumSpinBox->value(), instNum_);
			emit modified();
		}
	});

	//========== Tone/Noise ==========//
	ui->tnEditor->setMaximumDisplayedRowCount(16);
	ui->tnEditor->setDefaultRow(0);
	ui->tnEditor->AddRow("T");
	for (int i = 0; i < 32; ++i) {
		ui->tnEditor->AddRow("N " + QString::number(i));
	}
	for (int i = 0; i < 32; ++i) {
		ui->tnEditor->AddRow("T&N " + QString::number(i));
	}

	QObject::connect(ui->tnEditor, &VisualizedInstrumentMacroEditor::sequenceCommandAdded,
					 this, [&](int row, int col) {
		if (!isIgnoreEvent_) {
			bt_.lock()->addToneNoiseSSGSequenceCommand(
						ui->tnNumSpinBox->value(), row, ui->tnEditor->getSequenceDataAt(col));
			emit toneNoiseParameterChanged(ui->tnNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->tnEditor, &VisualizedInstrumentMacroEditor::sequenceCommandRemoved,
					 this, [&]() {
		if (!isIgnoreEvent_) {
			bt_.lock()->removeToneNoiseSSGSequenceCommand(ui->tnNumSpinBox->value());
			emit toneNoiseParameterChanged(ui->tnNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->tnEditor, &VisualizedInstrumentMacroEditor::sequenceCommandChanged,
					 this, [&](int row, int col) {
		if (!isIgnoreEvent_) {
			bt_.lock()->setToneNoiseSSGSequenceCommand(
						ui->tnNumSpinBox->value(), col, row, ui->tnEditor->getSequenceDataAt(col));
			emit toneNoiseParameterChanged(ui->tnNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->tnEditor, &VisualizedInstrumentMacroEditor::loopChanged,
					 this, [&](std::vector<int> begins, std::vector<int> ends, std::vector<int> times) {
		if (!isIgnoreEvent_) {
			bt_.lock()->setToneNoiseSSGLoops(
						ui->tnNumSpinBox->value(), std::move(begins), std::move(ends), std::move(times));
			emit toneNoiseParameterChanged(ui->tnNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->tnEditor, &VisualizedInstrumentMacroEditor::releaseChanged,
					 this, [&](VisualizedInstrumentMacroEditor::ReleaseType type, int point) {
		if (!isIgnoreEvent_) {
			ReleaseType t = convertReleaseTypeForData(type);
			bt_.lock()->setToneNoiseSSGRelease(ui->tnNumSpinBox->value(), t, point);
			emit toneNoiseParameterChanged(ui->tnNumSpinBox->value(), instNum_);
			emit modified();
		}
	});

	//========== Envelope ==========//
	ui->envEditor->setMaximumDisplayedRowCount(16);
	ui->envEditor->setDefaultRow(15);
	for (int i = 0; i < 16; ++i) {
		ui->envEditor->AddRow(QString::number(i));
	}
	for (int i = 0; i < 8; ++i) {
		ui->envEditor->AddRow("Hard " + QString::number(i));
	}
	ui->envEditor->setMultipleReleaseState(true);

	ui->hardFreqSpinBox->setSuffix(
				QString(" (%1Hz)").arg(QString::number(7800.0 / ui->hardFreqSpinBox->value(), 'f', 4)));

	QObject::connect(ui->envEditor, &VisualizedInstrumentMacroEditor::sequenceCommandAdded,
					 this, [&](int row, int col) {
		if (!isIgnoreEvent_) {
			if (row >= 16) {	// Set hard frequency
				ui->envEditor->setText(col, QString::number(ui->hardFreqSpinBox->value()));
				ui->envEditor->setData(col, ui->hardFreqSpinBox->value());
			}
			bt_.lock()->addEnvelopeSSGSequenceCommand(
						ui->envNumSpinBox->value(), row, ui->envEditor->getSequenceDataAt(col));
			emit envelopeParameterChanged(ui->envNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->envEditor, &VisualizedInstrumentMacroEditor::sequenceCommandRemoved,
					 this, [&]() {
		if (!isIgnoreEvent_) {
			bt_.lock()->removeEnvelopeSSGSequenceCommand(ui->envNumSpinBox->value());
			emit envelopeParameterChanged(ui->envNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->envEditor, &VisualizedInstrumentMacroEditor::sequenceCommandChanged,
					 this, [&](int row, int col) {
		if (!isIgnoreEvent_) {
			if (row >= 16) {	// Set hard frequency
				ui->envEditor->setText(col, QString::number(ui->hardFreqSpinBox->value()));
				ui->envEditor->setData(col, ui->hardFreqSpinBox->value());
			}
			bt_.lock()->setEnvelopeSSGSequenceCommand(
						ui->envNumSpinBox->value(), col, row, ui->envEditor->getSequenceDataAt(col));
			emit envelopeParameterChanged(ui->envNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->envEditor, &VisualizedInstrumentMacroEditor::loopChanged,
					 this, [&](std::vector<int> begins, std::vector<int> ends, std::vector<int> times) {
		if (!isIgnoreEvent_) {
			bt_.lock()->setEnvelopeSSGLoops(
						ui->envNumSpinBox->value(), std::move(begins), std::move(ends), std::move(times));
			emit envelopeParameterChanged(ui->envNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->envEditor, &VisualizedInstrumentMacroEditor::releaseChanged,
					 this, [&](VisualizedInstrumentMacroEditor::ReleaseType type, int point) {
		if (!isIgnoreEvent_) {
			ReleaseType t = convertReleaseTypeForData(type);
			bt_.lock()->setEnvelopeSSGRelease(ui->envNumSpinBox->value(), t, point);
			emit envelopeParameterChanged(ui->envNumSpinBox->value(), instNum_);
			emit modified();
		}
	});

	//========== Arpeggio ==========//
	ui->arpEditor->setMaximumDisplayedRowCount(15);
	ui->arpEditor->setDefaultRow(48);
	ui->arpEditor->setLabelDiaplayMode(true);
	for (int i = 0; i < 96; ++i) {
		ui->arpEditor->AddRow(QString::number(i - 48));
	}
	ui->arpEditor->setUpperRow(55);

	ui->arpTypeComboBox->addItem("Absolute", 0);
	ui->arpTypeComboBox->addItem("Fix", 1);
	ui->arpTypeComboBox->addItem("Relative", 2);

	QObject::connect(ui->arpEditor, &VisualizedInstrumentMacroEditor::sequenceCommandAdded,
					 this, [&](int row, int col) {
		if (!isIgnoreEvent_) {
			bt_.lock()->addArpeggioSSGSequenceCommand(
						ui->arpNumSpinBox->value(), row, ui->arpEditor->getSequenceDataAt(col));
			emit arpeggioParameterChanged(ui->arpNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->arpEditor, &VisualizedInstrumentMacroEditor::sequenceCommandRemoved,
					 this, [&]() {
		if (!isIgnoreEvent_) {
			bt_.lock()->removeArpeggioSSGSequenceCommand(ui->arpNumSpinBox->value());
			emit arpeggioParameterChanged(ui->arpNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->arpEditor, &VisualizedInstrumentMacroEditor::sequenceCommandChanged,
					 this, [&](int row, int col) {
		if (!isIgnoreEvent_) {
			bt_.lock()->setArpeggioSSGSequenceCommand(
						ui->arpNumSpinBox->value(), col, row, ui->arpEditor->getSequenceDataAt(col));
			emit arpeggioParameterChanged(ui->arpNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->arpEditor, &VisualizedInstrumentMacroEditor::loopChanged,
					 this, [&](std::vector<int> begins, std::vector<int> ends, std::vector<int> times) {
		if (!isIgnoreEvent_) {
			bt_.lock()->setArpeggioSSGLoops(
						ui->arpNumSpinBox->value(), std::move(begins), std::move(ends), std::move(times));
			emit arpeggioParameterChanged(ui->arpNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->arpEditor, &VisualizedInstrumentMacroEditor::releaseChanged,
					 this, [&](VisualizedInstrumentMacroEditor::ReleaseType type, int point) {
		if (!isIgnoreEvent_) {
			ReleaseType t = convertReleaseTypeForData(type);
			bt_.lock()->setArpeggioSSGRelease(ui->arpNumSpinBox->value(), t, point);
			emit arpeggioParameterChanged(ui->arpNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->arpTypeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
					 this, &InstrumentEditorSSGForm::onArpeggioTypeChanged);
}

InstrumentEditorSSGForm::~InstrumentEditorSSGForm()
{
	delete ui;
}

int InstrumentEditorSSGForm::getInstrumentNumber() const
{
	return instNum_;
}

void InstrumentEditorSSGForm::setCore(std::weak_ptr<BambooTracker> core)
{
	bt_ = core;
	updateInstrumentParameters();
}

ReleaseType InstrumentEditorSSGForm::convertReleaseTypeForData(VisualizedInstrumentMacroEditor::ReleaseType type)
{
	switch (type) {
	case VisualizedInstrumentMacroEditor::ReleaseType::NO_RELEASE:
		return ReleaseType::NO_RELEASE;
	case VisualizedInstrumentMacroEditor::ReleaseType::FIX:
		return ReleaseType::FIX;
	case VisualizedInstrumentMacroEditor::ReleaseType::ABSOLUTE:
		return ReleaseType::ABSOLUTE;
	case VisualizedInstrumentMacroEditor::ReleaseType::RELATIVE:
		return ReleaseType::RELATIVE;
	}
}

VisualizedInstrumentMacroEditor::ReleaseType InstrumentEditorSSGForm::convertReleaseTypeForUI(ReleaseType type)
{
	switch (type) {
	case ReleaseType::NO_RELEASE:
		return VisualizedInstrumentMacroEditor::ReleaseType::NO_RELEASE;
	case ReleaseType::FIX:
		return VisualizedInstrumentMacroEditor::ReleaseType::FIX;
	case ReleaseType::ABSOLUTE:
		return VisualizedInstrumentMacroEditor::ReleaseType::ABSOLUTE;
	case ReleaseType::RELATIVE:
		return VisualizedInstrumentMacroEditor::ReleaseType::RELATIVE;
	}
}

void InstrumentEditorSSGForm::updateInstrumentParameters()
{
	Ui::EventGuard eg(isIgnoreEvent_);

	std::unique_ptr<AbstructInstrument> inst = bt_.lock()->getInstrument(instNum_);
	auto instSSG = dynamic_cast<InstrumentSSG*>(inst.get());
	auto name = QString::fromUtf8(instSSG->getName().c_str(), instSSG->getName().length());
	setWindowTitle(QString("%1: %2").arg(instNum_, 2, 16, QChar('0')).toUpper().arg(name));

	setInstrumentWaveFormParameters();
	setInstrumentToneNoiseParameters();
	setInstrumentEnvelopeParameters();
	setInstrumentArpeggioParameters();

	ui->gateCountSpinBox->setValue(instSSG->getGateCount());
}

/********** Events **********/
// MUST DIRECT CONNECTION
void InstrumentEditorSSGForm::keyPressEvent(QKeyEvent *event)
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
void InstrumentEditorSSGForm::keyReleaseEvent(QKeyEvent *event)
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

//--- Wave form
int InstrumentEditorSSGForm::getWaveFormNumber() const
{
	return ui->waveEditGroupBox->isChecked() ? ui->waveNumSpinBox->value() : -1;
}

void InstrumentEditorSSGForm::setInstrumentWaveFormParameters()
{
	Ui::EventGuard ev(isIgnoreEvent_);

	std::unique_ptr<AbstructInstrument> inst = bt_.lock()->getInstrument(instNum_);
	auto instSSG = dynamic_cast<InstrumentSSG*>(inst.get());

	int num = instSSG->getWaveFormNumber();
	if (num == -1) {
		ui->waveEditGroupBox->setChecked(false);
	}
	else {
		ui->waveEditGroupBox->setChecked(true);
		ui->waveNumSpinBox->setValue(num);
		onWaveFormNumberChanged();
		ui->waveEditor->clearData();
		for (auto& com : instSSG->getWaveFormSequence()) {
			QString str = "";
			if (com.type >= 3) {
				switch (com.data / 32 % 12) {
					case 0:		str = "C";	break;
					case 1:		str = "C#";	break;
					case 2:		str = "D";	break;
					case 3:		str = "D#";	break;
					case 4:		str = "E";	break;
					case 5:		str = "F";	break;
					case 6:		str = "F#";	break;
					case 7:		str = "G";	break;
					case 8:		str = "G#";	break;
					case 9:		str = "A";	break;
					case 10:	str = "A#";	break;
					case 11:	str = "B";	break;
				}
				str += QString("%1+%2").arg(com.data / 32 / 12).arg(com.data % 32);
			}
			ui->waveEditor->addSequenceCommand(com.type, str, com.data);
		}
		for (auto& l : instSSG->getWaveFormLoops()) {
			ui->waveEditor->addLoop(l.begin, l.end, l.times);
		}
		ui->waveEditor->setRelease(convertReleaseTypeForUI(instSSG->getWaveFormRelease().type),
								   instSSG->getWaveFormRelease().begin);
	}
}

/********** Slots **********/
void InstrumentEditorSSGForm::onWaveFormNumberChanged()
{
	// Change users view
	QString str;
	std::vector<int> users = bt_.lock()->getWaveFormSSGUsers(ui->waveNumSpinBox->value());
	for (auto& n : users) {
		str += (QString::number(n) + ",");
	}
	str.chop(1);

	ui->waveUsersLineEdit->setText(str);
}

void InstrumentEditorSSGForm::onWaveFormParameterChanged(int wfNum)
{
	if (ui->waveNumSpinBox->value() == wfNum) {
		Ui::EventGuard eg(isIgnoreEvent_);
		setInstrumentWaveFormParameters();
	}
}

void InstrumentEditorSSGForm::on_waveEditGroupBox_toggled(bool arg1)
{
	if (!isIgnoreEvent_) {
		bt_.lock()->setInstrumentSSGWaveForm(instNum_, arg1 ? ui->waveNumSpinBox->value() : -1);
		setInstrumentWaveFormParameters();
		emit waveFormNumberChanged();
		emit modified();
	}

	onWaveFormNumberChanged();
}

void InstrumentEditorSSGForm::on_waveNumSpinBox_valueChanged(int arg1)
{
	if (!isIgnoreEvent_) {
		bt_.lock()->setInstrumentSSGWaveForm(instNum_, arg1);
		setInstrumentWaveFormParameters();
		emit waveFormNumberChanged();
		emit modified();
	}

	onWaveFormNumberChanged();
}

//--- Tone/Noise
int InstrumentEditorSSGForm::getToneNoiseNumber() const
{
	return ui->tnEditGroupBox->isChecked() ? ui->tnNumSpinBox->value() : -1;
}

void InstrumentEditorSSGForm::setInstrumentToneNoiseParameters()
{
	Ui::EventGuard ev(isIgnoreEvent_);

	std::unique_ptr<AbstructInstrument> inst = bt_.lock()->getInstrument(instNum_);
	auto instSSG = dynamic_cast<InstrumentSSG*>(inst.get());

	int num = instSSG->getToneNoiseNumber();
	if (num == -1) {
		ui->tnEditGroupBox->setChecked(false);
	}
	else {
		ui->tnEditGroupBox->setChecked(true);
		ui->tnNumSpinBox->setValue(num);
		onToneNoiseNumberChanged();
		ui->tnEditor->clearData();
		for (auto& com : instSSG->getToneNoiseSequence()) {
			ui->tnEditor->addSequenceCommand(com.type);
		}
		for (auto& l : instSSG->getToneNoiseLoops()) {
			ui->tnEditor->addLoop(l.begin, l.end, l.times);
		}
		ui->tnEditor->setRelease(convertReleaseTypeForUI(instSSG->getToneNoiseRelease().type),
								 instSSG->getToneNoiseRelease().begin);
	}
}

/********** Slots **********/
void InstrumentEditorSSGForm::onToneNoiseNumberChanged()
{
	// Change users view
	QString str;
	std::vector<int> users = bt_.lock()->getToneNoiseSSGUsers(ui->tnNumSpinBox->value());
	for (auto& n : users) {
		str += (QString::number(n) + ",");
	}
	str.chop(1);

	ui->tnUsersLineEdit->setText(str);
}

void InstrumentEditorSSGForm::onToneNoiseParameterChanged(int tnNum)
{
	if (ui->tnNumSpinBox->value() == tnNum) {
		Ui::EventGuard eg(isIgnoreEvent_);
		setInstrumentToneNoiseParameters();
	}
}

void InstrumentEditorSSGForm::on_tnEditGroupBox_toggled(bool arg1)
{
	if (!isIgnoreEvent_) {
		bt_.lock()->setInstrumentSSGToneNoise(instNum_, arg1 ? ui->tnNumSpinBox->value() : -1);
		setInstrumentToneNoiseParameters();
		emit toneNoiseNumberChanged();
		emit modified();
	}

	onToneNoiseNumberChanged();
}

void InstrumentEditorSSGForm::on_tnNumSpinBox_valueChanged(int arg1)
{
	if (!isIgnoreEvent_) {
		bt_.lock()->setInstrumentSSGToneNoise(instNum_, arg1);
		setInstrumentToneNoiseParameters();
		emit toneNoiseNumberChanged();
		emit modified();
	}

	onToneNoiseNumberChanged();
}

//--- Envelope
int InstrumentEditorSSGForm::getEnvelopeNumber() const
{
	return ui->envEditGroupBox->isChecked() ? ui->envNumSpinBox->value() : -1;
}

void InstrumentEditorSSGForm::setInstrumentEnvelopeParameters()
{
	Ui::EventGuard ev(isIgnoreEvent_);

	std::unique_ptr<AbstructInstrument> inst = bt_.lock()->getInstrument(instNum_);
	auto instSSG = dynamic_cast<InstrumentSSG*>(inst.get());

	int num = instSSG->getEnvelopeNumber();
	if (num == -1) {
		ui->envEditGroupBox->setChecked(false);
	}
	else {
		ui->envEditGroupBox->setChecked(true);
		ui->envNumSpinBox->setValue(num);
		onEnvelopeNumberChanged();
		ui->envEditor->clearData();
		for (auto& com : instSSG->getEnvelopeSequence()) {
			QString str = "";
			if (com.type >= 16) {
				str = QString("%1").arg(com.data);
			}
			ui->envEditor->addSequenceCommand(com.type, str, com.data);
		}
		for (auto& l : instSSG->getEnvelopeLoops()) {
			ui->envEditor->addLoop(l.begin, l.end, l.times);
		}
		ui->envEditor->setRelease(convertReleaseTypeForUI(instSSG->getEnvelopeRelease().type),
								  instSSG->getEnvelopeRelease().begin);
	}
}

/********** Slots **********/
void InstrumentEditorSSGForm::onEnvelopeNumberChanged()
{
	// Change users view
	QString str;
	std::vector<int> users = bt_.lock()->getEnvelopeSSGUsers(ui->envNumSpinBox->value());
	for (auto& n : users) {
		str += (QString::number(n) + ",");
	}
	str.chop(1);

	ui->envUsersLineEdit->setText(str);
}

void InstrumentEditorSSGForm::onEnvelopeParameterChanged(int envNum)
{
	if (ui->envNumSpinBox->value() == envNum) {
		Ui::EventGuard eg(isIgnoreEvent_);
		setInstrumentEnvelopeParameters();
	}
}

void InstrumentEditorSSGForm::on_envEditGroupBox_toggled(bool arg1)
{
	if (!isIgnoreEvent_) {
		bt_.lock()->setInstrumentSSGEnvelope(instNum_, arg1 ? ui->envNumSpinBox->value() : -1);
		setInstrumentEnvelopeParameters();
		emit envelopeNumberChanged();
		emit modified();
	}

	onEnvelopeNumberChanged();
}

void InstrumentEditorSSGForm::on_envNumSpinBox_valueChanged(int arg1)
{
	if (!isIgnoreEvent_) {
		bt_.lock()->setInstrumentSSGEnvelope(instNum_, arg1);
		setInstrumentEnvelopeParameters();
		emit envelopeNumberChanged();
		emit modified();
	}

	onEnvelopeNumberChanged();
}

void InstrumentEditorSSGForm::on_hardFreqSpinBox_valueChanged(int arg1)
{
	Q_UNUSED(arg1)

	ui->hardFreqSpinBox->setSuffix(
				QString(" (%1Hz)").arg(QString::number(7800.0 / ui->hardFreqSpinBox->value(), 'f', 4)));
}

//--- Arpeggio
int InstrumentEditorSSGForm::getArpeggioNumber() const
{
	return ui->arpEditGroupBox->isChecked() ? ui->arpNumSpinBox->value() : -1;
}

void InstrumentEditorSSGForm::setInstrumentArpeggioParameters()
{
	Ui::EventGuard ev(isIgnoreEvent_);

	std::unique_ptr<AbstructInstrument> inst = bt_.lock()->getInstrument(instNum_);
	auto instSSG = dynamic_cast<InstrumentSSG*>(inst.get());

	int num = instSSG->getArpeggioNumber();
	if (num == -1) {
		ui->arpEditGroupBox->setChecked(false);
	}
	else {
		ui->arpEditGroupBox->setChecked(true);
		ui->arpNumSpinBox->setValue(num);
		onArpeggioNumberChanged();
		ui->arpEditor->clearData();
		for (auto& com : instSSG->getArpeggioSequence()) {
			ui->arpEditor->addSequenceCommand(com.type);
		}
		for (auto& l : instSSG->getArpeggioLoops()) {
			ui->arpEditor->addLoop(l.begin, l.end, l.times);
		}
		ui->arpEditor->setRelease(convertReleaseTypeForUI(instSSG->getArpeggioRelease().type),
								  instSSG->getArpeggioRelease().begin);
		ui->arpTypeComboBox->setCurrentIndex(instSSG->getArpeggioType());
	}
}

/********** Slots **********/
void InstrumentEditorSSGForm::onArpeggioNumberChanged()
{
	// Change users view
	QString str;
	std::vector<int> users = bt_.lock()->getArpeggioSSGUsers(ui->arpNumSpinBox->value());
	for (auto& n : users) {
		str += (QString::number(n) + ",");
	}
	str.chop(1);

	ui->arpUsersLineEdit->setText(str);
}

void InstrumentEditorSSGForm::onArpeggioParameterChanged(int tnNum)
{
	if (ui->arpNumSpinBox->value() == tnNum) {
		Ui::EventGuard eg(isIgnoreEvent_);
		setInstrumentArpeggioParameters();
	}
}

void InstrumentEditorSSGForm::onArpeggioTypeChanged(int type)
{
	if (!isIgnoreEvent_) {
		bt_.lock()->setArpeggioSSGType(ui->arpNumSpinBox->value(), type);
		emit arpeggioParameterChanged(ui->arpNumSpinBox->value(), instNum_);
		emit modified();
	}

	// Update labels
	if (type == 1) {
		QString tn[] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
		for (int i = 0; i < 8; ++i) {
			for (int j = 0; j < 12; ++j) {
				ui->arpEditor->setLabel(i * 12 + j, tn[j] + QString::number(i));
			}
		}
	}
	else {
		for (int i = 0; i < 96; ++i) {
			int d = i - 48;
			auto text = QString::number(d);
			if (d > 0) text = "+" + text;
			ui->arpEditor->setLabel(i, text);
		}
	}
}

void InstrumentEditorSSGForm::on_arpEditGroupBox_toggled(bool arg1)
{
	if (!isIgnoreEvent_) {
		bt_.lock()->setInstrumentSSGArpeggio(instNum_, arg1 ? ui->arpNumSpinBox->value() : -1);
		setInstrumentArpeggioParameters();
		emit arpeggioNumberChanged();
		emit modified();
	}

	onArpeggioNumberChanged();
}

void InstrumentEditorSSGForm::on_arpNumSpinBox_valueChanged(int arg1)
{
	if (!isIgnoreEvent_) {
		bt_.lock()->setInstrumentSSGArpeggio(instNum_, arg1);
		setInstrumentArpeggioParameters();
		emit arpeggioNumberChanged();
		emit modified();
	}

	onArpeggioNumberChanged();
}

//--- Else
/********** Slots **********/
void InstrumentEditorSSGForm::on_gateCountSpinBox_valueChanged(int arg1)
{
	bt_.lock()->setInstrumentGateCount(instNum_, arg1);
	emit modified();
}
