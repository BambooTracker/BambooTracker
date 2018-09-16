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
			ReleaseType t;
			switch (type) {
			case VisualizedInstrumentMacroEditor::ReleaseType::NO_RELEASE:
				t = ReleaseType::NO_RELEASE;
				break;
			case VisualizedInstrumentMacroEditor::ReleaseType::FIX:
				t = ReleaseType::FIX;
				break;
			case VisualizedInstrumentMacroEditor::ReleaseType::ABSOLUTE:
				t = ReleaseType::ABSOLUTE;
				break;
			case VisualizedInstrumentMacroEditor::ReleaseType::RELATIVE:
				t = ReleaseType::RELATIVE;
				break;
			}
			bt_.lock()->setWaveFormSSGRelease(ui->waveNumSpinBox->value(), t, point);
			emit waveFormParameterChanged(ui->waveNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
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

void InstrumentEditorSSGForm::updateInstrumentParameters()
{
	Ui::EventGuard eg(isIgnoreEvent_);

	std::unique_ptr<AbstructInstrument> inst = bt_.lock()->getInstrument(instNum_);
	auto instSSG = dynamic_cast<InstrumentSSG*>(inst.get());
	auto name = QString::fromUtf8(instSSG->getName().c_str(), instSSG->getName().length());
	setWindowTitle(QString("%1: %2").arg(instNum_, 2, 16, QChar('0')).toUpper().arg(name));

	setInstrumentWaveFormParameters();

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
		ui->waveEditor->clear();
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
		VisualizedInstrumentMacroEditor::ReleaseType type;
		switch (instSSG->getWaveFormRelease().type) {
		case ReleaseType::NO_RELEASE:
			type = VisualizedInstrumentMacroEditor::ReleaseType::NO_RELEASE;
			break;
		case ReleaseType::FIX:
			type = VisualizedInstrumentMacroEditor::ReleaseType::FIX;
			break;
		case ReleaseType::ABSOLUTE:
			type = VisualizedInstrumentMacroEditor::ReleaseType::ABSOLUTE;
			break;
		case ReleaseType::RELATIVE:
			type = VisualizedInstrumentMacroEditor::ReleaseType::RELATIVE;
			break;
		}
		ui->waveEditor->setRelease(type, instSSG->getWaveFormRelease().begin);
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

//--- Else
/********** Slots **********/
void InstrumentEditorSSGForm::on_gateCountSpinBox_valueChanged(int arg1)
{
	bt_.lock()->setInstrumentGateCount(instNum_, arg1);
	emit modified();
}
