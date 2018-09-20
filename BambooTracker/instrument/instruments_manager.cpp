#include "instruments_manager.hpp"
#include <utility>
#include "instrument.hpp"

InstrumentsManager::InstrumentsManager()
{
	for (int i = 0; i < 128; ++i) {
		envFM_[i] = std::make_shared<EnvelopeFM>(i);
		lfoFM_[i] = std::make_shared<LFOFM>(i);

		wfSSG_[i] = std::make_shared<CommandSequence>(i, 0);
		tnSSG_[i] = std::make_shared<CommandSequence>(i, 0);
		envSSG_[i] = std::make_shared<CommandSequence>(i, 0, 15);
		arpSSG_[i] = std::make_shared<CommandSequence>(i, 0, 48);
	}
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
		int lfoNum = fm->getLFONumber();
		if (lfoNum != -1) lfoFM_.at(lfoNum)->registerUserInstrument(num);
		break;
	}
	case SoundSource::SSG:
		auto ssg = std::dynamic_pointer_cast<InstrumentSSG>(insts_[num]);
		int wfNum = ssg->getWaveFormNumber();
		if (wfNum != -1) wfSSG_.at(wfNum)->registerUserInstrument(num);
		int tnNum = ssg->getToneNoiseNumber();
		if (tnNum != -1) tnSSG_.at(tnNum)->registerUserInstrument(num);
		int envNum = ssg->getEnvelopeNumber();
		if (envNum != -1) envSSG_.at(envNum)->registerUserInstrument(num);
		int arpNum = ssg->getArpeggioNumber();
		if (arpNum != -1) arpSSG_.at(arpNum)->registerUserInstrument(num);
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
		setInstrumentFMLFO(cloneInstNum, refFm->getLFONumber());
		setInstrumentFMEnvelopeResetEnabled(cloneInstNum, refFm->getEnvelopeResetEnabled());
		break;
	}
	case SoundSource::SSG:
		auto refSsg = std::dynamic_pointer_cast<InstrumentSSG>(refInst);
		setInstrumentSSGWaveForm(cloneInstNum, refSsg->getWaveFormNumber());
		setInstrumentSSGToneNoise(cloneInstNum, refSsg->getToneNoiseNumber());
		setInstrumentSSGEnvelope(cloneInstNum, refSsg->getEnvelopeNumber());
		setInstrumentSSGArpeggio(cloneInstNum, refSsg->getArpeggioNumber());
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

		if (refFm->getLFONumber() != -1) {
			int lfoNum = cloneFMLFO(refFm->getLFONumber());
			cloneFm->setLFONumber(lfoNum);
			lfoFM_[lfoNum]->registerUserInstrument(cloneInstNum);
		}

		setInstrumentFMEnvelopeResetEnabled(cloneInstNum, refFm->getEnvelopeResetEnabled());
		break;
	}
	case SoundSource::SSG:
		auto refSsg = std::dynamic_pointer_cast<InstrumentSSG>(refInst);
		auto cloneSsg = std::dynamic_pointer_cast<InstrumentSSG>(insts_.at(cloneInstNum));

		if (refSsg->getWaveFormNumber() != -1) {
			int wfNum = cloneSSGWaveForm(refSsg->getWaveFormNumber());
			cloneSsg->setWaveFormNumber(wfNum);
			wfSSG_[wfNum]->registerUserInstrument(cloneInstNum);
		}
		if (refSsg->getToneNoiseNumber() != -1) {
			int tnNum = cloneSSGToneNoise(refSsg->getToneNoiseNumber());
			cloneSsg->setToneNoiseNumber(tnNum);
			tnSSG_[tnNum]->registerUserInstrument(cloneInstNum);
		}
		if (refSsg->getEnvelopeNumber() != -1) {
			int envNum = cloneSSGEnvelope(refSsg->getEnvelopeNumber());
			cloneSsg->setEnvelopeNumber(envNum);
			envSSG_[envNum]->registerUserInstrument(cloneInstNum);
		}
		if (refSsg->getArpeggioNumber() != -1) {
			int arpNum = cloneSSGArpeggio(refSsg->getArpeggioNumber());
			cloneSsg->setArpeggioNumber(arpNum);
			arpSSG_[arpNum]->registerUserInstrument(cloneInstNum);
		}
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

int InstrumentsManager::cloneFMLFO(int srcNum)
{
	int cloneNum = 0;
	for (auto& lfo : lfoFM_) {
		if (!lfo->isUserInstrument()) {
			lfo = lfoFM_.at(srcNum)->clone();
			lfo->setNumber(cloneNum);
			break;
		}
		++cloneNum;
	}
	return cloneNum;
}

int InstrumentsManager::cloneSSGWaveForm(int srcNum)
{
	int cloneNum = 0;
	for (auto& wf : wfSSG_) {
		if (!wf->isUserInstrument()) {
			wf = wfSSG_.at(srcNum)->clone();
			wf->setNumber(cloneNum);
			break;
		}
		++cloneNum;
	}
	return cloneNum;
}

int InstrumentsManager::cloneSSGToneNoise(int srcNum)
{
	int cloneNum = 0;
	for (auto& tn : tnSSG_) {
		if (!tn->isUserInstrument()) {
			tn = tnSSG_.at(srcNum)->clone();
			tn->setNumber(cloneNum);
			break;
		}
		++cloneNum;
	}
	return cloneNum;
}

int InstrumentsManager::cloneSSGEnvelope(int srcNum)
{
	int cloneNum = 0;
	for (auto& env : envSSG_) {
		if (!env->isUserInstrument()) {
			env = envSSG_.at(srcNum)->clone();
			env->setNumber(cloneNum);
			break;
		}
		++cloneNum;
	}
	return cloneNum;
}

int InstrumentsManager::cloneSSGArpeggio(int srcNum)
{
	int cloneNum = 0;
	for (auto& arp : arpSSG_) {
		if (!arp->isUserInstrument()) {
			arp = arpSSG_.at(srcNum)->clone();
			arp->setNumber(cloneNum);
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
		int lfoNum = fm->getLFONumber();
		if (lfoNum != -1) lfoFM_.at(lfoNum)->deregisterUserInstrument(instNum);
		break;
	}
	case SoundSource::SSG:
		auto ssg = std::dynamic_pointer_cast<InstrumentSSG>(insts_[instNum]);
		int wfNum = ssg->getWaveFormNumber();
		if (wfNum != -1) wfSSG_.at(wfNum)->deregisterUserInstrument(instNum);
		int tnNum = ssg->getToneNoiseNumber();
		if (tnNum != -1) tnSSG_.at(tnNum)->deregisterUserInstrument(instNum);
		int envNum = ssg->getEnvelopeNumber();
		if (envNum != -1) envSSG_.at(envNum)->deregisterUserInstrument(instNum);
		int arpNum = ssg->getArpeggioNumber();
		if (arpNum != -1) arpSSG_.at(arpNum)->deregisterUserInstrument(instNum);
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

void InstrumentsManager::setEnvelopeFMParameter(int envNum, FMEnvelopeParameter param, int value)
{
	envFM_.at(envNum)->setParameterValue(param, value);
}

int InstrumentsManager::getEnvelopeFMParameter(int envNum, FMEnvelopeParameter param) const
{
	return envFM_.at(envNum)->getParameterValue(param);
}

void InstrumentsManager::setEnvelopeFMOperatorEnabled(int envNum, int opNum, bool enabled)
{
	envFM_.at(envNum)->setOperatorEnabled(opNum, enabled);
}

bool InstrumentsManager::getEnvelopeFMOperatorEnabled(int envNum, int opNum) const
{
	return envFM_.at(envNum)->getOperatorEnabled(opNum);
}

std::vector<int> InstrumentsManager::getEnvelopeFMUsers(int envNum) const
{
	return envFM_.at(envNum)->getUserInstruments();
}

void InstrumentsManager::setInstrumentFMLFO(int instNum, int lfoNum)
{
	auto fm = std::dynamic_pointer_cast<InstrumentFM>(insts_.at(instNum));
	int prevLfo = fm->getLFONumber();
	if (prevLfo != -1)
		lfoFM_.at(prevLfo)->deregisterUserInstrument(instNum);
	if (lfoNum != -1)
		lfoFM_.at(lfoNum)->registerUserInstrument(instNum);

	fm->setLFONumber(lfoNum);
}

int InstrumentsManager::getInstrumentFMLFO(int instNum) const
{
	return std::dynamic_pointer_cast<InstrumentFM>(insts_[instNum])->getLFONumber();
}

void InstrumentsManager::setLFOFMParameter(int lfoNum, FMLFOParamter param, int value)
{
	lfoFM_.at(lfoNum)->setParameterValue(param, value);
}

int InstrumentsManager::getLFOFMparameter(int lfoNum, FMLFOParamter param) const
{
	return lfoFM_.at(lfoNum)->getParameterValue(param);
}

std::vector<int> InstrumentsManager::getLFOFMUsers(int lfoNum) const
{
	return lfoFM_.at(lfoNum)->getUserInstruments();
}

void InstrumentsManager::setInstrumentFMEnvelopeResetEnabled(int instNum, bool enabled)
{
	std::dynamic_pointer_cast<InstrumentFM>(insts_[instNum])->setEnvelopeResetEnabled(enabled);
}

//----- SSG methods -----
void InstrumentsManager::setInstrumentSSGWaveForm(int instNum, int wfNum)
{
	auto ssg = std::dynamic_pointer_cast<InstrumentSSG>(insts_.at(instNum));
	int prevWf = ssg->getWaveFormNumber();
	if (prevWf != -1)
		wfSSG_.at(prevWf)->deregisterUserInstrument(instNum);
	if (wfNum != -1)
		wfSSG_.at(wfNum)->registerUserInstrument(instNum);

	ssg->setWaveFormNumber(wfNum);
}

int InstrumentsManager::getInstrumentSSGWaveForm(int instNum)
{
	return std::dynamic_pointer_cast<InstrumentSSG>(insts_[instNum])->getWaveFormNumber();
}

void InstrumentsManager::addWaveFormSSGSequenceCommand(int wfNum, int type, int data)
{
	wfSSG_.at(wfNum)->addSequenceCommand(type, data);
}

void InstrumentsManager::removeWaveFormSSGSequenceCommand(int wfNum)
{
	wfSSG_.at(wfNum)->removeSequenceCommand();
}

void InstrumentsManager::setWaveFormSSGSequenceCommand(int wfNum, int cnt, int type, int data)
{
	wfSSG_.at(wfNum)->setSequenceCommand(cnt, type, data);
}

std::vector<CommandInSequence> InstrumentsManager::getWaveFormSSGSequence(int wfNum)
{
	return wfSSG_.at(wfNum)->getSequence();
}

void InstrumentsManager::setWaveFormSSGLoops(int wfNum, std::vector<int> begins, std::vector<int> ends, std::vector<int> times)
{
	wfSSG_.at(wfNum)->setLoops(std::move(begins), std::move(ends), std::move(times));
}

std::vector<Loop> InstrumentsManager::getWaveFormSSGLoops(int wfNum) const
{
	return wfSSG_.at(wfNum)->getLoops();
}

void InstrumentsManager::setWaveFormSSGRelease(int wfNum, ReleaseType type, int begin)
{
	wfSSG_.at(wfNum)->setRelease(type, begin);
}

Release InstrumentsManager::getWaveFormSSGRelease(int wfNum) const
{
	return wfSSG_.at(wfNum)->getRelease();
}

std::unique_ptr<CommandSequence::Iterator> InstrumentsManager::getWaveFormSSGIterator(int wfNum) const
{
	return wfSSG_.at(wfNum)->getIterator();
}

std::vector<int> InstrumentsManager::getWaveFormSSGUsers(int wfNum) const
{
	return wfSSG_.at(wfNum)->getUserInstruments();
}

void InstrumentsManager::setInstrumentSSGToneNoise(int instNum, int tnNum)
{
	auto ssg = std::dynamic_pointer_cast<InstrumentSSG>(insts_.at(instNum));
	int prevTn = ssg->getToneNoiseNumber();
	if (prevTn != -1)
		tnSSG_.at(prevTn)->deregisterUserInstrument(instNum);
	if (tnNum != -1)
		tnSSG_.at(tnNum)->registerUserInstrument(instNum);

	ssg->setToneNoiseNumber(tnNum);
}

int InstrumentsManager::getInstrumentSSGToneNoise(int instNum)
{
	return std::dynamic_pointer_cast<InstrumentSSG>(insts_[instNum])->getToneNoiseNumber();
}

void InstrumentsManager::addToneNoiseSSGSequenceCommand(int tnNum, int type, int data)
{
	tnSSG_.at(tnNum)->addSequenceCommand(type, data);
}

void InstrumentsManager::removeToneNoiseSSGSequenceCommand(int tnNum)
{
	tnSSG_.at(tnNum)->removeSequenceCommand();
}

void InstrumentsManager::setToneNoiseSSGSequenceCommand(int tnNum, int cnt, int type, int data)
{
	tnSSG_.at(tnNum)->setSequenceCommand(cnt, type, data);
}

std::vector<CommandInSequence> InstrumentsManager::getToneNoiseSSGSequence(int tnNum)
{
	return tnSSG_.at(tnNum)->getSequence();
}

void InstrumentsManager::setToneNoiseSSGLoops(int tnNum, std::vector<int> begins, std::vector<int> ends, std::vector<int> times)
{
	tnSSG_.at(tnNum)->setLoops(std::move(begins), std::move(ends), std::move(times));
}

std::vector<Loop> InstrumentsManager::getToneNoiseSSGLoops(int tnNum) const
{
	return tnSSG_.at(tnNum)->getLoops();
}

void InstrumentsManager::setToneNoiseSSGRelease(int tnNum, ReleaseType type, int begin)
{
	tnSSG_.at(tnNum)->setRelease(type, begin);
}

Release InstrumentsManager::getToneNoiseSSGRelease(int tnNum) const
{
	return tnSSG_.at(tnNum)->getRelease();
}

std::unique_ptr<CommandSequence::Iterator> InstrumentsManager::getToneNoiseSSGIterator(int tnNum) const
{
	return tnSSG_.at(tnNum)->getIterator();
}

std::vector<int> InstrumentsManager::getToneNoiseSSGUsers(int tnNum) const
{
	return tnSSG_.at(tnNum)->getUserInstruments();
}

void InstrumentsManager::setInstrumentSSGEnvelope(int instNum, int envNum)
{
	auto ssg = std::dynamic_pointer_cast<InstrumentSSG>(insts_.at(instNum));
	int prevEnv = ssg->getEnvelopeNumber();
	if (prevEnv != -1)
		envSSG_.at(prevEnv)->deregisterUserInstrument(instNum);
	if (envNum != -1)
		envSSG_.at(envNum)->registerUserInstrument(instNum);

	ssg->setEnvelopeNumber(envNum);
}

int InstrumentsManager::getInstrumentSSGEnvelope(int instNum)
{
	return std::dynamic_pointer_cast<InstrumentSSG>(insts_[instNum])->getEnvelopeNumber();
}

void InstrumentsManager::addEnvelopeSSGSequenceCommand(int envNum, int type, int data)
{
	envSSG_.at(envNum)->addSequenceCommand(type, data);
}

void InstrumentsManager::removeEnvelopeSSGSequenceCommand(int envNum)
{
	envSSG_.at(envNum)->removeSequenceCommand();
}

void InstrumentsManager::setEnvelopeSSGSequenceCommand(int envNum, int cnt, int type, int data)
{
	envSSG_.at(envNum)->setSequenceCommand(cnt, type, data);
}

std::vector<CommandInSequence> InstrumentsManager::getEnvelopeSSGSequence(int envNum)
{
	return envSSG_.at(envNum)->getSequence();
}

void InstrumentsManager::setEnvelopeSSGLoops(int envNum, std::vector<int> begins, std::vector<int> ends, std::vector<int> times)
{
	envSSG_.at(envNum)->setLoops(std::move(begins), std::move(ends), std::move(times));
}

std::vector<Loop> InstrumentsManager::getEnvelopeSSGLoops(int envNum) const
{
	return envSSG_.at(envNum)->getLoops();
}

void InstrumentsManager::setEnvelopeSSGRelease(int envNum, ReleaseType type, int begin)
{
	envSSG_.at(envNum)->setRelease(type, begin);
}

Release InstrumentsManager::getEnvelopeSSGRelease(int envNum) const
{
	return envSSG_.at(envNum)->getRelease();
}

std::unique_ptr<CommandSequence::Iterator> InstrumentsManager::getEnvelopeSSGIterator(int envNum) const
{
	return envSSG_.at(envNum)->getIterator();
}

std::vector<int> InstrumentsManager::getEnvelopeSSGUsers(int envNum) const
{
	return envSSG_.at(envNum)->getUserInstruments();
}

void InstrumentsManager::setInstrumentSSGArpeggio(int instNum, int arpNum)
{
	auto ssg = std::dynamic_pointer_cast<InstrumentSSG>(insts_.at(instNum));
	int prevArp = ssg->getArpeggioNumber();
	if (prevArp != -1)
		arpSSG_.at(prevArp)->deregisterUserInstrument(instNum);
	if (arpNum != -1)
		arpSSG_.at(arpNum)->registerUserInstrument(instNum);

	ssg->setArpeggioNumber(arpNum);
}

int InstrumentsManager::getInstrumentSSGArpeggio(int instNum)
{
	return std::dynamic_pointer_cast<InstrumentSSG>(insts_[instNum])->getArpeggioNumber();
}

void InstrumentsManager::setArpeggioType(int arpNum, int type)
{
	arpSSG_.at(arpNum)->setType(type);
}

int InstrumentsManager::getArpeggioType(int arpNum) const
{
	return arpSSG_.at(arpNum)->getType();
}

void InstrumentsManager::addArpeggioSSGSequenceCommand(int arpNum, int type, int data)
{
	arpSSG_.at(arpNum)->addSequenceCommand(type, data);
}

void InstrumentsManager::removeArpeggioSSGSequenceCommand(int arpNum)
{
	arpSSG_.at(arpNum)->removeSequenceCommand();
}

void InstrumentsManager::setArpeggioSSGSequenceCommand(int arpNum, int cnt, int type, int data)
{
	arpSSG_.at(arpNum)->setSequenceCommand(cnt, type, data);
}

std::vector<CommandInSequence> InstrumentsManager::getArpeggioSSGSequence(int arpNum)
{
	return arpSSG_.at(arpNum)->getSequence();
}

void InstrumentsManager::setArpeggioSSGLoops(int arpNum, std::vector<int> begins, std::vector<int> ends, std::vector<int> times)
{
	arpSSG_.at(arpNum)->setLoops(std::move(begins), std::move(ends), std::move(times));
}

std::vector<Loop> InstrumentsManager::getArpeggioSSGLoops(int arpNum) const
{
	return arpSSG_.at(arpNum)->getLoops();
}

void InstrumentsManager::setArpeggioSSGRelease(int arpNum, ReleaseType type, int begin)
{
	arpSSG_.at(arpNum)->setRelease(type, begin);
}

Release InstrumentsManager::getArpeggioSSGRelease(int arpNum) const
{
	return arpSSG_.at(arpNum)->getRelease();
}

std::unique_ptr<CommandSequence::Iterator> InstrumentsManager::getArpeggioSSGIterator(int arpNum) const
{
	return arpSSG_.at(arpNum)->getIterator();
}

std::vector<int> InstrumentsManager::getArpeggioSSGUsers(int arpNum) const
{
	return arpSSG_.at(arpNum)->getUserInstruments();
}
