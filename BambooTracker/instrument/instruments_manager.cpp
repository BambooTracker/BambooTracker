#include "instruments_manager.hpp"
#include <algorithm>
#include "instrument.hpp"

InstrumentsManager::InstrumentsManager()
{
	int cnt = 0;
	std::generate(envFM_.begin(), envFM_.end(), [&]() { return std::make_shared<EnvelopeFM>(cnt++); });
}

void InstrumentsManager::addInstrument(int instNum, SoundSource source, std::string name)
{
	switch (source) {
	case SoundSource::FM:
		insts_.at(instNum) = std::make_shared<InstrumentFM>(instNum, name, this);
		envFM_.at(0)->registerUserInstrument(instNum);	// Setted 0 in init
		break;
	case SoundSource::SSG:
		insts_.at(instNum) = std::make_shared<InstrumentSSG>(instNum, name, this);
		break;
	}
}

void InstrumentsManager::addInstrument(std::unique_ptr<AbstructInstrument> inst)
{	
	int num = inst->getNumber();
	insts_.at(num) = std::move(inst);

	switch (insts_[num]->getSoundSource()) {
	case SoundSource::FM:
	{
		auto fm = std::dynamic_pointer_cast<InstrumentFM>(insts_[num]);
		envFM_.at(fm->getEnvelopeNumber())->registerUserInstrument(num);
		break;
	}
	case SoundSource::SSG:
		// UNDONE: SSG memory number registering
		break;
	}
}

void InstrumentsManager::cloneInstrument(int cloneInstNum, int refInstNum)
{
	std::shared_ptr<AbstructInstrument> refInst = insts_.at(refInstNum);
	addInstrument(cloneInstNum, refInst->getSoundSource(), refInst->getName());

	setInstrumentGateCount(cloneInstNum, refInst->getGateCount());
	switch (refInst->getSoundSource()) {
	case SoundSource::FM:
	{
		auto refFm = std::dynamic_pointer_cast<InstrumentFM>(refInst);
		setInstrumentFMEnvelope(cloneInstNum, refFm->getEnvelopeNumber());
		setInstrumentFMEnvelopeResetEnabled(cloneInstNum, refFm->getEnvelopeResetEnabled());
		break;
	}
	case SoundSource::SSG:
		// UNODNE
		break;
	}
}

void InstrumentsManager::deepCloneInstrument(int cloneInstNum, int refInstNum)
{
	std::shared_ptr<AbstructInstrument> refInst = insts_.at(refInstNum);
	addInstrument(cloneInstNum, refInst->getSoundSource(), refInst->getName());

	setInstrumentGateCount(cloneInstNum, refInst->getGateCount());
	switch (refInst->getSoundSource()) {
	case SoundSource::FM:
	{
		auto refFm = std::dynamic_pointer_cast<InstrumentFM>(refInst);
		auto cloneFm = std::dynamic_pointer_cast<InstrumentFM>(insts_.at(cloneInstNum));
		
		envFM_[0]->deregisterUserInstrument(cloneInstNum);	// Remove temporary number
		int envNum = cloneFMEnvelope(refFm->getEnvelopeNumber());
		cloneFm->setEnvelopeNumber(envNum);
		envFM_[envNum]->registerUserInstrument(cloneInstNum);
		setInstrumentFMEnvelopeResetEnabled(cloneInstNum, refFm->getEnvelopeResetEnabled());
		break;
	}
	case SoundSource::SSG:
		// UNODNE: ssg envelope number registering
		break;
	}
}

int InstrumentsManager::cloneFMEnvelope(int srcNum)
{
	int cloneNum = 0;
	for (auto& env : envFM_) {
		if (!env->isUserInstrument()) {
			env = envFM_.at(srcNum)->clone();
			env->setNumber(cloneNum);
			break;
		}
		++cloneNum;
	}
	return cloneNum;
}

std::unique_ptr<AbstructInstrument> InstrumentsManager::removeInstrument(int instNum)
{	
	switch (insts_.at(instNum)->getSoundSource()) {
	case SoundSource::FM:
	{
		auto fm = std::dynamic_pointer_cast<InstrumentFM>(insts_[instNum]);
		envFM_.at(fm->getEnvelopeNumber())->deregisterUserInstrument(instNum);
		break;
	}
	case SoundSource::SSG:
		// UNODNE: ssg envelope number deregistering
		break;
	}

	std::unique_ptr<AbstructInstrument> clone = insts_[instNum]->clone();
	insts_[instNum]->setNumber(-1);	// Unregistered number
	insts_[instNum].reset();
	return clone;
}

std::shared_ptr<AbstructInstrument> InstrumentsManager::getInstrumentSharedPtr(int instNum)
{
	if (0 <= instNum && instNum < 128 && insts_.at(instNum) != nullptr) {
		return insts_[instNum];
	}
	else {
		return std::shared_ptr<AbstructInstrument>();	// Throw nullptr
	}
}

void InstrumentsManager::setInstrumentName(int instNum, std::string name)
{
	insts_.at(instNum)->setName(name);
}

std::string InstrumentsManager::getInstrumentName(int instNum) const
{
	return insts_.at(instNum)->getName();
}

/// Return:
///		-1: no free instrument
///		else: first free instrument number
int InstrumentsManager::findFirstFreeInstrument() const
{
	int num = 0;
	for (auto& inst : insts_) {
		if (inst == nullptr) return num;
		++num;
	}
	return -1;
}

void InstrumentsManager::setInstrumentGateCount(int instNum, int count)
{
	insts_.at(instNum)->setGateCount(count);
}

//----- FM methods -----
void InstrumentsManager::setInstrumentFMEnvelope(int instNum, int envNum)
{
	auto fm = std::dynamic_pointer_cast<InstrumentFM>(insts_.at(instNum));
	envFM_.at(fm->getEnvelopeNumber())->deregisterUserInstrument(instNum);
	envFM_.at(envNum)->registerUserInstrument(instNum);

	fm->setEnvelopeNumber(envNum);
}

int InstrumentsManager::getInstrumentFMEnvelope(int instNum) const
{
    return std::dynamic_pointer_cast<InstrumentFM>(insts_[instNum])->getEnvelopeNumber();
}

void InstrumentsManager::setEnvelopeFMParameter(int envNum, FMParameter param, int value)
{
	envFM_.at(envNum)->setParameterValue(param, value);
}

int InstrumentsManager::getEnvelopeFMParameter(int envNum, FMParameter param) const
{
	return envFM_.at(envNum)->getParameterValue(param);
}

void InstrumentsManager::setEnvelopeFMOperatorEnable(int envNum, int opNum, bool enable)
{
	envFM_.at(envNum)->setOperatorEnable(opNum, enable);
}

bool InstrumentsManager::getEnvelopeFMOperatorEnable(int envNum, int opNum) const
{
	return envFM_.at(envNum)->getOperatorEnable(opNum);
}

std::vector<int> InstrumentsManager::getEnvelopeFMUsers(int envNum) const
{
	return envFM_.at(envNum)->getUserInstruments();
}

void InstrumentsManager::setInstrumentFMEnvelopeResetEnabled(int instNum, bool enabled)
{
	std::dynamic_pointer_cast<InstrumentFM>(insts_[instNum])->setEnvelopeResetEnabled(enabled);
}
