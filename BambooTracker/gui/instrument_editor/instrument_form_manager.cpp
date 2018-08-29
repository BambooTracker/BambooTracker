#include "instrument_form_manager.hpp"
#include <map>
#include <utility>
#include "gui/instrument_editor/instrument_editor_fm_form.hpp"
#include "gui/instrument_editor/instrument_editor_ssg_form.hpp"
#include "misc.hpp"

InstrumentFormManager::InstrumentFormManager()
{
}

const std::unique_ptr<QWidget>& InstrumentFormManager::getForm(int n) const
{
	return map_.at(n);
}

void InstrumentFormManager::add(int n, std::unique_ptr<QWidget> form, QString instName, SoundSource instSrc)
{
	form->setProperty("Name", instName);
	form->setProperty("Shown", false);
	form->setProperty("SoundSource", static_cast<int>(instSrc));
	map_.emplace(n, std::move(form));

	// Parameter numbers update in MainWindow slot
}

void InstrumentFormManager::remove(int n)
{
	map_.at(n)->close();

	switch (getFormInstrumentSoundSource(n)) {
	case SoundSource::FM:
	{
		auto fm = qobject_cast<InstrumentEditorFMForm*>(map_.at(n).get());
		onInstrumentFMEnvelopeNumberChanged(fm->getEnvelopeNumber());
	}
	case SoundSource::SSG:
	{
		break;
	}
	}

	map_.erase(n);
}

void InstrumentFormManager::showForm(int n)
{
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

QString InstrumentFormManager::getFormInstrumentName(int n) const
{
	return map_.at(n)->property("Name").toString();
}

void InstrumentFormManager::setFormInstrumentName(int n, QString name)
{
	map_.at(n)->setProperty("Name", name);
}

SoundSource InstrumentFormManager::getFormInstrumentSoundSource(int n) const
{
	return static_cast<SoundSource>(map_.at(n)->property("SoundSource").toInt());
}

/********** Slots **********/
void InstrumentFormManager::onInstrumentFMEnvelopeParameterChanged(int envNum, int fromInstNum)
{
	for (auto& pair : map_) {
		if (pair.first != fromInstNum &&
				static_cast<SoundSource>(pair.second->property("SoundSource").toInt()) == SoundSource::FM) {
			qobject_cast<InstrumentEditorFMForm*>(pair.second.get())->onEnvelopeParameterChanged(envNum);
		}
	}
}

void InstrumentFormManager::onInstrumentFMEnvelopeNumberChanged(int envNum)
{
	for (auto& pair : map_) {
		if (static_cast<SoundSource>(pair.second->property("SoundSource").toInt()) == SoundSource::FM) {
			qobject_cast<InstrumentEditorFMForm*>(pair.second.get())->onEnvelopeNumberChanged(envNum);
		}
	}
}
