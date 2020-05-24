#include "instruments_manager.hpp"
#include <utility>
#include <unordered_map>
#include <algorithm>
#include "instrument.hpp"
#include <functional>

const FMEnvelopeParameter InstrumentsManager::ENV_FM_PARAMS_[38] = {
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

const FMOperatorType InstrumentsManager::FM_OP_TYPES_[5] = {
	FMOperatorType::All,
	FMOperatorType::Op1,
	FMOperatorType::Op2,
	FMOperatorType::Op3,
	FMOperatorType::Op4
};

InstrumentsManager::InstrumentsManager(bool unedited)
	: regardingUnedited_(unedited)
{
	clearAll();
}

void InstrumentsManager::addInstrument(int instNum, InstrumentType type, std::string name)
{
	if (instNum < 0 || static_cast<int>(insts_.size()) <= instNum) return;

	switch (type) {
	case InstrumentType::FM:
	{
		auto fm = std::make_shared<InstrumentFM>(instNum, name, this);
		int envNum = findFirstAssignableEnvelopeFM();
		if (envNum == -1) envNum = static_cast<int>(envFM_.size()) - 1;
		fm->setEnvelopeNumber(envNum);
		envFM_.at(static_cast<size_t>(envNum))->registerUserInstrument(instNum);
		int lfoNum = findFirstAssignableLFOFM();
		if (lfoNum == -1) lfoNum = static_cast<int>(lfoFM_.size()) - 1;
		fm->setLFONumber(lfoNum);
		fm->setLFOEnabled(false);
		for (auto param : ENV_FM_PARAMS_) {
			int opSeqNum = findFirstAssignableOperatorSequenceFM(param);
			if (opSeqNum == -1) opSeqNum = static_cast<int>(opSeqFM_.at(param).size()) - 1;
			fm->setOperatorSequenceNumber(param, opSeqNum);
			fm->setOperatorSequenceEnabled(param, false);
		}
		int arpNum = findFirstAssignableArpeggioFM();
		if (arpNum == -1) arpNum = static_cast<int>(arpFM_.size()) - 1;
		int ptNum = findFirstAssignablePitchFM();
		if (ptNum == -1) ptNum = static_cast<int>(ptFM_.size()) - 1;
		for (auto type : FM_OP_TYPES_) {
			fm->setArpeggioNumber(type, arpNum);
			fm->setArpeggioEnabled(type, false);
			fm->setPitchNumber(type, ptNum);
			fm->setPitchEnabled(type, false);
		}
		insts_.at(static_cast<size_t>(instNum)) = std::move(fm);
		break;
	}
	case InstrumentType::SSG:
	{
		auto ssg = std::make_shared<InstrumentSSG>(instNum, name, this);
		int wfNum = findFirstAssignableWaveformSSG();
		if (wfNum == -1) wfNum = static_cast<int>(wfSSG_.size()) - 1;
		ssg->setWaveformNumber(wfNum);
		ssg->setWaveformEnabled(false);
		int tnNum = findFirstAssignableToneNoiseSSG();
		if (tnNum == -1) tnNum = static_cast<int>(tnSSG_.size()) - 1;
		ssg->setToneNoiseNumber(tnNum);
		ssg->setToneNoiseEnabled(false);
		int envNum = findFirstAssignableEnvelopeSSG();
		if (envNum == -1) envNum = static_cast<int>(envSSG_.size()) - 1;
		ssg->setEnvelopeNumber(envNum);
		ssg->setEnvelopeEnabled(false);
		int arpNum = findFirstAssignableArpeggioSSG();
		if (arpNum == -1) arpNum = static_cast<int>(arpSSG_.size()) - 1;
		ssg->setArpeggioNumber(arpNum);
		ssg->setArpeggioEnabled(false);
		int ptNum = findFirstAssignablePitchSSG();
		if (ptNum == -1) ptNum = static_cast<int>(ptSSG_.size()) - 1;
		ssg->setPitchNumber(ptNum);
		ssg->setPitchEnabled(false);
		insts_.at(static_cast<size_t>(instNum)) = std::move(ssg);
		break;
	}
	case InstrumentType::ADPCM:
	{
		auto adpcm = std::make_shared<InstrumentADPCM>(instNum, name, this);
		int sampNum = findFirstAssignableSampleADPCM();
		if (sampNum == -1) sampNum = static_cast<int>(sampADPCM_.size()) - 1;
		adpcm->setSampleNumber(sampNum);
		sampADPCM_.at(static_cast<size_t>(sampNum))->registerUserInstrument(instNum);
		int envNum = findFirstAssignableEnvelopeADPCM();
		if (envNum == -1) envNum = static_cast<int>(envADPCM_.size()) - 1;
		adpcm->setEnvelopeNumber(envNum);
		adpcm->setEnvelopeEnabled(false);
		int arpNum = findFirstAssignableArpeggioADPCM();
		if (arpNum == -1) arpNum = static_cast<int>(arpADPCM_.size()) - 1;
		adpcm->setArpeggioNumber(arpNum);
		adpcm->setArpeggioEnabled(false);
		int ptNum = findFirstAssignablePitchADPCM();
		if (ptNum == -1) ptNum = static_cast<int>(ptADPCM_.size()) - 1;
		adpcm->setPitchNumber(ptNum);
		adpcm->setPitchEnabled(false);
		insts_.at(static_cast<size_t>(instNum)) = std::move(adpcm);
		break;
	}
	case InstrumentType::Drumkit:
	{
		auto kit = std::make_shared<InstrumentDrumkit>(instNum, name, this);
		insts_.at(static_cast<size_t>(instNum)) = std::move(kit);
		break;
	}
	default:
		throw std::invalid_argument("invalid instrument type");
	}
}

void InstrumentsManager::addInstrument(std::unique_ptr<AbstractInstrument> inst)
{	
	int num = inst->getNumber();
	insts_.at(static_cast<size_t>(num)) = std::move(inst);

	switch (insts_[static_cast<size_t>(num)]->getType()) {
	case InstrumentType::FM:
	{
		auto fm = std::dynamic_pointer_cast<InstrumentFM>(insts_[static_cast<size_t>(num)]);
		envFM_.at(static_cast<size_t>(fm->getEnvelopeNumber()))->registerUserInstrument(num);
		if (fm->getLFOEnabled()) lfoFM_.at(static_cast<size_t>(fm->getLFONumber()))->registerUserInstrument(num);
		for (auto p : ENV_FM_PARAMS_) {
			if (fm->getOperatorSequenceEnabled(p))
				opSeqFM_.at(p).at(static_cast<size_t>(fm->getOperatorSequenceNumber(p)))
						->registerUserInstrument(num);
		}
		for (auto t : FM_OP_TYPES_) {
			if (fm->getArpeggioEnabled(t))
				arpFM_.at(static_cast<size_t>(fm->getArpeggioNumber(t)))->registerUserInstrument(num);
			if (fm->getPitchEnabled(t))
				ptFM_.at(static_cast<size_t>(fm->getPitchNumber(t)))->registerUserInstrument(num);
		}
		break;
	}
	case InstrumentType::SSG:
	{
		auto ssg = std::dynamic_pointer_cast<InstrumentSSG>(insts_[static_cast<size_t>(num)]);
		if (ssg->getWaveformEnabled())
			wfSSG_.at(static_cast<size_t>(ssg->getWaveformNumber()))->registerUserInstrument(num);
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
	case InstrumentType::ADPCM:
	{
		auto adpcm = std::dynamic_pointer_cast<InstrumentADPCM>(insts_[static_cast<size_t>(num)]);
		sampADPCM_.at(static_cast<size_t>(adpcm->getSampleNumber()))->registerUserInstrument(num);
		if (adpcm->getEnvelopeEnabled())
			envADPCM_.at(static_cast<size_t>(adpcm->getEnvelopeNumber()))->registerUserInstrument(num);
		if (adpcm->getArpeggioEnabled())
			arpADPCM_.at(static_cast<size_t>(adpcm->getArpeggioNumber()))->registerUserInstrument(num);
		if (adpcm->getPitchEnabled())
			ptADPCM_.at(static_cast<size_t>(adpcm->getPitchNumber()))->registerUserInstrument(num);
		break;
	}
	case InstrumentType::Drumkit:
	{
		auto kit = std::dynamic_pointer_cast<InstrumentDrumkit>(insts_[static_cast<size_t>(num)]);
		for (const auto& key : kit->getAssignedKeys()) {
			sampADPCM_.at(static_cast<size_t>(kit->getSampleNumber(key)))->registerUserInstrument(num);
		}
		break;
	}
	default:
		throw std::invalid_argument("invalid instrument type");
	}
}

void InstrumentsManager::cloneInstrument(int cloneInstNum, int refInstNum)
{
	std::shared_ptr<AbstractInstrument> refInst = insts_.at(static_cast<size_t>(refInstNum));
	addInstrument(cloneInstNum, refInst->getType(), refInst->getName());

	switch (refInst->getType()) {
	case InstrumentType::FM:
	{
		auto refFm = std::dynamic_pointer_cast<InstrumentFM>(refInst);
		auto cloneFm = std::dynamic_pointer_cast<InstrumentFM>(insts_.at(static_cast<size_t>(cloneInstNum)));
		setInstrumentFMEnvelope(cloneInstNum, refFm->getEnvelopeNumber());
		setInstrumentFMLFO(cloneInstNum, refFm->getLFONumber());
		if (refFm->getLFOEnabled()) setInstrumentFMLFOEnabled(cloneInstNum, true);
		for (auto p : ENV_FM_PARAMS_) {
			setInstrumentFMOperatorSequence(cloneInstNum, p, refFm->getOperatorSequenceNumber(p));
			if (refFm->getOperatorSequenceEnabled(p)) setInstrumentFMOperatorSequenceEnabled(cloneInstNum, p, true);
		}
		for (auto t : FM_OP_TYPES_) {
			setInstrumentFMArpeggio(cloneInstNum, t, refFm->getArpeggioNumber(t));
			if (refFm->getArpeggioEnabled(t)) setInstrumentFMArpeggioEnabled(cloneInstNum, t, true);
			setInstrumentFMPitch(cloneInstNum, t, refFm->getPitchNumber(t));
			if (refFm->getPitchEnabled(t)) setInstrumentFMPitchEnabled(cloneInstNum, t, true);
			setInstrumentFMEnvelopeResetEnabled(cloneInstNum, t, refFm->getEnvelopeResetEnabled(t));
		}
		break;
	}
	case InstrumentType::SSG:
	{
		auto refSsg = std::dynamic_pointer_cast<InstrumentSSG>(refInst);
		auto cloneSsg = std::dynamic_pointer_cast<InstrumentSSG>(insts_.at(static_cast<size_t>(cloneInstNum)));
		setInstrumentSSGWaveform(cloneInstNum, refSsg->getWaveformNumber());
		if (refSsg->getWaveformEnabled()) setInstrumentSSGWaveformEnabled(cloneInstNum, true);
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
	case InstrumentType::ADPCM:
	{
		auto refAdpcm = std::dynamic_pointer_cast<InstrumentADPCM>(refInst);
		auto cloneAdpcm = std::dynamic_pointer_cast<InstrumentADPCM>(insts_.at(static_cast<size_t>(cloneInstNum)));
		setInstrumentADPCMSample(cloneInstNum, refAdpcm->getSampleNumber());
		setInstrumentADPCMEnvelope(cloneInstNum, refAdpcm->getEnvelopeNumber());
		if (refAdpcm->getEnvelopeEnabled()) setInstrumentADPCMEnvelopeEnabled(cloneInstNum, true);
		setInstrumentADPCMArpeggio(cloneInstNum, refAdpcm->getArpeggioNumber());
		if (refAdpcm->getArpeggioEnabled()) setInstrumentADPCMArpeggioEnabled(cloneInstNum, true);
		setInstrumentADPCMPitch(cloneInstNum, refAdpcm->getPitchNumber());
		if (refAdpcm->getPitchEnabled()) setInstrumentADPCMPitchEnabled(cloneInstNum, true);
		break;
	}
	case InstrumentType::Drumkit:
	{
		auto refKit = std::dynamic_pointer_cast<InstrumentDrumkit>(refInst);
		auto cloneKit = std::dynamic_pointer_cast<InstrumentDrumkit>(insts_.at(static_cast<size_t>(cloneInstNum)));
		for (const int& key : refKit->getAssignedKeys()) {
			setInstrumentDrumkitSamples(cloneInstNum, key, refKit->getSampleNumber(key));
			setInstrumentDrumkitPitch(cloneInstNum, key, refKit->getPitch(key));
		}
		break;
	}
	default:
		throw std::invalid_argument("invalid instrument type");
	}
}

void InstrumentsManager::deepCloneInstrument(int cloneInstNum, int refInstNum)
{
	std::shared_ptr<AbstractInstrument> refInst = insts_.at(static_cast<size_t>(refInstNum));
	addInstrument(cloneInstNum, refInst->getType(), refInst->getName());

	switch (refInst->getType()) {
	case InstrumentType::FM:
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

		for (auto p : ENV_FM_PARAMS_) {
			if (refFm->getOperatorSequenceEnabled(p)) {
				cloneFm->setOperatorSequenceEnabled(p, true);
				int opSeqNum = cloneFMOperatorSequence(p, refFm->getOperatorSequenceNumber(p));
				cloneFm->setOperatorSequenceNumber(p, opSeqNum);
				opSeqFM_.at(p)[static_cast<size_t>(opSeqNum)]->registerUserInstrument(cloneInstNum);
			}
		}

		std::unordered_map<int, int> arpNums;
		for (auto t : FM_OP_TYPES_) {
			if (refFm->getArpeggioEnabled(t)) {
				cloneFm->setArpeggioEnabled(t, true);
				arpNums.emplace(refFm->getArpeggioNumber(t), -1);
			}
		}
		for (auto& pair : arpNums) pair.second = cloneFMArpeggio(pair.first);
		for (auto t : FM_OP_TYPES_) {
			if (refFm->getArpeggioEnabled(t)) {
				int arpNum = arpNums.at(refFm->getArpeggioNumber(t));
				cloneFm->setArpeggioNumber(t, arpNum);
				arpFM_[static_cast<size_t>(arpNum)]->registerUserInstrument(cloneInstNum);
			}
		}

		std::unordered_map<int, int> ptNums;
		for (auto t : FM_OP_TYPES_) {
			if (refFm->getPitchEnabled(t)) {
				cloneFm->setPitchEnabled(t, true);
				ptNums.emplace(refFm->getPitchNumber(t), -1);
			}
		}
		for (auto& pair : ptNums) pair.second = cloneFMPitch(pair.first);
		for (auto t : FM_OP_TYPES_) {
			if (refFm->getPitchEnabled(t)) {
				int ptNum = ptNums.at(refFm->getPitchNumber(t));
				cloneFm->setPitchNumber(t, ptNum);
				ptFM_[static_cast<size_t>(ptNum)]->registerUserInstrument(cloneInstNum);
			}
		}

		for (auto t : FM_OP_TYPES_)
			setInstrumentFMEnvelopeResetEnabled(cloneInstNum, t, refFm->getEnvelopeResetEnabled(t));
		break;
	}
	case InstrumentType::SSG:
	{
		auto refSsg = std::dynamic_pointer_cast<InstrumentSSG>(refInst);
		auto cloneSsg = std::dynamic_pointer_cast<InstrumentSSG>(insts_.at(static_cast<size_t>(cloneInstNum)));

		if (refSsg->getWaveformEnabled()) {
			cloneSsg->setWaveformEnabled(true);
			int wfNum = cloneSSGWaveform(refSsg->getWaveformNumber());
			cloneSsg->setWaveformNumber(wfNum);
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
	case InstrumentType::ADPCM:
	{
		auto refAdpcm = std::dynamic_pointer_cast<InstrumentADPCM>(refInst);
		auto cloneAdpcm = std::dynamic_pointer_cast<InstrumentADPCM>(insts_.at(static_cast<size_t>(cloneInstNum)));

		sampADPCM_[static_cast<size_t>(cloneAdpcm->getSampleNumber())]->deregisterUserInstrument(cloneInstNum);	// Remove temporary number
		int sampNum = cloneADPCMSample(refAdpcm->getSampleNumber());
		cloneAdpcm->setSampleNumber(sampNum);
		sampADPCM_[static_cast<size_t>(sampNum)]->registerUserInstrument(cloneInstNum);
		if (refAdpcm->getEnvelopeEnabled()) {
			cloneAdpcm->setEnvelopeEnabled(true);
			int envNum = cloneADPCMEnvelope(refAdpcm->getEnvelopeNumber());
			cloneAdpcm->setEnvelopeNumber(envNum);
			envADPCM_[static_cast<size_t>(envNum)]->registerUserInstrument(cloneInstNum);
		}
		if (refAdpcm->getArpeggioEnabled()) {
			cloneAdpcm->setArpeggioEnabled(true);
			int arpNum = cloneADPCMArpeggio(refAdpcm->getArpeggioNumber());
			cloneAdpcm->setArpeggioNumber(arpNum);
			arpADPCM_[static_cast<size_t>(arpNum)]->registerUserInstrument(cloneInstNum);
		}
		if (refAdpcm->getPitchEnabled()) {
			cloneAdpcm->setPitchEnabled(true);
			int ptNum = cloneADPCMPitch(refAdpcm->getPitchNumber());
			cloneAdpcm->setPitchNumber(ptNum);
			ptADPCM_[static_cast<size_t>(ptNum)]->registerUserInstrument(cloneInstNum);
		}
		break;
	}
	case InstrumentType::Drumkit:
	{
		auto refKit = std::dynamic_pointer_cast<InstrumentDrumkit>(refInst);
		auto cloneKit = std::dynamic_pointer_cast<InstrumentDrumkit>(insts_.at(static_cast<size_t>(cloneInstNum)));
		std::unordered_map<int, int> sampMap;
		for (const int& key : refKit->getAssignedKeys()) {
			int n = refKit->getSampleNumber(key);
			if (!sampMap.count(n)) sampMap[n] = cloneADPCMSample(n);
			cloneKit->setSampleNumber(key, sampMap[n]);
			sampADPCM_[static_cast<size_t>(sampMap[n])]->registerUserInstrument(cloneInstNum);

			setInstrumentDrumkitPitch(cloneInstNum, key, refKit->getPitch(key));
		}
		break;
	}
	default:
		throw std::invalid_argument("invalid instrument type");
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

int InstrumentsManager::cloneSSGWaveform(int srcNum)
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

int InstrumentsManager::cloneADPCMSample(int srcNum)
{
	int cloneNum = 0;
	for (auto& samp : sampADPCM_) {
		if (!samp->isUserInstrument()) {
			samp = sampADPCM_.at(static_cast<size_t>(srcNum))->clone();
			samp->setNumber(cloneNum);
			break;
		}
		++cloneNum;
	}
	return cloneNum;
}

int InstrumentsManager::cloneADPCMEnvelope(int srcNum)
{
	int cloneNum = 0;
	for (auto& env : envADPCM_) {
		if (!env->isUserInstrument()) {
			env = envADPCM_.at(static_cast<size_t>(srcNum))->clone();
			env->setNumber(cloneNum);
			break;
		}
		++cloneNum;
	}
	return cloneNum;
}

int InstrumentsManager::cloneADPCMArpeggio(int srcNum)
{
	int cloneNum = 0;
	for (auto& arp : arpADPCM_) {
		if (!arp->isUserInstrument()) {
			arp = arpADPCM_.at(static_cast<size_t>(srcNum))->clone();
			arp->setNumber(cloneNum);
			break;
		}
		++cloneNum;
	}
	return cloneNum;
}

int InstrumentsManager::cloneADPCMPitch(int srcNum)
{
	int cloneNum = 0;
	for (auto& pt : ptADPCM_) {
		if (!pt->isUserInstrument()) {
			pt = ptADPCM_.at(static_cast<size_t>(srcNum))->clone();
			pt->setNumber(cloneNum);
			break;
		}
		++cloneNum;
	}
	return cloneNum;
}

void InstrumentsManager::swapInstruments(int inst1Num, int inst2Num)
{
	std::unique_ptr<AbstractInstrument> inst1 = removeInstrument(inst1Num);
	std::unique_ptr<AbstractInstrument> inst2 = removeInstrument(inst2Num);
	inst1->setNumber(inst2Num);
	inst2->setNumber(inst1Num);
	addInstrument(std::move(inst1));
	addInstrument(std::move(inst2));
}

std::unique_ptr<AbstractInstrument> InstrumentsManager::removeInstrument(int instNum)
{	
	switch (insts_.at(static_cast<size_t>(instNum))->getType()) {
	case InstrumentType::FM:
	{
		auto fm = std::dynamic_pointer_cast<InstrumentFM>(insts_[static_cast<size_t>(instNum)]);
		envFM_.at(static_cast<size_t>(fm->getEnvelopeNumber()))->deregisterUserInstrument(instNum);
		if (fm->getLFOEnabled())
			lfoFM_.at(static_cast<size_t>(fm->getLFONumber()))->deregisterUserInstrument(instNum);
		for (auto p : ENV_FM_PARAMS_) {
			if (fm->getOperatorSequenceEnabled(p))
				opSeqFM_.at(p).at(static_cast<size_t>(fm->getOperatorSequenceNumber(p)))
						->deregisterUserInstrument(instNum);
		}
		for (auto t : FM_OP_TYPES_) {
			if (fm->getArpeggioEnabled(t))
				arpFM_.at(static_cast<size_t>(fm->getArpeggioNumber(t)))->deregisterUserInstrument(instNum);
			if (fm->getPitchEnabled(t))
				ptFM_.at(static_cast<size_t>(fm->getPitchNumber(t)))->deregisterUserInstrument(instNum);
		}
		break;
	}
	case InstrumentType::SSG:
	{
		auto ssg = std::dynamic_pointer_cast<InstrumentSSG>(insts_[static_cast<size_t>(instNum)]);
		if (ssg->getWaveformEnabled())
			wfSSG_.at(static_cast<size_t>(ssg->getWaveformNumber()))->deregisterUserInstrument(instNum);
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
	case InstrumentType::ADPCM:
	{
		auto adpcm = std::dynamic_pointer_cast<InstrumentADPCM>(insts_[static_cast<size_t>(instNum)]);
		sampADPCM_.at(static_cast<size_t>(adpcm->getSampleNumber()))->deregisterUserInstrument(instNum);
		if (adpcm->getEnvelopeEnabled())
			envADPCM_.at(static_cast<size_t>(adpcm->getEnvelopeNumber()))->deregisterUserInstrument(instNum);
		if (adpcm->getArpeggioEnabled())
			arpADPCM_.at(static_cast<size_t>(adpcm->getArpeggioNumber()))->deregisterUserInstrument(instNum);
		if (adpcm->getPitchEnabled())
			ptADPCM_.at(static_cast<size_t>(adpcm->getPitchNumber()))->deregisterUserInstrument(instNum);
		break;
	}
	case InstrumentType::Drumkit:
	{
		auto kit = std::dynamic_pointer_cast<InstrumentDrumkit>(insts_[static_cast<size_t>(instNum)]);
		for (const int& key : kit->getAssignedKeys()) {
			sampADPCM_.at(static_cast<size_t>(kit->getSampleNumber(key)))->deregisterUserInstrument(instNum);
		}
		break;
	}
	default:
		throw std::invalid_argument("invalid instrument type");
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
	for (auto p : ENV_FM_PARAMS_) {
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
		envSSG_[i] = std::make_shared<CommandSequence>(i, SequenceType::NO_SEQUENCE_TYPE, 15);
		arpSSG_[i] = std::make_shared<CommandSequence>(i, SequenceType::ABSOLUTE_SEQUENCE, 48);
		ptSSG_[i] = std::make_shared<CommandSequence>(i, SequenceType::ABSOLUTE_SEQUENCE, 127);

		sampADPCM_[i] = std::make_shared<SampleADPCM>(i);
		envADPCM_[i] = std::make_shared<CommandSequence>(i, SequenceType::NO_SEQUENCE_TYPE, 255);
		arpADPCM_[i] = std::make_shared<CommandSequence>(i, SequenceType::ABSOLUTE_SEQUENCE, 48);
		ptADPCM_[i] = std::make_shared<CommandSequence>(i, SequenceType::ABSOLUTE_SEQUENCE, 127);
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

		if (!sampADPCM_[i]->isUserInstrument())
			sampADPCM_[i] = std::make_shared<SampleADPCM>(i);
		if (!envADPCM_[i]->isUserInstrument())
			envADPCM_[i] = std::make_shared<CommandSequence>(i, SequenceType::NO_SEQUENCE_TYPE, 255);
		if (!arpADPCM_[i]->isUserInstrument())
			arpADPCM_[i] = std::make_shared<CommandSequence>(i, SequenceType::ABSOLUTE_SEQUENCE, 48);
		if (!ptADPCM_[i]->isUserInstrument())
			ptADPCM_[i] = std::make_shared<CommandSequence>(i, SequenceType::ABSOLUTE_SEQUENCE, 127);
	}
}

/// Return:
///		-1: no free instrument
///		else: first free instrument number
int InstrumentsManager::findFirstFreeInstrument() const
{
	auto&& it = std::find_if_not(insts_.begin(), insts_.end(),
								 [](const std::shared_ptr<AbstractInstrument>& inst) { return inst; });
	return (it == insts_.end() ? -1 : std::distance(insts_.begin(), it));
}

std::vector<std::vector<int>> InstrumentsManager::checkDuplicateInstruments() const
{
	std::vector<std::vector<int>> dupList;
	std::vector<int> idcs = getEntriedInstrumentIndices();
	std::unordered_map<InstrumentType,
			bool (InstrumentsManager::*)(std::shared_ptr<AbstractInstrument>,
										 std::shared_ptr<AbstractInstrument>) const> eqCheck = {
	{ InstrumentType::FM, &InstrumentsManager::equalPropertiesFM },
	{ InstrumentType::SSG, &InstrumentsManager::equalPropertiesSSG },
	{ InstrumentType::ADPCM, &InstrumentsManager::equalPropertiesADPCM },
	{ InstrumentType::Drumkit, &InstrumentsManager::equalPropertiesDrumkit }
};

	for (size_t i = 0; i < idcs.size(); ++i) {
		int baseIdx = idcs[i];
		std::vector<int> group { baseIdx };
		std::shared_ptr<AbstractInstrument> base = insts_[baseIdx];

		for (size_t j = i + 1; j < idcs.size();) {
			int tgtIdx = idcs[j];
			std::shared_ptr<AbstractInstrument> tgt = insts_[tgtIdx];
			if (base->getType() == tgt->getType() && (this->*eqCheck.at(base->getType()))(base, tgt)) {
				group.push_back(tgtIdx);
				idcs.erase(idcs.begin() + j);
				continue;
			}
			++j;
		}

		if (group.size() > 1) dupList.push_back(group);
	}

	return dupList;
}

void InstrumentsManager::setPropertyFindMode(bool unedited)
{
	regardingUnedited_ = unedited;
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

int InstrumentsManager::findFirstAssignableEnvelopeFM() const
{
	std::function<bool(const std::shared_ptr<EnvelopeFM>&)> cond;
	if (regardingUnedited_)
		cond = [](const std::shared_ptr<EnvelopeFM>& env) { return (env->isUserInstrument() || env->isEdited()); };
	else
		cond = [](const std::shared_ptr<EnvelopeFM>& env) { return env->isUserInstrument(); };
	auto&& it = std::find_if_not(envFM_.begin(), envFM_.end(), cond);

	if (it == envFM_.end()) return -1;

	if (!regardingUnedited_) (*it)->clearParameters();
	return std::distance(envFM_.begin(), it);
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

int InstrumentsManager::findFirstAssignableLFOFM() const
{
	std::function<bool(const std::shared_ptr<LFOFM>&)> cond;
	if (regardingUnedited_)
		cond = [](const std::shared_ptr<LFOFM>& lfo) { return (lfo->isUserInstrument() || lfo->isEdited()); };
	else
		cond = [](const std::shared_ptr<LFOFM>& lfo) { return lfo->isUserInstrument(); };
	auto&& it = std::find_if_not(lfoFM_.begin(), lfoFM_.end(), cond);

	if (it == lfoFM_.end()) return -1;

	if (!regardingUnedited_) (*it)->clearParameters();
	return std::distance(lfoFM_.begin(), it);
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

int InstrumentsManager::findFirstAssignableOperatorSequenceFM(FMEnvelopeParameter param) const
{
	std::function<bool(const std::shared_ptr<CommandSequence>&)> cond;
	if (regardingUnedited_)
		cond = [](const std::shared_ptr<CommandSequence>& seq) { return (seq->isUserInstrument() || seq->isEdited()); };
	else
		cond = [](const std::shared_ptr<CommandSequence>& seq) { return seq->isUserInstrument(); };
	auto& opSeq = opSeqFM_.at(param);
	auto&& it = std::find_if_not(opSeq.begin(), opSeq.end(), cond);

	if (it == opSeq.end()) return -1;

	if (!regardingUnedited_) (*it)->clearParameters();
	return std::distance(opSeq.begin(), it);
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

int InstrumentsManager::findFirstAssignableArpeggioFM() const
{
	std::function<bool(const std::shared_ptr<CommandSequence>&)> cond;
	if (regardingUnedited_)
		cond = [](const std::shared_ptr<CommandSequence>& arp) { return (arp->isUserInstrument() || arp->isEdited()); };
	else
		cond = [](const std::shared_ptr<CommandSequence>& arp) { return arp->isUserInstrument(); };
	auto&& it = std::find_if_not(arpFM_.begin(), arpFM_.end(), cond);

	if (it == arpFM_.end()) return -1;

	if (!regardingUnedited_) (*it)->clearParameters();
	return std::distance(arpFM_.begin(), it);
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

int InstrumentsManager::findFirstAssignablePitchFM() const
{
	std::function<bool(const std::shared_ptr<CommandSequence>&)> cond;
	if (regardingUnedited_)
		cond = [](const std::shared_ptr<CommandSequence>& pt) { return (pt->isUserInstrument() || pt->isEdited()); };
	else
		cond = [](const std::shared_ptr<CommandSequence>& pt) { return pt->isUserInstrument(); };
	auto&& it = std::find_if_not(ptFM_.begin(), ptFM_.end(), cond);

	if (it == ptFM_.end()) return -1;

	if (!regardingUnedited_) (*it)->clearParameters();
	return std::distance(ptFM_.begin(), it);
}

bool InstrumentsManager::equalPropertiesFM(std::shared_ptr<AbstractInstrument> a, std::shared_ptr<AbstractInstrument> b) const
{
	auto aFm = std::dynamic_pointer_cast<InstrumentFM>(a);
	auto bFm = std::dynamic_pointer_cast<InstrumentFM>(b);

	if (*envFM_[aFm->getEnvelopeNumber()].get() != *envFM_[bFm->getEnvelopeNumber()].get())
		return false;
	if (aFm->getLFOEnabled() != bFm->getLFOEnabled())
		return false;
	if (aFm->getLFOEnabled() && *lfoFM_[aFm->getLFONumber()].get() != *lfoFM_[bFm->getLFONumber()].get())
		return false;
	for (auto& pair : opSeqFM_) {
		if (aFm->getOperatorSequenceEnabled(pair.first) != bFm->getOperatorSequenceEnabled(pair.first))
			return false;
		if (aFm->getOperatorSequenceEnabled(pair.first)
				&& *pair.second[aFm->getOperatorSequenceNumber(pair.first)].get() != *pair.second[bFm->getOperatorSequenceNumber(pair.first)].get())
			return false;
	}
	for (auto& type : FM_OP_TYPES_) {
		if (aFm->getArpeggioEnabled(type) != bFm->getArpeggioEnabled(type))
			return false;
		if (aFm->getArpeggioEnabled(type)
				&& *arpFM_[aFm->getArpeggioNumber(type)].get() != *arpFM_[bFm->getArpeggioNumber(type)].get())
			return false;
		if (aFm->getPitchEnabled(type) != bFm->getPitchEnabled(type))
			return false;
		if (aFm->getPitchEnabled(type)
				&& *ptFM_[aFm->getPitchNumber(type)].get() != *ptFM_[bFm->getPitchNumber(type)].get())
			return false;
		if (aFm->getEnvelopeResetEnabled(type) != bFm->getEnvelopeResetEnabled(type))
			return false;
	}
	return true;
}

//----- SSG methods -----
void InstrumentsManager::setInstrumentSSGWaveformEnabled(int instNum, bool enabled)
{
	auto ssg = std::dynamic_pointer_cast<InstrumentSSG>(insts_.at(static_cast<size_t>(instNum)));
	ssg->setWaveformEnabled(enabled);
	if (enabled)
		wfSSG_.at(static_cast<size_t>(ssg->getWaveformNumber()))->registerUserInstrument(instNum);
	else
		wfSSG_.at(static_cast<size_t>(ssg->getWaveformNumber()))->deregisterUserInstrument(instNum);
}

bool InstrumentsManager::getInstrumentSSGWaveformEnabled(int instNum) const
{
	return std::dynamic_pointer_cast<InstrumentSSG>(insts_.at(static_cast<size_t>(instNum)))->getWaveformEnabled();
}

void InstrumentsManager::setInstrumentSSGWaveform(int instNum, int wfNum)
{
	auto ssg = std::dynamic_pointer_cast<InstrumentSSG>(insts_.at(static_cast<size_t>(instNum)));
	if (ssg->getWaveformEnabled()) {
		wfSSG_.at(static_cast<size_t>(ssg->getWaveformNumber()))->deregisterUserInstrument(instNum);
		wfSSG_.at(static_cast<size_t>(wfNum))->registerUserInstrument(instNum);
	}
	ssg->setWaveformNumber(wfNum);
}

int InstrumentsManager::getInstrumentSSGWaveform(int instNum)
{
	return std::dynamic_pointer_cast<InstrumentSSG>(insts_[static_cast<size_t>(instNum)])->getWaveformNumber();
}

void InstrumentsManager::addWaveformSSGSequenceCommand(int wfNum, int type, int data)
{
	wfSSG_.at(static_cast<size_t>(wfNum))->addSequenceCommand(type, data);
}

void InstrumentsManager::removeWaveformSSGSequenceCommand(int wfNum)
{
	wfSSG_.at(static_cast<size_t>(wfNum))->removeSequenceCommand();
}

void InstrumentsManager::setWaveformSSGSequenceCommand(int wfNum, int cnt, int type, int data)
{
	wfSSG_.at(static_cast<size_t>(wfNum))->setSequenceCommand(cnt, type, data);
}

std::vector<CommandSequenceUnit> InstrumentsManager::getWaveformSSGSequence(int wfNum)
{
	return wfSSG_.at(static_cast<size_t>(wfNum))->getSequence();
}

void InstrumentsManager::setWaveformSSGLoops(int wfNum, std::vector<int> begins, std::vector<int> ends, std::vector<int> times)
{
	wfSSG_.at(static_cast<size_t>(wfNum))->setLoops(std::move(begins), std::move(ends), std::move(times));
}

std::vector<Loop> InstrumentsManager::getWaveformSSGLoops(int wfNum) const
{
	return wfSSG_.at(static_cast<size_t>(wfNum))->getLoops();
}

void InstrumentsManager::setWaveformSSGRelease(int wfNum, ReleaseType type, int begin)
{
	wfSSG_.at(static_cast<size_t>(wfNum))->setRelease(type, begin);
}

Release InstrumentsManager::getWaveformSSGRelease(int wfNum) const
{
	return wfSSG_.at(static_cast<size_t>(wfNum))->getRelease();
}

std::unique_ptr<CommandSequence::Iterator> InstrumentsManager::getWaveformSSGIterator(int wfNum) const
{
	return wfSSG_.at(static_cast<size_t>(wfNum))->getIterator();
}

std::vector<int> InstrumentsManager::getWaveformSSGUsers(int wfNum) const
{
	return wfSSG_.at(static_cast<size_t>(wfNum))->getUserInstruments();
}

std::vector<int> InstrumentsManager::getWaveformSSGEntriedIndices() const
{
	std::vector<int> idcs;
	int n = 0;
	for (auto& wf : wfSSG_) {
		if (wf->isUserInstrument() || wf->isEdited()) idcs.push_back(n);
		++n;
	}
	return idcs;
}

int InstrumentsManager::findFirstAssignableWaveformSSG() const
{
	std::function<bool(const std::shared_ptr<CommandSequence>&)> cond;
	if (regardingUnedited_)
		cond = [](const std::shared_ptr<CommandSequence>& wf) { return (wf->isUserInstrument() || wf->isEdited()); };
	else
		cond = [](const std::shared_ptr<CommandSequence>& wf) { return wf->isUserInstrument(); };
	auto&& it = std::find_if_not(wfSSG_.begin(), wfSSG_.end(), cond);

	if (it == wfSSG_.end()) return -1;

	if (!regardingUnedited_) (*it)->clearParameters();
	return std::distance(wfSSG_.begin(), it);
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

int InstrumentsManager::findFirstAssignableToneNoiseSSG() const
{
	std::function<bool(const std::shared_ptr<CommandSequence>&)> cond;
	if (regardingUnedited_)
		cond = [](const std::shared_ptr<CommandSequence>& tn) { return (tn->isUserInstrument() || tn->isEdited()); };
	else
		cond = [](const std::shared_ptr<CommandSequence>& tn) { return tn->isUserInstrument(); };
	auto&& it = std::find_if_not(tnSSG_.begin(), tnSSG_.end(), cond);

	if (it == tnSSG_.end()) return -1;

	if (!regardingUnedited_) (*it)->clearParameters();
	return std::distance(tnSSG_.begin(), it);
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

int InstrumentsManager::findFirstAssignableEnvelopeSSG() const
{
	std::function<bool(const std::shared_ptr<CommandSequence>&)> cond;
	if (regardingUnedited_)
		cond = [](const std::shared_ptr<CommandSequence>& env) { return (env->isUserInstrument() || env->isEdited()); };
	else
		cond = [](const std::shared_ptr<CommandSequence>& env) { return env->isUserInstrument(); };
	auto&& it = std::find_if_not(envSSG_.begin(), envSSG_.end(), cond);

	if (it == envSSG_.end()) return -1;

	if (!regardingUnedited_) (*it)->clearParameters();
	return std::distance(envSSG_.begin(), it);
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

int InstrumentsManager::findFirstAssignableArpeggioSSG() const
{
	std::function<bool(const std::shared_ptr<CommandSequence>&)> cond;
	if (regardingUnedited_)
		cond = [](const std::shared_ptr<CommandSequence>& arp) { return (arp->isUserInstrument() || arp->isEdited()); };
	else
		cond = [](const std::shared_ptr<CommandSequence>& arp) { return arp->isUserInstrument(); };
	auto&& it = std::find_if_not(arpSSG_.begin(), arpSSG_.end(), cond);

	if (it == arpSSG_.end()) return -1;

	if (!regardingUnedited_) (*it)->clearParameters();
	return std::distance(arpSSG_.begin(), it);
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

int InstrumentsManager::findFirstAssignablePitchSSG() const
{
	std::function<bool(const std::shared_ptr<CommandSequence>&)> cond;
	if (regardingUnedited_)
		cond = [](const std::shared_ptr<CommandSequence>& pt) { return (pt->isUserInstrument() || pt->isEdited()); };
	else
		cond = [](const std::shared_ptr<CommandSequence>& pt) { return pt->isUserInstrument(); };
	auto&& it = std::find_if_not(ptSSG_.begin(), ptSSG_.end(), cond);

	if (it == ptSSG_.end()) return -1;

	if (!regardingUnedited_) (*it)->clearParameters();
	return std::distance(ptSSG_.begin(), it);
}

bool InstrumentsManager::equalPropertiesSSG(std::shared_ptr<AbstractInstrument> a, std::shared_ptr<AbstractInstrument> b) const
{
	auto aSsg = std::dynamic_pointer_cast<InstrumentSSG>(a);
	auto bSsg = std::dynamic_pointer_cast<InstrumentSSG>(b);

	if (aSsg->getWaveformEnabled() != bSsg->getWaveformEnabled())
		return false;
	if (aSsg->getWaveformEnabled()
			&& *wfSSG_[aSsg->getWaveformNumber()].get() != *wfSSG_[bSsg->getWaveformNumber()].get())
		return false;
	if (aSsg->getToneNoiseEnabled() != bSsg->getToneNoiseEnabled())
		return false;
	if (aSsg->getToneNoiseEnabled()
			&& *tnSSG_[aSsg->getToneNoiseNumber()].get() != *tnSSG_[bSsg->getToneNoiseNumber()].get())
		return false;
	if (aSsg->getEnvelopeEnabled() != bSsg->getEnvelopeEnabled())
		return false;
	if (aSsg->getEnvelopeEnabled()
			&& *envSSG_[aSsg->getEnvelopeNumber()].get() != *envSSG_[bSsg->getEnvelopeNumber()].get())
		return false;
	if (aSsg->getArpeggioEnabled() != bSsg->getArpeggioEnabled())
		return false;
	if (aSsg->getArpeggioEnabled()
			&& *arpSSG_[aSsg->getArpeggioNumber()].get() != *arpSSG_[bSsg->getArpeggioNumber()].get())
		return false;
	if (aSsg->getPitchEnabled() != bSsg->getPitchEnabled())
		return false;
	if (aSsg->getPitchEnabled()
			&& *ptSSG_[aSsg->getPitchNumber()].get() != *ptSSG_[bSsg->getPitchNumber()].get())
		return false;
	return true;
}

//----- ADPCM methods -----
void InstrumentsManager::setInstrumentADPCMSample(int instNum, int sampNum)
{
	auto adpcm = std::dynamic_pointer_cast<InstrumentADPCM>(insts_.at(static_cast<size_t>(instNum)));
	sampADPCM_.at(static_cast<size_t>(adpcm->getSampleNumber()))->deregisterUserInstrument(instNum);
	sampADPCM_.at(static_cast<size_t>(sampNum))->registerUserInstrument(instNum);

	adpcm->setSampleNumber(sampNum);
}

int InstrumentsManager::getInstrumentADPCMSample(int instNum)
{
	return std::dynamic_pointer_cast<InstrumentADPCM>(insts_[static_cast<size_t>(instNum)])->getSampleNumber();
}

void InstrumentsManager::setSampleADPCMRootKeyNumber(int sampNum, int n)
{
	sampADPCM_.at(static_cast<size_t>(sampNum))->setRootKeyNumber(n);
}

int InstrumentsManager::getSampleADPCMRootKeyNumber(int sampNum) const
{
	return sampADPCM_.at(static_cast<size_t>(sampNum))->getRootKeyNumber();
}

void InstrumentsManager::setSampleADPCMRootDeltaN(int sampNum, int dn)
{
	sampADPCM_.at(static_cast<size_t>(sampNum))->setRootDeltaN(dn);
}

int InstrumentsManager::getSampleADPCMRootDeltaN(int sampNum) const
{
	return sampADPCM_.at(static_cast<size_t>(sampNum))->getRootDeltaN();
}

void InstrumentsManager::setSampleADPCMRepeatEnabled(int sampNum, bool enabled)
{
	sampADPCM_.at(static_cast<size_t>(sampNum))->setRepeatEnabled(enabled);
}

bool InstrumentsManager::isSampleADPCMRepeatable(int sampNum) const
{
	return sampADPCM_.at(static_cast<size_t>(sampNum))->isRepeatable();
}

void InstrumentsManager::storeSampleADPCMRawSample(int sampNum, std::vector<uint8_t> sample)
{
	sampADPCM_.at(static_cast<size_t>(sampNum))->storeSample(sample);
}

void InstrumentsManager::clearSampleADPCMRawSample(int sampNum)
{
	sampADPCM_.at(static_cast<size_t>(sampNum))->clearSample();
}

std::vector<uint8_t> InstrumentsManager::getSampleADPCMRawSample(int sampNum) const
{
	return sampADPCM_.at(static_cast<size_t>(sampNum))->getSamples();
}

void InstrumentsManager::setSampleADPCMStartAddress(int sampNum, size_t addr)
{
	sampADPCM_.at(static_cast<size_t>(sampNum))->setStartAddress(addr);
}

size_t InstrumentsManager::getSampleADPCMStartAddress(int sampNum) const
{
	return sampADPCM_.at(static_cast<size_t>(sampNum))->getStartAddress();
}

void InstrumentsManager::setSampleADPCMStopAddress(int sampNum, size_t addr)
{
	sampADPCM_.at(static_cast<size_t>(sampNum))->setStopAddress(addr);
}

size_t InstrumentsManager::getSampleADPCMStopAddress(int sampNum) const
{
	return sampADPCM_.at(static_cast<size_t>(sampNum))->getStopAddress();
}

std::vector<int> InstrumentsManager::getSampleADPCMUsers(int sampNum) const
{
	return sampADPCM_.at(static_cast<size_t>(sampNum))->getUserInstruments();
}

std::vector<int> InstrumentsManager::getSampleADPCMEntriedIndices() const
{
	std::vector<int> idcs;
	int n = 0;
	for (auto& samp : sampADPCM_) {
		if (samp->isEdited()) idcs.push_back(n);
		++n;
	}
	return idcs;
}

std::vector<int> InstrumentsManager::getSampleADPCMValidIndices() const
{
	std::vector<int> idcs;
	int n = 0;
	for (auto& samp : sampADPCM_) {
		if (samp->isUserInstrument() && samp->isEdited()) idcs.push_back(n);
		++n;
	}
	return idcs;
}

void InstrumentsManager::clearUnusedSamplesADPCM()
{
	for (size_t i = 0; i < 128; ++i) {
		if (!sampADPCM_[i]->isUserInstrument())
			sampADPCM_[i] = std::make_shared<SampleADPCM>(i);
	}
}

int InstrumentsManager::findFirstAssignableSampleADPCM(int startIndex) const
{
	std::function<bool(const std::shared_ptr<SampleADPCM>&)> cond;
	if (regardingUnedited_)
		cond = [](const std::shared_ptr<SampleADPCM>& samp) { return (samp->isUserInstrument() || samp->isEdited()); };
	else
		cond = [](const std::shared_ptr<SampleADPCM>& samp) { return samp->isUserInstrument(); };
	auto&& it = std::find_if_not(sampADPCM_.begin() + startIndex, sampADPCM_.end(), cond);

	if (it == sampADPCM_.end()) return -1;

	if (!regardingUnedited_) (*it)->clearParameters();
	return std::distance(sampADPCM_.begin(), it);
}

void InstrumentsManager::setInstrumentADPCMEnvelopeEnabled(int instNum, bool enabled)
{
	auto adpcm = std::dynamic_pointer_cast<InstrumentADPCM>(insts_.at(static_cast<size_t>(instNum)));
	adpcm->setEnvelopeEnabled(enabled);
	if (enabled)
		envADPCM_.at(static_cast<size_t>(adpcm->getEnvelopeNumber()))->registerUserInstrument(instNum);
	else
		envADPCM_.at(static_cast<size_t>(adpcm->getEnvelopeNumber()))->deregisterUserInstrument(instNum);
}

bool InstrumentsManager::getInstrumentADPCMEnvelopeEnabled(int instNum) const
{
	return std::dynamic_pointer_cast<InstrumentADPCM>(insts_.at(static_cast<size_t>(instNum)))->getEnvelopeEnabled();
}

void InstrumentsManager::setInstrumentADPCMEnvelope(int instNum, int envNum)
{
	auto adpcm = std::dynamic_pointer_cast<InstrumentADPCM>(insts_.at(static_cast<size_t>(instNum)));
	if (adpcm->getEnvelopeEnabled()) {
		envADPCM_.at(static_cast<size_t>(adpcm->getEnvelopeNumber()))->deregisterUserInstrument(instNum);
		envADPCM_.at(static_cast<size_t>(envNum))->registerUserInstrument(instNum);
	}
	adpcm->setEnvelopeNumber(envNum);
}

int InstrumentsManager::getInstrumentADPCMEnvelope(int instNum)
{
	return std::dynamic_pointer_cast<InstrumentADPCM>(insts_[static_cast<size_t>(instNum)])->getEnvelopeNumber();
}

void InstrumentsManager::addEnvelopeADPCMSequenceCommand(int envNum, int type, int data)
{
	envADPCM_.at(static_cast<size_t>(envNum))->addSequenceCommand(type, data);
}

void InstrumentsManager::removeEnvelopeADPCMSequenceCommand(int envNum)
{
	envADPCM_.at(static_cast<size_t>(envNum))->removeSequenceCommand();
}

void InstrumentsManager::setEnvelopeADPCMSequenceCommand(int envNum, int cnt, int type, int data)
{
	envADPCM_.at(static_cast<size_t>(envNum))->setSequenceCommand(cnt, type, data);
}

std::vector<CommandSequenceUnit> InstrumentsManager::getEnvelopeADPCMSequence(int envNum)
{
	return envADPCM_.at(static_cast<size_t>(envNum))->getSequence();
}

void InstrumentsManager::setEnvelopeADPCMLoops(int envNum, std::vector<int> begins, std::vector<int> ends, std::vector<int> times)
{
	envADPCM_.at(static_cast<size_t>(envNum))->setLoops(std::move(begins), std::move(ends), std::move(times));
}

std::vector<Loop> InstrumentsManager::getEnvelopeADPCMLoops(int envNum) const
{
	return envADPCM_.at(static_cast<size_t>(envNum))->getLoops();
}

void InstrumentsManager::setEnvelopeADPCMRelease(int envNum, ReleaseType type, int begin)
{
	envADPCM_.at(static_cast<size_t>(envNum))->setRelease(type, begin);
}

Release InstrumentsManager::getEnvelopeADPCMRelease(int envNum) const
{
	return envADPCM_.at(static_cast<size_t>(envNum))->getRelease();
}

std::unique_ptr<CommandSequence::Iterator> InstrumentsManager::getEnvelopeADPCMIterator(int envNum) const
{
	return envADPCM_.at(static_cast<size_t>(envNum))->getIterator();
}

std::vector<int> InstrumentsManager::getEnvelopeADPCMUsers(int envNum) const
{
	return envADPCM_.at(static_cast<size_t>(envNum))->getUserInstruments();
}

std::vector<int> InstrumentsManager::getEnvelopeADPCMEntriedIndices() const
{
	std::vector<int> idcs;
	int n = 0;
	for (auto& env : envADPCM_) {
		if (env->isUserInstrument() || env->isEdited()) idcs.push_back(n);
		++n;
	}
	return idcs;
}

int InstrumentsManager::findFirstAssignableEnvelopeADPCM() const
{
	std::function<bool(const std::shared_ptr<CommandSequence>&)> cond;
	if (regardingUnedited_)
		cond = [](const std::shared_ptr<CommandSequence>& env) { return (env->isUserInstrument() || env->isEdited()); };
	else
		cond = [](const std::shared_ptr<CommandSequence>& env) { return env->isUserInstrument(); };
	auto&& it = std::find_if_not(envADPCM_.begin(), envADPCM_.end(), cond);

	if (it == envADPCM_.end()) return -1;

	if (!regardingUnedited_) (*it)->clearParameters();
	return std::distance(envADPCM_.begin(), it);
}

void InstrumentsManager::setInstrumentADPCMArpeggioEnabled(int instNum, bool enabled)
{
	auto adpcm = std::dynamic_pointer_cast<InstrumentADPCM>(insts_.at(static_cast<size_t>(instNum)));
	adpcm->setArpeggioEnabled(enabled);
	if (enabled)
		arpADPCM_.at(static_cast<size_t>(adpcm->getArpeggioNumber()))->registerUserInstrument(instNum);
	else
		arpADPCM_.at(static_cast<size_t>(adpcm->getArpeggioNumber()))->deregisterUserInstrument(instNum);
}

bool InstrumentsManager::getInstrumentADPCMArpeggioEnabled(int instNum) const
{
	return std::dynamic_pointer_cast<InstrumentADPCM>(insts_.at(static_cast<size_t>(instNum)))->getArpeggioEnabled();
}

void InstrumentsManager::setInstrumentADPCMArpeggio(int instNum, int arpNum)
{
	auto adpcm = std::dynamic_pointer_cast<InstrumentADPCM>(insts_.at(static_cast<size_t>(instNum)));
	if (adpcm->getArpeggioEnabled()) {
		arpADPCM_.at(static_cast<size_t>(adpcm->getArpeggioNumber()))->deregisterUserInstrument(instNum);
		arpADPCM_.at(static_cast<size_t>(arpNum))->registerUserInstrument(instNum);
	}
	adpcm->setArpeggioNumber(arpNum);
}

int InstrumentsManager::getInstrumentADPCMArpeggio(int instNum)
{
	return std::dynamic_pointer_cast<InstrumentADPCM>(insts_[static_cast<size_t>(instNum)])->getArpeggioNumber();
}

void InstrumentsManager::setArpeggioADPCMType(int arpNum, SequenceType type)
{
	arpADPCM_.at(static_cast<size_t>(arpNum))->setType(type);
}

SequenceType InstrumentsManager::getArpeggioADPCMType(int arpNum) const
{
	return arpADPCM_.at(static_cast<size_t>(arpNum))->getType();
}

void InstrumentsManager::addArpeggioADPCMSequenceCommand(int arpNum, int type, int data)
{
	arpADPCM_.at(static_cast<size_t>(arpNum))->addSequenceCommand(type, data);
}

void InstrumentsManager::removeArpeggioADPCMSequenceCommand(int arpNum)
{
	arpADPCM_.at(static_cast<size_t>(arpNum))->removeSequenceCommand();
}

void InstrumentsManager::setArpeggioADPCMSequenceCommand(int arpNum, int cnt, int type, int data)
{
	arpADPCM_.at(static_cast<size_t>(arpNum))->setSequenceCommand(cnt, type, data);
}

std::vector<CommandSequenceUnit> InstrumentsManager::getArpeggioADPCMSequence(int arpNum)
{
	return arpADPCM_.at(static_cast<size_t>(arpNum))->getSequence();
}

void InstrumentsManager::setArpeggioADPCMLoops(int arpNum, std::vector<int> begins, std::vector<int> ends, std::vector<int> times)
{
	arpADPCM_.at(static_cast<size_t>(arpNum))->setLoops(std::move(begins), std::move(ends), std::move(times));
}

std::vector<Loop> InstrumentsManager::getArpeggioADPCMLoops(int arpNum) const
{
	return arpADPCM_.at(static_cast<size_t>(arpNum))->getLoops();
}

void InstrumentsManager::setArpeggioADPCMRelease(int arpNum, ReleaseType type, int begin)
{
	arpADPCM_.at(static_cast<size_t>(arpNum))->setRelease(type, begin);
}

Release InstrumentsManager::getArpeggioADPCMRelease(int arpNum) const
{
	return arpADPCM_.at(static_cast<size_t>(arpNum))->getRelease();
}

std::unique_ptr<CommandSequence::Iterator> InstrumentsManager::getArpeggioADPCMIterator(int arpNum) const
{
	return arpADPCM_.at(static_cast<size_t>(arpNum))->getIterator();
}

std::vector<int> InstrumentsManager::getArpeggioADPCMUsers(int arpNum) const
{
	return arpADPCM_.at(static_cast<size_t>(arpNum))->getUserInstruments();
}

std::vector<int> InstrumentsManager::getArpeggioADPCMEntriedIndices() const
{
	std::vector<int> idcs;
	int n = 0;
	for (auto& arp : arpADPCM_) {
		if (arp->isUserInstrument() || arp->isEdited()) idcs.push_back(n);
		++n;
	}
	return idcs;
}

int InstrumentsManager::findFirstAssignableArpeggioADPCM() const
{
	std::function<bool(const std::shared_ptr<CommandSequence>&)> cond;
	if (regardingUnedited_)
		cond = [](const std::shared_ptr<CommandSequence>& arp) { return (arp->isUserInstrument() || arp->isEdited()); };
	else
		cond = [](const std::shared_ptr<CommandSequence>& arp) { return arp->isUserInstrument(); };
	auto&& it = std::find_if_not(arpADPCM_.begin(), arpADPCM_.end(), cond);

	if (it == arpADPCM_.end()) return -1;

	if (!regardingUnedited_) (*it)->clearParameters();
	return std::distance(arpADPCM_.begin(), it);
}

void InstrumentsManager::setInstrumentADPCMPitchEnabled(int instNum, bool enabled)
{
	auto adpcm = std::dynamic_pointer_cast<InstrumentADPCM>(insts_.at(static_cast<size_t>(instNum)));
	adpcm->setPitchEnabled(enabled);
	if (enabled)
		ptADPCM_.at(static_cast<size_t>(adpcm->getPitchNumber()))->registerUserInstrument(instNum);
	else
		ptADPCM_.at(static_cast<size_t>(adpcm->getPitchNumber()))->deregisterUserInstrument(instNum);
}

bool InstrumentsManager::getInstrumentADPCMPitchEnabled(int instNum) const
{
	return std::dynamic_pointer_cast<InstrumentADPCM>(insts_.at(static_cast<size_t>(instNum)))->getPitchEnabled();
}

void InstrumentsManager::setInstrumentADPCMPitch(int instNum, int ptNum)
{
	auto adpcm = std::dynamic_pointer_cast<InstrumentADPCM>(insts_.at(static_cast<size_t>(instNum)));
	if (adpcm->getPitchEnabled()) {
		ptADPCM_.at(static_cast<size_t>(adpcm->getPitchNumber()))->deregisterUserInstrument(instNum);
		ptADPCM_.at(static_cast<size_t>(ptNum))->registerUserInstrument(instNum);
	}
	adpcm->setPitchNumber(ptNum);
}

int InstrumentsManager::getInstrumentADPCMPitch(int instNum)
{
	return std::dynamic_pointer_cast<InstrumentADPCM>(insts_[static_cast<size_t>(instNum)])->getPitchNumber();
}

void InstrumentsManager::setPitchADPCMType(int ptNum, SequenceType type)
{
	ptADPCM_.at(static_cast<size_t>(ptNum))->setType(type);
}

SequenceType InstrumentsManager::getPitchADPCMType(int ptNum) const
{
	return ptADPCM_.at(static_cast<size_t>(ptNum))->getType();
}

void InstrumentsManager::addPitchADPCMSequenceCommand(int ptNum, int type, int data)
{
	ptADPCM_.at(static_cast<size_t>(ptNum))->addSequenceCommand(type, data);
}

void InstrumentsManager::removePitchADPCMSequenceCommand(int ptNum)
{
	ptADPCM_.at(static_cast<size_t>(ptNum))->removeSequenceCommand();
}

void InstrumentsManager::setPitchADPCMSequenceCommand(int ptNum, int cnt, int type, int data)
{
	ptADPCM_.at(static_cast<size_t>(ptNum))->setSequenceCommand(cnt, type, data);
}

std::vector<CommandSequenceUnit> InstrumentsManager::getPitchADPCMSequence(int ptNum)
{
	return ptADPCM_.at(static_cast<size_t>(ptNum))->getSequence();
}

void InstrumentsManager::setPitchADPCMLoops(int ptNum, std::vector<int> begins, std::vector<int> ends, std::vector<int> times)
{
	ptADPCM_.at(static_cast<size_t>(ptNum))->setLoops(std::move(begins), std::move(ends), std::move(times));
}

std::vector<Loop> InstrumentsManager::getPitchADPCMLoops(int ptNum) const
{
	return ptADPCM_.at(static_cast<size_t>(ptNum))->getLoops();
}

void InstrumentsManager::setPitchADPCMRelease(int ptNum, ReleaseType type, int begin)
{
	ptADPCM_.at(static_cast<size_t>(ptNum))->setRelease(type, begin);
}

Release InstrumentsManager::getPitchADPCMRelease(int ptNum) const
{
	return ptADPCM_.at(static_cast<size_t>(ptNum))->getRelease();
}

std::unique_ptr<CommandSequence::Iterator> InstrumentsManager::getPitchADPCMIterator(int ptNum) const
{
	return ptADPCM_.at(static_cast<size_t>(ptNum))->getIterator();
}

std::vector<int> InstrumentsManager::getPitchADPCMUsers(int ptNum) const
{
	return ptADPCM_.at(static_cast<size_t>(ptNum))->getUserInstruments();
}

std::vector<int> InstrumentsManager::getPitchADPCMEntriedIndices() const
{
	std::vector<int> idcs;
	int n = 0;
	for (auto& pt : ptADPCM_) {
		if (pt->isUserInstrument() || pt->isEdited()) idcs.push_back(n);
		++n;
	}
	return idcs;
}

int InstrumentsManager::findFirstAssignablePitchADPCM() const
{
	std::function<bool(const std::shared_ptr<CommandSequence>&)> cond;
	if (regardingUnedited_)
		cond = [](const std::shared_ptr<CommandSequence>& pt) { return (pt->isUserInstrument() || pt->isEdited()); };
	else
		cond = [](const std::shared_ptr<CommandSequence>& pt) { return pt->isUserInstrument(); };
	auto&& it = std::find_if_not(ptADPCM_.begin(), ptADPCM_.end(), cond);

	if (it == ptADPCM_.end()) return -1;

	if (!regardingUnedited_) (*it)->clearParameters();
	return std::distance(ptADPCM_.begin(), it);
}

bool InstrumentsManager::equalPropertiesADPCM(std::shared_ptr<AbstractInstrument> a, std::shared_ptr<AbstractInstrument> b) const
{
	auto aAdpcm = std::dynamic_pointer_cast<InstrumentADPCM>(a);
	auto bAdpcm = std::dynamic_pointer_cast<InstrumentADPCM>(b);

	if (*sampADPCM_[aAdpcm->getSampleNumber()].get() != *sampADPCM_[bAdpcm->getSampleNumber()].get())
		return false;
	if (aAdpcm->getEnvelopeEnabled() != bAdpcm->getEnvelopeEnabled())
		return false;
	if (aAdpcm->getEnvelopeEnabled()
			&& *envADPCM_[aAdpcm->getEnvelopeNumber()].get() != *envADPCM_[bAdpcm->getEnvelopeNumber()].get())
		return false;
	if (aAdpcm->getArpeggioEnabled() != bAdpcm->getArpeggioEnabled())
		return false;
	if (aAdpcm->getArpeggioEnabled()
			&& *arpADPCM_[aAdpcm->getArpeggioNumber()].get() != *arpADPCM_[bAdpcm->getArpeggioNumber()].get())
		return false;
	if (aAdpcm->getPitchEnabled() != bAdpcm->getPitchEnabled())
		return false;
	if (aAdpcm->getPitchEnabled()
			&& *ptADPCM_[aAdpcm->getPitchNumber()].get() != *ptADPCM_[bAdpcm->getPitchNumber()].get())
		return false;
	return true;
}

//----- Drumkit methods -----
void InstrumentsManager::setInstrumentDrumkitSamplesEnabled(int instNum, int key, bool enabled)
{
	auto kit = std::dynamic_pointer_cast<InstrumentDrumkit>(insts_.at(static_cast<size_t>(instNum)));
	if (enabled) {
		kit->setSampleEnabled(key, true);
		sampADPCM_.at(static_cast<size_t>(kit->getSampleNumber(key)))->registerUserInstrument(instNum);
	}
	else {
		sampADPCM_.at(static_cast<size_t>(kit->getSampleNumber(key)))->deregisterUserInstrument(instNum);
		kit->setSampleEnabled(key, false);
	}
}

bool InstrumentsManager::getInstrumentDrumkitSamplesEnabled(int instNum, int key) const
{
	return std::dynamic_pointer_cast<InstrumentDrumkit>(insts_.at(static_cast<size_t>(instNum)))->getSampleEnabled(key);
}

void InstrumentsManager::setInstrumentDrumkitSamples(int instNum, int key, int sampNum)
{
	auto kit = std::dynamic_pointer_cast<InstrumentDrumkit>(insts_.at(static_cast<size_t>(instNum)));
	sampADPCM_.at(static_cast<size_t>(kit->getSampleNumber(key)))->deregisterUserInstrument(instNum);
	sampADPCM_.at(static_cast<size_t>(sampNum))->registerUserInstrument(instNum);

	kit->setSampleNumber(key, sampNum);
}

int InstrumentsManager::getInstrumentDrumkitSamples(int instNum, int key)
{
	return std::dynamic_pointer_cast<InstrumentDrumkit>(insts_[static_cast<size_t>(instNum)])->getSampleNumber(key);
}

void InstrumentsManager::setInstrumentDrumkitPitch(int instNum, int key, int pitch)
{
	std::dynamic_pointer_cast<InstrumentDrumkit>(insts_.at(static_cast<size_t>(instNum)))->setPitch(key, pitch);
}

bool InstrumentsManager::equalPropertiesDrumkit(std::shared_ptr<AbstractInstrument> a, std::shared_ptr<AbstractInstrument> b) const
{
	auto aKit = std::dynamic_pointer_cast<InstrumentDrumkit>(a);
	auto bKit = std::dynamic_pointer_cast<InstrumentDrumkit>(b);

	std::vector<int> aKeys = aKit->getAssignedKeys();
	std::vector<int> bKeys = bKit->getAssignedKeys();
	if (aKeys.size() != bKeys.size()) return false;
	std::sort(aKeys.begin(), aKeys.end());
	std::sort(bKeys.begin(), bKeys.end());
	if (!std::includes(aKeys.begin(), aKeys.end(), bKeys.begin(), bKeys.end())) return false;

	for (const int& key : aKeys) {
		if (*sampADPCM_[aKit->getSampleNumber(key)].get() != *sampADPCM_[bKit->getSampleNumber(key)].get())
			return false;
		if (aKit->getPitch(key) != bKit->getPitch(key))
			return false;
	}

	return true;
}
