/*
 * Copyright (C) 2020-2021 Rerrah
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#include "instrument_editor_adpcm_form.hpp"
#include "ui_instrument_editor_adpcm_form.h"
#include <set>
#include "instrument.hpp"
#include "gui/event_guard.hpp"
#include "gui/jam_layout.hpp"
#include "gui/instrument_editor/instrument_editor_utils.hpp"
#include "gui/gui_utils.hpp"

InstrumentEditorADPCMForm::InstrumentEditorADPCMForm(int num, QWidget *parent) :
	QWidget(parent),
	ui(new Ui::InstrumentEditorADPCMForm),
	instNum_(num),
	isIgnoreEvent_(false)
{
	ui->setupUi(this);

	//========== Sample ==========//
	QObject::connect(ui->sampleEditor, &ADPCMSampleEditor::modified, this, [&] { emit modified(); });
	QObject::connect(ui->sampleEditor, &ADPCMSampleEditor::sampleNumberChanged,
					 this, [&](int n) {
		bt_.lock()->setInstrumentADPCMSample(instNum_, n);
		setInstrumentSampleParameters();
		emit sampleNumberChanged();
		emit modified();

		if (config_.lock()->getWriteOnlyUsedSamples()) {
			emit sampleAssignRequested();
		}
	}, Qt::DirectConnection);
	QObject::connect(ui->sampleEditor, &ADPCMSampleEditor::sampleParameterChanged,
					 this, [&](int sampNum) {
		emit sampleParameterChanged(sampNum, instNum_);
	});
	QObject::connect(ui->sampleEditor, &ADPCMSampleEditor::sampleAssignRequested,
					 this, [&] { emit sampleAssignRequested(); });
	QObject::connect(ui->sampleEditor, &ADPCMSampleEditor::sampleMemoryChanged,
					 this, [&] { emit sampleMemoryChanged(); });

	//========== Envelope ==========//
	ui->envEditor->setMaximumDisplayedRowCount(64);
	ui->envEditor->setDefaultRow(255);
	ui->envEditor->setLabelDiaplayMode(true);
	for (int i = 0; i < 256; ++i) {
		ui->envEditor->AddRow(QString::number(i), false);
	}
	ui->envEditor->autoFitLabelWidth();
	ui->envEditor->setUpperRow(255);
	ui->envEditor->setMultipleReleaseState(true);
	ui->envEditor->setPermittedReleaseTypes(
				VisualizedInstrumentMacroEditor::PermittedReleaseFlag::ABSOLUTE_RELEASE
				| VisualizedInstrumentMacroEditor::PermittedReleaseFlag::RELATIVE_RELEASE
				| VisualizedInstrumentMacroEditor::PermittedReleaseFlag::FIXED_RELEASE);

	QObject::connect(ui->envEditor, &VisualizedInstrumentMacroEditor::sequenceDataAdded,
					 this, [&](int row, int) {
		if (!isIgnoreEvent_) {
			bt_.lock()->addEnvelopeADPCMSequenceData(ui->envNumSpinBox->value(), row);
			emit envelopeParameterChanged(ui->envNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->envEditor, &VisualizedInstrumentMacroEditor::sequenceDataRemoved,
					 this, [&] {
		if (!isIgnoreEvent_) {
			bt_.lock()->removeEnvelopeADPCMSequenceData(ui->envNumSpinBox->value());
			emit envelopeParameterChanged(ui->envNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->envEditor, &VisualizedInstrumentMacroEditor::sequenceDataChanged,
					 this, [&](int row, int col) {
		if (!isIgnoreEvent_) {
			bt_.lock()->setEnvelopeADPCMSequenceData(ui->envNumSpinBox->value(), col, row);
			emit envelopeParameterChanged(ui->envNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->envEditor, &VisualizedInstrumentMacroEditor::loopAdded,
					 this, [&](InstrumentSequenceLoop loop) {
		if (!isIgnoreEvent_) {
			bt_.lock()->addEnvelopeADPCMLoop(ui->envNumSpinBox->value(), loop);
			emit envelopeParameterChanged(ui->envNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->envEditor, &VisualizedInstrumentMacroEditor::loopRemoved,
					 this, [&](int begin, int end) {
		if (!isIgnoreEvent_) {
			bt_.lock()->removeEnvelopeADPCMLoop(ui->envNumSpinBox->value(), begin, end);
			emit envelopeParameterChanged(ui->envNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->envEditor, &VisualizedInstrumentMacroEditor::loopCleared,
					 this, [&] {
		if (!isIgnoreEvent_) {
			bt_.lock()->clearEnvelopeADPCMLoops(ui->envNumSpinBox->value());
			emit envelopeParameterChanged(ui->envNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->envEditor, &VisualizedInstrumentMacroEditor::loopChanged,
					 this, [&](int prevBegin, int prevEnd, InstrumentSequenceLoop loop) {
		if (!isIgnoreEvent_) {
			bt_.lock()->changeEnvelopeADPCMLoop(ui->envNumSpinBox->value(), prevBegin, prevEnd, loop);
			emit envelopeParameterChanged(ui->envNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->envEditor, &VisualizedInstrumentMacroEditor::releaseChanged,
					 this, [&](InstrumentSequenceRelease release) {
		if (!isIgnoreEvent_) {
			bt_.lock()->setEnvelopeADPCMRelease(ui->envNumSpinBox->value(), release);
			emit envelopeParameterChanged(ui->envNumSpinBox->value(), instNum_);
			emit modified();
		}
	});

	//========== Arpeggio ==========//
	ui->arpTypeComboBox->addItem(tr("Absolute"), static_cast<int>(SequenceType::AbsoluteSequence));
	ui->arpTypeComboBox->addItem(tr("Fixed"), static_cast<int>(SequenceType::FixedSequence));
	ui->arpTypeComboBox->addItem(tr("Relative"), static_cast<int>(SequenceType::RelativeSequence));

	QObject::connect(ui->arpEditor, &VisualizedInstrumentMacroEditor::sequenceDataAdded,
					 this, [&](int row, int) {
		if (!isIgnoreEvent_) {
			bt_.lock()->addArpeggioADPCMSequenceData(ui->arpNumSpinBox->value(), row);
			emit arpeggioParameterChanged(ui->arpNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->arpEditor, &VisualizedInstrumentMacroEditor::sequenceDataRemoved,
					 this, [&] {
		if (!isIgnoreEvent_) {
			bt_.lock()->removeArpeggioADPCMSequenceData(ui->arpNumSpinBox->value());
			emit arpeggioParameterChanged(ui->arpNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->arpEditor, &VisualizedInstrumentMacroEditor::sequenceDataChanged,
					 this, [&](int row, int col) {
		if (!isIgnoreEvent_) {
			bt_.lock()->setArpeggioADPCMSequenceData(ui->arpNumSpinBox->value(), col, row);
			emit arpeggioParameterChanged(ui->arpNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->arpEditor, &VisualizedInstrumentMacroEditor::loopAdded,
					 this, [&](InstrumentSequenceLoop loop) {
		if (!isIgnoreEvent_) {
			bt_.lock()->addArpeggioADPCMLoop(ui->arpNumSpinBox->value(), loop);
			emit arpeggioParameterChanged(ui->arpNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->arpEditor, &VisualizedInstrumentMacroEditor::loopRemoved,
					 this, [&](int begin, int end) {
		if (!isIgnoreEvent_) {
			bt_.lock()->removeArpeggioADPCMLoop(ui->arpNumSpinBox->value(), begin, end);
			emit arpeggioParameterChanged(ui->arpNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->arpEditor, &VisualizedInstrumentMacroEditor::loopCleared,
					 this, [&] {
		if (!isIgnoreEvent_) {
			bt_.lock()->clearArpeggioADPCMLoops(ui->arpNumSpinBox->value());
			emit arpeggioParameterChanged(ui->arpNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->arpEditor, &VisualizedInstrumentMacroEditor::loopChanged,
					 this, [&](int prevBegin, int prevEnd, InstrumentSequenceLoop loop) {
		if (!isIgnoreEvent_) {
			bt_.lock()->changeArpeggioADPCMLoop(ui->arpNumSpinBox->value(), prevBegin, prevEnd, loop);
			emit arpeggioParameterChanged(ui->arpNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->arpEditor, &VisualizedInstrumentMacroEditor::releaseChanged,
					 this, [&](InstrumentSequenceRelease release) {
		if (!isIgnoreEvent_) {
			bt_.lock()->setArpeggioADPCMRelease(ui->arpNumSpinBox->value(), release);
			emit arpeggioParameterChanged(ui->arpNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	// Leave Before Qt5.7.0 style due to windows xp
	QObject::connect(ui->arpTypeComboBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
					 this, &InstrumentEditorADPCMForm::onArpeggioTypeChanged);

	//========== Pitch ==========//
	ui->ptEditor->setMaximumDisplayedRowCount(15);
	ui->ptEditor->setDefaultRow(127);
	ui->ptEditor->setLabelDiaplayMode(true);
	for (int i = 0; i < 255; ++i) {
		ui->ptEditor->AddRow(QString::asprintf("%+d", i - 127), false);
	}
	ui->ptEditor->autoFitLabelWidth();
	ui->ptEditor->setUpperRow(134);
	ui->ptEditor->setMMLDisplay0As(-127);

	ui->ptTypeComboBox->addItem(tr("Absolute"), static_cast<int>(SequenceType::AbsoluteSequence));
	ui->ptTypeComboBox->addItem(tr("Relative"), static_cast<int>(SequenceType::RelativeSequence));

	QObject::connect(ui->ptEditor, &VisualizedInstrumentMacroEditor::sequenceDataAdded,
					 this, [&](int row, int) {
		if (!isIgnoreEvent_) {
			bt_.lock()->addPitchADPCMSequenceData(ui->ptNumSpinBox->value(), row);
			emit pitchParameterChanged(ui->ptNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->ptEditor, &VisualizedInstrumentMacroEditor::sequenceDataRemoved,
					 this, [&] {
		if (!isIgnoreEvent_) {
			bt_.lock()->removePitchADPCMSequenceData(ui->ptNumSpinBox->value());
			emit pitchParameterChanged(ui->ptNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->ptEditor, &VisualizedInstrumentMacroEditor::sequenceDataChanged,
					 this, [&](int row, int col) {
		if (!isIgnoreEvent_) {
			bt_.lock()->setPitchADPCMSequenceData(ui->ptNumSpinBox->value(), col, row);
			emit pitchParameterChanged(ui->ptNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->ptEditor, &VisualizedInstrumentMacroEditor::loopAdded,
					 this, [&](InstrumentSequenceLoop loop) {
		if (!isIgnoreEvent_) {
			bt_.lock()->addPitchADPCMLoop(ui->ptNumSpinBox->value(), loop);
			emit pitchParameterChanged(ui->ptNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->ptEditor, &VisualizedInstrumentMacroEditor::loopRemoved,
					 this, [&](int begin, int end) {
		if (!isIgnoreEvent_) {
			bt_.lock()->removePitchADPCMLoop(ui->ptNumSpinBox->value(), begin, end);
			emit pitchParameterChanged(ui->ptNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->ptEditor, &VisualizedInstrumentMacroEditor::loopCleared,
					 this, [&] {
		if (!isIgnoreEvent_) {
			bt_.lock()->clearPitchADPCMLoops(ui->ptNumSpinBox->value());
			emit pitchParameterChanged(ui->ptNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->ptEditor, &VisualizedInstrumentMacroEditor::loopChanged,
					 this, [&](int prevBegin, int prevEnd, InstrumentSequenceLoop loop) {
		if (!isIgnoreEvent_) {
			bt_.lock()->changePitchADPCMLoop(ui->ptNumSpinBox->value(), prevBegin, prevEnd, loop);
			emit pitchParameterChanged(ui->ptNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->ptEditor, &VisualizedInstrumentMacroEditor::releaseChanged,
					 this, [&](InstrumentSequenceRelease release) {
		if (!isIgnoreEvent_) {
			bt_.lock()->setPitchADPCMRelease(ui->ptNumSpinBox->value(), release);
			emit pitchParameterChanged(ui->ptNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	// Leave Before Qt5.7.0 style due to windows xp
	QObject::connect(ui->ptTypeComboBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
					 this, &InstrumentEditorADPCMForm::onPitchTypeChanged);
}

InstrumentEditorADPCMForm::~InstrumentEditorADPCMForm()
{
	delete ui;
}

void InstrumentEditorADPCMForm::setInstrumentNumber(int num)
{
	instNum_ = num;
}

int InstrumentEditorADPCMForm::getInstrumentNumber() const
{
	return instNum_;
}

void InstrumentEditorADPCMForm::setCore(std::weak_ptr<BambooTracker> core)
{
	bt_ = core;
	ui->sampleEditor->setCore(core);
	updateInstrumentParameters();
}

void InstrumentEditorADPCMForm::setConfiguration(std::weak_ptr<Configuration> config)
{
	config_ = config;
	ui->sampleEditor->setConfiguration(config);
}

void InstrumentEditorADPCMForm::updateConfigurationForDisplay()
{
	ui->arpEditor->onNoteNamesUpdated();
}

void InstrumentEditorADPCMForm::setColorPalette(std::shared_ptr<ColorPalette> palette)
{
	palette_ = palette;
	ui->sampleEditor->setColorPalette(palette);
	ui->envEditor->setColorPalette(palette);
	ui->arpEditor->setColorPalette(palette);
	ui->ptEditor->setColorPalette(palette);
}

void InstrumentEditorADPCMForm::updateInstrumentParameters()
{
	Ui::EventGuard eg(isIgnoreEvent_);

	auto name = gui_utils::utf8ToQString(bt_.lock()->getInstrument(instNum_)->getName());
	setWindowTitle(QString("%1: %2").arg(instNum_, 2, 16, QChar('0')).toUpper().arg(name));

	setInstrumentSampleParameters();
	setInstrumentEnvelopeParameters();
	setInstrumentArpeggioParameters();
	setInstrumentPitchParameters();
}

/********** Events **********/
// MUST DIRECT CONNECTION
void InstrumentEditorADPCMForm::keyPressEvent(QKeyEvent *event)
{
	// General keys
	switch (event->key()) {
	case Qt::Key_Escape:
		close();
		break;
	default:
		// For jam key on
		if (!event->isAutoRepeat()) {
			// Musical keyboard
			Qt::Key qtKey = static_cast<Qt::Key>(event->key());
			try {
				JamKey jk = getJamKeyFromLayoutMapping(qtKey, config_);
				emit jamKeyOnEvent(jk);
			} catch (std::invalid_argument&) {}
		}
		break;
	}
}

// MUST DIRECT CONNECTION
void InstrumentEditorADPCMForm::keyReleaseEvent(QKeyEvent *event)
{
	// For jam key off
	if (!event->isAutoRepeat()) {
		Qt::Key qtKey = static_cast<Qt::Key>(event->key());
		try {
			JamKey jk = getJamKeyFromLayoutMapping(qtKey, config_);
			emit jamKeyOffEvent(jk);
		} catch (std::invalid_argument&) {}
	}
}

//--- Sample
void InstrumentEditorADPCMForm::setInstrumentSampleParameters()
{
	std::unique_ptr<AbstractInstrument> inst = bt_.lock()->getInstrument(instNum_);
	auto instADPCM = dynamic_cast<InstrumentADPCM*>(inst.get());

	ui->sampleEditor->setInstrumentSampleParameters(
				instADPCM->getSampleNumber(), instADPCM->isSampleRepeatable(),
				instADPCM->getSampleRootKeyNumber(), instADPCM->getSampleRootDeltaN(),
				instADPCM->getSampleStartAddress(), instADPCM->getSampleStopAddress(),
				instADPCM->getRawSample());
}

/********** Slots **********/
void InstrumentEditorADPCMForm::onSampleNumberChanged()
{
	ui->sampleEditor->onSampleNumberChanged();
}

void InstrumentEditorADPCMForm::onSampleParameterChanged(int sampNum)
{
	if (ui->sampleEditor->getSampleNumber() == sampNum) {
		setInstrumentSampleParameters();
	}
}

void InstrumentEditorADPCMForm::onSampleMemoryUpdated()
{
	std::unique_ptr<AbstractInstrument> inst = bt_.lock()->getInstrument(instNum_);
	auto instADPCM = dynamic_cast<InstrumentADPCM*>(inst.get());

	ui->sampleEditor->onSampleMemoryUpdated(instADPCM->getSampleStartAddress(), instADPCM->getSampleStopAddress());
}

//--- Envelope
void InstrumentEditorADPCMForm::setInstrumentEnvelopeParameters()
{
	Ui::EventGuard ev(isIgnoreEvent_);

	std::unique_ptr<AbstractInstrument> inst = bt_.lock()->getInstrument(instNum_);
	auto instADPCM = dynamic_cast<InstrumentADPCM*>(inst.get());

	ui->envNumSpinBox->setValue(instADPCM->getEnvelopeNumber());
	ui->envEditor->clearData();
	for (auto& unit : instADPCM->getEnvelopeSequence()) {
		ui->envEditor->addSequenceData(unit.data);
	}
	for (auto& loop : instADPCM->getEnvelopeLoopRoot().getAllLoops()) {
		ui->envEditor->addLoop(loop.getBeginPos(), loop.getEndPos(), loop.getTimes());
	}
	ui->envEditor->setRelease(instADPCM->getEnvelopeRelease());
	if (instADPCM->getEnvelopeEnabled()) {
		ui->envEditGroupBox->setChecked(true);
		onEnvelopeNumberChanged();
	}
	else {
		ui->envEditGroupBox->setChecked(false);
	}
}

/********** Slots **********/
void InstrumentEditorADPCMForm::onEnvelopeNumberChanged()
{
	// Change users view
	std::multiset<int> users = bt_.lock()->getEnvelopeADPCMUsers(ui->envNumSpinBox->value());
	ui->envUsersLineEdit->setText(inst_edit_utils::generateUsersString(users));
}

void InstrumentEditorADPCMForm::onEnvelopeParameterChanged(int envNum)
{
	if (ui->envNumSpinBox->value() == envNum) {
		Ui::EventGuard eg(isIgnoreEvent_);
		setInstrumentEnvelopeParameters();
	}
}

void InstrumentEditorADPCMForm::on_envEditGroupBox_toggled(bool arg1)
{
	if (!isIgnoreEvent_) {
		bt_.lock()->setInstrumentADPCMEnvelopeEnabled(instNum_, arg1);
		setInstrumentEnvelopeParameters();
		emit envelopeNumberChanged();
		emit modified();
	}

	onEnvelopeNumberChanged();
}

void InstrumentEditorADPCMForm::on_envNumSpinBox_valueChanged(int arg1)
{
	if (!isIgnoreEvent_) {
		bt_.lock()->setInstrumentADPCMEnvelope(instNum_, arg1);
		setInstrumentEnvelopeParameters();
		emit envelopeNumberChanged();
		emit modified();
	}

	onEnvelopeNumberChanged();
}

//--- Arpeggio
void InstrumentEditorADPCMForm::setInstrumentArpeggioParameters()
{
	Ui::EventGuard ev(isIgnoreEvent_);

	std::unique_ptr<AbstractInstrument> inst = bt_.lock()->getInstrument(instNum_);
	auto instADPCM = dynamic_cast<InstrumentADPCM*>(inst.get());

	ui->arpNumSpinBox->setValue(instADPCM->getArpeggioNumber());
	ui->arpEditor->clearData();
	for (auto& unit : instADPCM->getArpeggioSequence()) {
		ui->arpEditor->addSequenceData(unit.data);
	}
	for (auto& loop : instADPCM->getArpeggioLoopRoot().getAllLoops()) {
		ui->arpEditor->addLoop(loop.getBeginPos(), loop.getEndPos(), loop.getTimes());
	}
	ui->arpEditor->setRelease(instADPCM->getArpeggioRelease());
	for (int i = 0; i < ui->arpTypeComboBox->count(); ++i) {
		if (instADPCM->getArpeggioType() == static_cast<SequenceType>(ui->arpTypeComboBox->itemData(i).toInt())) {
			ui->arpTypeComboBox->setCurrentIndex(i);
			break;
		}
	}
	if (instADPCM->getArpeggioEnabled()) {
		ui->arpEditGroupBox->setChecked(true);
		onArpeggioNumberChanged();
	}
	else {
		ui->arpEditGroupBox->setChecked(false);
	}
}

/********** Slots **********/
void InstrumentEditorADPCMForm::onArpeggioNumberChanged()
{
	// Change users view
	std::multiset<int> users = bt_.lock()->getArpeggioADPCMUsers(ui->arpNumSpinBox->value());
	ui->arpUsersLineEdit->setText(inst_edit_utils::generateUsersString(users));
}

void InstrumentEditorADPCMForm::onArpeggioParameterChanged(int tnNum)
{
	if (ui->arpNumSpinBox->value() == tnNum) {
		Ui::EventGuard eg(isIgnoreEvent_);
		setInstrumentArpeggioParameters();
	}
}

void InstrumentEditorADPCMForm::onArpeggioTypeChanged(int index)
{
	Q_UNUSED(index)

	auto type = static_cast<SequenceType>(ui->arpTypeComboBox->currentData(Qt::UserRole).toInt());
	if (!isIgnoreEvent_) {
		bt_.lock()->setArpeggioADPCMType(ui->arpNumSpinBox->value(), type);
		emit arpeggioParameterChanged(ui->arpNumSpinBox->value(), instNum_);
		emit modified();
	}

	ui->arpEditor->setSequenceType(type);
}

void InstrumentEditorADPCMForm::on_arpEditGroupBox_toggled(bool arg1)
{
	if (!isIgnoreEvent_) {
		bt_.lock()->setInstrumentADPCMArpeggioEnabled(instNum_, arg1);
		setInstrumentArpeggioParameters();
		emit arpeggioNumberChanged();
		emit modified();
	}

	onArpeggioNumberChanged();
}

void InstrumentEditorADPCMForm::on_arpNumSpinBox_valueChanged(int arg1)
{
	if (!isIgnoreEvent_) {
		bt_.lock()->setInstrumentADPCMArpeggio(instNum_, arg1);
		setInstrumentArpeggioParameters();
		emit arpeggioNumberChanged();
		emit modified();
	}

	onArpeggioNumberChanged();
}

//--- Pitch
void InstrumentEditorADPCMForm::setInstrumentPitchParameters()
{
	Ui::EventGuard ev(isIgnoreEvent_);

	std::unique_ptr<AbstractInstrument> inst = bt_.lock()->getInstrument(instNum_);
	auto instADPCM = dynamic_cast<InstrumentADPCM*>(inst.get());

	ui->ptNumSpinBox->setValue(instADPCM->getPitchNumber());
	ui->ptEditor->clearData();
	for (auto& unit : instADPCM->getPitchSequence()) {
		ui->ptEditor->addSequenceData(unit.data);
	}
	for (auto& loop : instADPCM->getPitchLoopRoot().getAllLoops()) {
		ui->ptEditor->addLoop(loop.getBeginPos(), loop.getEndPos(), loop.getTimes());
	}
	ui->ptEditor->setRelease(instADPCM->getPitchRelease());
	for (int i = 0; i < ui->ptTypeComboBox->count(); ++i) {
		if (instADPCM->getPitchType() == static_cast<SequenceType>(ui->ptTypeComboBox->itemData(i).toInt())) {
			ui->ptTypeComboBox->setCurrentIndex(i);
			break;
		}
	}
	if (instADPCM->getPitchEnabled()) {
		ui->ptEditGroupBox->setChecked(true);
		onPitchNumberChanged();
	}
	else {
		ui->ptEditGroupBox->setChecked(false);
	}
}

/********** Slots **********/
void InstrumentEditorADPCMForm::onPitchNumberChanged()
{
	// Change users view
	std::multiset<int> users = bt_.lock()->getPitchADPCMUsers(ui->ptNumSpinBox->value());
	ui->ptUsersLineEdit->setText(inst_edit_utils::generateUsersString(users));
}

void InstrumentEditorADPCMForm::onPitchParameterChanged(int tnNum)
{
	if (ui->ptNumSpinBox->value() == tnNum) {
		Ui::EventGuard eg(isIgnoreEvent_);
		setInstrumentPitchParameters();
	}
}

void InstrumentEditorADPCMForm::onPitchTypeChanged(int index)
{
	Q_UNUSED(index)

	auto type = static_cast<SequenceType>(ui->ptTypeComboBox->currentData(Qt::UserRole).toInt());
	if (!isIgnoreEvent_) {
		bt_.lock()->setPitchADPCMType(ui->ptNumSpinBox->value(), type);
		emit pitchParameterChanged(ui->ptNumSpinBox->value(), instNum_);
		emit modified();
	}

	ui->ptEditor->setSequenceType(type);
}

void InstrumentEditorADPCMForm::on_ptEditGroupBox_toggled(bool arg1)
{
	if (!isIgnoreEvent_) {
		bt_.lock()->setInstrumentADPCMPitchEnabled(instNum_, arg1);
		setInstrumentPitchParameters();
		emit pitchNumberChanged();
		emit modified();
	}

	onPitchNumberChanged();
}

void InstrumentEditorADPCMForm::on_ptNumSpinBox_valueChanged(int arg1)
{
	if (!isIgnoreEvent_) {
		bt_.lock()->setInstrumentADPCMPitch(instNum_, arg1);
		setInstrumentPitchParameters();
		emit pitchNumberChanged();
		emit modified();
	}

	onPitchNumberChanged();
}
