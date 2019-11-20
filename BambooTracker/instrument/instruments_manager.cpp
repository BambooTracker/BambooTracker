#include "instruments_manager.hpp"
#include <utility>
#include <set>
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
	fmOpTypes_ = {
		FMOperatorType::All,
		FMOperatorType::Op1,
		FMOperatorType::Op2,
		FMOperatorType::Op3,
		FMOperatorType::Op4
	};

	clearAll();
}

void InstrumentsManager::addInstrument(int instNum, SoundSource source, std::string name)
{
	if (instNum < 0 || static_cast<int>(insts_.size()) <= instNum) return;

	switch (source) {
	case SoundSource::FM:
	{
		auto fm = std::make_shared<InstrumentFM>(instNum, name, this);
		int envNum = findFirstFreePlainEnvelopeFM();
		if (envNum == -1) envNum = static_cast<int>(envFM_.size()) - 1;
		fm->setEnvelopeNumber(envNum);
		envFM_.at(static_cast<size_t>(envNum))->registerUserInstrument(instNum);
		int lfoNum = findFirstFreePlainLFOFM();
		if (lfoNum == -1) lfoNum = static_cast<int>(lfoFM_.size()) - 1;
		fm->setLFONumber(lfoNum);
		fm->setLFOEnabled(false);
		for (auto param : envFMParams_) {
			int opSeqNum = findFirstFreePlainOperatorSequenceFM(param);
			if (opSeqNum == -1) opSeqNum = static_cast<int>(opSeqFM_.at(param).size()) - 1;
			fm->setOperatorSequenceNumber(param, opSeqNum);
			fm->setOperatorSequenceEnabled(param, false);
		}
		int arpNum = findFirstFreePlainArpeggioFM();
		if (arpNum == -1) arpNum = static_cast<int>(arpFM_.size()) - 1;
		int ptNum = findFirstFreePlainPitchFM();
		if (ptNum == -1) ptNum = static_cast<int>(ptFM_.size()) - 1;
		for (auto type : fmOpTypes_) {
			fm->setArpeggioNumber(type, arpNum);
			fm->setArpeggioEnabled(type, false);
			fm->setPitchNumber(type, ptNum);
			fm->setPitchEnabled(type, false);
		}
		insts_.at(static_cast<size_t>(instNum)) = std::move(fm);
		break;
	}
	case SoundSource::SSG:
	{
		auto ssg = std::make_shared<InstrumentSSG>(instNum, name, this);
		int wfNum = findFirstFreePlainWaveFormSSG();
		if (wfNum == -1) wfNum = static_cast<int>(wfSSG_.size()) - 1;
		ssg->setWaveFormNumber(wfNum);
		ssg->setWaveFormEnabled(false);
		int tnNum = findFirstFreePlainToneNoiseSSG();
		if (tnNum == -1) tnNum = static_cast<int>(tnSSG_.size()) - 1;
		ssg->setToneNoiseNumber(tnNum);
		ssg->setToneNoiseEnabled(false);
		int envNum = findFirstFreePlainEnvelopeSSG();
		if (envNum == -1) envNum = static_cast<int>(envSSG_.size()) - 1;
		ssg->setEnvelopeNumber(envNum);
		ssg->setEnvelopeEnabled(false);
		int arpNum = findFirstFreePlainArpeggioSSG();
		if (arpNum == -1) arpNum = static_cast<int>(arpSSG_.size()) - 1;
		ssg->setArpeggioNumber(arpNum);
		ssg->setArpeggioEnabled(false);
		int ptNum = findFirstFreePlainPitchSSG();
		if (ptNum == -1) ptNum = static_cast<int>(ptSSG_.size()) - 1;
		ssg->setPitchNumber(ptNum);
		ssg->setPitchEnabled(false);
		insts_.at(static_cast<size_t>(instNum)) = std::move(ssg);
		break;
	}
	default:
		break;
	}
}

void InstrumentsManager::addInstrument(std::unique_ptr<AbstractInstrument> inst)
{	
	int num = inst->getNumber();
	insts_.at(static_cast<size_t>(num)) = std::move(inst);

	switch (insts_[static_cast<size_t>(num)]->getSoundSource()) {
	case SoundSource::FM:
	{
		auto fm = std::dynamic_pointer_cast<InstrumentFM>(insts_[static_cast<size_t>(num)]);
		envFM_.at(static_cast<size_t>(fm->getEnvelopeNumber()))->registerUserInstrument(num);
		if (fm->getLFOEnabled()) lfoFM_.at(static_cast<size_t>(fm->getLFONumber()))->registerUserInstrument(num);
		for (auto p : envFMParams_) {
			if (fm->getOperatorSequenceEnabled(p))
				opSeqFM_.at(p).at(static_cast<size_t>(fm->getOperatorSequenceNumber(p)))
						->registerUserInstrument(num);
		}
		for (auto t : fmOpTypes_) {
			if (fm->getArpeggioEnabled(t))
				arpFM_.at(static_cast<size_t>(fm->getArpeggioNumber(t)))->registerUserInstrument(num);
			if (fm->getPitchEnabled(t))
				ptFM_.at(static_cast<size_t>(fm->getPitchNumber(t)))->registerUserInstrument(num);
		}
		break;
	}
	case SoundSource::SSG:
	{
		auto ssg = std::dynamic_pointer_cast<InstrumentSSG>(insts_[static_cast<size_t>(num)]);
		if (ssg->getWaveFormEnabled())
			wfSSG_.at(static_cast<size_t>(ssg->getWaveFormNumber()))->registerUserInstrument(num);
		if (ssg->getToneNoiseEnabled())
			tnSSG_.at(static_cast<size_t>(ssg->getToneNoiseNumber()))->registerUserInstrument(num);
		if (ssg->getEnvelopeEnabled())
			envSSG_.at(static_cast<size_t>(ssg->getEnvelopeNumber()))->registerUserInstrument(num);
		if (ssg->getArpeggioEnabled())
			arpSSG_.at(static_cast<size_t>(ssg->getArpeggioNumber()))->registerUserInstrument(num);
		if (ssg->getPitchEnabled())
			ptSSG_.at(static_cast<size_t>(ssg->getPitchNumber()))->registerUserInstrument(num);
		break;
	}
	default:
		break;
	}
}

void InstrumentsManager::cloneInstrument(int cloneInstNum, int refInstNum)
{
	std::shared_ptr<AbstractInstrument> refInst = insts_.at(static_cast<size_t>(refInstNum));
	addInstrument(cloneInstNum, refInst->getSoundSource(), refInst->getName());

	switch (refInst->getSoundSource()) {
	case SoundSource::FM:
	{
		auto refFm = std::dynamic_pointer_cast<InstrumentFM>(refInst);
		auto cloneFm = std::dynamic_pointer_cast<InstrumentFM>(insts_.at(static_cast<size_t>(cloneInstNum)));
		setInstrumentFMEnvelope(cloneInstNum, refFm->getEnvelopeNumber());
		setInstrumentFMLFO(cloneInstNum, refFm->getLFONumber());
		if (refFm->getLFOEnabled()) setInstrumentFMLFOEnabled(cloneInstNum, true);
		for (auto p : envFMParams_) {
			setInstrumentFMOperatorSequence(cloneInstNum, p, refFm->getOperatorSequenceNumber(p));
			if (refFm->getOperatorSequenceEnabled(p)) setInstrumentFMOperatorSequenceEnabled(cloneInstNum, p, true);
		}
		for (auto t : fmOpTypes_) {
			setInstrumentFMArpeggio(cloneInstNum, t, refFm->getArpeggioNumber(t));
			if (refFm->getArpeggioEnabled(t)) setInstrumentFMArpeggioEnabled(cloneInstNum, t, true);
			setInstrumentFMPitch(cloneInstNum, t, refFm->getPitchNumber(t));
			if (refFm->getPitchEnabled(t)) setInstrumentFMPitchEnabled(cloneInstNum, t, true);
			setInstrumentFMEnvelopeResetEnabled(cloneInstNum, t, refFm->getEnvelopeResetEnabled(t));
		}
		break;
	}
	case SoundSource::SSG:
	{
		auto refSsg = std::dynamic_pointer_cast<InstrumentSSG>(refInst);
		auto cloneSsg = std::dynamic_pointer_cast<InstrumentSSG>(insts_.at(static_cast<size_t>(cloneInstNum)));
		setInstrumentSSGWaveForm(cloneInstNum, refSsg->getWaveFormNumber());
		if (refSsg->getWaveFormEnabled()) setInstrumentSSGWaveFormEnabled(cloneInstNum, true);
		setInstrumentSSGToneNoise(cloneInstNum, refSsg->getToneNoiseNumber());
		if (refSsg->getToneNoiseEnabled()) setInstrumentSSGToneNoiseEnabled(cloneInstNum, true);
		setInstrumentSSGEnvelope(cloneInstNum, refSsg->getEnvelopeNumber());
		if (refSsg->getEnvelopeEnabled()) setInstrumentSSGEnvelopeEnabled(cloneInstNum, true);
		setInstrumentSSGArpeggio(cloneInstNum, refSsg->getArpeggioNumber());
		if (refSsg->getArpeggioEnabled()) setInstrumentSSGArpeggioEnabled(cloneInstNum, true);
		setInstrumentSSGPitch(cloneInstNum, refSsg->getPitchNumber());
		if (refSsg->getPitchEnabled()) setInstrumentSSGPitchEnabled(cloneInstNum, true);
		break;
	}
	default:
		break;
	}
}

void InstrumentsManager::deepCloneInstrument(int cloneInstNum, int refInstNum)
{
	std::shared_ptr<AbstractInstrument> refInst = insts_.at(static_cast<size_t>(refInstNum));
	addInstrument(cloneInstNum, refInst->getSoundSource(), refInst->getName());

	switch (refInst->getSoundSource()) {
	case SoundSource::FM:
	{
		auto refFm = std::dynamic_pointer_cast<InstrumentFM>(refInst);
		auto cloneFm = std::dynamic_pointer_cast<InstrumentFM>(insts_.at(static_cast<size_t>(cloneInstNum)));
		
		envFM_[static_cast<size_t>(cloneFm->getEnvelopeNumber())]->deregisterUserInstrument(cloneInstNum);	// Remove temporary number
		int envNum = cloneFMEnvelope(refFm->getEnvelopeNumber());
		cloneFm->setEnvelopeNumber(envNum);
		envFM_[static_cast<size_t>(envNum)]->registerUserInstrument(cloneInstNum);
		if (refFm->getLFOEnabled()) {
			cloneFm->setLFOEnabled(true);
			int lfoNum = cloneFMLFO(refFm->getLFONumber());
			cloneFm->setLFONumber(lfoNum);
			lfoFM_[static_cast<size_t>(lfoNum)]->registerUserInstrument(cloneInstNum);
		}

		for (auto p : envFMParams_) {
			if (refFm->getOperatorSequenceEnabled(p)) {
				cloneFm->setOperatorSequenceEnabled(p, true);
				int opSeqNum = cloneFMOperatorSequence(p, refFm->getOperatorSequenceNumber(p));
				cloneFm->setOperatorSequenceNumber(p, opSeqNum);
				opSeqFM_.at(p)[static_cast<size_t>(opSeqNum)]->registerUserInstrument(cloneInstNum);
			}
		}

		std::map<int, int> arpNums;
		for (auto t : fmOpTypes_) {
			if (refFm->getArpeggioEnabled(t)) {
				cloneFm->setArpeggioEnabled(t, true);
				arpNums.emplace(refFm->getArpeggioNumber(t), -1);
			}
		}
		for (auto& pair : arpNums) pair.second = cloneFMArpeggio(pair.first);
		for (auto t : fmOpTypes_) {
			if (refFm->getArpeggioEnabled(t)) {
				int arpNum = arpNums.at(refFm->getArpeggioNumber(t));
				cloneFm->setArpeggioNumber(t, arpNum);
				arpFM_[static_cast<size_t>(arpNum)]->registerUserInstrument(cloneInstNum);
			}
		}

		std::map<int, int> ptNums;
		for (auto t : fmOpTypes_) {
			if (refFm->getPitchEnabled(t)) {
				cloneFm->setPitchEnabled(t, true);
				ptNums.emplace(refFm->getPitchNumber(t), -1);
			}
		}
		for (auto& pair : ptNums) pair.second = cloneFMPitch(pair.first);
		for (auto t : fmOpTypes_) {
			if (refFm->getPitchEnabled(t)) {
				int ptNum = ptNums.at(refFm->getPitchNumber(t));
				cloneFm->setPitchNumber(t, ptNum);
				ptFM_[static_cast<size_t>(ptNum)]->registerUserInstrument(cloneInstNum);
			}
		}

		for (auto t : fmOpTypes_)
			setInstrumentFMEnvelopeResetEnabled(cloneInstNum, t, refFm->getEnvelopeResetEnabled(t));
		break;
	}
	case SoundSource::SSG:
	{
		auto refSsg = std::dynamic_pointer_cast<InstrumentSSG>(refInst);
		auto cloneSsg = std::dynamic_pointer_cast<InstrumentSSG>(insts_.at(static_cast<size_t>(cloneInstNum)));

		if (refSsg->getWaveFormEnabled()) {
			cloneSsg->setWaveFormEnabled(true);
			int wfNum = cloneSSGWaveForm(refSsg->getWaveFormNumber());
			cloneSsg->setWaveFormNumber(wfNum);
			wfSSG_[static_cast<size_t>(wfNum)]->registerUserInstrument(cloneInstNum);
		}
		if (refSsg->getToneNoiseEnabled()) {
			cloneSsg->setToneNoiseEnabled(true);
			int tnNum = cloneSSGToneNoise(refSsg->getToneNoiseNumber());
			cloneSsg->setToneNoiseNumber(tnNum);
			tnSSG_[static_cast<size_t>(tnNum)]->registerUserInstrument(cloneInstNum);
		}
		if (refSsg->getEnvelopeEnabled()) {
			cloneSsg->setEnvelopeEnabled(true);
			int envNum = cloneSSGEnvelope(refSsg->getEnvelopeNumber());
			cloneSsg->setEnvelopeNumber(envNum);
			envSSG_[static_cast<size_t>(envNum)]->registerUserInstrument(cloneInstNum);
		}
		if (refSsg->getArpeggioEnabled()) {
			cloneSsg->setArpeggioEnabled(true);
			int arpNum = cloneSSGArpeggio(refSsg->getArpeggioNumber());
			cloneSsg->setArpeggioNumber(arpNum);
			arpSSG_[static_cast<size_t>(arpNum)]->registerUserInstrument(cloneInstNum);
		}
		if (refSsg->getPitchEnabled()) {
			cloneSsg->setPitchEnabled(true);
			int ptNum = cloneSSGPitch(refSsg->getPitchNumber());
			cloneSsg->setPitchNumber(ptNum);
			ptSSG_[static_cast<size_t>(ptNum)]->registerUserInstrument(cloneInstNum);
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
			env = envFM_.at(static_cast<size_t>(srcNum))->clone();
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
			lfo = lfoFM_.at(static_cast<size_t>(srcNum))->clone();
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
			opSeq = opSeqFM_.at(param).at(static_cast<size_t>(srcNum))->clone();
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
			arp = arpFM_.at(static_cast<size_t>(srcNum))->clone();
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
			pt = ptFM_.at(static_cast<size_t>(srcNum))->clone();
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
			wf = wfSSG_.at(static_cast<size_t>(srcNum))->clone();
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
			tn = tnSSG_.at(static_cast<size_t>(srcNum))->clone();
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
			env = envSSG_.at(static_cast<size_t>(srcNum))->clone();
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
			arp = arpSSG_.at(static_cast<size_t>(srcNum))->clone();
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
			pt = ptSSG_.at(static_cast<size_t>(srcNum))->clone();
			pt->setNumber(cloneNum);
			break;
		}
		++cloneNum;
	}
	return cloneNum;
}

std::unique_ptr<AbstractInstrument> InstrumentsManager::removeInstrument(int instNum)
{	
	switch (insts_.at(static_cast<size_t>(instNum))->getSoundSource()) {
	case SoundSource::FM:
	{
		auto fm = std::dynamic_pointer_cast<InstrumentFM>(insts_[static_cast<size_t>(instNum)]);
		envFM_.at(static_cast<size_t>(fm->getEnvelopeNumber()))->deregisterUserInstrument(instNum);
		if (fm->getLFOEnabled())
			lfoFM_.at(static_cast<size_t>(fm->getLFONumber()))->deregisterUserInstrument(instNum);
		for (auto p : envFMParams_) {
			if (fm->getOperatorSequenceEnabled(p))
				opSeqFM_.at(p).at(static_cast<size_t>(fm->getOperatorSequenceNumber(p)))
						->deregisterUserInstrument(instNum);
		}
		for (auto t : fmOpTypes_) {
			if (fm->getArpeggioEnabled(t))
				arpFM_.at(static_cast<size_t>(fm->getArpeggioNumber(t)))->deregisterUserInstrument(instNum);
			if (fm->getPitchEnabled(t))
				ptFM_.at(static_cast<size_t>(fm->getPitchNumber(t)))->deregisterUserInstrument(instNum);
		}
		break;
	}
	case SoundSource::SSG:
	{
		auto ssg = std::dynamic_pointer_cast<InstrumentSSG>(insts_[static_cast<size_t>(instNum)]);
		if (ssg->getWaveFormEnabled())
			wfSSG_.at(static_cast<size_t>(ssg->getWaveFormNumber()))->deregisterUserInstrument(instNum);
		if (ssg->getToneNoiseEnabled())
			tnSSG_.at(static_cast<size_t>(ssg->getToneNoiseNumber()))->deregisterUserInstrument(instNum);
		if (ssg->getEnvelopeEnabled())
			envSSG_.at(static_cast<size_t>(ssg->getEnvelopeNumber()))->deregisterUserInstrument(instNum);
		if (ssg->getArpeggioEnabled())
			arpSSG_.at(static_cast<size_t>(ssg->getArpeggioNumber()))->deregisterUserInstrument(instNum);
		if (ssg->getPitchEnabled())
			ptSSG_.at(static_cast<size_t>(ssg->getPitchNumber()))->deregisterUserInstrument(instNum);
		break;
	}
	default:
		break;
	}

	std::unique_ptr<AbstractInstrument> clone = insts_[static_cast<size_t>(instNum)]->clone();
	insts_[static_cast<size_t>(instNum)].reset();
	return clone;
}

std::shared_ptr<AbstractInstrument> InstrumentsManager::getInstrumentSharedPtr(int instNum)
{
	if (0 <= instNum && instNum < static_cast<int>(insts_.size())
			&& insts_.at(static_cast<size_t>(instNum)) != nullptr) {
		return insts_[static_cast<size_t>(instNum)];
	}
	else {
		return std::shared_ptr<AbstractInstrument>();	// Return nullptr
	}
}

void InstrumentsManager::clearAll()
{
	for (auto p : envFMParams_) {
		opSeqFM_.emplace(p, std::array<std::shared_ptr<CommandSequence>, 128>());
	}

	for (size_t i = 0; i < 128; ++i) {
		insts_[i].reset();

		envFM_[i] = std::make_shared<EnvelopeFM>(i);
		lfoFM_[i] = std::make_shared<LFOFM>(i);
		for (auto& p : opSeqFM_) {
			p.second[i] = std::make_shared<CommandSequence>(i);
		}
		arpFM_[i] = std::make_shared<CommandSequence>(i, SequenceType::ABSOLUTE_SEQUENCE, 48);
		ptFM_[i] = std::make_shared<CommandSequence>(i, SequenceType::ABSOLUTE_SEQUENCE, 127);

		wfSSG_[i] = std::make_shared<CommandSequence>(i);
		tnSSG_[i] = std::make_shared<CommandSequence>(i);
		envSSG_[i] = std::make_shared<CommandSequence>(i,SequenceType::NO_SEQUENCE_TYPE, 15);
		arpSSG_[i] = std::make_shared<CommandSequence>(i, SequenceType::ABSOLUTE_SEQUENCE, 48);
		ptSSG_[i] = std::make_shared<CommandSequence>(i, SequenceType::ABSOLUTE_SEQUENCE, 127);
	}
}

std::vector<int> InstrumentsManager::getInstrumentIndices() const
{
	std::vector<int> idcs;
	for (size_t i = 0; i < insts_.size(); ++i) {
		if (insts_[i]) idcs.push_back(static_cast<int>(i));
	}
	return idcs;
}

void InstrumentsManager::setInstrumentName(int instNum, std::string name)
{
	insts_.at(static_cast<size_t>(instNum))->setName(name);
}

std::string InstrumentsManager::getInstrumentName(int instNum) const
{
	return insts_.at(static_cast<size_t>(instNum))->getName();
}

std::vector<std::string> InstrumentsManager::getInstrumentNameList() const
{
	std::vector<std::string> names;
	for (auto& inst : insts_) {
		if (inst) names.push_back(inst->getName());
	}
	return names;
}

std::vector<int> InstrumentsManager::getEntriedInstrumentIndices() const
{
	std::vector<int> idcs;
	int n = 0;
	for (auto& inst : insts_) {
		if (inst) idcs.push_back(n);
		++n;
	}
	return idcs;
}

void InstrumentsManager::clearUnusedInstrumentProperties()
{
	for (size_t i = 0; i < 128; ++i) {
		if (!envFM_[i]->isUserInstrument())
			envFM_[i] = std::make_shared<EnvelopeFM>(i);
		if (!lfoFM_[i]->isUserInstrument())
			lfoFM_[i] = std::make_shared<LFOFM>(i);
		for (auto& p : opSeqFM_) {
			if (!p.second[i]->isUserInstrument())
				p.second[i] = std::make_shared<CommandSequence>(i);
		}
		if (!arpFM_[i]->isUserInstrument())
			arpFM_[i] = std::make_shared<CommandSequence>(i, SequenceType::ABSOLUTE_SEQUENCE, 48);
		if (!ptFM_[i]->isUserInstrument())
			ptFM_[i] = std::make_shared<CommandSequence>(i, SequenceType::ABSOLUTE_SEQUENCE, 127);

		if (!wfSSG_[i]->isUserInstrument())
			wfSSG_[i] = std::make_shared<CommandSequence>(i);
		if (!tnSSG_[i]->isUserInstrument())
			tnSSG_[i] = std::make_shared<CommandSequence>(i);
		if (!envSSG_[i]->isUserInstrument())
			envSSG_[i] = std::make_shared<CommandSequence>(i, SequenceType::NO_SEQUENCE_TYPE, 15);
		if (!arpSSG_[i]->isUserInstrument())
			arpSSG_[i] = std::make_shared<CommandSequence>(i, SequenceType::ABSOLUTE_SEQUENCE, 48);
		if (!ptSSG_[i]->isUserInstrument())
			ptSSG_[i] = std::make_shared<CommandSequence>(i, SequenceType::ABSOLUTE_SEQUENCE, 127);
	}
}

/// Return:
///		-1: no free instrument
///		else: first free instrument number
int InstrumentsManager::findFirstFreeInstrument() const
{
	int num = 0;
	for (auto& inst : insts_) {
		if (!inst) return num;
		++num;
	}
	return -1;
}

std::vector<std::vector<int>> InstrumentsManager::checkDuplicateInstruments() const
{
	std::vector<std::vector<int>> dupList;
	std::vector<int> idcs = getEntriedInstrumentIndices();
	for (size_t i = 0; i < idcs.size(); ++i) {
		int baseIdx = idcs[i];
		std::vector<int> group { baseIdx };
		std::shared_ptr<AbstractInstrument> base = insts_[baseIdx];

		for (size_t j = i + 1; j < idcs.size();) {
			int tgtIdx = idcs[j];
			std::shared_ptr<AbstractInstrument> tgt = insts_[tgtIdx];

			if (base->getSoundSource() == tgt->getSoundSource()) {
				switch (base->getSoundSource()) {
				case SoundSource::FM:
				{
					if (equalPropertiesFM(std::dynamic_pointer_cast<InstrumentFM>(base),
										  std::dynamic_pointer_cast<InstrumentFM>(tgt))) {
						group.push_back(tgtIdx);
						idcs.erase(idcs.begin() + j);
						continue;
					}
					break;
				}
				case SoundSource::SSG:
				{
					if (equalPropertiesSSG(std::dynamic_pointer_cast<InstrumentSSG>(base),
										  std::dynamic_pointer_cast<InstrumentSSG>(tgt))) {
						group.push_back(tgtIdx);
						idcs.erase(idcs.begin() + j);
						continue;
					}
					break;
				}
				case SoundSource::DRUM:
					break;
				}
			}
			++j;
		}

		if (group.size() > 1) dupList.push_back(group);
	}

	return dupList;
}

//----- FM methods -----
void InstrumentsManager::setInstrumentFMEnvelope(int instNum, int envNum)
{
	auto fm = std::dynamic_pointer_cast<InstrumentFM>(insts_.at(static_cast<size_t>(instNum)));
	envFM_.at(static_cast<size_t>(fm->getEnvelopeNumber()))->deregisterUserInstrument(instNum);
	envFM_.at(static_cast<size_t>(envNum))->registerUserInstrument(instNum);

	fm->setEnvelopeNumber(envNum);
}

int InstrumentsManager::getInstrumentFMEnvelope(int instNum) const
{
	return std::dynamic_pointer_cast<InstrumentFM>(insts_[static_cast<size_t>(instNum)])->getEnvelopeNumber();
}

void InstrumentsManager::setEnvelopeFMParameter(int envNum, FMEnvelopeParameter param, int value)
{
	envFM_.at(static_cast<size_t>(envNum))->setParameterValue(param, value);
}

int InstrumentsManager::getEnvelopeFMParameter(int envNum, FMEnvelopeParameter param) const
{
	return envFM_.at(static_cast<size_t>(envNum))->getParameterValue(param);
}

void InstrumentsManager::setEnvelopeFMOperatorEnabled(int envNum, int opNum, bool enabled)
{
	envFM_.at(static_cast<size_t>(envNum))->setOperatorEnabled(opNum, enabled);
}

bool InstrumentsManager::getEnvelopeFMOperatorEnabled(int envNum, int opNum) const
{
	return envFM_.at(static_cast<size_t>(envNum))->getOperatorEnabled(opNum);
}

std::vector<int> InstrumentsManager::getEnvelopeFMUsers(int envNum) const
{
	return envFM_.at(static_cast<size_t>(envNum))->getUserInstruments();
}

std::vector<int> InstrumentsManager::getEnvelopeFMEntriedIndices() const
{
	std::vector<int> idcs;
	int n = 0;
	for (auto& env : envFM_) {
		if (env->isEdited()) idcs.push_back(n);
		++n;
	}
	return idcs;
}

int InstrumentsManager::findFirstFreeEnvelopeFM() const
{
	for (size_t i = 0; i < envFM_.size(); ++i) {
		if (!envFM_[i]->isUserInstrument()) return static_cast<int>(i);
	}
	return -1;
}

int InstrumentsManager::findFirstFreePlainEnvelopeFM() const
{
	for (size_t i = 0; i < envFM_.size(); ++i) {
		if (!envFM_[i]->isUserInstrument() && !envFM_[i]->isEdited()) return static_cast<int>(i);
	}
	return -1;
}

void InstrumentsManager::setInstrumentFMLFOEnabled(int instNum, bool enabled)
{
	auto fm = std::dynamic_pointer_cast<InstrumentFM>(insts_.at(static_cast<size_t>(instNum)));
	fm->setLFOEnabled(enabled);
	if (enabled) lfoFM_.at(static_cast<size_t>(fm->getLFONumber()))->registerUserInstrument(instNum);
	else lfoFM_.at(static_cast<size_t>(fm->getLFONumber()))->deregisterUserInstrument(instNum);
}
bool InstrumentsManager::getInstrumentFMLFOEnabled(int instNum) const
{
	return std::dynamic_pointer_cast<InstrumentFM>(insts_.at(static_cast<size_t>(instNum)))->getLFOEnabled();
}

void InstrumentsManager::setInstrumentFMLFO(int instNum, int lfoNum)
{
	auto fm = std::dynamic_pointer_cast<InstrumentFM>(insts_.at(static_cast<size_t>(instNum)));
	if (fm->getLFOEnabled()) {
		lfoFM_.at(static_cast<size_t>(fm->getLFONumber()))->deregisterUserInstrument(instNum);
		lfoFM_.at(static_cast<size_t>(lfoNum))->registerUserInstrument(instNum);
	}
	fm->setLFONumber(lfoNum);
}

int InstrumentsManager::getInstrumentFMLFO(int instNum) const
{
	return std::dynamic_pointer_cast<InstrumentFM>(insts_[static_cast<size_t>(instNum)])->getLFONumber();
}

void InstrumentsManager::setLFOFMParameter(int lfoNum, FMLFOParameter param, int value)
{
	lfoFM_.at(static_cast<size_t>(lfoNum))->setParameterValue(param, value);
}

int InstrumentsManager::getLFOFMparameter(int lfoNum, FMLFOParameter param) const
{
	return lfoFM_.at(static_cast<size_t>(lfoNum))->getParameterValue(param);
}

std::vector<int> InstrumentsManager::getLFOFMUsers(int lfoNum) const
{
	return lfoFM_.at(static_cast<size_t>(lfoNum))->getUserInstruments();
}

std::vector<int> InstrumentsManager::getLFOFMEntriedIndices() const
{
	std::vector<int> idcs;
	int n = 0;
	for (auto& lfo : lfoFM_) {
		if (lfo->isEdited()) idcs.push_back(n);
		++n;
	}
	return idcs;
}

int InstrumentsManager::findFirstFreeLFOFM() const
{
	for (size_t i = 0; i < lfoFM_.size(); ++i) {
		if (!lfoFM_[i]->isUserInstrument()) return static_cast<int>(i);
	}
	return -1;
}

int InstrumentsManager::findFirstFreePlainLFOFM() const
{
	for (size_t i = 0; i < lfoFM_.size(); ++i) {
		if (!lfoFM_[i]->isUserInstrument() && !lfoFM_[i]->isEdited()) return static_cast<int>(i);
	}
	return -1;
}

void InstrumentsManager::setInstrumentFMOperatorSequenceEnabled(int instNum, FMEnvelopeParameter param, bool enabled)
{
	auto fm = std::dynamic_pointer_cast<InstrumentFM>(insts_.at(static_cast<size_t>(instNum)));
	fm->setOperatorSequenceEnabled(param, enabled);
	if (enabled)
		opSeqFM_.at(param).at(static_cast<size_t>(fm->getOperatorSequenceNumber(param)))->registerUserInstrument(instNum);
	else
		opSeqFM_.at(param).at(static_cast<size_t>(fm->getOperatorSequenceNumber(param)))->deregisterUserInstrument(instNum);
}

bool InstrumentsManager::getInstrumentFMOperatorSequenceEnabled(int instNum, FMEnvelopeParameter param) const
{
	return std::dynamic_pointer_cast<InstrumentFM>(insts_.at(static_cast<size_t>(instNum)))->getOperatorSequenceEnabled(param);
}

void InstrumentsManager::setInstrumentFMOperatorSequence(int instNum, FMEnvelopeParameter param, int opSeqNum)
{
	auto fm = std::dynamic_pointer_cast<InstrumentFM>(insts_.at(static_cast<size_t>(instNum)));
	if (fm->getOperatorSequenceEnabled(param)) {
		opSeqFM_.at(param).at(static_cast<size_t>(fm->getOperatorSequenceNumber(param)))->deregisterUserInstrument(instNum);
		opSeqFM_.at(param).at(static_cast<size_t>(opSeqNum))->registerUserInstrument(instNum);
	}
	fm->setOperatorSequenceNumber(param, opSeqNum);
}

int InstrumentsManager::getInstrumentFMOperatorSequence(int instNum, FMEnvelopeParameter param)
{
	return std::dynamic_pointer_cast<InstrumentFM>(insts_[static_cast<size_t>(instNum)])->getOperatorSequenceNumber(param);
}

void InstrumentsManager::addOperatorSequenceFMSequenceCommand(FMEnvelopeParameter param, int opSeqNum, int type, int data)
{
	opSeqFM_.at(param).at(static_cast<size_t>(opSeqNum))->addSequenceCommand(type, data);
}

void InstrumentsManager::removeOperatorSequenceFMSequenceCommand(FMEnvelopeParameter param, int opSeqNum)
{
	opSeqFM_.at(param).at(static_cast<size_t>(opSeqNum))->removeSequenceCommand();
}

void InstrumentsManager::setOperatorSequenceFMSequenceCommand(FMEnvelopeParameter param, int opSeqNum, int cnt, int type, int data)
{
	opSeqFM_.at(param).at(static_cast<size_t>(opSeqNum))->setSequenceCommand(cnt, type, data);
}

std::vector<CommandSequenceUnit> InstrumentsManager::getOperatorSequenceFMSequence(FMEnvelopeParameter param, int opSeqNum)
{
	return opSeqFM_.at(param).at(static_cast<size_t>(opSeqNum))->getSequence();
}

void InstrumentsManager::setOperatorSequenceFMLoops(FMEnvelopeParameter param, int opSeqNum, std::vector<int> begins, std::vector<int> ends, std::vector<int> times)
{
	opSeqFM_.at(param).at(static_cast<size_t>(opSeqNum))->setLoops(std::move(begins), std::move(ends), std::move(times));
}

std::vector<Loop> InstrumentsManager::getOperatorSequenceFMLoops(FMEnvelopeParameter param, int opSeqNum) const
{
	return opSeqFM_.at(param).at(static_cast<size_t>(opSeqNum))->getLoops();
}

void InstrumentsManager::setOperatorSequenceFMRelease(FMEnvelopeParameter param, int opSeqNum, ReleaseType type, int begin)
{
	opSeqFM_.at(param).at(static_cast<size_t>(opSeqNum))->setRelease(type, begin);
}

Release InstrumentsManager::getOperatorSequenceFMRelease(FMEnvelopeParameter param, int opSeqNum) const
{
	return opSeqFM_.at(param).at(static_cast<size_t>(opSeqNum))->getRelease();
}

std::unique_ptr<CommandSequence::Iterator> InstrumentsManager::getOperatorSequenceFMIterator(FMEnvelopeParameter param, int opSeqNum) const
{
	return opSeqFM_.at(param).at(static_cast<size_t>(opSeqNum))->getIterator();
}

std::vector<int> InstrumentsManager::getOperatorSequenceFMUsers(FMEnvelopeParameter param, int opSeqNum) const
{
	return opSeqFM_.at(param).at(static_cast<size_t>(opSeqNum))->getUserInstruments();
}

std::vector<int> InstrumentsManager::getOperatorSequenceFMEntriedIndices(FMEnvelopeParameter param) const
{
	std::vector<int> idcs;
	int n = 0;
	for (auto& seq : opSeqFM_.at(param)) {
		if (seq->isUserInstrument() || seq->isEdited()) idcs.push_back(n);
		++n;
	}
	return idcs;
}

int InstrumentsManager::findFirstFreeOperatorSequenceFM(FMEnvelopeParameter param) const
{
	for (size_t i = 0; i < opSeqFM_.at(param).size(); ++i) {
		if (!opSeqFM_.at(param)[i]->isUserInstrument()) return static_cast<int>(i);
	}
	return -1;
}

int InstrumentsManager::findFirstFreePlainOperatorSequenceFM(FMEnvelopeParameter param) const
{
	for (size_t i = 0; i < opSeqFM_.at(param).size(); ++i) {
		if (!opSeqFM_.at(param)[i]->isUserInstrument() && !opSeqFM_.at(param)[i]->isEdited())
			return static_cast<int>(i);
	}
	return -1;
}

void InstrumentsManager::setInstrumentFMArpeggioEnabled(int instNum, FMOperatorType op, bool enabled)
{
	auto fm = std::dynamic_pointer_cast<InstrumentFM>(insts_.at(static_cast<size_t>(instNum)));
	fm->setArpeggioEnabled(op, enabled);
	if (enabled)
		arpFM_.at(static_cast<size_t>(fm->getArpeggioNumber(op)))->registerUserInstrument(instNum);
	else
		arpFM_.at(static_cast<size_t>(fm->getArpeggioNumber(op)))->deregisterUserInstrument(instNum);
}

bool InstrumentsManager::getInstrumentFMArpeggioEnabled(int instNum, FMOperatorType op) const
{
	return std::dynamic_pointer_cast<InstrumentFM>(insts_.at(static_cast<size_t>(instNum)))->getArpeggioEnabled(op);
}

void InstrumentsManager::setInstrumentFMArpeggio(int instNum, FMOperatorType op, int arpNum)
{
	auto fm = std::dynamic_pointer_cast<InstrumentFM>(insts_.at(static_cast<size_t>(instNum)));
	if (fm->getArpeggioEnabled(op)) {
		arpFM_.at(static_cast<size_t>(fm->getArpeggioNumber(op)))->deregisterUserInstrument(instNum);
		arpFM_.at(static_cast<size_t>(arpNum))->registerUserInstrument(instNum);
	}
	fm->setArpeggioNumber(op, arpNum);
}

int InstrumentsManager::getInstrumentFMArpeggio(int instNum, FMOperatorType op)
{
	return std::dynamic_pointer_cast<InstrumentFM>(insts_[static_cast<size_t>(instNum)])->getArpeggioNumber(op);
}

void InstrumentsManager::setArpeggioFMType(int arpNum, SequenceType type)
{
	arpFM_.at(static_cast<size_t>(arpNum))->setType(type);
}

SequenceType InstrumentsManager::getArpeggioFMType(int arpNum) const
{
	return arpFM_.at(static_cast<size_t>(arpNum))->getType();
}

void InstrumentsManager::addArpeggioFMSequenceCommand(int arpNum, int type, int data)
{
	arpFM_.at(static_cast<size_t>(arpNum))->addSequenceCommand(type, data);
}

void InstrumentsManager::removeArpeggioFMSequenceCommand(int arpNum)
{
	arpFM_.at(static_cast<size_t>(arpNum))->removeSequenceCommand();
}

void InstrumentsManager::setArpeggioFMSequenceCommand(int arpNum, int cnt, int type, int data)
{
	arpFM_.at(static_cast<size_t>(arpNum))->setSequenceCommand(cnt, type, data);
}

std::vector<CommandSequenceUnit> InstrumentsManager::getArpeggioFMSequence(int arpNum)
{
	return arpFM_.at(static_cast<size_t>(arpNum))->getSequence();
}

void InstrumentsManager::setArpeggioFMLoops(int arpNum, std::vector<int> begins, std::vector<int> ends, std::vector<int> times)
{
	arpFM_.at(static_cast<size_t>(arpNum))->setLoops(std::move(begins), std::move(ends), std::move(times));
}

std::vector<Loop> InstrumentsManager::getArpeggioFMLoops(int arpNum) const
{
	return arpFM_.at(static_cast<size_t>(arpNum))->getLoops();
}

void InstrumentsManager::setArpeggioFMRelease(int arpNum, ReleaseType type, int begin)
{
	arpFM_.at(static_cast<size_t>(arpNum))->setRelease(type, begin);
}

Release InstrumentsManager::getArpeggioFMRelease(int arpNum) const
{
	return arpFM_.at(static_cast<size_t>(arpNum))->getRelease();
}

std::unique_ptr<CommandSequence::Iterator> InstrumentsManager::getArpeggioFMIterator(int arpNum) const
{
	return arpFM_.at(static_cast<size_t>(arpNum))->getIterator();
}

std::vector<int> InstrumentsManager::getArpeggioFMUsers(int arpNum) const
{
	return arpFM_.at(static_cast<size_t>(arpNum))->getUserInstruments();
}

std::vector<int> InstrumentsManager::getArpeggioFMEntriedIndices() const
{
	std::vector<int> idcs;
	int n = 0;
	for (auto& arp : arpFM_) {
		if (arp->isUserInstrument() || arp->isEdited()) idcs.push_back(n);
		++n;
	}
	return idcs;
}

int InstrumentsManager::findFirstFreeArpeggioFM() const
{
	for (size_t i = 0; i < arpFM_.size(); ++i) {
		if (!arpFM_[i]->isUserInstrument()) return static_cast<int>(i);
	}
	return -1;
}

int InstrumentsManager::findFirstFreePlainArpeggioFM() const
{
	for (size_t i = 0; i < arpFM_.size(); ++i) {
		if (!arpFM_[i]->isUserInstrument() && !arpFM_[i]->isEdited())
			return static_cast<int>(i);
	}
	return -1;
}

void InstrumentsManager::setInstrumentFMPitchEnabled(int instNum, FMOperatorType op, bool enabled)
{
	auto fm = std::dynamic_pointer_cast<InstrumentFM>(insts_.at(static_cast<size_t>(instNum)));
	fm->setPitchEnabled(op, enabled);
	if (enabled)
		ptFM_.at(static_cast<size_t>(fm->getPitchNumber(op)))->registerUserInstrument(instNum);
	else
		ptFM_.at(static_cast<size_t>(fm->getPitchNumber(op)))->deregisterUserInstrument(instNum);
}

bool InstrumentsManager::getInstrumentFMPitchEnabled(int instNum, FMOperatorType op) const
{
	return std::dynamic_pointer_cast<InstrumentFM>(insts_.at(static_cast<size_t>(instNum)))->getPitchEnabled(op);
}

void InstrumentsManager::setInstrumentFMPitch(int instNum, FMOperatorType op, int ptNum)
{
	auto fm = std::dynamic_pointer_cast<InstrumentFM>(insts_.at(static_cast<size_t>(instNum)));
	if (fm->getPitchEnabled(op)) {
		ptFM_.at(static_cast<size_t>(fm->getPitchNumber(op)))->deregisterUserInstrument(instNum);
		ptFM_.at(static_cast<size_t>(ptNum))->registerUserInstrument(instNum);
	}
	fm->setPitchNumber(op, ptNum);
}

int InstrumentsManager::getInstrumentFMPitch(int instNum, FMOperatorType op)
{
	return std::dynamic_pointer_cast<InstrumentFM>(insts_[static_cast<size_t>(instNum)])->getPitchNumber(op);
}

void InstrumentsManager::setPitchFMType(int ptNum, SequenceType type)
{
	ptFM_.at(static_cast<size_t>(ptNum))->setType(type);
}

SequenceType InstrumentsManager::getPitchFMType(int ptNum) const
{
	return ptFM_.at(static_cast<size_t>(ptNum))->getType();
}

void InstrumentsManager::addPitchFMSequenceCommand(int ptNum, int type, int data)
{
	ptFM_.at(static_cast<size_t>(ptNum))->addSequenceCommand(type, data);
}

void InstrumentsManager::removePitchFMSequenceCommand(int ptNum)
{
	ptFM_.at(static_cast<size_t>(ptNum))->removeSequenceCommand();
}

void InstrumentsManager::setPitchFMSequenceCommand(int ptNum, int cnt, int type, int data)
{
	ptFM_.at(static_cast<size_t>(ptNum))->setSequenceCommand(cnt, type, data);
}

std::vector<CommandSequenceUnit> InstrumentsManager::getPitchFMSequence(int ptNum)
{
	return ptFM_.at(static_cast<size_t>(ptNum))->getSequence();
}

void InstrumentsManager::setPitchFMLoops(int ptNum, std::vector<int> begins, std::vector<int> ends, std::vector<int> times)
{
	ptFM_.at(static_cast<size_t>(ptNum))->setLoops(std::move(begins), std::move(ends), std::move(times));
}

std::vector<Loop> InstrumentsManager::getPitchFMLoops(int ptNum) const
{
	return ptFM_.at(static_cast<size_t>(ptNum))->getLoops();
}

void InstrumentsManager::setPitchFMRelease(int ptNum, ReleaseType type, int begin)
{
	ptFM_.at(static_cast<size_t>(ptNum))->setRelease(type, begin);
}

Release InstrumentsManager::getPitchFMRelease(int ptNum) const
{
	return ptFM_.at(static_cast<size_t>(ptNum))->getRelease();
}

std::unique_ptr<CommandSequence::Iterator> InstrumentsManager::getPitchFMIterator(int ptNum) const
{
	return ptFM_.at(static_cast<size_t>(ptNum))->getIterator();
}

std::vector<int> InstrumentsManager::getPitchFMUsers(int ptNum) const
{
	return ptFM_.at(static_cast<size_t>(ptNum))->getUserInstruments();
}

void InstrumentsManager::setInstrumentFMEnvelopeResetEnabled(int instNum, FMOperatorType op, bool enabled)
{
	std::dynamic_pointer_cast<InstrumentFM>(insts_[static_cast<size_t>(instNum)])->setEnvelopeResetEnabled(op, enabled);
}

std::vector<int> InstrumentsManager::getPitchFMEntriedIndices() const
{
	std::vector<int> idcs;
	int n = 0;
	for (auto& pt : ptFM_) {
		if (pt->isUserInstrument() || pt->isEdited()) idcs.push_back(n);
		++n;
	}
	return idcs;
}

int InstrumentsManager::findFirstFreePitchFM() const
{
	for (size_t i = 0; i < ptFM_.size(); ++i) {
		if (!ptFM_[i]->isUserInstrument()) return static_cast<int>(i);
	}
	return -1;
}

int InstrumentsManager::findFirstFreePlainPitchFM() const
{
	for (size_t i = 0; i < ptFM_.size(); ++i) {
		if (!ptFM_[i]->isUserInstrument() && !ptFM_[i]->isEdited())
			return static_cast<int>(i);
	}
	return -1;
}

bool InstrumentsManager::equalPropertiesFM(std::shared_ptr<InstrumentFM> a, std::shared_ptr<InstrumentFM> b) const
{
	if (*envFM_[a->getEnvelopeNumber()].get() != *envFM_[b->getEnvelopeNumber()].get())
		return false;
	if (a->getLFOEnabled() != b->getLFOEnabled())
		return false;
	if (a->getLFOEnabled() && *lfoFM_[a->getLFONumber()].get() != *lfoFM_[b->getLFONumber()].get())
		return false;
	for (auto& pair : opSeqFM_) {
		if (a->getOperatorSequenceEnabled(pair.first) != b->getOperatorSequenceEnabled(pair.first))
			return false;
		if (a->getOperatorSequenceEnabled(pair.first)
				&& *pair.second[a->getOperatorSequenceNumber(pair.first)].get() != *pair.second[b->getOperatorSequenceNumber(pair.first)].get())
			return false;
	}
	for (auto& type : fmOpTypes_) {
		if (a->getArpeggioEnabled(type) != b->getArpeggioEnabled(type))
			return false;
		if (a->getArpeggioEnabled(type)
				&& *arpFM_[a->getArpeggioNumber(type)].get() != *arpFM_[b->getArpeggioNumber(type)].get())
			return false;
		if (a->getPitchEnabled(type) != b->getPitchEnabled(type))
			return false;
		if (a->getPitchEnabled(type)
				&& *ptFM_[a->getPitchNumber(type)].get() != *ptFM_[b->getPitchNumber(type)].get())
			return false;
		if (a->getEnvelopeResetEnabled(type) != b->getEnvelopeResetEnabled(type))
			return false;
	}
	return true;
}

//----- SSG methods -----
void InstrumentsManager::setInstrumentSSGWaveFormEnabled(int instNum, bool enabled)
{
	auto ssg = std::dynamic_pointer_cast<InstrumentSSG>(insts_.at(static_cast<size_t>(instNum)));
	ssg->setWaveFormEnabled(enabled);
	if (enabled)
		wfSSG_.at(static_cast<size_t>(ssg->getWaveFormNumber()))->registerUserInstrument(instNum);
	else
		wfSSG_.at(static_cast<size_t>(ssg->getWaveFormNumber()))->deregisterUserInstrument(instNum);
}

bool InstrumentsManager::getInstrumentSSGWaveFormEnabled(int instNum) const
{
	return std::dynamic_pointer_cast<InstrumentSSG>(insts_.at(static_cast<size_t>(instNum)))->getWaveFormEnabled();
}

void InstrumentsManager::setInstrumentSSGWaveForm(int instNum, int wfNum)
{
	auto ssg = std::dynamic_pointer_cast<InstrumentSSG>(insts_.at(static_cast<size_t>(instNum)));
	if (ssg->getWaveFormEnabled()) {
		wfSSG_.at(static_cast<size_t>(ssg->getWaveFormNumber()))->deregisterUserInstrument(instNum);
		wfSSG_.at(static_cast<size_t>(wfNum))->registerUserInstrument(instNum);
	}
	ssg->setWaveFormNumber(wfNum);
}

int InstrumentsManager::getInstrumentSSGWaveForm(int instNum)
{
	return std::dynamic_pointer_cast<InstrumentSSG>(insts_[static_cast<size_t>(instNum)])->getWaveFormNumber();
}

void InstrumentsManager::addWaveFormSSGSequenceCommand(int wfNum, int type, int data)
{
	wfSSG_.at(static_cast<size_t>(wfNum))->addSequenceCommand(type, data);
}

void InstrumentsManager::removeWaveFormSSGSequenceCommand(int wfNum)
{
	wfSSG_.at(static_cast<size_t>(wfNum))->removeSequenceCommand();
}

void InstrumentsManager::setWaveFormSSGSequenceCommand(int wfNum, int cnt, int type, int data)
{
	wfSSG_.at(static_cast<size_t>(wfNum))->setSequenceCommand(cnt, type, data);
}

std::vector<CommandSequenceUnit> InstrumentsManager::getWaveFormSSGSequence(int wfNum)
{
	return wfSSG_.at(static_cast<size_t>(wfNum))->getSequence();
}

void InstrumentsManager::setWaveFormSSGLoops(int wfNum, std::vector<int> begins, std::vector<int> ends, std::vector<int> times)
{
	wfSSG_.at(static_cast<size_t>(wfNum))->setLoops(std::move(begins), std::move(ends), std::move(times));
}

std::vector<Loop> InstrumentsManager::getWaveFormSSGLoops(int wfNum) const
{
	return wfSSG_.at(static_cast<size_t>(wfNum))->getLoops();
}

void InstrumentsManager::setWaveFormSSGRelease(int wfNum, ReleaseType type, int begin)
{
	wfSSG_.at(static_cast<size_t>(wfNum))->setRelease(type, begin);
}

Release InstrumentsManager::getWaveFormSSGRelease(int wfNum) const
{
	return wfSSG_.at(static_cast<size_t>(wfNum))->getRelease();
}

std::unique_ptr<CommandSequence::Iterator> InstrumentsManager::getWaveFormSSGIterator(int wfNum) const
{
	return wfSSG_.at(static_cast<size_t>(wfNum))->getIterator();
}

std::vector<int> InstrumentsManager::getWaveFormSSGUsers(int wfNum) const
{
	return wfSSG_.at(static_cast<size_t>(wfNum))->getUserInstruments();
}

std::vector<int> InstrumentsManager::getWaveFormSSGEntriedIndices() const
{
	std::vector<int> idcs;
	int n = 0;
	for (auto& wf : wfSSG_) {
		if (wf->isUserInstrument() || wf->isEdited()) idcs.push_back(n);
		++n;
	}
	return idcs;
}

int InstrumentsManager::findFirstFreeWaveFormSSG() const
{
	for (size_t i = 0; i < wfSSG_.size(); ++i) {
		if (!wfSSG_[i]->isUserInstrument()) return static_cast<int>(i);
	}
	return -1;
}

int InstrumentsManager::findFirstFreePlainWaveFormSSG() const
{
	for (size_t i = 0; i < wfSSG_.size(); ++i) {
		if (!wfSSG_[i]->isUserInstrument() && !wfSSG_[i]->isEdited())
			return static_cast<int>(i);
	}
	return -1;
}

void InstrumentsManager::setInstrumentSSGToneNoiseEnabled(int instNum, bool enabled)
{
	auto ssg = std::dynamic_pointer_cast<InstrumentSSG>(insts_.at(static_cast<size_t>(instNum)));
	ssg->setToneNoiseEnabled(enabled);
	if (enabled)
		tnSSG_.at(static_cast<size_t>(ssg->getToneNoiseNumber()))->registerUserInstrument(instNum);
	else
		tnSSG_.at(static_cast<size_t>(ssg->getToneNoiseNumber()))->deregisterUserInstrument(instNum);
}

bool InstrumentsManager::getInstrumentSSGToneNoiseEnabled(int instNum) const
{
	return std::dynamic_pointer_cast<InstrumentSSG>(insts_.at(static_cast<size_t>(instNum)))->getToneNoiseEnabled();
}

void InstrumentsManager::setInstrumentSSGToneNoise(int instNum, int tnNum)
{
	auto ssg = std::dynamic_pointer_cast<InstrumentSSG>(insts_.at(static_cast<size_t>(instNum)));
	if (ssg->getToneNoiseEnabled()) {
		tnSSG_.at(static_cast<size_t>(ssg->getToneNoiseNumber()))->deregisterUserInstrument(instNum);
		tnSSG_.at(static_cast<size_t>(tnNum))->registerUserInstrument(instNum);
	}
	ssg->setToneNoiseNumber(tnNum);
}

int InstrumentsManager::getInstrumentSSGToneNoise(int instNum)
{
	return std::dynamic_pointer_cast<InstrumentSSG>(insts_[static_cast<size_t>(instNum)])->getToneNoiseNumber();
}

void InstrumentsManager::addToneNoiseSSGSequenceCommand(int tnNum, int type, int data)
{
	tnSSG_.at(static_cast<size_t>(tnNum))->addSequenceCommand(type, data);
}

void InstrumentsManager::removeToneNoiseSSGSequenceCommand(int tnNum)
{
	tnSSG_.at(static_cast<size_t>(tnNum))->removeSequenceCommand();
}

void InstrumentsManager::setToneNoiseSSGSequenceCommand(int tnNum, int cnt, int type, int data)
{
	tnSSG_.at(static_cast<size_t>(tnNum))->setSequenceCommand(cnt, type, data);
}

std::vector<CommandSequenceUnit> InstrumentsManager::getToneNoiseSSGSequence(int tnNum)
{
	return tnSSG_.at(static_cast<size_t>(tnNum))->getSequence();
}

void InstrumentsManager::setToneNoiseSSGLoops(int tnNum, std::vector<int> begins, std::vector<int> ends, std::vector<int> times)
{
	tnSSG_.at(static_cast<size_t>(tnNum))->setLoops(std::move(begins), std::move(ends), std::move(times));
}

std::vector<Loop> InstrumentsManager::getToneNoiseSSGLoops(int tnNum) const
{
	return tnSSG_.at(static_cast<size_t>(tnNum))->getLoops();
}

void InstrumentsManager::setToneNoiseSSGRelease(int tnNum, ReleaseType type, int begin)
{
	tnSSG_.at(static_cast<size_t>(tnNum))->setRelease(type, begin);
}

Release InstrumentsManager::getToneNoiseSSGRelease(int tnNum) const
{
	return tnSSG_.at(static_cast<size_t>(tnNum))->getRelease();
}

std::unique_ptr<CommandSequence::Iterator> InstrumentsManager::getToneNoiseSSGIterator(int tnNum) const
{
	return tnSSG_.at(static_cast<size_t>(tnNum))->getIterator();
}

std::vector<int> InstrumentsManager::getToneNoiseSSGUsers(int tnNum) const
{
	return tnSSG_.at(static_cast<size_t>(tnNum))->getUserInstruments();
}

std::vector<int> InstrumentsManager::getToneNoiseSSGEntriedIndices() const
{
	std::vector<int> idcs;
	int n = 0;
	for (auto& tn : tnSSG_) {
		if (tn->isUserInstrument() || tn->isEdited()) idcs.push_back(n);
		++n;
	}
	return idcs;
}

int InstrumentsManager::findFirstFreeToneNoiseSSG() const
{
	for (size_t i = 0; i < tnSSG_.size(); ++i) {
		if (!tnSSG_[i]->isUserInstrument()) return static_cast<int>(i);
	}
	return -1;
}

int InstrumentsManager::findFirstFreePlainToneNoiseSSG() const
{
	for (size_t i = 0; i < tnSSG_.size(); ++i) {
		if (!tnSSG_[i]->isUserInstrument() && !tnSSG_[i]->isEdited())
			return static_cast<int>(i);
	}
	return -1;
}

void InstrumentsManager::setInstrumentSSGEnvelopeEnabled(int instNum, bool enabled)
{
	auto ssg = std::dynamic_pointer_cast<InstrumentSSG>(insts_.at(static_cast<size_t>(instNum)));
	ssg->setEnvelopeEnabled(enabled);
	if (enabled)
		envSSG_.at(static_cast<size_t>(ssg->getEnvelopeNumber()))->registerUserInstrument(instNum);
	else
		envSSG_.at(static_cast<size_t>(ssg->getEnvelopeNumber()))->deregisterUserInstrument(instNum);
}

bool InstrumentsManager::getInstrumentSSGEnvelopeEnabled(int instNum) const
{
	return std::dynamic_pointer_cast<InstrumentSSG>(insts_.at(static_cast<size_t>(instNum)))->getEnvelopeEnabled();
}

void InstrumentsManager::setInstrumentSSGEnvelope(int instNum, int envNum)
{
	auto ssg = std::dynamic_pointer_cast<InstrumentSSG>(insts_.at(static_cast<size_t>(instNum)));
	if (ssg->getEnvelopeEnabled()) {
		envSSG_.at(static_cast<size_t>(ssg->getEnvelopeNumber()))->deregisterUserInstrument(instNum);
		envSSG_.at(static_cast<size_t>(envNum))->registerUserInstrument(instNum);
	}
	ssg->setEnvelopeNumber(envNum);
}

int InstrumentsManager::getInstrumentSSGEnvelope(int instNum)
{
	return std::dynamic_pointer_cast<InstrumentSSG>(insts_[static_cast<size_t>(instNum)])->getEnvelopeNumber();
}

void InstrumentsManager::addEnvelopeSSGSequenceCommand(int envNum, int type, int data)
{
	envSSG_.at(static_cast<size_t>(envNum))->addSequenceCommand(type, data);
}

void InstrumentsManager::removeEnvelopeSSGSequenceCommand(int envNum)
{
	envSSG_.at(static_cast<size_t>(envNum))->removeSequenceCommand();
}

void InstrumentsManager::setEnvelopeSSGSequenceCommand(int envNum, int cnt, int type, int data)
{
	envSSG_.at(static_cast<size_t>(envNum))->setSequenceCommand(cnt, type, data);
}

std::vector<CommandSequenceUnit> InstrumentsManager::getEnvelopeSSGSequence(int envNum)
{
	return envSSG_.at(static_cast<size_t>(envNum))->getSequence();
}

void InstrumentsManager::setEnvelopeSSGLoops(int envNum, std::vector<int> begins, std::vector<int> ends, std::vector<int> times)
{
	envSSG_.at(static_cast<size_t>(envNum))->setLoops(std::move(begins), std::move(ends), std::move(times));
}

std::vector<Loop> InstrumentsManager::getEnvelopeSSGLoops(int envNum) const
{
	return envSSG_.at(static_cast<size_t>(envNum))->getLoops();
}

void InstrumentsManager::setEnvelopeSSGRelease(int envNum, ReleaseType type, int begin)
{
	envSSG_.at(static_cast<size_t>(envNum))->setRelease(type, begin);
}

Release InstrumentsManager::getEnvelopeSSGRelease(int envNum) const
{
	return envSSG_.at(static_cast<size_t>(envNum))->getRelease();
}

std::unique_ptr<CommandSequence::Iterator> InstrumentsManager::getEnvelopeSSGIterator(int envNum) const
{
	return envSSG_.at(static_cast<size_t>(envNum))->getIterator();
}

std::vector<int> InstrumentsManager::getEnvelopeSSGUsers(int envNum) const
{
	return envSSG_.at(static_cast<size_t>(envNum))->getUserInstruments();
}

std::vector<int> InstrumentsManager::getEnvelopeSSGEntriedIndices() const
{
	std::vector<int> idcs;
	int n = 0;
	for (auto& env : envSSG_) {
		if (env->isUserInstrument() || env->isEdited()) idcs.push_back(n);
		++n;
	}
	return idcs;
}

int InstrumentsManager::findFirstFreeEnvelopeSSG() const
{
	for (size_t i = 0; i < envSSG_.size(); ++i) {
		if (!envSSG_[i]->isUserInstrument()) return static_cast<int>(i);
	}
	return -1;
}

int InstrumentsManager::findFirstFreePlainEnvelopeSSG() const
{
	for (size_t i = 0; i < envSSG_.size(); ++i) {
		if (!envSSG_[i]->isUserInstrument() && !envSSG_[i]->isEdited())
			return static_cast<int>(i);
	}
	return -1;
}

void InstrumentsManager::setInstrumentSSGArpeggioEnabled(int instNum, bool enabled)
{
	auto ssg = std::dynamic_pointer_cast<InstrumentSSG>(insts_.at(static_cast<size_t>(instNum)));
	ssg->setArpeggioEnabled(enabled);
	if (enabled)
		arpSSG_.at(static_cast<size_t>(ssg->getArpeggioNumber()))->registerUserInstrument(instNum);
	else
		arpSSG_.at(static_cast<size_t>(ssg->getArpeggioNumber()))->deregisterUserInstrument(instNum);
}

bool InstrumentsManager::getInstrumentSSGArpeggioEnabled(int instNum) const
{
	return std::dynamic_pointer_cast<InstrumentSSG>(insts_.at(static_cast<size_t>(instNum)))->getArpeggioEnabled();
}

void InstrumentsManager::setInstrumentSSGArpeggio(int instNum, int arpNum)
{
	auto ssg = std::dynamic_pointer_cast<InstrumentSSG>(insts_.at(static_cast<size_t>(instNum)));
	if (ssg->getArpeggioEnabled()) {
		arpSSG_.at(static_cast<size_t>(ssg->getArpeggioNumber()))->deregisterUserInstrument(instNum);
		arpSSG_.at(static_cast<size_t>(arpNum))->registerUserInstrument(instNum);
	}
	ssg->setArpeggioNumber(arpNum);
}

int InstrumentsManager::getInstrumentSSGArpeggio(int instNum)
{
	return std::dynamic_pointer_cast<InstrumentSSG>(insts_[static_cast<size_t>(instNum)])->getArpeggioNumber();
}

void InstrumentsManager::setArpeggioSSGType(int arpNum, SequenceType type)
{
	arpSSG_.at(static_cast<size_t>(arpNum))->setType(type);
}

SequenceType InstrumentsManager::getArpeggioSSGType(int arpNum) const
{
	return arpSSG_.at(static_cast<size_t>(arpNum))->getType();
}

void InstrumentsManager::addArpeggioSSGSequenceCommand(int arpNum, int type, int data)
{
	arpSSG_.at(static_cast<size_t>(arpNum))->addSequenceCommand(type, data);
}

void InstrumentsManager::removeArpeggioSSGSequenceCommand(int arpNum)
{
	arpSSG_.at(static_cast<size_t>(arpNum))->removeSequenceCommand();
}

void InstrumentsManager::setArpeggioSSGSequenceCommand(int arpNum, int cnt, int type, int data)
{
	arpSSG_.at(static_cast<size_t>(arpNum))->setSequenceCommand(cnt, type, data);
}

std::vector<CommandSequenceUnit> InstrumentsManager::getArpeggioSSGSequence(int arpNum)
{
	return arpSSG_.at(static_cast<size_t>(arpNum))->getSequence();
}

void InstrumentsManager::setArpeggioSSGLoops(int arpNum, std::vector<int> begins, std::vector<int> ends, std::vector<int> times)
{
	arpSSG_.at(static_cast<size_t>(arpNum))->setLoops(std::move(begins), std::move(ends), std::move(times));
}

std::vector<Loop> InstrumentsManager::getArpeggioSSGLoops(int arpNum) const
{
	return arpSSG_.at(static_cast<size_t>(arpNum))->getLoops();
}

void InstrumentsManager::setArpeggioSSGRelease(int arpNum, ReleaseType type, int begin)
{
	arpSSG_.at(static_cast<size_t>(arpNum))->setRelease(type, begin);
}

Release InstrumentsManager::getArpeggioSSGRelease(int arpNum) const
{
	return arpSSG_.at(static_cast<size_t>(arpNum))->getRelease();
}

std::unique_ptr<CommandSequence::Iterator> InstrumentsManager::getArpeggioSSGIterator(int arpNum) const
{
	return arpSSG_.at(static_cast<size_t>(arpNum))->getIterator();
}

std::vector<int> InstrumentsManager::getArpeggioSSGUsers(int arpNum) const
{
	return arpSSG_.at(static_cast<size_t>(arpNum))->getUserInstruments();
}

std::vector<int> InstrumentsManager::getArpeggioSSGEntriedIndices() const
{
	std::vector<int> idcs;
	int n = 0;
	for (auto& arp : arpSSG_) {
		if (arp->isUserInstrument() || arp->isEdited()) idcs.push_back(n);
		++n;
	}
	return idcs;
}

int InstrumentsManager::findFirstFreeArpeggioSSG() const
{
	for (size_t i = 0; i < arpSSG_.size(); ++i) {
		if (!arpSSG_[i]->isUserInstrument()) return static_cast<int>(i);
	}
	return -1;
}

int InstrumentsManager::findFirstFreePlainArpeggioSSG() const
{
	for (size_t i = 0; i < arpSSG_.size(); ++i) {
		if (!arpSSG_[i]->isUserInstrument() && !arpSSG_[i]->isEdited())
			return static_cast<int>(i);
	}
	return -1;
}

void InstrumentsManager::setInstrumentSSGPitchEnabled(int instNum, bool enabled)
{
	auto ssg = std::dynamic_pointer_cast<InstrumentSSG>(insts_.at(static_cast<size_t>(instNum)));
	ssg->setPitchEnabled(enabled);
	if (enabled)
		ptSSG_.at(static_cast<size_t>(ssg->getPitchNumber()))->registerUserInstrument(instNum);
	else
		ptSSG_.at(static_cast<size_t>(ssg->getPitchNumber()))->deregisterUserInstrument(instNum);
}

bool InstrumentsManager::getInstrumentSSGPitchEnabled(int instNum) const
{
	return std::dynamic_pointer_cast<InstrumentSSG>(insts_.at(static_cast<size_t>(instNum)))->getPitchEnabled();
}

void InstrumentsManager::setInstrumentSSGPitch(int instNum, int ptNum)
{
	auto ssg = std::dynamic_pointer_cast<InstrumentSSG>(insts_.at(static_cast<size_t>(instNum)));
	if (ssg->getPitchEnabled()) {
		ptSSG_.at(static_cast<size_t>(ssg->getPitchNumber()))->deregisterUserInstrument(instNum);
		ptSSG_.at(static_cast<size_t>(ptNum))->registerUserInstrument(instNum);
	}
	ssg->setPitchNumber(ptNum);
}

int InstrumentsManager::getInstrumentSSGPitch(int instNum)
{
	return std::dynamic_pointer_cast<InstrumentSSG>(insts_[static_cast<size_t>(instNum)])->getPitchNumber();
}

void InstrumentsManager::setPitchSSGType(int ptNum, SequenceType type)
{
	ptSSG_.at(static_cast<size_t>(ptNum))->setType(type);
}

SequenceType InstrumentsManager::getPitchSSGType(int ptNum) const
{
	return ptSSG_.at(static_cast<size_t>(ptNum))->getType();
}

void InstrumentsManager::addPitchSSGSequenceCommand(int ptNum, int type, int data)
{
	ptSSG_.at(static_cast<size_t>(ptNum))->addSequenceCommand(type, data);
}

void InstrumentsManager::removePitchSSGSequenceCommand(int ptNum)
{
	ptSSG_.at(static_cast<size_t>(ptNum))->removeSequenceCommand();
}

void InstrumentsManager::setPitchSSGSequenceCommand(int ptNum, int cnt, int type, int data)
{
	ptSSG_.at(static_cast<size_t>(ptNum))->setSequenceCommand(cnt, type, data);
}

std::vector<CommandSequenceUnit> InstrumentsManager::getPitchSSGSequence(int ptNum)
{
	return ptSSG_.at(static_cast<size_t>(ptNum))->getSequence();
}

void InstrumentsManager::setPitchSSGLoops(int ptNum, std::vector<int> begins, std::vector<int> ends, std::vector<int> times)
{
	ptSSG_.at(static_cast<size_t>(ptNum))->setLoops(std::move(begins), std::move(ends), std::move(times));
}

std::vector<Loop> InstrumentsManager::getPitchSSGLoops(int ptNum) const
{
	return ptSSG_.at(static_cast<size_t>(ptNum))->getLoops();
}

void InstrumentsManager::setPitchSSGRelease(int ptNum, ReleaseType type, int begin)
{
	ptSSG_.at(static_cast<size_t>(ptNum))->setRelease(type, begin);
}

Release InstrumentsManager::getPitchSSGRelease(int ptNum) const
{
	return ptSSG_.at(static_cast<size_t>(ptNum))->getRelease();
}

std::unique_ptr<CommandSequence::Iterator> InstrumentsManager::getPitchSSGIterator(int ptNum) const
{
	return ptSSG_.at(static_cast<size_t>(ptNum))->getIterator();
}

std::vector<int> InstrumentsManager::getPitchSSGUsers(int ptNum) const
{
	return ptSSG_.at(static_cast<size_t>(ptNum))->getUserInstruments();
}

std::vector<int> InstrumentsManager::getPitchSSGEntriedIndices() const
{
	std::vector<int> idcs;
	int n = 0;
	for (auto& pt : ptSSG_) {
		if (pt->isUserInstrument() || pt->isEdited()) idcs.push_back(n);
		++n;
	}
	return idcs;
}

int InstrumentsManager::findFirstFreePitchSSG() const
{
	for (size_t i = 0; i < ptSSG_.size(); ++i) {
		if (!ptSSG_[i]->isUserInstrument()) return static_cast<int>(i);
	}
	return -1;
}

int InstrumentsManager::findFirstFreePlainPitchSSG() const
{
	for (size_t i = 0; i < ptSSG_.size(); ++i) {
		if (!ptSSG_[i]->isUserInstrument() && !ptSSG_[i]->isEdited())
			return static_cast<int>(i);
	}
	return -1;
}

bool InstrumentsManager::equalPropertiesSSG(std::shared_ptr<InstrumentSSG> a, std::shared_ptr<InstrumentSSG> b) const
{
	if (a->getWaveFormEnabled() != b->getWaveFormEnabled())
		return false;
	if (a->getWaveFormEnabled()
			&& *wfSSG_[a->getWaveFormNumber()].get() != *wfSSG_[b->getWaveFormNumber()].get())
		return false;
	if (a->getToneNoiseEnabled() != b->getToneNoiseEnabled())
		return false;
	if (a->getToneNoiseEnabled()
			&& *tnSSG_[a->getToneNoiseNumber()].get() != *tnSSG_[b->getToneNoiseNumber()].get())
		return false;
	if (a->getEnvelopeEnabled() != b->getEnvelopeEnabled())
		return false;
	if (a->getEnvelopeEnabled()
			&& *envSSG_[a->getEnvelopeNumber()].get() != *envSSG_[b->getEnvelopeNumber()].get())
		return false;
	if (a->getArpeggioEnabled() != b->getArpeggioEnabled())
		return false;
	if (a->getArpeggioEnabled()
			&& *arpSSG_[a->getArpeggioNumber()].get() != *arpSSG_[b->getArpeggioNumber()].get())
		return false;
	if (a->getPitchEnabled() != b->getPitchEnabled())
		return false;
	if (a->getPitchEnabled()
			&& *ptSSG_[a->getPitchNumber()].get() != *ptSSG_[b->getPitchNumber()].get())
		return false;
	return true;
}
