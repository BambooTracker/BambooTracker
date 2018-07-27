#include "instruments_manager.hpp"
#include <utility>
#include "instrument.hpp"

InstrumentsManager::InstrumentsManager()
{
	for (int i = 0; i < 128; ++i) {
		envFM_[i] = std::make_shared<EnvelopeFM>();
	}
}

void InstrumentsManager::addInstrument(int num, SoundSource source, std::string name)
{
	switch (source) {
	case SoundSource::FM:	insts_[num] = std::make_unique<InstrumentFM>(num, name, this);	break;
	case SoundSource::PSG:	insts_[num] = std::make_unique<InstrumentPSG>(num, name, this);	break;
	}
}

void InstrumentsManager::addInstrument(std::unique_ptr<AbstructInstrument> inst)
{
	int num = inst->getNumber();
	insts_[num] = std::move(inst);
}

std::unique_ptr<AbstructInstrument> InstrumentsManager::removeInstrument(int n)
{
	return std::move(insts_[n]);
}

std::unique_ptr<AbstructInstrument> InstrumentsManager::getInstrumentCopy(int n)
{
	if (0 <= n && n < 128 && insts_[n] != nullptr) {
		return insts_[n]->clone();
	}
	else {
		return std::unique_ptr<AbstructInstrument>();	// Throw nullptr
	}
}

void InstrumentsManager::setInstrumentName(int num, std::string name)
{
	insts_[num]->setName(name);
}

void InstrumentsManager::setFMEnvelopeParameter(int envNum, FMParameter param, int value)
{
	envFM_[envNum]->setParameterValue(param, value);
}

int InstrumentsManager::getFMEnvelopeParameter(int envNum, FMParameter param) const
{
	return envFM_[envNum]->getParameterValue(param);
}

void InstrumentsManager::setFMOperatorEnable(int envNum, int opNum, bool enable)
{
	envFM_[envNum]->setOperatorEnable(opNum, enable);
}

bool InstrumentsManager::getFMOperatorEnable(int envNum, int opNum) const
{
	return envFM_[envNum]->getOperatorEnable(opNum);
}
