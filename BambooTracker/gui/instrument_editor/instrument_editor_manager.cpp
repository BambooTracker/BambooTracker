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

InstrumentEditorManager::~InstrumentEditorManager()
{
	removeAll();
}

void InstrumentEditorManager::updateByConfiguration()
{
	for (const auto& idcs : qAsConst(typeIndices_)) {
		for (auto idx : idcs) {
			if (auto& editor = editors_[idx]) editor->updateByConfigurationChange();
		}
	}
}

bool InstrumentEditorManager::add(int index, InstrumentEditor* editor)
{
	if (!assertIndex(index)) return false;

	typeIndices_[editor->getInstrumentType()].insert(index);
	editors_[index] = editor;
	// Editor is deleted automatically in closed.
	editor->setAttribute(Qt::WA_DeleteOnClose);

	// Parameter numbers update in MainWindow

	return true;
}

void InstrumentEditorManager::swap(int index1, int index2)
{
	if (!assertIndex(index1) || !assertIndex(index2)) return;

	if (editors_[index1]) {
		InstrumentType typeA = editors_[index1]->getInstrumentType();
		typeIndices_[typeA].remove(index1);
		if (editors_[index2]) {
			InstrumentType typeB = editors_[index2]->getInstrumentType();
			typeIndices_[typeB].remove(index2);
			std::swap(editors_[index1], editors_[index2]);
			editors_[index1]->setInstrumentNumber(index1);
			typeIndices_[typeB].insert(index1);
		}
		else {
			editors_[index2] = editors_[index1];
			editors_[index1] = nullptr;
		}
		editors_[index2]->setInstrumentNumber(index2);
		typeIndices_[typeA].insert(index2);
	}
	else {
		if (editors_[index2]) {
			InstrumentType typeB = editors_[index2]->getInstrumentType();
			typeIndices_[typeB].remove(index2);
			editors_[index1] = editors_[index2];
			editors_[index2] = nullptr;
			editors_[index1]->setInstrumentNumber(index1);
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
	if (!assertIndex(index)) return false;

	if (editors_[index]) {
		editors_[index]->close();
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

void InstrumentEditorManager::showEditor(int index)
{
	if (!assertIndex(index)) return;

	auto& editor = editors_[index];
	if (!editor) return;

	if (editor->isVisible()) {
		editor->activateWindow();
	}
	else {
		editor->show();
	}
}

void InstrumentEditorManager::removeAll()
{
	for (const auto& editor : editors_) {
		if (!editor) continue;
		if (!editor->close()) delete editor.data();
	}
}

bool InstrumentEditorManager::hasShownEditor(int index) const
{
	return (assertIndex(index) && !editors_[index].isNull());
}

int InstrumentEditorManager::getActivatedEditorIndex() const
{
	const QWidget* win = QApplication::activeWindow();
	for (const auto& idcs : qAsConst(typeIndices_)) {
		for (auto idx : idcs) {
			if (editors_[idx] == win) return idx;
		}
	}
	return -1;
}

/********** Slots **********/
void InstrumentEditorManager::onInstrumentNameChanged(int instNum)
{
	if (!assertIndex(instNum)) return;
	if (auto& editor = editors_[instNum])
	{
		editor->updateWindowTitle();
	}
}

void InstrumentEditorManager::onInstrumentFMEnvelopeParameterChanged(int envNum, int fromInstNum)
{
	const auto idcs = getIndicesForParameterUpdating(InstrumentType::FM, fromInstNum);
	for (auto idx : idcs) {
		if (auto editor = qobject_cast<FmInstrumentEditor*>(editors_[idx])) {
			editor->onEnvelopeParameterChanged(envNum);
		}
	}
}

void InstrumentEditorManager::onInstrumentFMEnvelopeNumberChanged()
{
	const auto idcs = getIndicesForParameterUpdating(InstrumentType::FM);
	for (auto idx : idcs) {
		if (auto editor = qobject_cast<FmInstrumentEditor*>(editors_[idx])) {
			editor->onEnvelopeNumberChanged();
		}
	}
}

void InstrumentEditorManager::onInstrumentFMLFOParameterChanged(int lfoNum, int fromInstNum)
{
	const auto idcs = getIndicesForParameterUpdating(InstrumentType::FM, fromInstNum);
	for (auto idx : idcs) {
		if (auto editor = qobject_cast<FmInstrumentEditor*>(editors_[idx])) {
			editor->onLFOParameterChanged(lfoNum);
		}
	}
}

void InstrumentEditorManager::onInstrumentFMLFONumberChanged()
{
	const auto idcs = getIndicesForParameterUpdating(InstrumentType::FM);
	for (auto idx : idcs) {
		if (auto editor = qobject_cast<FmInstrumentEditor*>(editors_[idx])) {
			editor->onLFONumberChanged();
		}
	}
}

void InstrumentEditorManager::onInstrumentFMOperatorSequenceParameterChanged(FMEnvelopeParameter param, int opSeqNum, int fromInstNum)
{
	const auto idcs = getIndicesForParameterUpdating(InstrumentType::FM, fromInstNum);
	for (auto idx : idcs) {
		if (auto editor = qobject_cast<FmInstrumentEditor*>(editors_[idx])) {
			editor->onOperatorSequenceParameterChanged(param, opSeqNum);
		}
	}
}

void InstrumentEditorManager::onInstrumentFMOperatorSequenceNumberChanged()
{
	const auto idcs = getIndicesForParameterUpdating(InstrumentType::FM);
	for (auto idx : idcs) {
		if (auto editor = qobject_cast<FmInstrumentEditor*>(editors_[idx])) {
			editor->onOperatorSequenceNumberChanged();
		}
	}
}

void InstrumentEditorManager::onInstrumentFMArpeggioParameterChanged(int arpNum, int fromInstNum)
{
	const auto idcs = getIndicesForParameterUpdating(InstrumentType::FM, fromInstNum);
	for (auto idx : idcs) {
		if (auto editor = qobject_cast<FmInstrumentEditor*>(editors_[idx])) {
			editor->onArpeggioParameterChanged(arpNum);
		}
	}
}

void InstrumentEditorManager::onInstrumentFMArpeggioNumberChanged()
{
	const auto idcs = getIndicesForParameterUpdating(InstrumentType::FM);
	for (auto idx : idcs) {
		if (auto editor = qobject_cast<FmInstrumentEditor*>(editors_[idx])) {
			editor->onArpeggioNumberChanged();
		}
	}
}

void InstrumentEditorManager::onInstrumentFMPitchParameterChanged(int ptNum, int fromInstNum)
{
	const auto idcs = getIndicesForParameterUpdating(InstrumentType::FM, fromInstNum);
	for (auto idx : idcs) {
		if (auto editor = qobject_cast<FmInstrumentEditor*>(editors_[idx])) {
			editor->onPitchParameterChanged(ptNum);
		}
	}
}

void InstrumentEditorManager::onInstrumentFMPitchNumberChanged()
{
	const auto idcs = getIndicesForParameterUpdating(InstrumentType::FM);
	for (auto idx : idcs) {
		if (auto editor = qobject_cast<FmInstrumentEditor*>(editors_[idx])) {
			editor->onPitchNumberChanged();
		}
	}
}

void InstrumentEditorManager::onInstrumentSSGWaveformParameterChanged(int wfNum, int fromInstNum)
{
	const auto idcs = getIndicesForParameterUpdating(InstrumentType::SSG, fromInstNum);
	for (auto idx : idcs) {
		if (auto editor = qobject_cast<SsgInstrumentEditor*>(editors_[idx])) {
			editor->onWaveformParameterChanged(wfNum);
		}
	}
}

void InstrumentEditorManager::onInstrumentSSGWaveformNumberChanged()
{
	const auto idcs = getIndicesForParameterUpdating(InstrumentType::SSG);
	for (auto idx : idcs) {
		if (auto editor = qobject_cast<SsgInstrumentEditor*>(editors_[idx])) {
			editor->onWaveformNumberChanged();
		}
	}
}

void InstrumentEditorManager::onInstrumentSSGToneNoiseParameterChanged(int tnNum, int fromInstNum)
{
	const auto idcs = getIndicesForParameterUpdating(InstrumentType::SSG, fromInstNum);
	for (auto idx : idcs) {
		if (auto editor = qobject_cast<SsgInstrumentEditor*>(editors_[idx])) {
			editor->onToneNoiseParameterChanged(tnNum);
		}
	}
}

void InstrumentEditorManager::onInstrumentSSGToneNoiseNumberChanged()
{
	const auto idcs = getIndicesForParameterUpdating(InstrumentType::SSG);
	for (auto idx : idcs) {
		if (auto editor = qobject_cast<SsgInstrumentEditor*>(editors_[idx])) {
			editor->onToneNoiseNumberChanged();
		}
	}
}

void InstrumentEditorManager::onInstrumentSSGEnvelopeParameterChanged(int envNum, int fromInstNum)
{
	const auto idcs = getIndicesForParameterUpdating(InstrumentType::SSG, fromInstNum);
	for (auto idx : idcs) {
		if (auto editor = qobject_cast<SsgInstrumentEditor*>(editors_[idx])) {
			editor->onEnvelopeParameterChanged(envNum);
		}
	}
}

void InstrumentEditorManager::onInstrumentSSGEnvelopeNumberChanged()
{
	const auto idcs = getIndicesForParameterUpdating(InstrumentType::SSG);
	for (auto idx : idcs) {
		if (auto editor = qobject_cast<SsgInstrumentEditor*>(editors_[idx])) {
			editor->onEnvelopeNumberChanged();
		}
	}
}

void InstrumentEditorManager::onInstrumentSSGArpeggioParameterChanged(int arpNum, int fromInstNum)
{
	const auto idcs = getIndicesForParameterUpdating(InstrumentType::SSG, fromInstNum);
	for (auto idx : idcs) {
		if (auto editor = qobject_cast<SsgInstrumentEditor*>(editors_[idx])) {
			editor->onArpeggioParameterChanged(arpNum);
		}
	}
}

void InstrumentEditorManager::onInstrumentSSGArpeggioNumberChanged()
{
	const auto idcs = getIndicesForParameterUpdating(InstrumentType::SSG);
	for (auto idx : idcs) {
		if (auto editor = qobject_cast<SsgInstrumentEditor*>(editors_[idx])) {
			editor->onArpeggioNumberChanged();
		}
	}
}

void InstrumentEditorManager::onInstrumentSSGPitchParameterChanged(int ptNum, int fromInstNum)
{
	const auto idcs = getIndicesForParameterUpdating(InstrumentType::SSG, fromInstNum);
	for (auto idx : idcs) {
		if (auto editor = qobject_cast<SsgInstrumentEditor*>(editors_[idx])) {
			editor->onPitchParameterChanged(ptNum);
		}
	}
}

void InstrumentEditorManager::onInstrumentSSGPitchNumberChanged()
{
	const auto idcs = getIndicesForParameterUpdating(InstrumentType::SSG);
	for (auto idx : idcs) {
		if (auto editor = qobject_cast<SsgInstrumentEditor*>(editors_[idx])) {
			editor->onPitchNumberChanged();
		}
	}
}

void InstrumentEditorManager::onInstrumentADPCMSampleParameterChanged(int sampNum, int fromInstNum)
{
	Q_UNUSED(fromInstNum)	// Update all adpcm editor to change sample memory viewer and sample viewer

	const auto idcsAdpcm = getIndicesForParameterUpdating(InstrumentType::ADPCM);
	for (auto idx : idcsAdpcm) {
		if (auto editor = qobject_cast<AdpcmInstrumentEditor*>(editors_[idx])) {
			editor->onSampleParameterChanged(sampNum);
		}
	}

	const auto idcsKit = getIndicesForParameterUpdating(InstrumentType::Drumkit);
	for (auto idx : idcsKit) {
		if (auto editor = qobject_cast<AdpcmDrumkitEditor*>(editors_[idx])) {
			editor->onSampleParameterChanged(sampNum);
		}
	}
}

void InstrumentEditorManager::onInstrumentADPCMSampleNumberChanged()
{
	const auto idcsAdpcm = getIndicesForParameterUpdating(InstrumentType::ADPCM);
	for (auto idx : idcsAdpcm) {
		if (auto editor = qobject_cast<AdpcmInstrumentEditor*>(editors_[idx])) {
			editor->onSampleNumberChanged();
		}
	}

	const auto idcsKit = getIndicesForParameterUpdating(InstrumentType::Drumkit);
	for (auto idx : idcsKit) {
		if (auto editor = qobject_cast<AdpcmDrumkitEditor*>(editors_[idx])) {
			editor->onSampleNumberChanged();
		}
	}
}

void InstrumentEditorManager::onInstrumentADPCMSampleMemoryUpdated()
{
	const auto idcsAdpcm = getIndicesForParameterUpdating(InstrumentType::ADPCM);
	for (auto idx : idcsAdpcm) {
		if (auto editor = qobject_cast<AdpcmInstrumentEditor*>(editors_[idx])) {
			editor->onSampleMemoryUpdated();
		}
	}

	const auto idcsKit = getIndicesForParameterUpdating(InstrumentType::Drumkit);
	for (auto idx : idcsKit) {
		if (auto editor = qobject_cast<AdpcmDrumkitEditor*>(editors_[idx])) {
			editor->onSampleMemoryUpdated();
		}
	}
}

void InstrumentEditorManager::onInstrumentADPCMEnvelopeParameterChanged(int envNum, int fromInstNum)
{
	const auto idcs = getIndicesForParameterUpdating(InstrumentType::ADPCM, fromInstNum);
	for (auto idx : idcs) {
		if (auto editor = qobject_cast<AdpcmInstrumentEditor*>(editors_[idx])) {
			editor->onEnvelopeParameterChanged(envNum);
		}
	}
}

void InstrumentEditorManager::onInstrumentADPCMEnvelopeNumberChanged()
{
	const auto idcs = getIndicesForParameterUpdating(InstrumentType::ADPCM);
	for (auto idx : idcs) {
		if (auto editor = qobject_cast<AdpcmInstrumentEditor*>(editors_[idx])) {
			editor->onEnvelopeNumberChanged();
		}
	}
}

void InstrumentEditorManager::onInstrumentADPCMArpeggioParameterChanged(int arpNum, int fromInstNum)
{
	const auto idcs = getIndicesForParameterUpdating(InstrumentType::ADPCM, fromInstNum);
	for (auto idx : idcs) {
		if (auto editor = qobject_cast<AdpcmInstrumentEditor*>(editors_[idx])) {
			editor->onArpeggioParameterChanged(arpNum);
		}
	}
}

void InstrumentEditorManager::onInstrumentADPCMArpeggioNumberChanged()
{
	const auto idcs = getIndicesForParameterUpdating(InstrumentType::ADPCM);
	for (auto idx : idcs) {
		if (auto editor = qobject_cast<AdpcmInstrumentEditor*>(editors_[idx])) {
			editor->onArpeggioNumberChanged();
		}
	}
}

void InstrumentEditorManager::onInstrumentADPCMPitchParameterChanged(int ptNum, int fromInstNum)
{
	const auto idcs = getIndicesForParameterUpdating(InstrumentType::ADPCM, fromInstNum);
	for (auto idx : idcs) {
		if (auto editor = qobject_cast<AdpcmInstrumentEditor*>(editors_[idx])) {
			editor->onPitchParameterChanged(ptNum);
		}
	}
}

void InstrumentEditorManager::onInstrumentADPCMPitchNumberChanged()
{
	const auto idcs = getIndicesForParameterUpdating(InstrumentType::ADPCM);
	for (auto idx : idcs) {
		if (auto editor = qobject_cast<AdpcmInstrumentEditor*>(editors_[idx])) {
			editor->onPitchNumberChanged();
		}
	}
}
