#include "instrument_form_manager.hpp"
#include <map>
#include <utility>
#include <QApplication>
#include "instrument.hpp"
#include "gui/instrument_editor/instrument_editor_fm_form.hpp"
#include "gui/instrument_editor/instrument_editor_ssg_form.hpp"
#include "gui/instrument_editor/instrument_editor_adpcm_form.hpp"
#include "gui/instrument_editor/instrument_editor_drumkit_form.hpp"
#include "misc.hpp"

InstrumentFormManager::InstrumentFormManager() {}

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

void InstrumentFormManager::remove(int n)
{
	if (!map_.count(n)) return;

	InstrumentType type = getFormInstrumentType(n);

	map_.at(n)->close();
	map_.erase(n);

	switch (type) {
	case InstrumentType::FM:
		onInstrumentFMEnvelopeNumberChanged();
		onInstrumentFMLFONumberChanged();
		onInstrumentFMOperatorSequenceNumberChanged();
		onInstrumentFMArpeggioNumberChanged();
		onInstrumentFMPitchNumberChanged();
		break;
	case InstrumentType::SSG:
		onInstrumentSSGWaveformNumberChanged();
		onInstrumentSSGEnvelopeNumberChanged();
		onInstrumentSSGToneNoiseNumberChanged();
		onInstrumentSSGArpeggioNumberChanged();
		onInstrumentSSGPitchNumberChanged();
		break;
	case InstrumentType::ADPCM:
		onInstrumentADPCMSampleNumberChanged();
		onInstrumentADPCMEnvelopeNumberChanged();
		onInstrumentADPCMArpeggioNumberChanged();
		onInstrumentADPCMPitchNumberChanged();
		break;
	case InstrumentType::Drumkit:
		onInstrumentADPCMSampleNumberChanged();
		break;
	default:
		throw std::invalid_argument("invalid instrument type");
	}
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
