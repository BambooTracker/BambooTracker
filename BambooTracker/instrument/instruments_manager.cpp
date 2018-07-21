#include "instruments_manager.hpp"
#include <utility>
#include "instrument_fm.hpp"
#include "instrument_psg.hpp"

InstrumentsManager::InstrumentsManager(chip::Chip& chip) :
	chip_(chip) {}

std::unique_ptr<AbstructInstrument> InstrumentsManager::addInstrument(int num, SoundSource source, std::string name)
{
	switch (source) {
	case SoundSource::FM:	map_.emplace(num, std::make_unique<InstrumentFM>(num, name));	break;
	case SoundSource::PSG:	map_.emplace(num, std::make_unique<InstrumentPSG>(num, name));	break;
	}

	return map_.at(num)->clone();
}

std::unique_ptr<AbstructInstrument> InstrumentsManager::addInstrument(std::unique_ptr<AbstructInstrument> inst)
{
	int num = inst->getNumber();
	map_.emplace(num, std::move(inst));
	return map_.at(num)->clone();
}

std::unique_ptr<AbstructInstrument> InstrumentsManager::removeInstrument(int n)
{
	std::unique_ptr<AbstructInstrument> ret = map_.at(n)->clone();
	map_.erase(n);
	return ret;
}

std::unique_ptr<AbstructInstrument> InstrumentsManager::getInstrument(int n) const
{
	return map_.at(n)->clone();
}

void InstrumentsManager::setInstrumentName(int num, std::string name)
{
	map_.at(num)->setName(name);
}

void InstrumentsManager::setFMParameterValue(int n, FMParameter param, int value)
{
	auto inst = map_.at(n).get();
	if (inst->getSoundSource() != SoundSource::FM) return;
	return dynamic_cast<InstrumentFM*>(inst)->setParameterValue(param, value);
}
