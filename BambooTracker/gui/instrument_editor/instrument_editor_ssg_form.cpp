/*
 * Copyright (C) 2018-2021 Rerrah
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

#include "instrument_editor_ssg_form.hpp"
#include "ui_instrument_editor_ssg_form.h"
#include <vector>
#include <set>
#include <utility>
#include <stdexcept>
#include "gui/event_guard.hpp"
#include "note.hpp"
#include "sequence_property.hpp"
#include "gui/jam_layout.hpp"
#include "gui/instrument_editor/instrument_editor_utils.hpp"
#include "gui/gui_utils.hpp"

namespace
{
bool isModulatedWaveformSSG(int type)
{
	switch (type) {
	case SSGWaveformType::SQUARE:
	case SSGWaveformType::TRIANGLE:
	case SSGWaveformType::SAW:
	case SSGWaveformType::INVSAW:
		return false;
	case SSGWaveformType::SQM_TRIANGLE:
	case SSGWaveformType::SQM_SAW:
	case SSGWaveformType::SQM_INVSAW:
		return true;
	default:
		throw std::invalid_argument("Invalid SSGWaveformType");
	}
}
}

InstrumentEditorSSGForm::InstrumentEditorSSGForm(int num, QWidget *parent) :
	QWidget(parent),
	ui(new Ui::InstrumentEditorSSGForm),
	instNum_(num)
{
	ui->setupUi(this);

	//========== Waveform ==========//
	ui->waveEditor->setMaximumDisplayedRowCount(7);
	ui->waveEditor->setDefaultRow(0);
	ui->waveEditor->AddRow(tr("Sq"), false);
	ui->waveEditor->AddRow(tr("Tri"), false);
	ui->waveEditor->AddRow(tr("Saw"), false);
	ui->waveEditor->AddRow(tr("InvSaw"), false);
	ui->waveEditor->AddRow(tr("SMTri"), false);
	ui->waveEditor->AddRow(tr("SMSaw"), false);
	ui->waveEditor->AddRow(tr("SMInvSaw"), false);
	ui->waveEditor->autoFitLabelWidth();

	QObject::connect(ui->waveEditor, &VisualizedInstrumentMacroEditor::sequenceDataAdded,
					 this, [&](int row, int col) {
		if (!isIgnoreEvent_) {
			if (isModulatedWaveformSSG(row)) {
				SSGWaveformUnit data = setWaveformSequenceColumn(col, row);	// Set square-mask frequency
				bt_.lock()->addWaveformSSGSequenceData(ui->waveNumSpinBox->value(), data);
			}
			else {
				bt_.lock()->addWaveformSSGSequenceData(ui->waveNumSpinBox->value(), SSGWaveformUnit::makeOnlyDataUnit(row));
			}
			emit waveformParameterChanged(ui->waveNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->waveEditor, &VisualizedInstrumentMacroEditor::sequenceDataRemoved,
					 this, [&] {
		if (!isIgnoreEvent_) {
			bt_.lock()->removeWaveformSSGSequenceData(ui->waveNumSpinBox->value());
			emit waveformParameterChanged(ui->waveNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->waveEditor, &VisualizedInstrumentMacroEditor::sequenceDataChanged,
					 this, [&](int row, int col) {
		if (!isIgnoreEvent_) {
			if (isModulatedWaveformSSG(row)) {
				SSGWaveformUnit data = setWaveformSequenceColumn(col, row);	// Set square-mask frequency
				bt_.lock()->setWaveformSSGSequenceData(ui->waveNumSpinBox->value(), col, data);
			}
			else {
				bt_.lock()->setWaveformSSGSequenceData(ui->waveNumSpinBox->value(), col, SSGWaveformUnit::makeOnlyDataUnit(row));
			}
			emit waveformParameterChanged(ui->waveNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->waveEditor, &VisualizedInstrumentMacroEditor::loopAdded,
					 this, [&](InstrumentSequenceLoop loop) {
		if (!isIgnoreEvent_) {
			bt_.lock()->addWaveformSSGLoop(ui->waveNumSpinBox->value(), loop);
			emit waveformParameterChanged(ui->waveNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->waveEditor, &VisualizedInstrumentMacroEditor::loopRemoved,
					 this, [&](int begin, int end) {
		if (!isIgnoreEvent_) {
			bt_.lock()->removeWaveformSSGLoop(ui->waveNumSpinBox->value(), begin, end);
			emit waveformParameterChanged(ui->waveNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->waveEditor, &VisualizedInstrumentMacroEditor::loopCleared,
					 this, [&] {
		if (!isIgnoreEvent_) {
			bt_.lock()->clearWaveformSSGLoops(ui->waveNumSpinBox->value());
			emit waveformParameterChanged(ui->waveNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->waveEditor, &VisualizedInstrumentMacroEditor::loopChanged,
					 this, [&](int prevBegin, int prevEnd, InstrumentSequenceLoop loop) {
		if (!isIgnoreEvent_) {
			bt_.lock()->changeWaveformSSGLoop(ui->waveNumSpinBox->value(), prevBegin, prevEnd, loop);
			emit waveformParameterChanged(ui->waveNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->waveEditor, &VisualizedInstrumentMacroEditor::releaseChanged,
					 this, [&](InstrumentSequenceRelease release) {
		if (!isIgnoreEvent_) {
			bt_.lock()->setWaveformSSGRelease(ui->waveNumSpinBox->value(), release);
			emit waveformParameterChanged(ui->waveNumSpinBox->value(), instNum_);
			emit modified();
		}
	});

	//========== Tone/Noise ==========//
	QObject::connect(ui->tnEditor, &VisualizedInstrumentMacroEditor::sequenceDataAdded,
					 this, [&](int row, int) {
		if (!isIgnoreEvent_) {
			bt_.lock()->addToneNoiseSSGSequenceData(ui->tnNumSpinBox->value(), row);
			emit toneNoiseParameterChanged(ui->tnNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->tnEditor, &VisualizedInstrumentMacroEditor::sequenceDataRemoved,
					 this, [&] {
		if (!isIgnoreEvent_) {
			bt_.lock()->removeToneNoiseSSGSequenceData(ui->tnNumSpinBox->value());
			emit toneNoiseParameterChanged(ui->tnNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->tnEditor, &VisualizedInstrumentMacroEditor::sequenceDataChanged,
					 this, [&](int row, int col) {
		if (!isIgnoreEvent_) {
			bt_.lock()->setToneNoiseSSGSequenceData(ui->tnNumSpinBox->value(), col, row);
			emit toneNoiseParameterChanged(ui->tnNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->tnEditor, &VisualizedInstrumentMacroEditor::loopAdded,
					 this, [&](InstrumentSequenceLoop loop) {
		if (!isIgnoreEvent_) {
			bt_.lock()->addToneNoiseSSGLoop(ui->tnNumSpinBox->value(), loop);
			emit toneNoiseParameterChanged(ui->tnNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->tnEditor, &VisualizedInstrumentMacroEditor::loopRemoved,
					 this, [&](int begin, int end) {
		if (!isIgnoreEvent_) {
			bt_.lock()->removeToneNoiseSSGLoop(ui->tnNumSpinBox->value(), begin, end);
			emit toneNoiseParameterChanged(ui->tnNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->tnEditor, &VisualizedInstrumentMacroEditor::loopCleared,
					 this, [&] {
		if (!isIgnoreEvent_) {
			bt_.lock()->clearToneNoiseSSGLoops(ui->tnNumSpinBox->value());
			emit toneNoiseParameterChanged(ui->tnNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->tnEditor, &VisualizedInstrumentMacroEditor::loopChanged,
					 this, [&](int prevBegin, int prevEnd, InstrumentSequenceLoop loop) {
		if (!isIgnoreEvent_) {
			bt_.lock()->changeToneNoiseSSGLoop(ui->tnNumSpinBox->value(), prevBegin, prevEnd, loop);
			emit toneNoiseParameterChanged(ui->tnNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->tnEditor, &VisualizedInstrumentMacroEditor::releaseChanged,
					 this, [&](InstrumentSequenceRelease release) {
		if (!isIgnoreEvent_) {
			bt_.lock()->setToneNoiseSSGRelease(ui->tnNumSpinBox->value(), release);
			emit toneNoiseParameterChanged(ui->tnNumSpinBox->value(), instNum_);
			emit modified();
		}
	});

	//========== Envelope ==========//
	ui->envEditor->setMaximumDisplayedRowCount(16);
	ui->envEditor->setDefaultRow(15);
	for (int i = 0; i < 16; ++i) {
		ui->envEditor->AddRow(QString::number(i), false);
	}
	for (int i = 0; i < 8; ++i) {
		ui->envEditor->AddRow(tr("HEnv %1").arg(i), false);
	}
	ui->envEditor->autoFitLabelWidth();
	ui->envEditor->setMultipleReleaseState(true);
	ui->envEditor->setPermittedReleaseTypes(
				VisualizedInstrumentMacroEditor::PermittedReleaseFlag::ABSOLUTE_RELEASE
				| VisualizedInstrumentMacroEditor::PermittedReleaseFlag::RELATIVE_RELEASE
				| VisualizedInstrumentMacroEditor::PermittedReleaseFlag::FIXED_RELEASE);

	QObject::connect(ui->envEditor, &VisualizedInstrumentMacroEditor::sequenceDataAdded,
					 this, [&](int row, int col) {
		if (!isIgnoreEvent_) {
			if (row >= 16) {
				SSGEnvelopeUnit data = setEnvelopeSequenceColumn(col, row);	// Set hard frequency
				bt_.lock()->addEnvelopeSSGSequenceData(ui->envNumSpinBox->value(), data);
			}
			else {
				bt_.lock()->addEnvelopeSSGSequenceData(ui->envNumSpinBox->value(), SSGEnvelopeUnit::makeOnlyDataUnit(row));
			}
			emit envelopeParameterChanged(ui->envNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->envEditor, &VisualizedInstrumentMacroEditor::sequenceDataRemoved,
					 this, [&] {
		if (!isIgnoreEvent_) {
			bt_.lock()->removeEnvelopeSSGSequenceData(ui->envNumSpinBox->value());
			emit envelopeParameterChanged(ui->envNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->envEditor, &VisualizedInstrumentMacroEditor::sequenceDataChanged,
					 this, [&](int row, int col) {
		if (!isIgnoreEvent_) {
			if (row >= 16) {
				SSGEnvelopeUnit data = setEnvelopeSequenceColumn(col, row);	// Set hard frequency
				bt_.lock()->setEnvelopeSSGSequenceData(ui->envNumSpinBox->value(), col, data);
			}
			else {
				bt_.lock()->setEnvelopeSSGSequenceData(ui->envNumSpinBox->value(), col, SSGEnvelopeUnit::makeOnlyDataUnit(row));
			}
			emit envelopeParameterChanged(ui->envNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->envEditor, &VisualizedInstrumentMacroEditor::loopAdded,
					 this, [&](InstrumentSequenceLoop loop) {
		if (!isIgnoreEvent_) {
			bt_.lock()->addEnvelopeSSGLoop(ui->envNumSpinBox->value(), loop);
			emit envelopeParameterChanged(ui->envNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->envEditor, &VisualizedInstrumentMacroEditor::loopRemoved,
					 this, [&](int begin, int end) {
		if (!isIgnoreEvent_) {
			bt_.lock()->removeEnvelopeSSGLoop(ui->envNumSpinBox->value(), begin, end);
			emit envelopeParameterChanged(ui->envNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->envEditor, &VisualizedInstrumentMacroEditor::loopCleared,
					 this, [&] {
		if (!isIgnoreEvent_) {
			bt_.lock()->clearEnvelopeSSGLoops(ui->envNumSpinBox->value());
			emit envelopeParameterChanged(ui->envNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->envEditor, &VisualizedInstrumentMacroEditor::loopChanged,
					 this, [&](int prevBegin, int prevEnd, InstrumentSequenceLoop loop) {
		if (!isIgnoreEvent_) {
			bt_.lock()->changeEnvelopeSSGLoop(ui->envNumSpinBox->value(), prevBegin, prevEnd, loop);
			emit envelopeParameterChanged(ui->envNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->envEditor, &VisualizedInstrumentMacroEditor::releaseChanged,
					 this, [&](InstrumentSequenceRelease release) {
		if (!isIgnoreEvent_) {
			bt_.lock()->setEnvelopeSSGRelease(ui->envNumSpinBox->value(), release);
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
			bt_.lock()->addArpeggioSSGSequenceData(ui->arpNumSpinBox->value(), row);
			emit arpeggioParameterChanged(ui->arpNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->arpEditor, &VisualizedInstrumentMacroEditor::sequenceDataRemoved,
					 this, [&] {
		if (!isIgnoreEvent_) {
			bt_.lock()->removeArpeggioSSGSequenceData(ui->arpNumSpinBox->value());
			emit arpeggioParameterChanged(ui->arpNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->arpEditor, &VisualizedInstrumentMacroEditor::sequenceDataChanged,
					 this, [&](int row, int col) {
		if (!isIgnoreEvent_) {
			bt_.lock()->setArpeggioSSGSequenceData(ui->arpNumSpinBox->value(), col, row);
			emit arpeggioParameterChanged(ui->arpNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->arpEditor, &VisualizedInstrumentMacroEditor::loopAdded,
					 this, [&](InstrumentSequenceLoop loop) {
		if (!isIgnoreEvent_) {
			bt_.lock()->addArpeggioSSGLoop(ui->arpNumSpinBox->value(), loop);
			emit arpeggioParameterChanged(ui->arpNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->arpEditor, &VisualizedInstrumentMacroEditor::loopRemoved,
					 this, [&](int begin, int end) {
		if (!isIgnoreEvent_) {
			bt_.lock()->removeArpeggioSSGLoop(ui->arpNumSpinBox->value(), begin, end);
			emit arpeggioParameterChanged(ui->arpNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->arpEditor, &VisualizedInstrumentMacroEditor::loopCleared,
					 this, [&] {
		if (!isIgnoreEvent_) {
			bt_.lock()->clearArpeggioSSGLoops(ui->arpNumSpinBox->value());
			emit arpeggioParameterChanged(ui->arpNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->arpEditor, &VisualizedInstrumentMacroEditor::loopChanged,
					 this, [&](int prevBegin, int prevEnd, InstrumentSequenceLoop loop) {
		if (!isIgnoreEvent_) {
			bt_.lock()->changeArpeggioSSGLoop(ui->arpNumSpinBox->value(), prevBegin, prevEnd, loop);
			emit arpeggioParameterChanged(ui->arpNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->arpEditor, &VisualizedInstrumentMacroEditor::releaseChanged,
					 this, [&](InstrumentSequenceRelease release) {
		if (!isIgnoreEvent_) {
			bt_.lock()->setArpeggioSSGRelease(ui->arpNumSpinBox->value(), release);
			emit arpeggioParameterChanged(ui->arpNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	// Leave Before Qt5.7.0 style due to windows xp
	QObject::connect(ui->arpTypeComboBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
					 this, &InstrumentEditorSSGForm::onArpeggioTypeChanged);

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
			bt_.lock()->addPitchSSGSequenceData(ui->ptNumSpinBox->value(), row);
			emit pitchParameterChanged(ui->ptNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->ptEditor, &VisualizedInstrumentMacroEditor::sequenceDataRemoved,
					 this, [&] {
		if (!isIgnoreEvent_) {
			bt_.lock()->removePitchSSGSequenceData(ui->ptNumSpinBox->value());
			emit pitchParameterChanged(ui->ptNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->ptEditor, &VisualizedInstrumentMacroEditor::sequenceDataChanged,
					 this, [&](int row, int col) {
		if (!isIgnoreEvent_) {
			bt_.lock()->setPitchSSGSequenceData(ui->ptNumSpinBox->value(), col, row);
			emit pitchParameterChanged(ui->ptNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->ptEditor, &VisualizedInstrumentMacroEditor::loopAdded,
					 this, [&](InstrumentSequenceLoop loop) {
		if (!isIgnoreEvent_) {
			bt_.lock()->addPitchSSGLoop(ui->ptNumSpinBox->value(), loop);
			emit pitchParameterChanged(ui->ptNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->ptEditor, &VisualizedInstrumentMacroEditor::loopRemoved,
					 this, [&](int begin, int end) {
		if (!isIgnoreEvent_) {
			bt_.lock()->removePitchSSGLoop(ui->ptNumSpinBox->value(), begin, end);
			emit pitchParameterChanged(ui->ptNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->ptEditor, &VisualizedInstrumentMacroEditor::loopCleared,
					 this, [&] {
		if (!isIgnoreEvent_) {
			bt_.lock()->clearPitchSSGLoops(ui->ptNumSpinBox->value());
			emit pitchParameterChanged(ui->ptNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->ptEditor, &VisualizedInstrumentMacroEditor::loopChanged,
					 this, [&](int prevBegin, int prevEnd, InstrumentSequenceLoop loop) {
		if (!isIgnoreEvent_) {
			bt_.lock()->changePitchSSGLoop(ui->ptNumSpinBox->value(), prevBegin, prevEnd, loop);
			emit pitchParameterChanged(ui->ptNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	QObject::connect(ui->ptEditor, &VisualizedInstrumentMacroEditor::releaseChanged,
					 this, [&](InstrumentSequenceRelease release) {
		if (!isIgnoreEvent_) {
			bt_.lock()->setPitchSSGRelease(ui->ptNumSpinBox->value(), release);
			emit pitchParameterChanged(ui->ptNumSpinBox->value(), instNum_);
			emit modified();
		}
	});
	// Leave Before Qt5.7.0 style due to windows xp
	QObject::connect(ui->ptTypeComboBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
					 this, &InstrumentEditorSSGForm::onPitchTypeChanged);
}

InstrumentEditorSSGForm::~InstrumentEditorSSGForm()
{
	delete ui;
}

void InstrumentEditorSSGForm::setInstrumentNumber(int num)
{
	instNum_ = num;
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

void InstrumentEditorSSGForm::setConfiguration(std::weak_ptr<Configuration> config)
{
	config_ = config;
}

void InstrumentEditorSSGForm::updateConfigurationForDisplay()
{
	ui->arpEditor->onNoteNamesUpdated();
}

void InstrumentEditorSSGForm::setColorPalette(std::shared_ptr<ColorPalette> palette)
{
	ui->waveEditor->setColorPalette(palette);
	ui->tnEditor->setColorPalette(palette);
	ui->envEditor->setColorPalette(palette);
	ui->arpEditor->setColorPalette(palette);
	ui->ptEditor->setColorPalette(palette);
}

void InstrumentEditorSSGForm::updateInstrumentParameters()
{
	Ui::EventGuard eg(isIgnoreEvent_);

	auto name = gui_utils::utf8ToQString(bt_.lock()->getInstrument(instNum_)->getName());
	setWindowTitle(QString("%1: %2").arg(instNum_, 2, 16, QChar('0')).toUpper().arg(name));

	setInstrumentWaveformParameters();
	setInstrumentToneNoiseParameters();
	setInstrumentEnvelopeParameters();
	setInstrumentArpeggioParameters();
	setInstrumentPitchParameters();
}

/********** Events **********/
// MUST DIRECT CONNECTION
void InstrumentEditorSSGForm::keyPressEvent(QKeyEvent *event)
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
void InstrumentEditorSSGForm::keyReleaseEvent(QKeyEvent *event)
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

//--- Waveform
void InstrumentEditorSSGForm::setInstrumentWaveformParameters()
{
	Ui::EventGuard ev(isIgnoreEvent_);

	std::unique_ptr<AbstractInstrument> inst = bt_.lock()->getInstrument(instNum_);
	auto instSSG = dynamic_cast<InstrumentSSG*>(inst.get());

	ui->waveNumSpinBox->setValue(instSSG->getWaveformNumber());
	ui->waveEditor->clearData();
	for (auto& unit : instSSG->getWaveformSequence()) {
		QString str("");
		if (isModulatedWaveformSSG(unit.data)) {
			if (unit.type == SSGWaveformUnit::RatioSubdata) {
				int r1, r2;
				unit.getSubdataAsRatio(r1, r2);
				str = QString("%1/%2").arg(r1).arg(r2);
			}
			else {
				str = QString::number(unit.subdata);
			}
		}
		ui->waveEditor->addSequenceData(unit.data, str, unit.subdata);
	}
	for (auto& loop : instSSG->getWaveformLoopRoot().getAllLoops()) {
		ui->waveEditor->addLoop(loop.getBeginPos(), loop.getEndPos(), loop.getTimes());
	}
	ui->waveEditor->setRelease(instSSG->getWaveformRelease());
	if (instSSG->getWaveformEnabled()) {
		ui->waveEditGroupBox->setChecked(true);
		onWaveformNumberChanged();
	}
	else {
		ui->waveEditGroupBox->setChecked(false);
	}
}

SSGWaveformUnit InstrumentEditorSSGForm::setWaveformSequenceColumn(int col, int wfRow)
{
	auto button = ui->squareMaskButtonGroup->checkedButton();
	if (button == ui->squareMaskRawRadioButton) {
		SSGWaveformUnit unit = SSGWaveformUnit::makeRawUnit(wfRow, ui->squareMaskRawSpinBox->value());
		ui->waveEditor->setText(col, QString::number(unit.subdata));
		ui->waveEditor->setSubdata(col, unit.subdata);
		return unit;
	}
	else {
		SSGWaveformUnit unit = SSGWaveformUnit::makeRatioUnit(wfRow, ui->squareMaskToneSpinBox->value(),
															  ui->squareMaskMaskSpinBox->value());
		ui->waveEditor->setText(col, QString::number(ui->squareMaskToneSpinBox->value()) + "/"
								+ QString::number(ui->squareMaskMaskSpinBox->value()));
		ui->waveEditor->setSubdata(col, unit.subdata);
		return unit;
	}
}

/********** Slots **********/
void InstrumentEditorSSGForm::onWaveformNumberChanged()
{
	// Change users view
	std::multiset<int> users = bt_.lock()->getWaveformSSGUsers(ui->waveNumSpinBox->value());
	ui->waveUsersLineEdit->setText(inst_edit_utils::generateUsersString(users));
}

void InstrumentEditorSSGForm::onWaveformParameterChanged(int wfNum)
{
	if (ui->waveNumSpinBox->value() == wfNum) {
		Ui::EventGuard eg(isIgnoreEvent_);
		setInstrumentWaveformParameters();
	}
}

void InstrumentEditorSSGForm::on_waveEditGroupBox_toggled(bool arg1)
{
	if (!isIgnoreEvent_) {
		bt_.lock()->setInstrumentSSGWaveformEnabled(instNum_, arg1);
		setInstrumentWaveformParameters();
		emit waveformNumberChanged();
		emit modified();
	}

	onWaveformNumberChanged();
}

void InstrumentEditorSSGForm::on_waveNumSpinBox_valueChanged(int arg1)
{
	if (!isIgnoreEvent_) {
		bt_.lock()->setInstrumentSSGWaveform(instNum_, arg1);
		setInstrumentWaveformParameters();
		emit waveformNumberChanged();
		emit modified();
	}

	onWaveformNumberChanged();
}

void InstrumentEditorSSGForm::on_squareMaskRawSpinBox_valueChanged(int arg1)
{
	ui->squareMaskRawSpinBox->setSuffix(
				QString(" (0x") + QString("%1 | ").arg(arg1, 3, 16, QChar('0')).toUpper()
				+ QString("%1Hz)").arg(arg1 ? QString::number(124800.0 / arg1, 'f', 4) : "-")
				);
}

//--- Tone/Noise
void InstrumentEditorSSGForm::setInstrumentToneNoiseParameters()
{
	Ui::EventGuard ev(isIgnoreEvent_);

	std::unique_ptr<AbstractInstrument> inst = bt_.lock()->getInstrument(instNum_);
	auto instSSG = dynamic_cast<InstrumentSSG*>(inst.get());

	ui->tnNumSpinBox->setValue(instSSG->getToneNoiseNumber());
	ui->tnEditor->clearData();
	for (auto& unit : instSSG->getToneNoiseSequence()) {
		ui->tnEditor->addSequenceData(unit.data);
	}
	for (auto& loop : instSSG->getToneNoiseLoopRoot().getAllLoops()) {
		ui->tnEditor->addLoop(loop.getBeginPos(), loop.getEndPos(), loop.getTimes());
	}
	ui->tnEditor->setRelease(instSSG->getToneNoiseRelease());
	if (instSSG->getToneNoiseEnabled()) {
		ui->tnEditGroupBox->setChecked(true);
		onToneNoiseNumberChanged();
	}
	else {
		ui->tnEditGroupBox->setChecked(false);
	}
}

/********** Slots **********/
void InstrumentEditorSSGForm::onToneNoiseNumberChanged()
{
	// Change users view
	std::multiset<int> users = bt_.lock()->getToneNoiseSSGUsers(ui->tnNumSpinBox->value());
	ui->tnUsersLineEdit->setText(inst_edit_utils::generateUsersString(users));
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
		bt_.lock()->setInstrumentSSGToneNoiseEnabled(instNum_, arg1);
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
void InstrumentEditorSSGForm::setInstrumentEnvelopeParameters()
{
	Ui::EventGuard ev(isIgnoreEvent_);

	std::unique_ptr<AbstractInstrument> inst = bt_.lock()->getInstrument(instNum_);
	auto instSSG = dynamic_cast<InstrumentSSG*>(inst.get());

	ui->envNumSpinBox->setValue(instSSG->getEnvelopeNumber());
	ui->envEditor->clearData();
	for (auto& unit : instSSG->getEnvelopeSequence()) {
		QString str("");
		if (unit.data >= 16) {
			if (unit.type == SSGEnvelopeUnit::RatioSubdata) {
				int r1, r2;
				unit.getSubdataAsRatio(r1, r2);
				str = QString("%1/%2").arg(r1).arg(r2);
			}
			else {
				str = QString::number(unit.subdata);
			}
		}
		ui->envEditor->addSequenceData(unit.data, str, unit.subdata);
	}
	for (auto& loop : instSSG->getEnvelopeLoopRoot().getAllLoops()) {
		ui->envEditor->addLoop(loop.getBeginPos(), loop.getEndPos(), loop.getTimes());
	}
	ui->envEditor->setRelease(instSSG->getEnvelopeRelease());
	if (instSSG->getEnvelopeEnabled()) {
		ui->envEditGroupBox->setChecked(true);
		onEnvelopeNumberChanged();
	}
	else {
		ui->envEditGroupBox->setChecked(false);
	}
}

SSGEnvelopeUnit InstrumentEditorSSGForm::setEnvelopeSequenceColumn(int col, int envRow)
{
	if (ui->hardFreqButtonGroup->checkedButton() == ui->hardFreqRawRadioButton) {
		SSGEnvelopeUnit unit = SSGEnvelopeUnit::makeRawUnit(envRow, ui->hardFreqRawSpinBox->value());
		ui->envEditor->setText(col, QString::number(unit.subdata));
		ui->envEditor->setSubdata(col, unit.subdata);
		return unit;
	}
	else {
		SSGEnvelopeUnit unit = SSGEnvelopeUnit::makeRatioUnit(envRow, ui->hardFreqToneSpinBox->value(),
															  ui->hardFreqHardSpinBox->value());
		ui->envEditor->setText(col, QString::number(ui->hardFreqToneSpinBox->value()) + "/"
							   + QString::number(ui->hardFreqHardSpinBox->value()));
		ui->envEditor->setSubdata(col, unit.subdata);
		return unit;
	}
}

/********** Slots **********/
void InstrumentEditorSSGForm::onEnvelopeNumberChanged()
{
	// Change users view
	std::multiset<int> users = bt_.lock()->getEnvelopeSSGUsers(ui->envNumSpinBox->value());
	ui->envUsersLineEdit->setText(inst_edit_utils::generateUsersString(users));
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
		bt_.lock()->setInstrumentSSGEnvelopeEnabled(instNum_, arg1);
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

void InstrumentEditorSSGForm::on_hardFreqRawSpinBox_valueChanged(int arg1)
{
	ui->hardFreqRawSpinBox->setSuffix(
				QString(" (0x") + QString("%1 | ").arg(arg1, 4, 16, QChar('0')).toUpper()
				+ QString("%1Hz").arg(arg1 ? QString::number(7800.0 / arg1, 'f', 4) : "-"));
}

//--- Arpeggio
void InstrumentEditorSSGForm::setInstrumentArpeggioParameters()
{
	Ui::EventGuard ev(isIgnoreEvent_);

	std::unique_ptr<AbstractInstrument> inst = bt_.lock()->getInstrument(instNum_);
	auto instSSG = dynamic_cast<InstrumentSSG*>(inst.get());

	ui->arpNumSpinBox->setValue(instSSG->getArpeggioNumber());
	ui->arpEditor->clearData();
	for (auto& unit : instSSG->getArpeggioSequence()) {
		ui->arpEditor->addSequenceData(unit.data);
	}
	for (auto& loop : instSSG->getArpeggioLoopRoot().getAllLoops()) {
		ui->arpEditor->addLoop(loop.getBeginPos(), loop.getEndPos(), loop.getTimes());
	}
	ui->arpEditor->setRelease(instSSG->getArpeggioRelease());
	for (int i = 0; i < ui->arpTypeComboBox->count(); ++i) {
		if (instSSG->getArpeggioType() == static_cast<SequenceType>(ui->arpTypeComboBox->itemData(i).toInt())) {
			ui->arpTypeComboBox->setCurrentIndex(i);
			break;
		}
	}
	if (instSSG->getArpeggioEnabled()) {
		ui->arpEditGroupBox->setChecked(true);
		onArpeggioNumberChanged();
	}
	else {
		ui->arpEditGroupBox->setChecked(false);
	}
}

/********** Slots **********/
void InstrumentEditorSSGForm::onArpeggioNumberChanged()
{
	// Change users view
	std::multiset<int> users = bt_.lock()->getArpeggioSSGUsers(ui->arpNumSpinBox->value());
	ui->arpUsersLineEdit->setText(inst_edit_utils::generateUsersString(users));
}

void InstrumentEditorSSGForm::onArpeggioParameterChanged(int tnNum)
{
	if (ui->arpNumSpinBox->value() == tnNum) {
		Ui::EventGuard eg(isIgnoreEvent_);
		setInstrumentArpeggioParameters();
	}
}

void InstrumentEditorSSGForm::onArpeggioTypeChanged(int)
{
	auto type = static_cast<SequenceType>(ui->arpTypeComboBox->currentData(Qt::UserRole).toInt());
	if (!isIgnoreEvent_) {
		bt_.lock()->setArpeggioSSGType(ui->arpNumSpinBox->value(), type);
		emit arpeggioParameterChanged(ui->arpNumSpinBox->value(), instNum_);
		emit modified();
	}

	ui->arpEditor->setSequenceType(type);
}

void InstrumentEditorSSGForm::on_arpEditGroupBox_toggled(bool arg1)
{
	if (!isIgnoreEvent_) {
		bt_.lock()->setInstrumentSSGArpeggioEnabled(instNum_, arg1);
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

//--- Pitch
void InstrumentEditorSSGForm::setInstrumentPitchParameters()
{
	Ui::EventGuard ev(isIgnoreEvent_);

	std::unique_ptr<AbstractInstrument> inst = bt_.lock()->getInstrument(instNum_);
	auto instSSG = dynamic_cast<InstrumentSSG*>(inst.get());

	ui->ptNumSpinBox->setValue(instSSG->getPitchNumber());
	ui->ptEditor->clearData();
	for (auto& unit : instSSG->getPitchSequence()) {
		ui->ptEditor->addSequenceData(unit.data);
	}
	for (auto& loop : instSSG->getPitchLoopRoot().getAllLoops()) {
		ui->ptEditor->addLoop(loop.getBeginPos(), loop.getEndPos(), loop.getTimes());
	}
	ui->ptEditor->setRelease(instSSG->getPitchRelease());
	for (int i = 0; i < ui->ptTypeComboBox->count(); ++i) {
		if (instSSG->getPitchType() == static_cast<SequenceType>(ui->ptTypeComboBox->itemData(i).toInt())) {
			ui->ptTypeComboBox->setCurrentIndex(i);
			break;
		}
	}
	if (instSSG->getPitchEnabled()) {
		ui->ptEditGroupBox->setChecked(true);
		onPitchNumberChanged();
	}
	else {
		ui->ptEditGroupBox->setChecked(false);
	}
}

/********** Slots **********/
void InstrumentEditorSSGForm::onPitchNumberChanged()
{
	// Change users view
	std::multiset<int> users = bt_.lock()->getPitchSSGUsers(ui->ptNumSpinBox->value());
	ui->ptUsersLineEdit->setText(inst_edit_utils::generateUsersString(users));
}

void InstrumentEditorSSGForm::onPitchParameterChanged(int ptNum)
{
	if (ui->ptNumSpinBox->value() == ptNum) {
		Ui::EventGuard eg(isIgnoreEvent_);
		setInstrumentPitchParameters();
	}
}

void InstrumentEditorSSGForm::onPitchTypeChanged(int)
{
	auto type = static_cast<SequenceType>(ui->ptTypeComboBox->currentData(Qt::UserRole).toInt());
	if (!isIgnoreEvent_) {
		bt_.lock()->setPitchSSGType(ui->ptNumSpinBox->value(), type);
		emit pitchParameterChanged(ui->ptNumSpinBox->value(), instNum_);
		emit modified();
	}

	ui->ptEditor->setSequenceType(type);
}

void InstrumentEditorSSGForm::on_ptEditGroupBox_toggled(bool arg1)
{
	if (!isIgnoreEvent_) {
		bt_.lock()->setInstrumentSSGPitchEnabled(instNum_, arg1);
		setInstrumentPitchParameters();
		emit pitchNumberChanged();
		emit modified();
	}

	onPitchNumberChanged();
}

void InstrumentEditorSSGForm::on_ptNumSpinBox_valueChanged(int arg1)
{
	if (!isIgnoreEvent_) {
		bt_.lock()->setInstrumentSSGPitch(instNum_, arg1);
		setInstrumentPitchParameters();
		emit pitchNumberChanged();
		emit modified();
	}

	onPitchNumberChanged();
}
