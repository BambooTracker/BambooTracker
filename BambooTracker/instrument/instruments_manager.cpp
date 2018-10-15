#include "instruments_manager.hpp"
#include <utility>
#include "instrument.hpp"

InstrumentsManager::InstrumentsManager()
{
	envFMParams_ = {
		FMEnvelopeParameter::AL,
		FMEnvelopeParameter::FB,
		FMEnvelopeParameter::AR1,
		FMEnvelopeParameter::DR1,
		FMEnvelopeParameter::SR1,
		FMEnvelopeParameter::RR1,
		FMEnvelopeParameter::SL1,
		FMEnvelopeParameter::TL1,
		FMEnvelopeParameter::KS1,
		FMEnvelopeParameter::ML1,
		FMEnvelopeParameter::DT1,
		FMEnvelopeParameter::AR2,
		FMEnvelopeParameter::DR2,
		FMEnvelopeParameter::SR2,
		FMEnvelopeParameter::RR2,
		FMEnvelopeParameter::SL2,
		FMEnvelopeParameter::TL2,
		FMEnvelopeParameter::KS2,
		FMEnvelopeParameter::ML2,
		FMEnvelopeParameter::DT2,
		FMEnvelopeParameter::AR3,
		FMEnvelopeParameter::DR3,
		FMEnvelopeParameter::SR3,
		FMEnvelopeParameter::RR3,
		FMEnvelopeParameter::SL3,
		FMEnvelopeParameter::TL3,
		FMEnvelopeParameter::KS3,
		FMEnvelopeParameter::ML3,
		FMEnvelopeParameter::DT3,
		FMEnvelopeParameter::AR4,
		FMEnvelopeParameter::DR4,
		FMEnvelopeParameter::SR4,
		FMEnvelopeParameter::RR4,
		FMEnvelopeParameter::SL4,
		FMEnvelopeParameter::TL4,
		FMEnvelopeParameter::KS4,
		FMEnvelopeParameter::ML4,
		FMEnvelopeParameter::DT4
	};

	for (auto p : envFMParams_) {
		opSeqFM_.emplace(p, std::array<std::shared_ptr<CommandSequence>, 128>());
	}

	for (size_t i = 0; i < 128; ++i) {
		envFM_[i] = std::make_shared<EnvelopeFM>(i);
		lfoFM_[i] = std::make_shared<LFOFM>(i);
		for (auto& p : opSeqFM_) {
			p.second[i] = std::make_shared<CommandSequence>(i, 0);
		}
		arpFM_[i] = std::make_shared<CommandSequence>(i, 0, 48);
		ptFM_[i] = std::make_shared<CommandSequence>(i, 0, 127);

		wfSSG_[i] = std::make_shared<CommandSequence>(i, 0);
		tnSSG_[i] = std::make_shared<CommandSequence>(i, 0);
		envSSG_[i] = std::make_shared<CommandSequence>(i, 0, 15);
		arpSSG_[i] = std::make_shared<CommandSequence>(i, 0, 48);
		ptSSG_[i] = std::make_shared<CommandSequence>(i, 0, 127);
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
	default:
		break;
	}
}

void InstrumentsManager::addInstrument(std::unique_ptr<AbstractInstrument> inst)
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
		for (auto p : envFMParams_) {
			int opSeqNum = fm->getOperatorSequenceNumber(p);
			if (opSeqNum != -1) opSeqFM_.at(p).at(opSeqNum)->registerUserInstrument(num);
		}
		int arpNum = fm->getArpeggioNumber();
		if (arpNum != -1) arpFM_.at(arpNum)->registerUserInstrument(num);
		int ptNum = fm->getPitchNumber();
		if (ptNum != -1) ptFM_.at(ptNum)->registerUserInstrument(num);
		break;
	}
	case SoundSource::SSG:
	{
		auto ssg = std::dynamic_pointer_cast<InstrumentSSG>(insts_[num]);
		int wfNum = ssg->getWaveFormNumber();
		if (wfNum != -1) wfSSG_.at(wfNum)->registerUserInstrument(num);
		int tnNum = ssg->getToneNoiseNumber();
		if (tnNum != -1) tnSSG_.at(tnNum)->registerUserInstrument(num);
		int envNum = ssg->getEnvelopeNumber();
		if (envNum != -1) envSSG_.at(envNum)->registerUserInstrument(num);
		int arpNum = ssg->getArpeggioNumber();
		if (arpNum != -1) arpSSG_.at(arpNum)->registerUserInstrument(num);
		int ptNum = ssg->getPitchNumber();
		if (ptNum != -1) ptSSG_.at(ptNum)->registerUserInstrument(num);
		break;
	}
	default:
		break;
	}
}

void InstrumentsManager::cloneInstrument(int cloneInstNum, int refInstNum)
{
	std::shared_ptr<AbstractInstrument> refInst = insts_.at(refInstNum);
	addInstrument(cloneInstNum, refInst->getSoundSource(), refInst->getName());

	switch (refInst->getSoundSource()) {
	case SoundSource::FM:
	{
		auto refFm = std::dynamic_pointer_cast<InstrumentFM>(refInst);
		setInstrumentFMEnvelope(cloneInstNum, refFm->getEnvelopeNumber());
		setInstrumentFMLFO(cloneInstNum, refFm->getLFONumber());
		for (auto p : envFMParams_) {
			setInstrumentFMOperatorSequence(cloneInstNum, p, refFm->getOperatorSequenceNumber(p));
		}
		setInstrumentFMArpeggio(cloneInstNum, refFm->getArpeggioNumber());
		setInstrumentFMPitch(cloneInstNum, refFm->getPitchNumber());
		setInstrumentFMEnvelopeResetEnabled(cloneInstNum, refFm->getEnvelopeResetEnabled());
		break;
	}
	case SoundSource::SSG:
	{
		auto refSsg = std::dynamic_pointer_cast<InstrumentSSG>(refInst);
		setInstrumentSSGWaveForm(cloneInstNum, refSsg->getWaveFormNumber());
		setInstrumentSSGToneNoise(cloneInstNum, refSsg->getToneNoiseNumber());
		setInstrumentSSGEnvelope(cloneInstNum, refSsg->getEnvelopeNumber());
		setInstrumentSSGArpeggio(cloneInstNum, refSsg->getArpeggioNumber());
		setInstrumentSSGPitch(cloneInstNum, refSsg->getPitchNumber());
		break;
	}
	default:
		break;
	}
}

void InstrumentsManager::deepCloneInstrument(int cloneInstNum, int refInstNum)
{
	std::shared_ptr<AbstractInstrument> refInst = insts_.at(refInstNum);
	addInstrument(cloneInstNum, refInst->getSoundSource(), refInst->getName());

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
		for (auto p : envFMParams_) {
			if (refFm->getOperatorSequenceNumber(p) != -1) {
				int opSeqNum = cloneFMOperatorSequence(p, refFm->getOperatorSequenceNumber(p));
				cloneFm->setOperatorSequenceNumber(p, opSeqNum);
				opSeqFM_.at(p)[opSeqNum]->registerUserInstrument(cloneInstNum);
			}
		}
		if (refFm->getArpeggioNumber() != -1) {
			int arpNum = cloneFMArpeggio(refFm->getArpeggioNumber());
			cloneFm->setArpeggioNumber(arpNum);
			arpFM_[arpNum]->registerUserInstrument(cloneInstNum);
		}
		if (refFm->getPitchNumber() != -1) {
			int ptNum = cloneFMPitch(refFm->getPitchNumber());
			cloneFm->setPitchNumber(ptNum);
			ptFM_[ptNum]->registerUserInstrument(cloneInstNum);
		}
		setInstrumentFMEnvelopeResetEnabled(cloneInstNum, refFm->getEnvelopeResetEnabled());
		break;
	}
	case SoundSource::SSG:
	{
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
		if (refSsg->getPitchNumber() != -1) {
			int ptNum = cloneSSGPitch(refSsg->getPitchNumber());
			cloneSsg->setPitchNumber(ptNum);
			ptSSG_[ptNum]->registerUserInstrument(cloneInstNum);
		}
		break;
	}
	default:
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

int InstrumentsManager::cloneFMOperatorSequence(FMEnvelopeParameter param, int srcNum)
{
	int cloneNum = 0;
	for (auto& opSeq : opSeqFM_.at(param)) {
		if (!opSeq->isUserInstrument()) {
			opSeq = opSeqFM_.at(param).at(srcNum)->clone();
			opSeq->setNumber(cloneNum);
			break;
		}
		++cloneNum;
	}
	return cloneNum;
}

int InstrumentsManager::cloneFMArpeggio(int srcNum)
{
	int cloneNum = 0;
	for (auto& arp : arpFM_) {
		if (!arp->isUserInstrument()) {
			arp = arpFM_.at(srcNum)->clone();
			arp->setNumber(cloneNum);
			break;
		}
		++cloneNum;
	}
	return cloneNum;
}

int InstrumentsManager::cloneFMPitch(int srcNum)
{
	int cloneNum = 0;
	for (auto& pt : ptFM_) {
		if (!pt->isUserInstrument()) {
			pt = ptFM_.at(srcNum)->clone();
			pt->setNumber(cloneNum);
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

int InstrumentsManager::cloneSSGPitch(int srcNum)
{
	int cloneNum = 0;
	for (auto& pt : ptSSG_) {
		if (!pt->isUserInstrument()) {
			pt = ptSSG_.at(srcNum)->clone();
			pt->setNumber(cloneNum);
			break;
		}
		++cloneNum;
	}
	return cloneNum;
}

std::unique_ptr<AbstractInstrument> InstrumentsManager::removeInstrument(int instNum)
{	
	switch (insts_.at(instNum)->getSoundSource()) {
	case SoundSource::FM:
	{
		auto fm = std::dynamic_pointer_cast<InstrumentFM>(insts_[instNum]);
		envFM_.at(fm->getEnvelopeNumber())->deregisterUserInstrument(instNum);
		int lfoNum = fm->getLFONumber();
		if (lfoNum != -1) lfoFM_.at(lfoNum)->deregisterUserInstrument(instNum);
		for (auto p : envFMParams_) {
			int opSeqNum = fm->getOperatorSequenceNumber(p);
			if (opSeqNum != -1) opSeqFM_.at(p).at(opSeqNum)->deregisterUserInstrument(instNum);
		}
		int arpNum = fm->getArpeggioNumber();
		if (arpNum != -1) arpFM_.at(arpNum)->deregisterUserInstrument(instNum);
		int ptNum = fm->getPitchNumber();
		if (ptNum != -1) ptFM_.at(ptNum)->deregisterUserInstrument(instNum);
		break;
	}
	case SoundSource::SSG:
	{
		auto ssg = std::dynamic_pointer_cast<InstrumentSSG>(insts_[instNum]);
		int wfNum = ssg->getWaveFormNumber();
		if (wfNum != -1) wfSSG_.at(wfNum)->deregisterUserInstrument(instNum);
		int tnNum = ssg->getToneNoiseNumber();
		if (tnNum != -1) tnSSG_.at(tnNum)->deregisterUserInstrument(instNum);
		int envNum = ssg->getEnvelopeNumber();
		if (envNum != -1) envSSG_.at(envNum)->deregisterUserInstrument(instNum);
		int arpNum = ssg->getArpeggioNumber();
		if (arpNum != -1) arpSSG_.at(arpNum)->deregisterUserInstrument(instNum);
		int ptNum = ssg->getPitchNumber();
		if (ptNum != -1) ptSSG_.at(ptNum)->deregisterUserInstrument(instNum);
		break;
	}
	default:
		break;
	}

	std::unique_ptr<AbstractInstrument> clone = insts_[instNum]->clone();
	insts_[instNum]->setNumber(-1);	// Unregistered number
	insts_[instNum].reset();
	return clone;
}

std::shared_ptr<AbstractInstrument> InstrumentsManager::getInstrumentSharedPtr(int instNum)
{
	if (0 <= instNum && instNum < 128 && insts_.at(instNum) != nullptr) {
		return insts_[instNum];
	}
	else {
		return std::shared_ptr<AbstractInstrument>();	// Throw nullptr
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

void InstrumentsManager::setLFOFMParameter(int lfoNum, FMLFOParameter param, int value)
{
	lfoFM_.at(lfoNum)->setParameterValue(param, value);
}

int InstrumentsManager::getLFOFMparameter(int lfoNum, FMLFOParameter param) const
{
	return lfoFM_.at(lfoNum)->getParameterValue(param);
}

std::vector<int> InstrumentsManager::getLFOFMUsers(int lfoNum) const
{
	return lfoFM_.at(lfoNum)->getUserInstruments();
}

void InstrumentsManager::setInstrumentFMOperatorSequence(int instNum, FMEnvelopeParameter param, int opSeqNum)
{
	auto fm = std::dynamic_pointer_cast<InstrumentFM>(insts_.at(instNum));
	int prevOpSeq = fm->getOperatorSequenceNumber(param);
	if (prevOpSeq != -1)
		opSeqFM_.at(param).at(prevOpSeq)->deregisterUserInstrument(instNum);
	if (opSeqNum != -1)
		opSeqFM_.at(param).at(opSeqNum)->registerUserInstrument(instNum);

	fm->setOperatorSequenceNumber(param, opSeqNum);
}

int InstrumentsManager::getInstrumentFMOperatorSequence(int instNum, FMEnvelopeParameter param)
{
	return std::dynamic_pointer_cast<InstrumentFM>(insts_[instNum])->getOperatorSequenceNumber(param);
}

void InstrumentsManager::addOperatorSequenceFMSequenceCommand(FMEnvelopeParameter param, int opSeqNum, int type, int data)
{
	opSeqFM_.at(param).at(opSeqNum)->addSequenceCommand(type, data);
}

void InstrumentsManager::removeOperatorSequenceFMSequenceCommand(FMEnvelopeParameter param, int opSeqNum)
{
	opSeqFM_.at(param).at(opSeqNum)->removeSequenceCommand();
}

void InstrumentsManager::setOperatorSequenceFMSequenceCommand(FMEnvelopeParameter param, int opSeqNum, int cnt, int type, int data)
{
	opSeqFM_.at(param).at(opSeqNum)->setSequenceCommand(cnt, type, data);
}

std::vector<CommandInSequence> InstrumentsManager::getOperatorSequenceFMSequence(FMEnvelopeParameter param, int opSeqNum)
{
	return opSeqFM_.at(param).at(opSeqNum)->getSequence();
}

void InstrumentsManager::setOperatorSequenceFMLoops(FMEnvelopeParameter param, int opSeqNum, std::vector<int> begins, std::vector<int> ends, std::vector<int> times)
{
	opSeqFM_.at(param).at(opSeqNum)->setLoops(std::move(begins), std::move(ends), std::move(times));
}

std::vector<Loop> InstrumentsManager::getOperatorSequenceFMLoops(FMEnvelopeParameter param, int opSeqNum) const
{
	return opSeqFM_.at(param).at(opSeqNum)->getLoops();
}

void InstrumentsManager::setOperatorSequenceFMRelease(FMEnvelopeParameter param, int opSeqNum, ReleaseType type, int begin)
{
	opSeqFM_.at(param).at(opSeqNum)->setRelease(type, begin);
}

Release InstrumentsManager::getOperatorSequenceFMRelease(FMEnvelopeParameter param, int opSeqNum) const
{
	return opSeqFM_.at(param).at(opSeqNum)->getRelease();
}

std::unique_ptr<CommandSequence::Iterator> InstrumentsManager::getOperatorSequenceFMIterator(FMEnvelopeParameter param, int opSeqNum) const
{
	return opSeqFM_.at(param).at(opSeqNum)->getIterator();
}

std::vector<int> InstrumentsManager::getOperatorSequenceFMUsers(FMEnvelopeParameter param, int opSeqNum) const
{
	return opSeqFM_.at(param).at(opSeqNum)->getUserInstruments();
}

void InstrumentsManager::setInstrumentFMArpeggio(int instNum, int arpNum)
{
	auto fm = std::dynamic_pointer_cast<InstrumentFM>(insts_.at(instNum));
	int prevArp = fm->getArpeggioNumber();
	if (prevArp != -1)
		arpFM_.at(prevArp)->deregisterUserInstrument(instNum);
	if (arpNum != -1)
		arpFM_.at(arpNum)->registerUserInstrument(instNum);

	fm->setArpeggioNumber(arpNum);
}

int InstrumentsManager::getInstrumentFMArpeggio(int instNum)
{
	return std::dynamic_pointer_cast<InstrumentFM>(insts_[instNum])->getArpeggioNumber();
}

void InstrumentsManager::setArpeggioFMType(int arpNum, int type)
{
	arpFM_.at(arpNum)->setType(type);
}

int InstrumentsManager::getArpeggioFMType(int arpNum) const
{
	return arpFM_.at(arpNum)->getType();
}

void InstrumentsManager::addArpeggioFMSequenceCommand(int arpNum, int type, int data)
{
	arpFM_.at(arpNum)->addSequenceCommand(type, data);
}

void InstrumentsManager::removeArpeggioFMSequenceCommand(int arpNum)
{
	arpFM_.at(arpNum)->removeSequenceCommand();
}

void InstrumentsManager::setArpeggioFMSequenceCommand(int arpNum, int cnt, int type, int data)
{
	arpFM_.at(arpNum)->setSequenceCommand(cnt, type, data);
}

std::vector<CommandInSequence> InstrumentsManager::getArpeggioFMSequence(int arpNum)
{
	return arpFM_.at(arpNum)->getSequence();
}

void InstrumentsManager::setArpeggioFMLoops(int arpNum, std::vector<int> begins, std::vector<int> ends, std::vector<int> times)
{
	arpFM_.at(arpNum)->setLoops(std::move(begins), std::move(ends), std::move(times));
}

std::vector<Loop> InstrumentsManager::getArpeggioFMLoops(int arpNum) const
{
	return arpFM_.at(arpNum)->getLoops();
}

void InstrumentsManager::setArpeggioFMRelease(int arpNum, ReleaseType type, int begin)
{
	arpFM_.at(arpNum)->setRelease(type, begin);
}

Release InstrumentsManager::getArpeggioFMRelease(int arpNum) const
{
	return arpFM_.at(arpNum)->getRelease();
}

std::unique_ptr<CommandSequence::Iterator> InstrumentsManager::getArpeggioFMIterator(int arpNum) const
{
	return arpFM_.at(arpNum)->getIterator();
}

std::vector<int> InstrumentsManager::getArpeggioFMUsers(int arpNum) const
{
	return arpFM_.at(arpNum)->getUserInstruments();
}

void InstrumentsManager::setInstrumentFMPitch(int instNum, int ptNum)
{
	auto fm = std::dynamic_pointer_cast<InstrumentFM>(insts_.at(instNum));
	int prevPt = fm->getPitchNumber();
	if (prevPt != -1)
		ptFM_.at(prevPt)->deregisterUserInstrument(instNum);
	if (ptNum != -1)
		ptFM_.at(ptNum)->registerUserInstrument(instNum);

	fm->setPitchNumber(ptNum);
}

int InstrumentsManager::getInstrumentFMPitch(int instNum)
{
	return std::dynamic_pointer_cast<InstrumentFM>(insts_[instNum])->getPitchNumber();
}

void InstrumentsManager::setPitchFMType(int ptNum, int type)
{
	ptFM_.at(ptNum)->setType(type);
}

int InstrumentsManager::getPitchFMType(int ptNum) const
{
	return ptFM_.at(ptNum)->getType();
}

void InstrumentsManager::addPitchFMSequenceCommand(int ptNum, int type, int data)
{
	ptFM_.at(ptNum)->addSequenceCommand(type, data);
}

void InstrumentsManager::removePitchFMSequenceCommand(int ptNum)
{
	ptFM_.at(ptNum)->removeSequenceCommand();
}

void InstrumentsManager::setPitchFMSequenceCommand(int ptNum, int cnt, int type, int data)
{
	ptFM_.at(ptNum)->setSequenceCommand(cnt, type, data);
}

std::vector<CommandInSequence> InstrumentsManager::getPitchFMSequence(int ptNum)
{
	return ptFM_.at(ptNum)->getSequence();
}

void InstrumentsManager::setPitchFMLoops(int ptNum, std::vector<int> begins, std::vector<int> ends, std::vector<int> times)
{
	ptFM_.at(ptNum)->setLoops(std::move(begins), std::move(ends), std::move(times));
}

std::vector<Loop> InstrumentsManager::getPitchFMLoops(int ptNum) const
{
	return ptFM_.at(ptNum)->getLoops();
}

void InstrumentsManager::setPitchFMRelease(int ptNum, ReleaseType type, int begin)
{
	ptFM_.at(ptNum)->setRelease(type, begin);
}

Release InstrumentsManager::getPitchFMRelease(int ptNum) const
{
	return ptFM_.at(ptNum)->getRelease();
}

std::unique_ptr<CommandSequence::Iterator> InstrumentsManager::getPitchFMIterator(int ptNum) const
{
	return ptFM_.at(ptNum)->getIterator();
}

std::vector<int> InstrumentsManager::getPitchFMUsers(int ptNum) const
{
	return ptFM_.at(ptNum)->getUserInstruments();
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

void InstrumentsManager::setArpeggioSSGType(int arpNum, int type)
{
	arpSSG_.at(arpNum)->setType(type);
}

int InstrumentsManager::getArpeggioSSGType(int arpNum) const
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

void InstrumentsManager::setInstrumentSSGPitch(int instNum, int ptNum)
{
	auto ssg = std::dynamic_pointer_cast<InstrumentSSG>(insts_.at(instNum));
	int prevPt = ssg->getPitchNumber();
	if (prevPt != -1)
		ptSSG_.at(prevPt)->deregisterUserInstrument(instNum);
	if (ptNum != -1)
		ptSSG_.at(ptNum)->registerUserInstrument(instNum);

	ssg->setPitchNumber(ptNum);
}

int InstrumentsManager::getInstrumentSSGPitch(int instNum)
{
	return std::dynamic_pointer_cast<InstrumentSSG>(insts_[instNum])->getPitchNumber();
}

void InstrumentsManager::setPitchSSGType(int ptNum, int type)
{
	ptSSG_.at(ptNum)->setType(type);
}

int InstrumentsManager::getPitchSSGType(int ptNum) const
{
	return ptSSG_.at(ptNum)->getType();
}

void InstrumentsManager::addPitchSSGSequenceCommand(int ptNum, int type, int data)
{
	ptSSG_.at(ptNum)->addSequenceCommand(type, data);
}

void InstrumentsManager::removePitchSSGSequenceCommand(int ptNum)
{
	ptSSG_.at(ptNum)->removeSequenceCommand();
}

void InstrumentsManager::setPitchSSGSequenceCommand(int ptNum, int cnt, int type, int data)
{
	ptSSG_.at(ptNum)->setSequenceCommand(cnt, type, data);
}

std::vector<CommandInSequence> InstrumentsManager::getPitchSSGSequence(int ptNum)
{
	return ptSSG_.at(ptNum)->getSequence();
}

void InstrumentsManager::setPitchSSGLoops(int ptNum, std::vector<int> begins, std::vector<int> ends, std::vector<int> times)
{
	ptSSG_.at(ptNum)->setLoops(std::move(begins), std::move(ends), std::move(times));
}

std::vector<Loop> InstrumentsManager::getPitchSSGLoops(int ptNum) const
{
	return ptSSG_.at(ptNum)->getLoops();
}

void InstrumentsManager::setPitchSSGRelease(int ptNum, ReleaseType type, int begin)
{
	ptSSG_.at(ptNum)->setRelease(type, begin);
}

Release InstrumentsManager::getPitchSSGRelease(int ptNum) const
{
	return ptSSG_.at(ptNum)->getRelease();
}

std::unique_ptr<CommandSequence::Iterator> InstrumentsManager::getPitchSSGIterator(int ptNum) const
{
	return ptSSG_.at(ptNum)->getIterator();
}

std::vector<int> InstrumentsManager::getPitchSSGUsers(int ptNum) const
{
	return ptSSG_.at(ptNum)->getUserInstruments();
}
