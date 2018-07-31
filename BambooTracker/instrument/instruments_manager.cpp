#include "instruments_manager.hpp"
#include <utility>
#include "instrument.hpp"

InstrumentsManager::InstrumentsManager()
{
	for (int i = 0; i < 128; ++i) {
		envFM_[i] = std::make_shared<EnvelopeFM>();
	}
}

void InstrumentsManager::addInstrument(int instNum, SoundSource source, std::string name)
{
	switch (source) {
	case SoundSource::FM:	insts_[instNum] = std::make_shared<InstrumentFM>(instNum, name, this);	break;
	case SoundSource::PSG:	insts_[instNum] = std::make_shared<InstrumentPSG>(instNum, name, this);	break;
	}
}

void InstrumentsManager::addInstrument(std::unique_ptr<AbstructInstrument> inst)
{
	int num = inst->getNumber();
	insts_[num] = std::move(inst);
}

std::unique_ptr<AbstructInstrument> InstrumentsManager::removeInstrument(int instNum)
{
	std::unique_ptr<AbstructInstrument> clone = insts_[instNum]->clone();
	insts_[instNum]->setNumber(-1);	// Unregistered number
	insts_[instNum].reset();
	return std::move(clone);
}

std::shared_ptr<AbstructInstrument> InstrumentsManager::getInstrumentSharedPtr(int instNum)
{
	if (0 <= instNum && instNum < 128 && insts_[instNum] != nullptr) {
		return insts_[instNum];
	}
	else {
		return std::shared_ptr<AbstructInstrument>();	// Throw nullptr
	}
}

void InstrumentsManager::setInstrumentName(int instNum, std::string name)
{
	insts_[instNum]->setName(name);
}

std::string InstrumentsManager::getInstrumentName(int instNum) const
{
	return insts_[instNum]->getName();
}

void InstrumentsManager::setInstrumentFMEnvelope(int instNum, int envNum)
{
	std::dynamic_pointer_cast<InstrumentFM>(insts_[instNum])->setEnvelopeNumber(envNum);
}

int InstrumentsManager::getInstrumentFMEnvelope(int instNum) const
{
    return std::dynamic_pointer_cast<InstrumentFM>(insts_[instNum])->getEnvelopeNumber();
}

void InstrumentsManager::setEnvelopeFMParameter(int envNum, FMParameter param, int value)
{
	envFM_[envNum]->setParameterValue(param, value);
}

int InstrumentsManager::getEnvelopeFMParameter(int envNum, FMParameter param) const
{
	return envFM_[envNum]->getParameterValue(param);
}

void InstrumentsManager::setEnvelopeFMOperatorEnable(int envNum, int opNum, bool enable)
{
	envFM_[envNum]->setOperatorEnable(opNum, enable);
}

bool InstrumentsManager::getEnvelopeFMOperatorEnable(int envNum, int opNum) const
{
	return envFM_[envNum]->getOperatorEnable(opNum);
}
