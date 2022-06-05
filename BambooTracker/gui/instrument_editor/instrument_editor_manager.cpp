/*
 * Copyright (C) 2018-2022 Rerrah
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

#include "instrument_editor_manager.hpp"
#include <utility>
#include <stdexcept>
#include <QApplication>
#include "instrument.hpp"
#include "gui/instrument_editor/fm_instrument_editor.hpp"
#include "gui/instrument_editor/ssg_instrument_editor.hpp"
#include "gui/instrument_editor/adpcm_instrument_editor.hpp"
#include "gui/instrument_editor/adpcm_drumkit_editor.hpp"

InstrumentEditorManager::InstrumentEditorManager()
	: typeIndices_ {
{ InstrumentType::FM, {} },
{ InstrumentType::SSG, {} },
{ InstrumentType::ADPCM, {} },
{ InstrumentType::Drumkit, {} },
		  }
{
}

void InstrumentEditorManager::updateByConfiguration()
{
	for (const auto& idcs : typeIndices_) {
		for (auto idx : idcs) {
			if (auto& dialog = dialogs_[idx]) dialog->updateByConfigurationChange();
		}
	}
}

const QSharedPointer<InstrumentEditor> InstrumentEditorManager::getDialog(int index) const
{
	return (assertIndex(index) ? dialogs_[index] : QSharedPointer<InstrumentEditor>());
}

bool InstrumentEditorManager::add(int index, QSharedPointer<InstrumentEditor> dialog)
{
	if (!assertIndex(index)) return false;

	typeIndices_[dialog->getInstrumentType()].insert(index);
	dialogs_[index] = std::move(dialog);

	// Parameter numbers update in MainWindow

	return true;
}

void InstrumentEditorManager::swap(int index1, int index2)
{
	if (!assertIndex(index1) || !assertIndex(index2)) return;

	if (dialogs_[index1]) {
		InstrumentType typeA = dialogs_[index1]->getInstrumentType();
		typeIndices_[typeA].remove(index1);
		if (dialogs_[index2]) {
			InstrumentType typeB = dialogs_[index2]->getInstrumentType();
			typeIndices_[typeB].remove(index2);
			std::swap(dialogs_[index1], dialogs_[index2]);
			dialogs_[index1]->setInstrumentNumber(index1);
			typeIndices_[typeB].insert(index1);
		}
		else {
			dialogs_[index2] = dialogs_[index1];
			dialogs_[index1].reset();
		}
		dialogs_[index2]->setInstrumentNumber(index2);
		typeIndices_[typeA].insert(index2);
	}
	else {
		if (dialogs_[index2]) {
			InstrumentType typeB = dialogs_[index2]->getInstrumentType();
			typeIndices_[typeB].remove(index2);
			dialogs_[index1] = dialogs_[index2];
			dialogs_[index2].reset();
			dialogs_[index1]->setInstrumentNumber(index1);
			typeIndices_[typeB].insert(index1);
		}
	}

	onInstrumentFMEnvelopeNumberChanged();
	onInstrumentFMLFONumberChanged();
	onInstrumentFMOperatorSequenceNumberChanged();
	onInstrumentFMArpeggioNumberChanged();
	onInstrumentFMPitchNumberChanged();

	onInstrumentSSGWaveformNumberChanged();
	onInstrumentSSGEnvelopeNumberChanged();
	onInstrumentSSGToneNoiseNumberChanged();
	onInstrumentSSGArpeggioNumberChanged();
	onInstrumentSSGPitchNumberChanged();

	onInstrumentADPCMSampleNumberChanged();
	onInstrumentADPCMEnvelopeNumberChanged();
	onInstrumentADPCMArpeggioNumberChanged();
	onInstrumentADPCMPitchNumberChanged();

	onInstrumentADPCMSampleNumberChanged();
}

bool InstrumentEditorManager::remove(int index)
{
	if (assertIndex(index)) return false;

	if (dialogs_[index]) {
		dialogs_[index]->close();
		dialogs_[index].reset();
	}

	onInstrumentFMEnvelopeNumberChanged();
	onInstrumentFMLFONumberChanged();
	onInstrumentFMOperatorSequenceNumberChanged();
	onInstrumentFMArpeggioNumberChanged();
	onInstrumentFMPitchNumberChanged();

	onInstrumentSSGWaveformNumberChanged();
	onInstrumentSSGEnvelopeNumberChanged();
	onInstrumentSSGToneNoiseNumberChanged();
	onInstrumentSSGArpeggioNumberChanged();
	onInstrumentSSGPitchNumberChanged();

	onInstrumentADPCMSampleNumberChanged();
	onInstrumentADPCMEnvelopeNumberChanged();
	onInstrumentADPCMArpeggioNumberChanged();
	onInstrumentADPCMPitchNumberChanged();

	onInstrumentADPCMSampleNumberChanged();

	return true;
}

void InstrumentEditorManager::showDialog(int index)
{
	if (!assertIndex(index)) return;

	auto& dialog = dialogs_[index];
	if (!dialog) return;

	if (dialog->isVisible()) {
		dialog->activateWindow();
	}
	else {
		dialog->show();
	}
}

void InstrumentEditorManager::closeAll()
{
	for (const auto& idcs : typeIndices_) {
		for (auto idx : idcs) {
			auto& dialog = dialogs_[idx];
			if (dialog) dialog->close();
		}
	}
}

void InstrumentEditorManager::clearAll()
{
	// Close all dialogs before disposing.
	closeAll();

	for (auto& idcs : typeIndices_) {
		for (auto idx : idcs) {
			dialogs_[idx].reset();
		}
		idcs.clear();
	}
}

int InstrumentEditorManager::getActivatedDialogIndex() const
{
	const QWidget* win = QApplication::activeWindow();
	for (const auto& idcs : typeIndices_) {
		for (auto idx : idcs) {
			if (dialogs_[idx] == win) return idx;
		}
	}
	return -1;
}

/********** Slots **********/
void InstrumentEditorManager::onInstrumentFMEnvelopeParameterChanged(int envNum, int fromInstNum)
{
	const auto idcs = getIndicesForParameterUpdating(InstrumentType::FM, fromInstNum);
	for (auto idx : idcs) {
		if (auto dialog = dialogs_[idx].objectCast<FmInstrumentEditor>()) {
			dialog->onEnvelopeParameterChanged(envNum);
		}
	}
}

void InstrumentEditorManager::onInstrumentFMEnvelopeNumberChanged()
{
	const auto idcs = getIndicesForParameterUpdating(InstrumentType::FM);
	for (auto idx : idcs) {
		if (auto dialog = dialogs_[idx].objectCast<FmInstrumentEditor>()) {
			dialog->onEnvelopeNumberChanged();
		}
	}
}

void InstrumentEditorManager::onInstrumentFMLFOParameterChanged(int lfoNum, int fromInstNum)
{
	const auto idcs = getIndicesForParameterUpdating(InstrumentType::FM, fromInstNum);
	for (auto idx : idcs) {
		if (auto dialog = dialogs_[idx].objectCast<FmInstrumentEditor>()) {
			dialog->onLFOParameterChanged(lfoNum);
		}
	}
}

void InstrumentEditorManager::onInstrumentFMLFONumberChanged()
{
	const auto idcs = getIndicesForParameterUpdating(InstrumentType::FM);
	for (auto idx : idcs) {
		if (auto dialog = dialogs_[idx].objectCast<FmInstrumentEditor>()) {
			dialog->onLFONumberChanged();
		}
	}
}

void InstrumentEditorManager::onInstrumentFMOperatorSequenceParameterChanged(FMEnvelopeParameter param, int opSeqNum, int fromInstNum)
{
	const auto idcs = getIndicesForParameterUpdating(InstrumentType::FM, fromInstNum);
	for (auto idx : idcs) {
		if (auto dialog = dialogs_[idx].objectCast<FmInstrumentEditor>()) {
			dialog->onOperatorSequenceParameterChanged(param, opSeqNum);
		}
	}
}

void InstrumentEditorManager::onInstrumentFMOperatorSequenceNumberChanged()
{
	const auto idcs = getIndicesForParameterUpdating(InstrumentType::FM);
	for (auto idx : idcs) {
		if (auto dialog = dialogs_[idx].objectCast<FmInstrumentEditor>()) {
			dialog->onOperatorSequenceNumberChanged();
		}
	}
}

void InstrumentEditorManager::onInstrumentFMArpeggioParameterChanged(int arpNum, int fromInstNum)
{
	const auto idcs = getIndicesForParameterUpdating(InstrumentType::FM, fromInstNum);
	for (auto idx : idcs) {
		if (auto dialog = dialogs_[idx].objectCast<FmInstrumentEditor>()) {
			dialog->onArpeggioParameterChanged(arpNum);
		}
	}
}

void InstrumentEditorManager::onInstrumentFMArpeggioNumberChanged()
{
	const auto idcs = getIndicesForParameterUpdating(InstrumentType::FM);
	for (auto idx : idcs) {
		if (auto dialog = dialogs_[idx].objectCast<FmInstrumentEditor>()) {
			dialog->onArpeggioNumberChanged();
		}
	}
}

void InstrumentEditorManager::onInstrumentFMPitchParameterChanged(int ptNum, int fromInstNum)
{
	const auto idcs = getIndicesForParameterUpdating(InstrumentType::FM, fromInstNum);
	for (auto idx : idcs) {
		if (auto dialog = dialogs_[idx].objectCast<FmInstrumentEditor>()) {
			dialog->onPitchParameterChanged(ptNum);
		}
	}
}

void InstrumentEditorManager::onInstrumentFMPitchNumberChanged()
{
	const auto idcs = getIndicesForParameterUpdating(InstrumentType::FM);
	for (auto idx : idcs) {
		if (auto dialog = dialogs_[idx].objectCast<FmInstrumentEditor>()) {
			dialog->onPitchNumberChanged();
		}
	}
}

void InstrumentEditorManager::onInstrumentSSGWaveformParameterChanged(int wfNum, int fromInstNum)
{
	const auto idcs = getIndicesForParameterUpdating(InstrumentType::SSG, fromInstNum);
	for (auto idx : idcs) {
		if (auto dialog = dialogs_[idx].objectCast<SsgInstrumentEditor>()) {
			dialog->onWaveformParameterChanged(wfNum);
		}
	}
}

void InstrumentEditorManager::onInstrumentSSGWaveformNumberChanged()
{
	const auto idcs = getIndicesForParameterUpdating(InstrumentType::SSG);
	for (auto idx : idcs) {
		if (auto dialog = dialogs_[idx].objectCast<SsgInstrumentEditor>()) {
			dialog->onWaveformNumberChanged();
		}
	}
}

void InstrumentEditorManager::onInstrumentSSGToneNoiseParameterChanged(int tnNum, int fromInstNum)
{
	const auto idcs = getIndicesForParameterUpdating(InstrumentType::SSG, fromInstNum);
	for (auto idx : idcs) {
		if (auto dialog = dialogs_[idx].objectCast<SsgInstrumentEditor>()) {
			dialog->onToneNoiseParameterChanged(tnNum);
		}
	}
}

void InstrumentEditorManager::onInstrumentSSGToneNoiseNumberChanged()
{
	const auto idcs = getIndicesForParameterUpdating(InstrumentType::SSG);
	for (auto idx : idcs) {
		if (auto dialog = dialogs_[idx].objectCast<SsgInstrumentEditor>()) {
			dialog->onToneNoiseNumberChanged();
		}
	}
}

void InstrumentEditorManager::onInstrumentSSGEnvelopeParameterChanged(int envNum, int fromInstNum)
{
	const auto idcs = getIndicesForParameterUpdating(InstrumentType::SSG, fromInstNum);
	for (auto idx : idcs) {
		if (auto dialog = dialogs_[idx].objectCast<SsgInstrumentEditor>()) {
			dialog->onEnvelopeParameterChanged(envNum);
		}
	}
}

void InstrumentEditorManager::onInstrumentSSGEnvelopeNumberChanged()
{
	const auto idcs = getIndicesForParameterUpdating(InstrumentType::SSG);
	for (auto idx : idcs) {
		if (auto dialog = dialogs_[idx].objectCast<SsgInstrumentEditor>()) {
			dialog->onEnvelopeNumberChanged();
		}
	}
}

void InstrumentEditorManager::onInstrumentSSGArpeggioParameterChanged(int arpNum, int fromInstNum)
{
	const auto idcs = getIndicesForParameterUpdating(InstrumentType::SSG, fromInstNum);
	for (auto idx : idcs) {
		if (auto dialog = dialogs_[idx].objectCast<SsgInstrumentEditor>()) {
			dialog->onArpeggioParameterChanged(arpNum);
		}
	}
}

void InstrumentEditorManager::onInstrumentSSGArpeggioNumberChanged()
{
	const auto idcs = getIndicesForParameterUpdating(InstrumentType::SSG);
	for (auto idx : idcs) {
		if (auto dialog = dialogs_[idx].objectCast<SsgInstrumentEditor>()) {
			dialog->onArpeggioNumberChanged();
		}
	}
}

void InstrumentEditorManager::onInstrumentSSGPitchParameterChanged(int ptNum, int fromInstNum)
{
	const auto idcs = getIndicesForParameterUpdating(InstrumentType::SSG, fromInstNum);
	for (auto idx : idcs) {
		if (auto dialog = dialogs_[idx].objectCast<SsgInstrumentEditor>()) {
			dialog->onPitchParameterChanged(ptNum);
		}
	}
}

void InstrumentEditorManager::onInstrumentSSGPitchNumberChanged()
{
	const auto idcs = getIndicesForParameterUpdating(InstrumentType::SSG);
	for (auto idx : idcs) {
		if (auto dialog = dialogs_[idx].objectCast<SsgInstrumentEditor>()) {
			dialog->onPitchNumberChanged();
		}
	}
}

void InstrumentEditorManager::onInstrumentADPCMSampleParameterChanged(int sampNum, int fromInstNum)
{
	Q_UNUSED(fromInstNum)	// Update all adpcm editor to change sample memory viewer and sample viewer

	const auto idcsAdpcm = getIndicesForParameterUpdating(InstrumentType::ADPCM);
	for (auto idx : idcsAdpcm) {
		if (auto dialog = dialogs_[idx].objectCast<AdpcmInstrumentEditor>()) {
			dialog->onSampleParameterChanged(sampNum);
		}
	}

	const auto idcsKit = getIndicesForParameterUpdating(InstrumentType::Drumkit);
	for (auto idx : idcsKit) {
		if (auto dialog = dialogs_[idx].objectCast<AdpcmDrumkitEditor>()) {
			dialog->onSampleParameterChanged(sampNum);
		}
	}
}

void InstrumentEditorManager::onInstrumentADPCMSampleNumberChanged()
{
	const auto idcsAdpcm = getIndicesForParameterUpdating(InstrumentType::ADPCM);
	for (auto idx : idcsAdpcm) {
		if (auto dialog = dialogs_[idx].objectCast<AdpcmInstrumentEditor>()) {
			dialog->onSampleNumberChanged();
		}
	}

	const auto idcsKit = getIndicesForParameterUpdating(InstrumentType::Drumkit);
	for (auto idx : idcsKit) {
		if (auto dialog = dialogs_[idx].objectCast<AdpcmDrumkitEditor>()) {
			dialog->onSampleNumberChanged();
		}
	}
}

void InstrumentEditorManager::onInstrumentADPCMSampleMemoryUpdated()
{
	const auto idcsAdpcm = getIndicesForParameterUpdating(InstrumentType::ADPCM);
	for (auto idx : idcsAdpcm) {
		if (auto dialog = dialogs_[idx].objectCast<AdpcmInstrumentEditor>()) {
			dialog->onSampleMemoryUpdated();
		}
	}

	const auto idcsKit = getIndicesForParameterUpdating(InstrumentType::Drumkit);
	for (auto idx : idcsKit) {
		if (auto dialog = dialogs_[idx].objectCast<AdpcmDrumkitEditor>()) {
			dialog->onSampleMemoryUpdated();
		}
	}
}

void InstrumentEditorManager::onInstrumentADPCMEnvelopeParameterChanged(int envNum, int fromInstNum)
{
	const auto idcs = getIndicesForParameterUpdating(InstrumentType::ADPCM, fromInstNum);
	for (auto idx : idcs) {
		if (auto dialog = dialogs_[idx].objectCast<AdpcmInstrumentEditor>()) {
			dialog->onEnvelopeParameterChanged(envNum);
		}
	}
}

void InstrumentEditorManager::onInstrumentADPCMEnvelopeNumberChanged()
{
	const auto idcs = getIndicesForParameterUpdating(InstrumentType::ADPCM);
	for (auto idx : idcs) {
		if (auto dialog = dialogs_[idx].objectCast<AdpcmInstrumentEditor>()) {
			dialog->onEnvelopeNumberChanged();
		}
	}
}

void InstrumentEditorManager::onInstrumentADPCMArpeggioParameterChanged(int arpNum, int fromInstNum)
{
	const auto idcs = getIndicesForParameterUpdating(InstrumentType::ADPCM, fromInstNum);
	for (auto idx : idcs) {
		if (auto dialog = dialogs_[idx].objectCast<AdpcmInstrumentEditor>()) {
			dialog->onArpeggioParameterChanged(arpNum);
		}
	}
}

void InstrumentEditorManager::onInstrumentADPCMArpeggioNumberChanged()
{
	const auto idcs = getIndicesForParameterUpdating(InstrumentType::ADPCM);
	for (auto idx : idcs) {
		if (auto dialog = dialogs_[idx].objectCast<AdpcmInstrumentEditor>()) {
			dialog->onArpeggioNumberChanged();
		}
	}
}

void InstrumentEditorManager::onInstrumentADPCMPitchParameterChanged(int ptNum, int fromInstNum)
{
	const auto idcs = getIndicesForParameterUpdating(InstrumentType::ADPCM, fromInstNum);
	for (auto idx : idcs) {
		if (auto dialog = dialogs_[idx].objectCast<AdpcmInstrumentEditor>()) {
			dialog->onPitchParameterChanged(ptNum);
		}
	}
}

void InstrumentEditorManager::onInstrumentADPCMPitchNumberChanged()
{
	const auto idcs = getIndicesForParameterUpdating(InstrumentType::ADPCM);
	for (auto idx : idcs) {
		if (auto dialog = dialogs_[idx].objectCast<AdpcmInstrumentEditor>()) {
			dialog->onPitchNumberChanged();
		}
	}
}
