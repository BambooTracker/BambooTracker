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

#include "instrument_form_manager.hpp"
#include <utility>
#include <QApplication>
#include "instrument.hpp"
#include "gui/instrument_editor/instrument_editor_fm_form.hpp"
#include "gui/instrument_editor/instrument_editor_ssg_form.hpp"
#include "gui/instrument_editor/instrument_editor_adpcm_form.hpp"
#include "gui/instrument_editor/instrument_editor_drumkit_form.hpp"

void InstrumentFormManager::updateByConfiguration()
{
	for (auto& pair : map_) {
		if (static_cast<InstrumentType>(pair.second->property("Type").toInt()) == InstrumentType::FM) {
			qobject_cast<InstrumentEditorFMForm*>(pair.second.get())->updateConfigurationForDisplay();
		}
	}
}

const std::shared_ptr<QWidget> InstrumentFormManager::getForm(int n) const
{
	if (map_.count(n)) return map_.at(n);
	else return std::shared_ptr<QWidget>();	// nullptr
}

void InstrumentFormManager::add(int n, std::shared_ptr<QWidget> form, SoundSource src, InstrumentType type)
{
	form->setProperty("Shown", false);
	form->setProperty("Source", static_cast<int>(src));
	form->setProperty("Type", static_cast<int>(type));
	map_.emplace(n, std::move(form));

	// Parameter numbers update in MainWindow
}

void InstrumentFormManager::swap(int a, int b)
{
	auto changeNumber = [&](int id, InstrumentType type) {
		QWidget* form = map_.at(id).get();
		switch (type) {
		case InstrumentType::FM:
			qobject_cast<InstrumentEditorFMForm*>(form)->setInstrumentNumber(id);
			break;
		case InstrumentType::SSG:
			qobject_cast<InstrumentEditorSSGForm*>(form)->setInstrumentNumber(id);
			break;
		case InstrumentType::ADPCM:
			qobject_cast<InstrumentEditorADPCMForm*>(form)->setInstrumentNumber(id);
			break;
		case InstrumentType::Drumkit:
			qobject_cast<InstrumentEditorDrumkitForm*>(form)->setInstrumentNumber(id);
			break;
		}
	};

	if (map_.count(a)) {
		if (map_.count(b)) {
			std::swap(map_.at(a), map_.at(b));
			changeNumber(a, getFormInstrumentType(a));
			changeNumber(b, getFormInstrumentType(b));
		}
		else {
			map_[b] = map_.at(a);
			map_.erase(a);
			changeNumber(b, getFormInstrumentType(b));
		}
	}
	else {
		if (map_.count(b)) {
			map_[a] = map_.at(b);
			map_.erase(b);
			changeNumber(a, getFormInstrumentType(a));
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

void InstrumentFormManager::remove(int n)
{
	if (map_.count(n)) {
		map_.at(n)->close();
		map_.erase(n);
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

void InstrumentFormManager::showForm(int n)
{
	if (!map_.count(n)) return;
	auto& form = map_.at(n);
	if (form->isVisible()) {
		form->activateWindow();
	}
	else {
		form->setProperty("Shown", true);
		form->show();
	}
}

void InstrumentFormManager::closeAll()
{
	for (auto& pair : map_) {
		pair.second->close();
	}
}

void InstrumentFormManager::clearAll()
{
	closeAll();
	map_.clear();
}

SoundSource InstrumentFormManager::getFormInstrumentSoundSource(int n) const
{
	return static_cast<SoundSource>(map_.at(n)->property("Source").toInt());
}

InstrumentType InstrumentFormManager::getFormInstrumentType(int n) const
{
	return static_cast<InstrumentType>(map_.at(n)->property("Type").toInt());
}

int InstrumentFormManager::checkActivatedFormNumber() const
{
	const QWidget* win = QApplication::activeWindow();
	auto it = std::find_if(map_.begin(), map_.end(),
						   [win](const std::pair<const int, std::shared_ptr<QWidget>> p) {
		return p.second.get() == win;
	});

	return (it == map_.end() ? -1 : it->first);
}

/********** Slots **********/
void InstrumentFormManager::onInstrumentFMEnvelopeParameterChanged(int envNum, int fromInstNum)
{
	for (auto& pair : map_) {
		if (pair.first != fromInstNum &&
				static_cast<SoundSource>(pair.second->property("Source").toInt()) == SoundSource::FM) {
			qobject_cast<InstrumentEditorFMForm*>(pair.second.get())->onEnvelopeParameterChanged(envNum);
		}
	}
}

void InstrumentFormManager::onInstrumentFMEnvelopeNumberChanged()
{
	for (auto& pair : map_) {
		if (static_cast<SoundSource>(pair.second->property("Source").toInt()) == SoundSource::FM) {
			qobject_cast<InstrumentEditorFMForm*>(pair.second.get())->onEnvelopeNumberChanged();
		}
	}
}

void InstrumentFormManager::onInstrumentFMLFOParameterChanged(int lfoNum, int fromInstNum)
{
	for (auto& pair : map_) {
		if (pair.first != fromInstNum &&
				static_cast<SoundSource>(pair.second->property("Source").toInt()) == SoundSource::FM) {
			qobject_cast<InstrumentEditorFMForm*>(pair.second.get())->onLFOParameterChanged(lfoNum);
		}
	}
}

void InstrumentFormManager::onInstrumentFMLFONumberChanged()
{
	for (auto& pair : map_) {
		if (static_cast<SoundSource>(pair.second->property("Source").toInt()) == SoundSource::FM) {
			qobject_cast<InstrumentEditorFMForm*>(pair.second.get())->onLFONumberChanged();
		}
	}
}

void InstrumentFormManager::onInstrumentFMOperatorSequenceParameterChanged(FMEnvelopeParameter param, int opSeqNum, int fromInstNum)
{
	for (auto& pair : map_) {
		if (pair.first != fromInstNum &&
				static_cast<SoundSource>(pair.second->property("Source").toInt()) == SoundSource::FM) {
			qobject_cast<InstrumentEditorFMForm*>(pair.second.get())->onOperatorSequenceParameterChanged(param, opSeqNum);
		}
	}
}

void InstrumentFormManager::onInstrumentFMOperatorSequenceNumberChanged()
{
	for (auto& pair : map_) {
		if (static_cast<SoundSource>(pair.second->property("Source").toInt()) == SoundSource::FM) {
			qobject_cast<InstrumentEditorFMForm*>(pair.second.get())->onOperatorSequenceNumberChanged();
		}
	}
}

void InstrumentFormManager::onInstrumentFMArpeggioParameterChanged(int arpNum, int fromInstNum)
{
	for (auto& pair : map_) {
		if (pair.first != fromInstNum &&
				static_cast<SoundSource>(pair.second->property("Source").toInt()) == SoundSource::FM) {
			qobject_cast<InstrumentEditorFMForm*>(pair.second.get())->onArpeggioParameterChanged(arpNum);
		}
	}
}

void InstrumentFormManager::onInstrumentFMArpeggioNumberChanged()
{
	for (auto& pair : map_) {
		if (static_cast<SoundSource>(pair.second->property("Source").toInt()) == SoundSource::FM) {
			qobject_cast<InstrumentEditorFMForm*>(pair.second.get())->onArpeggioNumberChanged();
		}
	}
}

void InstrumentFormManager::onInstrumentFMPitchParameterChanged(int ptNum, int fromInstNum)
{
	for (auto& pair : map_) {
		if (pair.first != fromInstNum &&
				static_cast<SoundSource>(pair.second->property("Source").toInt()) == SoundSource::FM) {
			qobject_cast<InstrumentEditorFMForm*>(pair.second.get())->onPitchParameterChanged(ptNum);
		}
	}
}

void InstrumentFormManager::onInstrumentFMPitchNumberChanged()
{
	for (auto& pair : map_) {
		if (static_cast<SoundSource>(pair.second->property("Source").toInt()) == SoundSource::FM) {
			qobject_cast<InstrumentEditorFMForm*>(pair.second.get())->onPitchNumberChanged();
		}
	}
}

void InstrumentFormManager::onInstrumentSSGWaveformParameterChanged(int wfNum, int fromInstNum)
{
	for (auto& pair : map_) {
		if (pair.first != fromInstNum &&
				static_cast<SoundSource>(pair.second->property("Source").toInt()) == SoundSource::SSG) {
			qobject_cast<InstrumentEditorSSGForm*>(pair.second.get())->onWaveformParameterChanged(wfNum);
		}
	}
}

void InstrumentFormManager::onInstrumentSSGWaveformNumberChanged()
{
	for (auto& pair : map_) {
		if (static_cast<SoundSource>(pair.second->property("Source").toInt()) == SoundSource::SSG) {
			qobject_cast<InstrumentEditorSSGForm*>(pair.second.get())->onWaveformNumberChanged();
		}
	}
}

void InstrumentFormManager::onInstrumentSSGToneNoiseParameterChanged(int tnNum, int fromInstNum)
{
	for (auto& pair : map_) {
		if (pair.first != fromInstNum &&
				static_cast<SoundSource>(pair.second->property("Source").toInt()) == SoundSource::SSG) {
			qobject_cast<InstrumentEditorSSGForm*>(pair.second.get())->onToneNoiseParameterChanged(tnNum);
		}
	}
}

void InstrumentFormManager::onInstrumentSSGToneNoiseNumberChanged()
{
	for (auto& pair : map_) {
		if (static_cast<SoundSource>(pair.second->property("Source").toInt()) == SoundSource::SSG) {
			qobject_cast<InstrumentEditorSSGForm*>(pair.second.get())->onToneNoiseNumberChanged();
		}
	}
}

void InstrumentFormManager::onInstrumentSSGEnvelopeParameterChanged(int envNum, int fromInstNum)
{
	for (auto& pair : map_) {
		if (pair.first != fromInstNum &&
				static_cast<SoundSource>(pair.second->property("Source").toInt()) == SoundSource::SSG) {
			qobject_cast<InstrumentEditorSSGForm*>(pair.second.get())->onEnvelopeParameterChanged(envNum);
		}
	}
}

void InstrumentFormManager::onInstrumentSSGEnvelopeNumberChanged()
{
	for (auto& pair : map_) {
		if (static_cast<SoundSource>(pair.second->property("Source").toInt()) == SoundSource::SSG) {
			qobject_cast<InstrumentEditorSSGForm*>(pair.second.get())->onEnvelopeNumberChanged();
		}
	}
}

void InstrumentFormManager::onInstrumentSSGArpeggioParameterChanged(int arpNum, int fromInstNum)
{
	for (auto& pair : map_) {
		if (pair.first != fromInstNum &&
				static_cast<SoundSource>(pair.second->property("Source").toInt()) == SoundSource::SSG) {
			qobject_cast<InstrumentEditorSSGForm*>(pair.second.get())->onArpeggioParameterChanged(arpNum);
		}
	}
}

void InstrumentFormManager::onInstrumentSSGArpeggioNumberChanged()
{
	for (auto& pair : map_) {
		if (static_cast<SoundSource>(pair.second->property("Source").toInt()) == SoundSource::SSG) {
			qobject_cast<InstrumentEditorSSGForm*>(pair.second.get())->onArpeggioNumberChanged();
		}
	}
}

void InstrumentFormManager::onInstrumentSSGPitchParameterChanged(int ptNum, int fromInstNum)
{
	for (auto& pair : map_) {
		if (pair.first != fromInstNum &&
				static_cast<SoundSource>(pair.second->property("Source").toInt()) == SoundSource::SSG) {
			qobject_cast<InstrumentEditorSSGForm*>(pair.second.get())->onPitchParameterChanged(ptNum);
		}
	}
}

void InstrumentFormManager::onInstrumentSSGPitchNumberChanged()
{
	for (auto& pair : map_) {
		if (static_cast<SoundSource>(pair.second->property("Source").toInt()) == SoundSource::SSG) {
			qobject_cast<InstrumentEditorSSGForm*>(pair.second.get())->onPitchNumberChanged();
		}
	}
}

void InstrumentFormManager::onInstrumentADPCMSampleParameterChanged(int sampNum, int fromInstNum)
{
	Q_UNUSED(fromInstNum)	// Update all adpcm editor to change sample memory viewer and sample viewer
	for (auto& pair : map_) {
		if (static_cast<SoundSource>(pair.second->property("Source").toInt()) == SoundSource::ADPCM) {
			if (auto adpcmForm = qobject_cast<InstrumentEditorADPCMForm*>(pair.second.get()))
				adpcmForm->onSampleParameterChanged(sampNum);
			else if (auto kitForm = qobject_cast<InstrumentEditorDrumkitForm*>(pair.second.get()))
				kitForm->onSampleParameterChanged(sampNum);
		}
	}
}

void InstrumentFormManager::onInstrumentADPCMSampleNumberChanged()
{
	for (auto& pair : map_) {
		if (static_cast<SoundSource>(pair.second->property("Source").toInt()) == SoundSource::ADPCM) {
			if (auto adpcmForm = qobject_cast<InstrumentEditorADPCMForm*>(pair.second.get()))
				adpcmForm->onSampleNumberChanged();
			else if (auto kitForm = qobject_cast<InstrumentEditorDrumkitForm*>(pair.second.get()))
				kitForm->onSampleNumberChanged();
		}
	}
}

void InstrumentFormManager::onInstrumentADPCMSampleMemoryUpdated()
{
	for (auto& pair : map_) {
		if (static_cast<SoundSource>(pair.second->property("Source").toInt()) == SoundSource::ADPCM) {
			if (auto adpcmForm = qobject_cast<InstrumentEditorADPCMForm*>(pair.second.get()))
				adpcmForm->onSampleMemoryUpdated();
			else if (auto kitForm = qobject_cast<InstrumentEditorDrumkitForm*>(pair.second.get()))
				kitForm->onSampleMemoryUpdated();
		}
	}
}

void InstrumentFormManager::onInstrumentADPCMEnvelopeParameterChanged(int envNum, int fromInstNum)
{
	for (auto& pair : map_) {
		if (pair.first != fromInstNum &&
				static_cast<InstrumentType>(pair.second->property("Type").toInt()) == InstrumentType::ADPCM) {
			qobject_cast<InstrumentEditorADPCMForm*>(pair.second.get())->onEnvelopeParameterChanged(envNum);
		}
	}
}

void InstrumentFormManager::onInstrumentADPCMEnvelopeNumberChanged()
{
	for (auto& pair : map_) {
		if (static_cast<InstrumentType>(pair.second->property("Type").toInt()) == InstrumentType::ADPCM) {
			qobject_cast<InstrumentEditorADPCMForm*>(pair.second.get())->onEnvelopeNumberChanged();
		}
	}
}

void InstrumentFormManager::onInstrumentADPCMArpeggioParameterChanged(int arpNum, int fromInstNum)
{
	for (auto& pair : map_) {
		if (pair.first != fromInstNum &&
				static_cast<InstrumentType>(pair.second->property("Type").toInt()) == InstrumentType::ADPCM) {
			qobject_cast<InstrumentEditorADPCMForm*>(pair.second.get())->onArpeggioParameterChanged(arpNum);
		}
	}
}

void InstrumentFormManager::onInstrumentADPCMArpeggioNumberChanged()
{
	for (auto& pair : map_) {
		if (static_cast<InstrumentType>(pair.second->property("Type").toInt()) == InstrumentType::ADPCM) {
			qobject_cast<InstrumentEditorADPCMForm*>(pair.second.get())->onArpeggioNumberChanged();
		}
	}
}

void InstrumentFormManager::onInstrumentADPCMPitchParameterChanged(int ptNum, int fromInstNum)
{
	for (auto& pair : map_) {
		if (pair.first != fromInstNum &&
				static_cast<InstrumentType>(pair.second->property("Type").toInt()) == InstrumentType::ADPCM) {
			qobject_cast<InstrumentEditorADPCMForm*>(pair.second.get())->onPitchParameterChanged(ptNum);
		}
	}
}

void InstrumentFormManager::onInstrumentADPCMPitchNumberChanged()
{
	for (auto& pair : map_) {
		if (static_cast<InstrumentType>(pair.second->property("Type").toInt()) == InstrumentType::ADPCM) {
			qobject_cast<InstrumentEditorADPCMForm*>(pair.second.get())->onPitchNumberChanged();
		}
	}
}
