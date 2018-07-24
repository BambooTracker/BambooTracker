#include "instruments_manager.hpp"
#include <utility>
#include "instrument_fm.hpp"
#include "instrument_psg.hpp"

std::unique_ptr<AbstructInstrument> InstrumentsManager::addInstrument(int num, SoundSource source, std::string name)
{
	switch (source) {
	case SoundSource::FM:	instMap_.emplace(num, std::make_unique<InstrumentFM>(num, name));	break;
	case SoundSource::PSG:	instMap_.emplace(num, std::make_unique<InstrumentPSG>(num, name));	break;
	}

	return instMap_.at(num)->clone();
}

std::unique_ptr<AbstructInstrument> InstrumentsManager::addInstrument(std::unique_ptr<AbstructInstrument> inst)
{
	int num = inst->getNumber();
	instMap_.emplace(num, std::move(inst));
	return instMap_.at(num)->clone();
}

std::unique_ptr<AbstructInstrument> InstrumentsManager::removeInstrument(int n)
{
	std::unique_ptr<AbstructInstrument> ret = instMap_.at(n)->clone();
	instMap_.erase(n);
	return ret;
}

std::unique_ptr<AbstructInstrument> InstrumentsManager::getInstrumentCopy(int n)
{
	try {
		return instMap_.at(n)->clone();
	}
	catch (...) {
		return std::unique_ptr<AbstructInstrument>();	// Throw nullptr
	}
}

void InstrumentsManager::setInstrumentName(int num, std::string name)
{
	instMap_.at(num)->setName(name);
}

void InstrumentsManager::setFMParameterValue(int n, FMParameter param, int value)
{
	auto inst = instMap_.at(n).get();
	if (inst->getSoundSource() != SoundSource::FM) return;
	dynamic_cast<InstrumentFM*>(inst)->setParameterValue(param, value);
}

void InstrumentsManager::setFMOperatorEnable(int instNum, int opNum, bool enable)
{
	dynamic_cast<InstrumentFM*>(instMap_.at(instNum).get())->setOperatorEnable(opNum, enable);
}
