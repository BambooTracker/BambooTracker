/*
 * Copyright (C) 2018-2023 Rerrah
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#include "instruments_manager.hpp"
#include <utility>
#include <unordered_map>
#include <algorithm>
#include <functional>
#include <type_traits>
#include <stdexcept>
#include "instrument.hpp"
#include "note.hpp"
#include "utils.hpp"

namespace
{
// Constant
const FMEnvelopeParameter ENV_FM_PARAMS[] = {
	FMEnvelopeParameter::AL, FMEnvelopeParameter::FB, FMEnvelopeParameter::AR1,
	FMEnvelopeParameter::DR1, FMEnvelopeParameter::SR1, FMEnvelopeParameter::RR1,
	FMEnvelopeParameter::SL1, FMEnvelopeParameter::TL1, FMEnvelopeParameter::KS1,
	FMEnvelopeParameter::ML1, FMEnvelopeParameter::DT1, FMEnvelopeParameter::AR2,
	FMEnvelopeParameter::DR2, FMEnvelopeParameter::SR2, FMEnvelopeParameter::RR2,
	FMEnvelopeParameter::SL2, FMEnvelopeParameter::TL2, FMEnvelopeParameter::KS2,
	FMEnvelopeParameter::ML2, FMEnvelopeParameter::DT2, FMEnvelopeParameter::AR3,
	FMEnvelopeParameter::DR3, FMEnvelopeParameter::SR3, FMEnvelopeParameter::RR3,
	FMEnvelopeParameter::SL3, FMEnvelopeParameter::TL3, FMEnvelopeParameter::KS3,
	FMEnvelopeParameter::ML3, FMEnvelopeParameter::DT3, FMEnvelopeParameter::AR4,
	FMEnvelopeParameter::DR4, FMEnvelopeParameter::SR4, FMEnvelopeParameter::RR4,
	FMEnvelopeParameter::SL4, FMEnvelopeParameter::TL4, FMEnvelopeParameter::KS4,
	FMEnvelopeParameter::ML4, FMEnvelopeParameter::DT4 };

const FMOperatorType FM_OP_TYPES[] = {
	FMOperatorType::All, FMOperatorType::Op1, FMOperatorType::Op2,
	FMOperatorType::Op3, FMOperatorType::Op4
};

// Property initialization helper
inline auto makeEnvelopeFMSharedPtr(int n)
{
	return std::make_shared<EnvelopeFM>(n);
}

inline auto makeLFOFMSharedPtr(int n)
{
	return std::make_shared<LFOFM>(n);
}

inline auto makeOperatorSequenceFMSharedPtr(int n)
{
	return std::make_shared<InstrumentSequenceProperty<
			FMOperatorSequenceUnit>>(n, SequenceType::PlainSequence, FMOperatorSequenceUnit(0), FMOperatorSequenceUnit());
}

inline auto makeArpeggioSharedPtr(int n)
{
	return std::make_shared<InstrumentSequenceProperty<
			ArpeggioUnit>>(n, SequenceType::AbsoluteSequence, ArpeggioUnit(Note::DEFAULT_NOTE_NUM), ArpeggioUnit());
}

inline auto makePitchSharedPtr(int n)
{
	return std::make_shared<InstrumentSequenceProperty<
			PitchUnit>>(n, SequenceType::AbsoluteSequence, PitchUnit(SEQ_PITCH_CENTER), PitchUnit());
}

inline auto makePanSharedPtr(int n)
{
	return std::make_shared<InstrumentSequenceProperty<
			PanUnit>>(n, SequenceType::AbsoluteSequence, PanUnit(PanType::CENTER), PanUnit());
}

inline auto makeWaveformSSGSharedPtr(int n)
{
	return std::make_shared<InstrumentSequenceProperty<
			SSGWaveformUnit>>(n, SequenceType::PlainSequence,
							  SSGWaveformUnit::makeOnlyDataUnit(SSGWaveformType::SQUARE),
							  SSGWaveformUnit());
}

inline auto makeToneNoiseSSGSharedPtr(int n)
{
	return std::make_shared<InstrumentSequenceProperty<
			SSGToneNoiseUnit>>(n, SequenceType::PlainSequence, SSGToneNoiseUnit(0), SSGToneNoiseUnit());
}

inline auto makeEnvelopeSSGSharedPtr(int n)
{
	return std::make_shared<InstrumentSequenceProperty<
			SSGWaveformUnit>>(n, SequenceType::PlainSequence,
							  SSGEnvelopeUnit::makeOnlyDataUnit(15), SSGEnvelopeUnit(), 15);
}

inline auto makeSampleADPCMSharedPtr(int n)
{
	return std::make_shared<SampleADPCM>(n);
}

inline auto makeEnvelopeADPCMSharedPtr(int n)
{
	return std::make_shared<InstrumentSequenceProperty<
			ADPCMEnvelopeUnit>>(n, SequenceType::PlainSequence,
								ADPCMEnvelopeUnit(255), ADPCMEnvelopeUnit(), 127);
}

// Utility functor
template <class propArray>
struct IsUsed
{
	using ArgType = typename std::add_const<propArray>::type::const_reference;
	bool operator()(ArgType prop) const
	{
		return prop->isUserInstrument();
	}
};

template <class propArray>
struct IsEdited
{
	using ArgType = typename std::add_const<propArray>::type::const_reference;
	bool operator()(ArgType prop) const
	{
		return prop->isEdited();
	}
};

template <class propArray>
struct IsUsedOrEdited
{
	using ArgType = typename std::add_const<propArray>::type::const_reference;
	bool operator()(ArgType prop) const
	{
		return (prop->isUserInstrument() || prop->isEdited());
	}
};

// Common process for properties
template <class propArray>
int cloneProperty(propArray& aryRef, int srcNum)
{
	// Must find an unused property since the count of used properties <= the count of instruments
	auto&& it = std::find_if_not(aryRef.begin(), aryRef.end(), IsUsed<propArray>());
	int cloneNum = std::distance(aryRef.begin(), it);
	*it = aryRef.at(static_cast<size_t>(srcNum))->clone();
	(*it)->setNumber(cloneNum);
	return cloneNum;
}

template <class propArray>
int findFirstAssignableProperty(propArray& aryRef, bool regardingUnedited, int startOffs = 0)
{
	std::function<bool(typename propArray::const_reference)> cond;
	if (regardingUnedited) cond = IsUsedOrEdited<propArray>();
	else cond = IsUsed<propArray>();
	auto&& it = std::find_if_not(aryRef.cbegin() + startOffs, aryRef.cend(), cond);

	if (it == aryRef.cend()) return -1;

	if (!regardingUnedited) (*it)->clearParameters();
	return std::distance(aryRef.cbegin(), it);
}
}

InstrumentsManager::InstrumentsManager(bool unedited)
	: regardingUnedited_(unedited)
{
	clearAll();
}

void InstrumentsManager::addInstrument(int instNum, InstrumentType type, const std::string& name)
{
	if (instNum < 0 || static_cast<int>(insts_.size()) <= instNum) return;

	switch (type) {
	case InstrumentType::FM:
	{
		auto fm = new InstrumentFM(instNum, name, this);
		int envNum = findFirstAssignableEnvelopeFM();
		if (envNum == -1) envNum = static_cast<int>(envFM_.size()) - 1;
		fm->setEnvelopeNumber(envNum);
		envFM_.at(static_cast<size_t>(envNum))->registerUserInstrument(instNum);
		int lfoNum = findFirstAssignableLFOFM();
		if (lfoNum == -1) lfoNum = static_cast<int>(lfoFM_.size()) - 1;
		fm->setLFONumber(lfoNum);
		fm->setLFOEnabled(false);
		for (auto param : ENV_FM_PARAMS) {
			int opSeqNum = findFirstAssignableOperatorSequenceFM(param);
			if (opSeqNum == -1) opSeqNum = static_cast<int>(opSeqFM_.at(param).size()) - 1;
			fm->setOperatorSequenceNumber(param, opSeqNum);
			fm->setOperatorSequenceEnabled(param, false);
		}
		int arpNum = findFirstAssignableArpeggioFM();
		if (arpNum == -1) arpNum = static_cast<int>(arpFM_.size()) - 1;
		int ptNum = findFirstAssignablePitchFM();
		if (ptNum == -1) ptNum = static_cast<int>(ptFM_.size()) - 1;
		for (auto type : FM_OP_TYPES) {
			fm->setArpeggioNumber(type, arpNum);
			fm->setArpeggioEnabled(type, false);
			fm->setPitchNumber(type, ptNum);
			fm->setPitchEnabled(type, false);
		}
		int panNum = findFirstAssignablePanFM();
		if (panNum == -1) panNum = static_cast<int>(panFM_.size()) - 1;
		fm->setPanNumber(panNum);
		fm->setPanEnabled(false);
		insts_.at(static_cast<size_t>(instNum)).reset(fm);
		break;
	}
	case InstrumentType::SSG:
	{
		auto ssg = new InstrumentSSG(instNum, name, this);
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
		insts_.at(static_cast<size_t>(instNum)).reset(ssg);
		break;
	}
	case InstrumentType::ADPCM:
	{
		auto adpcm = new InstrumentADPCM(instNum, name, this);
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
		int panNum = findFirstAssignablePanADPCM();
		if (panNum == -1) panNum = static_cast<int>(panADPCM_.size()) - 1;
		adpcm->setPanNumber(panNum);
		adpcm->setPanEnabled(false);
		insts_.at(static_cast<size_t>(instNum)).reset(adpcm);
		break;
	}
	case InstrumentType::Drumkit:
	{
		insts_.at(static_cast<size_t>(instNum)).reset(new InstrumentDrumkit(instNum, name, this));
		break;
	}
	default:
		throw std::invalid_argument("invalid instrument type");
	}
}

void InstrumentsManager::addInstrument(AbstractInstrument* newInstPtr)
{	
	int num = newInstPtr->getNumber();
	std::shared_ptr<AbstractInstrument>& inst = insts_.at(static_cast<size_t>(num));
	inst.reset(newInstPtr);

	switch (inst->getType()) {
	case InstrumentType::FM:
	{
		auto fm = std::dynamic_pointer_cast<InstrumentFM>(inst);
		envFM_.at(static_cast<size_t>(fm->getEnvelopeNumber()))->registerUserInstrument(num);
		if (fm->getLFOEnabled()) lfoFM_.at(static_cast<size_t>(fm->getLFONumber()))->registerUserInstrument(num);
		for (auto p : ENV_FM_PARAMS) {
			if (fm->getOperatorSequenceEnabled(p))
				opSeqFM_.at(p).at(static_cast<size_t>(fm->getOperatorSequenceNumber(p)))
						->registerUserInstrument(num);
		}
		for (auto t : FM_OP_TYPES) {
			if (fm->getArpeggioEnabled(t))
				arpFM_.at(static_cast<size_t>(fm->getArpeggioNumber(t)))->registerUserInstrument(num);
			if (fm->getPitchEnabled(t))
				ptFM_.at(static_cast<size_t>(fm->getPitchNumber(t)))->registerUserInstrument(num);
		}
		if (fm->getPanEnabled())
			panFM_.at(static_cast<size_t>(fm->getPanNumber()))->registerUserInstrument(num);
		break;
	}
	case InstrumentType::SSG:
	{
		auto ssg = std::dynamic_pointer_cast<InstrumentSSG>(inst);
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
		auto adpcm = std::dynamic_pointer_cast<InstrumentADPCM>(inst);
		sampADPCM_.at(static_cast<size_t>(adpcm->getSampleNumber()))->registerUserInstrument(num);
		if (adpcm->getEnvelopeEnabled())
			envADPCM_.at(static_cast<size_t>(adpcm->getEnvelopeNumber()))->registerUserInstrument(num);
		if (adpcm->getArpeggioEnabled())
			arpADPCM_.at(static_cast<size_t>(adpcm->getArpeggioNumber()))->registerUserInstrument(num);
		if (adpcm->getPitchEnabled())
			ptADPCM_.at(static_cast<size_t>(adpcm->getPitchNumber()))->registerUserInstrument(num);
		if (adpcm->getPanEnabled())
			panADPCM_.at(static_cast<size_t>(adpcm->getPanNumber()))->registerUserInstrument(num);
		break;
	}
	case InstrumentType::Drumkit:
	{
		auto kit = std::dynamic_pointer_cast<InstrumentDrumkit>(inst);
		for (const auto& key : kit->getAssignedKeys()) {
			sampADPCM_.at(static_cast<size_t>(kit->getSampleNumber(key)))->registerUserInstrument(num);
		}
		break;
	}
	default:
		throw std::invalid_argument("invalid instrument type");
	}
}

std::unique_ptr<AbstractInstrument> InstrumentsManager::removeInstrument(int instNum)
{
	std::shared_ptr<AbstractInstrument>& inst = insts_.at(static_cast<size_t>(instNum));
	switch (inst->getType()) {
	case InstrumentType::FM:
	{
		auto fm = std::dynamic_pointer_cast<InstrumentFM>(inst);
		envFM_.at(static_cast<size_t>(fm->getEnvelopeNumber()))->deregisterUserInstrument(instNum);
		if (fm->getLFOEnabled())
			lfoFM_.at(static_cast<size_t>(fm->getLFONumber()))->deregisterUserInstrument(instNum);
		for (auto p : ENV_FM_PARAMS) {
			if (fm->getOperatorSequenceEnabled(p))
				opSeqFM_.at(p).at(static_cast<size_t>(fm->getOperatorSequenceNumber(p)))
						->deregisterUserInstrument(instNum);
		}
		for (auto t : FM_OP_TYPES) {
			if (fm->getArpeggioEnabled(t))
				arpFM_.at(static_cast<size_t>(fm->getArpeggioNumber(t)))->deregisterUserInstrument(instNum);
			if (fm->getPitchEnabled(t))
				ptFM_.at(static_cast<size_t>(fm->getPitchNumber(t)))->deregisterUserInstrument(instNum);
		}
		if (fm->getPanEnabled())
			panFM_.at(static_cast<size_t>(fm->getPanNumber()))->deregisterUserInstrument(instNum);
		break;
	}
	case InstrumentType::SSG:
	{
		auto ssg = std::dynamic_pointer_cast<InstrumentSSG>(inst);
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
		auto adpcm = std::dynamic_pointer_cast<InstrumentADPCM>(inst);
		sampADPCM_.at(static_cast<size_t>(adpcm->getSampleNumber()))->deregisterUserInstrument(instNum);
		if (adpcm->getEnvelopeEnabled())
			envADPCM_.at(static_cast<size_t>(adpcm->getEnvelopeNumber()))->deregisterUserInstrument(instNum);
		if (adpcm->getArpeggioEnabled())
			arpADPCM_.at(static_cast<size_t>(adpcm->getArpeggioNumber()))->deregisterUserInstrument(instNum);
		if (adpcm->getPitchEnabled())
			ptADPCM_.at(static_cast<size_t>(adpcm->getPitchNumber()))->deregisterUserInstrument(instNum);
		if (adpcm->getPanEnabled())
			panADPCM_.at(static_cast<size_t>(adpcm->getPanNumber()))->deregisterUserInstrument(instNum);
		break;
	}
	case InstrumentType::Drumkit:
	{
		auto kit = std::dynamic_pointer_cast<InstrumentDrumkit>(inst);
		for (const int& key : kit->getAssignedKeys()) {
			sampADPCM_.at(static_cast<size_t>(kit->getSampleNumber(key)))->deregisterUserInstrument(instNum);
		}
		break;
	}
	default:
		throw std::invalid_argument("invalid instrument type");
	}

	std::unique_ptr<AbstractInstrument> clone(inst->clone());
	inst.reset();
	return clone;
}

void InstrumentsManager::cloneInstrument(int cloneInstNum, int refInstNum)
{
	std::shared_ptr<AbstractInstrument>& refInst = insts_.at(static_cast<size_t>(refInstNum));
	addInstrument(cloneInstNum, refInst->getType(), refInst->getName());

	switch (refInst->getType()) {
	case InstrumentType::FM:
	{
		auto refFm = std::dynamic_pointer_cast<InstrumentFM>(refInst);
		auto cloneFm = std::dynamic_pointer_cast<InstrumentFM>(insts_.at(static_cast<size_t>(cloneInstNum)));
		setInstrumentFMEnvelope(cloneInstNum, refFm->getEnvelopeNumber());
		setInstrumentFMLFO(cloneInstNum, refFm->getLFONumber());
		if (refFm->getLFOEnabled()) setInstrumentFMLFOEnabled(cloneInstNum, true);
		for (auto p : ENV_FM_PARAMS) {
			setInstrumentFMOperatorSequence(cloneInstNum, p, refFm->getOperatorSequenceNumber(p));
			if (refFm->getOperatorSequenceEnabled(p)) setInstrumentFMOperatorSequenceEnabled(cloneInstNum, p, true);
		}
		for (auto t : FM_OP_TYPES) {
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
			setInstrumentDrumkitPan(cloneInstNum, key, refKit->getPan(key));
		}
		break;
	}
	default:
		throw std::invalid_argument("invalid instrument type");
	}
}

void InstrumentsManager::deepCloneInstrument(int cloneInstNum, int refInstNum)
{
	std::shared_ptr<AbstractInstrument>& refInst = insts_.at(static_cast<size_t>(refInstNum));
	addInstrument(cloneInstNum, refInst->getType(), refInst->getName());

	switch (refInst->getType()) {
	case InstrumentType::FM:
	{
		auto refFm = std::dynamic_pointer_cast<InstrumentFM>(refInst);
		auto cloneFm = std::dynamic_pointer_cast<InstrumentFM>(insts_.at(static_cast<size_t>(cloneInstNum)));
		
		envFM_[static_cast<size_t>(cloneFm->getEnvelopeNumber())]->deregisterUserInstrument(cloneInstNum);	// Remove temporary number
		int envNum = cloneProperty(envFM_, refFm->getEnvelopeNumber());
		cloneFm->setEnvelopeNumber(envNum);
		envFM_[static_cast<size_t>(envNum)]->registerUserInstrument(cloneInstNum);
		if (refFm->getLFOEnabled()) {
			cloneFm->setLFOEnabled(true);
			int lfoNum = cloneProperty(lfoFM_, refFm->getLFONumber());
			cloneFm->setLFONumber(lfoNum);
			lfoFM_[static_cast<size_t>(lfoNum)]->registerUserInstrument(cloneInstNum);
		}

		for (auto envParam : ENV_FM_PARAMS) {
			if (refFm->getOperatorSequenceEnabled(envParam)) {
				cloneFm->setOperatorSequenceEnabled(envParam, true);
				int opSeqNum = cloneProperty(opSeqFM_.at(envParam), refFm->getOperatorSequenceNumber(envParam));
				cloneFm->setOperatorSequenceNumber(envParam, opSeqNum);
				opSeqFM_.at(envParam)[static_cast<size_t>(opSeqNum)]->registerUserInstrument(cloneInstNum);
			}
		}

		std::unordered_map<int, int> arpCloneMap;
		for (auto opType : FM_OP_TYPES) {
			if (refFm->getArpeggioEnabled(opType)) {
				cloneFm->setArpeggioEnabled(opType, true);
				int srcNum = refFm->getArpeggioNumber(opType);
				if (!arpCloneMap.count(srcNum)) {
					arpCloneMap[srcNum] = cloneProperty(arpFM_, srcNum);
				}
				cloneFm->setArpeggioNumber(opType, arpCloneMap[srcNum]);
				arpFM_[static_cast<size_t>(arpCloneMap[srcNum])]->registerUserInstrument(cloneInstNum);
			}
		}

		std::unordered_map<int, int> ptCloneMap;
		for (auto opType : FM_OP_TYPES) {
			if (refFm->getPitchEnabled(opType)) {
				cloneFm->setPitchEnabled(opType, true);
				int srcNum = refFm->getPitchNumber(opType);
				if (!ptCloneMap.count(srcNum)) {
					ptCloneMap[srcNum] = cloneProperty(ptFM_, srcNum);
				}
				cloneFm->setPitchNumber(opType, ptCloneMap[srcNum]);
				ptFM_[static_cast<size_t>(ptCloneMap[srcNum])]->registerUserInstrument(cloneInstNum);
			}
		}

		if (refFm->getPanEnabled()) {
			cloneFm->setPanEnabled(true);
			int panNum = cloneProperty(panFM_, refFm->getPanNumber());
			cloneFm->setPanNumber(panNum);
			panFM_[static_cast<size_t>(panNum)]->registerUserInstrument(cloneInstNum);
		}

		for (auto t : FM_OP_TYPES)
			setInstrumentFMEnvelopeResetEnabled(cloneInstNum, t, refFm->getEnvelopeResetEnabled(t));
		break;
	}
	case InstrumentType::SSG:
	{
		auto refSsg = std::dynamic_pointer_cast<InstrumentSSG>(refInst);
		auto cloneSsg = std::dynamic_pointer_cast<InstrumentSSG>(insts_.at(static_cast<size_t>(cloneInstNum)));

		if (refSsg->getWaveformEnabled()) {
			cloneSsg->setWaveformEnabled(true);
			int wfNum = cloneProperty(wfSSG_, refSsg->getWaveformNumber());
			cloneSsg->setWaveformNumber(wfNum);
			wfSSG_[static_cast<size_t>(wfNum)]->registerUserInstrument(cloneInstNum);
		}
		if (refSsg->getToneNoiseEnabled()) {
			cloneSsg->setToneNoiseEnabled(true);
			int tnNum = cloneProperty(tnSSG_, refSsg->getToneNoiseNumber());
			cloneSsg->setToneNoiseNumber(tnNum);
			tnSSG_[static_cast<size_t>(tnNum)]->registerUserInstrument(cloneInstNum);
		}
		if (refSsg->getEnvelopeEnabled()) {
			cloneSsg->setEnvelopeEnabled(true);
			int envNum = cloneProperty(envSSG_, refSsg->getEnvelopeNumber());
			cloneSsg->setEnvelopeNumber(envNum);
			envSSG_[static_cast<size_t>(envNum)]->registerUserInstrument(cloneInstNum);
		}
		if (refSsg->getArpeggioEnabled()) {
			cloneSsg->setArpeggioEnabled(true);
			int arpNum = cloneProperty(arpSSG_, refSsg->getArpeggioNumber());
			cloneSsg->setArpeggioNumber(arpNum);
			arpSSG_[static_cast<size_t>(arpNum)]->registerUserInstrument(cloneInstNum);
		}
		if (refSsg->getPitchEnabled()) {
			cloneSsg->setPitchEnabled(true);
			int ptNum = cloneProperty(ptSSG_, refSsg->getPitchNumber());
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
		int sampNum = cloneProperty(sampADPCM_, refAdpcm->getSampleNumber());
		cloneAdpcm->setSampleNumber(sampNum);
		sampADPCM_[static_cast<size_t>(sampNum)]->registerUserInstrument(cloneInstNum);
		if (refAdpcm->getEnvelopeEnabled()) {
			cloneAdpcm->setEnvelopeEnabled(true);
			int envNum = cloneProperty(envADPCM_, refAdpcm->getEnvelopeNumber());
			cloneAdpcm->setEnvelopeNumber(envNum);
			envADPCM_[static_cast<size_t>(envNum)]->registerUserInstrument(cloneInstNum);
		}
		if (refAdpcm->getArpeggioEnabled()) {
			cloneAdpcm->setArpeggioEnabled(true);
			int arpNum = cloneProperty(arpADPCM_, refAdpcm->getArpeggioNumber());
			cloneAdpcm->setArpeggioNumber(arpNum);
			arpADPCM_[static_cast<size_t>(arpNum)]->registerUserInstrument(cloneInstNum);
		}
		if (refAdpcm->getPitchEnabled()) {
			cloneAdpcm->setPitchEnabled(true);
			int ptNum = cloneProperty(ptADPCM_, refAdpcm->getPitchNumber());
			cloneAdpcm->setPitchNumber(ptNum);
			ptADPCM_[static_cast<size_t>(ptNum)]->registerUserInstrument(cloneInstNum);
		}
		if (refAdpcm->getPanEnabled()) {
			cloneAdpcm->setPanEnabled(true);
			int panNum = cloneProperty(panADPCM_, refAdpcm->getPanNumber());
			cloneAdpcm->setPanNumber(panNum);
			panADPCM_[static_cast<size_t>(panNum)]->registerUserInstrument(cloneInstNum);
		}
		break;
	}
	case InstrumentType::Drumkit:
	{
		auto refKit = std::dynamic_pointer_cast<InstrumentDrumkit>(refInst);
		auto cloneKit = std::dynamic_pointer_cast<InstrumentDrumkit>(insts_.at(static_cast<size_t>(cloneInstNum)));
		std::unordered_map<int, int> sampCloneMap;
		for (const int& key : refKit->getAssignedKeys()) {
			int srcNum = refKit->getSampleNumber(key);
			if (!sampCloneMap.count(srcNum)) sampCloneMap[srcNum] = cloneProperty(sampADPCM_, srcNum);
			cloneKit->setSampleNumber(key, sampCloneMap[srcNum]);
			sampADPCM_[static_cast<size_t>(sampCloneMap[srcNum])]->registerUserInstrument(cloneInstNum);

			setInstrumentDrumkitPitch(cloneInstNum, key, refKit->getPitch(key));
			setInstrumentDrumkitPan(cloneInstNum, key, refKit->getPan(key));
		}
		break;
	}
	default:
		throw std::invalid_argument("invalid instrument type");
	}
}

void InstrumentsManager::swapInstruments(int inst1Num, int inst2Num)
{
	std::unique_ptr<AbstractInstrument> inst1 = removeInstrument(inst1Num);
	std::unique_ptr<AbstractInstrument> inst2 = removeInstrument(inst2Num);
	inst1->setNumber(inst2Num);
	inst2->setNumber(inst1Num);
	addInstrument(inst1.release());
	addInstrument(inst2.release());
}

/// [Return] instrument shared pointer or nullptr
std::shared_ptr<AbstractInstrument> InstrumentsManager::getInstrumentSharedPtr(int instNum)
{
	if (0 <= instNum && instNum < static_cast<int>(insts_.size()))
		return insts_[static_cast<size_t>(instNum)];
	else
		return std::shared_ptr<AbstractInstrument>();
}

void InstrumentsManager::clearAll()
{
	for (auto p : ENV_FM_PARAMS) {
		opSeqFM_.emplace(p, std::array<std::shared_ptr<InstrumentSequenceProperty<FMOperatorSequenceUnit>>, 128>());
	}

	for (size_t i = 0; i < 128; ++i) {
		insts_[i].reset();

		envFM_[i] = makeEnvelopeFMSharedPtr(i);
		lfoFM_[i] = makeLFOFMSharedPtr(i);
		for (auto& p : opSeqFM_) p.second[i] = makeOperatorSequenceFMSharedPtr(i);
		arpFM_[i] = makeArpeggioSharedPtr(i);
		ptFM_[i] = makePitchSharedPtr(i);
		panFM_[i] = makePanSharedPtr(i);

		wfSSG_[i] = makeWaveformSSGSharedPtr(i);
		tnSSG_[i] = makeToneNoiseSSGSharedPtr(i);
		envSSG_[i] = makeEnvelopeSSGSharedPtr(i);
		arpSSG_[i] = makeArpeggioSharedPtr(i);
		ptSSG_[i] = makePitchSharedPtr(i);

		sampADPCM_[i] = makeSampleADPCMSharedPtr(i);
		envADPCM_[i] = makeEnvelopeADPCMSharedPtr(i);
		arpADPCM_[i] = makeArpeggioSharedPtr(i);
		ptADPCM_[i] = makePitchSharedPtr(i);
		panADPCM_[i] = makePanSharedPtr(i);
	}
}

std::vector<int> InstrumentsManager::getInstrumentIndices() const
{
	return utils::findIndicesIf(insts_, [](decltype(insts_)::const_reference inst) { return inst; });
}

void InstrumentsManager::setInstrumentName(int instNum, const std::string& name)
{
	insts_.at(static_cast<size_t>(instNum))->setName(name);
}

std::string InstrumentsManager::getInstrumentName(int instNum) const
{
	return insts_.at(static_cast<size_t>(instNum))->getName();
}

std::vector<std::string> InstrumentsManager::getInstrumentNameList() const
{
	using InstRef = decltype(insts_)::const_reference;
	return utils::transformIf(insts_, [](InstRef inst) { return inst; },
	[](InstRef inst) { return inst->getName(); });
}

void InstrumentsManager::clearUnusedInstrumentProperties()
{
	for (size_t i = 0; i < 128; ++i) {
		if (!envFM_[i]->isUserInstrument())
			envFM_[i] = makeEnvelopeFMSharedPtr(i);
		if (!lfoFM_[i]->isUserInstrument())
			lfoFM_[i] = makeLFOFMSharedPtr(i);
		for (auto& p : opSeqFM_) {
			if (!p.second[i]->isUserInstrument())
				p.second[i] = makeOperatorSequenceFMSharedPtr(i);
		}
		if (!arpFM_[i]->isUserInstrument())
			arpFM_[i] = makeArpeggioSharedPtr(i);
		if (!ptFM_[i]->isUserInstrument())
			ptFM_[i] = makePitchSharedPtr(i);
		if (!panFM_[i]->isUserInstrument())
			panFM_[i] = makePanSharedPtr(i);

		if (!wfSSG_[i]->isUserInstrument())
			wfSSG_[i] = makeWaveformSSGSharedPtr(i);
		if (!tnSSG_[i]->isUserInstrument())
			tnSSG_[i] = makeToneNoiseSSGSharedPtr(i);
		if (!envSSG_[i]->isUserInstrument())
			envSSG_[i] = makeEnvelopeSSGSharedPtr(i);
		if (!arpSSG_[i]->isUserInstrument())
			arpSSG_[i] = makeArpeggioSharedPtr(i);
		if (!ptSSG_[i]->isUserInstrument())
			ptSSG_[i] = makePitchSharedPtr(i);

		if (!sampADPCM_[i]->isUserInstrument())
			sampADPCM_[i] = makeSampleADPCMSharedPtr(i);
		if (!envADPCM_[i]->isUserInstrument())
			envADPCM_[i] = makeEnvelopeADPCMSharedPtr(i);
		if (!arpADPCM_[i]->isUserInstrument())
			arpADPCM_[i] = makeArpeggioSharedPtr(i);
		if (!ptADPCM_[i]->isUserInstrument())
			ptADPCM_[i] = makePitchSharedPtr(i);
		if (!panADPCM_[i]->isUserInstrument())
			panADPCM_[i] = makePanSharedPtr(i);
	}
}

/// [Return]
///		-1: no free instrument
///		else: first free instrument number
int InstrumentsManager::findFirstFreeInstrument() const
{
	auto&& it = std::find_if_not(insts_.cbegin(), insts_.cend(),
								 [](const std::shared_ptr<AbstractInstrument>& inst) { return inst; });
	return (it == insts_.cend() ? -1 : std::distance(insts_.cbegin(), it));
}

std::unordered_map<int, int> InstrumentsManager::getDuplicateInstrumentMap() const
{
	std::unordered_map<int, int> dupMap;
	std::vector<int> idcs = getInstrumentIndices();
	static const std::unordered_map<InstrumentType,
			bool (InstrumentsManager::*)(std::shared_ptr<AbstractInstrument>,
										 std::shared_ptr<AbstractInstrument>) const> eqCheck = {
	{ InstrumentType::FM, &InstrumentsManager::equalPropertiesFM },
	{ InstrumentType::SSG, &InstrumentsManager::equalPropertiesSSG },
	{ InstrumentType::ADPCM, &InstrumentsManager::equalPropertiesADPCM },
	{ InstrumentType::Drumkit, &InstrumentsManager::equalPropertiesDrumkit }
};

	for (size_t i = 0; i < idcs.size(); ++i) {
		int baseIdx = idcs[i];
		std::shared_ptr<AbstractInstrument> base = insts_[baseIdx];

		for (size_t j = i + 1; j < idcs.size();) {
			int tgtIdx = idcs[j];
			std::shared_ptr<AbstractInstrument> tgt = insts_[tgtIdx];
			if (base->getType() == tgt->getType() && (this->*eqCheck.at(base->getType()))(base, tgt)) {
				dupMap[tgtIdx] = baseIdx;
				idcs.erase(idcs.begin() + j);
				continue;
			}
			++j;
		}
	}

	return dupMap;
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
	return std::dynamic_pointer_cast<InstrumentFM>(insts_.at(static_cast<size_t>(instNum)))->getEnvelopeNumber();
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

std::multiset<int> InstrumentsManager::getEnvelopeFMUsers(int envNum) const
{
	return envFM_.at(static_cast<size_t>(envNum))->getUserInstruments();
}

std::vector<int> InstrumentsManager::getEnvelopeFMEntriedIndices() const
{
	return utils::findIndicesIf(envFM_, IsEdited<decltype(envFM_)>());
}

int InstrumentsManager::findFirstAssignableEnvelopeFM() const
{
	return findFirstAssignableProperty(envFM_, regardingUnedited_);
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
	return std::dynamic_pointer_cast<InstrumentFM>(insts_.at(static_cast<size_t>(instNum)))->getLFONumber();
}

void InstrumentsManager::setLFOFMParameter(int lfoNum, FMLFOParameter param, int value)
{
	lfoFM_.at(static_cast<size_t>(lfoNum))->setParameterValue(param, value);
}

int InstrumentsManager::getLFOFMparameter(int lfoNum, FMLFOParameter param) const
{
	return lfoFM_.at(static_cast<size_t>(lfoNum))->getParameterValue(param);
}

std::multiset<int> InstrumentsManager::getLFOFMUsers(int lfoNum) const
{
	return lfoFM_.at(static_cast<size_t>(lfoNum))->getUserInstruments();
}

std::vector<int> InstrumentsManager::getLFOFMEntriedIndices() const
{
	return utils::findIndicesIf(lfoFM_, IsEdited<decltype(lfoFM_)>());
}

int InstrumentsManager::findFirstAssignableLFOFM() const
{
	return findFirstAssignableProperty(lfoFM_, regardingUnedited_);
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
	return std::dynamic_pointer_cast<InstrumentFM>(insts_.at(static_cast<size_t>(instNum)))->getOperatorSequenceNumber(param);
}

void InstrumentsManager::addOperatorSequenceFMSequenceData(FMEnvelopeParameter param, int opSeqNum, int data)
{
	opSeqFM_.at(param).at(static_cast<size_t>(opSeqNum))->addSequenceUnit(FMOperatorSequenceUnit(data));
}

void InstrumentsManager::removeOperatorSequenceFMSequenceData(FMEnvelopeParameter param, int opSeqNum)
{
	opSeqFM_.at(param).at(static_cast<size_t>(opSeqNum))->removeSequenceUnit();
}

void InstrumentsManager::setOperatorSequenceFMSequenceData(FMEnvelopeParameter param, int opSeqNum, int cnt, int data)
{
	opSeqFM_.at(param).at(static_cast<size_t>(opSeqNum))->setSequenceUnit(cnt, FMOperatorSequenceUnit(data));
}

std::vector<FMOperatorSequenceUnit> InstrumentsManager::getOperatorSequenceFMSequence(FMEnvelopeParameter param, int opSeqNum)
{
	return opSeqFM_.at(param).at(static_cast<size_t>(opSeqNum))->getSequence();
}

void InstrumentsManager::addOperatorSequenceFMLoop(FMEnvelopeParameter param, int opSeqNum, const InstrumentSequenceLoop& loop)
{
	opSeqFM_.at(param).at(static_cast<size_t>(opSeqNum))->addLoop(loop);
}

void InstrumentsManager::removeOperatorSequenceFMLoop(FMEnvelopeParameter param, int opSeqNum, int begin, int end)
{
	opSeqFM_.at(param).at(static_cast<size_t>(opSeqNum))->removeLoop(begin, end);
}

void InstrumentsManager::changeOperatorSequenceFMLoop(FMEnvelopeParameter param, int opSeqNum, int prevBegin, int prevEnd, const InstrumentSequenceLoop& loop)
{
	opSeqFM_.at(param).at(static_cast<size_t>(opSeqNum))->changeLoop(prevBegin, prevEnd, loop);
}

void InstrumentsManager::clearOperatorSequenceFMLoops(FMEnvelopeParameter param, int opSeqNum)
{
	opSeqFM_.at(param).at(static_cast<size_t>(opSeqNum))->clearLoops();
}

InstrumentSequenceLoopRoot InstrumentsManager::getOperatorSequenceFMLoopRoot(FMEnvelopeParameter param, int opSeqNum) const
{
	return opSeqFM_.at(param).at(static_cast<size_t>(opSeqNum))->getLoopRoot();
}

void InstrumentsManager::setOperatorSequenceFMRelease(FMEnvelopeParameter param, int opSeqNum, const InstrumentSequenceRelease& release)
{
	opSeqFM_.at(param).at(static_cast<size_t>(opSeqNum))->setRelease(release);
}

InstrumentSequenceRelease InstrumentsManager::getOperatorSequenceFMRelease(FMEnvelopeParameter param, int opSeqNum) const
{
	return opSeqFM_.at(param).at(static_cast<size_t>(opSeqNum))->getRelease();
}

FMOperatorSequenceIter InstrumentsManager::getOperatorSequenceFMIterator(FMEnvelopeParameter param, int opSeqNum) const
{
	return opSeqFM_.at(param).at(static_cast<size_t>(opSeqNum))->getIterator();
}

std::multiset<int> InstrumentsManager::getOperatorSequenceFMUsers(FMEnvelopeParameter param, int opSeqNum) const
{
	return opSeqFM_.at(param).at(static_cast<size_t>(opSeqNum))->getUserInstruments();
}

std::vector<int> InstrumentsManager::getOperatorSequenceFMEntriedIndices(FMEnvelopeParameter param) const
{
	return utils::findIndicesIf(opSeqFM_.at(param), IsUsedOrEdited<decltype(opSeqFM_)::mapped_type>());
}

int InstrumentsManager::findFirstAssignableOperatorSequenceFM(FMEnvelopeParameter param) const
{
	return findFirstAssignableProperty(opSeqFM_.at(param), regardingUnedited_);
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
	return std::dynamic_pointer_cast<InstrumentFM>(insts_.at(static_cast<size_t>(instNum)))->getArpeggioNumber(op);
}

void InstrumentsManager::setArpeggioFMType(int arpNum, SequenceType type)
{
	arpFM_.at(static_cast<size_t>(arpNum))->setType(type);
}

SequenceType InstrumentsManager::getArpeggioFMType(int arpNum) const
{
	return arpFM_.at(static_cast<size_t>(arpNum))->getType();
}

void InstrumentsManager::addArpeggioFMSequenceData(int arpNum, int data)
{
	arpFM_.at(static_cast<size_t>(arpNum))->addSequenceUnit(ArpeggioUnit(data));
}

void InstrumentsManager::removeArpeggioFMSequenceData(int arpNum)
{
	arpFM_.at(static_cast<size_t>(arpNum))->removeSequenceUnit();
}

void InstrumentsManager::setArpeggioFMSequenceData(int arpNum, int cnt, int data)
{
	arpFM_.at(static_cast<size_t>(arpNum))->setSequenceUnit(cnt, ArpeggioUnit(data));
}

std::vector<ArpeggioUnit> InstrumentsManager::getArpeggioFMSequence(int arpNum)
{
	return arpFM_.at(static_cast<size_t>(arpNum))->getSequence();
}

void InstrumentsManager::addArpeggioFMLoop(int arpNum, const InstrumentSequenceLoop& loop)
{
	arpFM_.at(static_cast<size_t>(arpNum))->addLoop(loop);
}

void InstrumentsManager::removeArpeggioFMLoop(int arpNum, int begin, int end)
{
	arpFM_.at(static_cast<size_t>(arpNum))->removeLoop(begin, end);
}

void InstrumentsManager::changeArpeggioFMLoop(int arpNum, int prevBegin, int prevEnd, const InstrumentSequenceLoop& loop)
{
	arpFM_.at(static_cast<size_t>(arpNum))->changeLoop(prevBegin, prevEnd, loop);
}

void InstrumentsManager::clearArpeggioFMLoops(int arpNum)
{
	arpFM_.at(static_cast<size_t>(arpNum))->clearLoops();
}

InstrumentSequenceLoopRoot InstrumentsManager::getArpeggioFMLoopRoot(int arpNum) const
{
	return arpFM_.at(static_cast<size_t>(arpNum))->getLoopRoot();
}

void InstrumentsManager::setArpeggioFMRelease(int arpNum, const InstrumentSequenceRelease& release)
{
	arpFM_.at(static_cast<size_t>(arpNum))->setRelease(release);
}

InstrumentSequenceRelease InstrumentsManager::getArpeggioFMRelease(int arpNum) const
{
	return arpFM_.at(static_cast<size_t>(arpNum))->getRelease();
}

ArpeggioIter InstrumentsManager::getArpeggioFMIterator(int arpNum) const
{
	return arpFM_.at(static_cast<size_t>(arpNum))->getIterator();
}

std::multiset<int> InstrumentsManager::getArpeggioFMUsers(int arpNum) const
{
	return arpFM_.at(static_cast<size_t>(arpNum))->getUserInstruments();
}

std::vector<int> InstrumentsManager::getArpeggioFMEntriedIndices() const
{
	return utils::findIndicesIf(arpFM_, IsUsedOrEdited<decltype(arpFM_)>());
}

int InstrumentsManager::findFirstAssignableArpeggioFM() const
{
	return findFirstAssignableProperty(arpFM_, regardingUnedited_);
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
	return std::dynamic_pointer_cast<InstrumentFM>(insts_.at(static_cast<size_t>(instNum)))->getPitchNumber(op);
}

void InstrumentsManager::setPitchFMType(int ptNum, SequenceType type)
{
	ptFM_.at(static_cast<size_t>(ptNum))->setType(type);
}

SequenceType InstrumentsManager::getPitchFMType(int ptNum) const
{
	return ptFM_.at(static_cast<size_t>(ptNum))->getType();
}

void InstrumentsManager::addPitchFMSequenceData(int ptNum, int data)
{
	ptFM_.at(static_cast<size_t>(ptNum))->addSequenceUnit(PitchUnit(data));
}

void InstrumentsManager::removePitchFMSequenceData(int ptNum)
{
	ptFM_.at(static_cast<size_t>(ptNum))->removeSequenceUnit();
}

void InstrumentsManager::setPitchFMSequenceData(int ptNum, int cnt, int data)
{
	ptFM_.at(static_cast<size_t>(ptNum))->setSequenceUnit(cnt, PitchUnit(data));
}

std::vector<PitchUnit> InstrumentsManager::getPitchFMSequence(int ptNum)
{
	return ptFM_.at(static_cast<size_t>(ptNum))->getSequence();
}

void InstrumentsManager::addPitchFMLoop(int ptNum, const InstrumentSequenceLoop& loop)
{
	ptFM_.at(static_cast<size_t>(ptNum))->addLoop(loop);
}

void InstrumentsManager::removePitchFMLoop(int ptNum, int begin, int end)
{
	ptFM_.at(static_cast<size_t>(ptNum))->removeLoop(begin, end);
}

void InstrumentsManager::changePitchFMLoop(int ptNum, int prevBegin, int prevEnd, const InstrumentSequenceLoop& loop)
{
	ptFM_.at(static_cast<size_t>(ptNum))->changeLoop(prevBegin, prevEnd, loop);
}

void InstrumentsManager::clearPitchFMLoops(int ptNum)
{
	ptFM_.at(static_cast<size_t>(ptNum))->clearLoops();
}

InstrumentSequenceLoopRoot InstrumentsManager::getPitchFMLoopRoot(int ptNum) const
{
	return ptFM_.at(static_cast<size_t>(ptNum))->getLoopRoot();
}

void InstrumentsManager::setPitchFMRelease(int ptNum, const InstrumentSequenceRelease& release)
{
	ptFM_.at(static_cast<size_t>(ptNum))->setRelease(release);
}

InstrumentSequenceRelease InstrumentsManager::getPitchFMRelease(int ptNum) const
{
	return ptFM_.at(static_cast<size_t>(ptNum))->getRelease();
}

PitchIter InstrumentsManager::getPitchFMIterator(int ptNum) const
{
	return ptFM_.at(static_cast<size_t>(ptNum))->getIterator();
}

std::multiset<int> InstrumentsManager::getPitchFMUsers(int ptNum) const
{
	return ptFM_.at(static_cast<size_t>(ptNum))->getUserInstruments();
}

std::vector<int> InstrumentsManager::getPitchFMEntriedIndices() const
{
	return utils::findIndicesIf(ptFM_, IsUsedOrEdited<decltype(ptFM_)>());
}

int InstrumentsManager::findFirstAssignablePitchFM() const
{
	return findFirstAssignableProperty(ptFM_, regardingUnedited_);
}

void InstrumentsManager::setInstrumentFMPanEnabled(int instNum, bool enabled)
{
	auto fm = std::dynamic_pointer_cast<InstrumentFM>(insts_.at(static_cast<size_t>(instNum)));
	fm->setPanEnabled(enabled);
	if (enabled)
		panFM_.at(static_cast<size_t>(fm->getPanNumber()))->registerUserInstrument(instNum);
	else
		panFM_.at(static_cast<size_t>(fm->getPanNumber()))->deregisterUserInstrument(instNum);
}

bool InstrumentsManager::getInstrumentFMPanEnabled(int instNum) const
{
	return std::dynamic_pointer_cast<InstrumentFM>(insts_.at(static_cast<size_t>(instNum)))->getPanEnabled();
}

void InstrumentsManager::setInstrumentFMPan(int instNum, int panNum)
{
	auto fm = std::dynamic_pointer_cast<InstrumentFM>(insts_.at(static_cast<size_t>(instNum)));
	if (fm->getPanEnabled()) {
		panFM_.at(static_cast<size_t>(fm->getPanNumber()))->deregisterUserInstrument(instNum);
		panFM_.at(static_cast<size_t>(panNum))->registerUserInstrument(instNum);
	}
	fm->setPanNumber(panNum);
}

int InstrumentsManager::getInstrumentFMPan(int instNum)
{
	return std::dynamic_pointer_cast<InstrumentFM>(insts_.at(static_cast<size_t>(instNum)))->getPanNumber();
}

void InstrumentsManager::addPanFMSequenceData(int panNum, int data)
{
	panFM_.at(static_cast<size_t>(panNum))->addSequenceUnit(PanUnit(data));
}

void InstrumentsManager::removePanFMSequenceData(int panNum)
{
	panFM_.at(static_cast<size_t>(panNum))->removeSequenceUnit();
}

void InstrumentsManager::setPanFMSequenceData(int panNum, int cnt, int data)
{
	panFM_.at(static_cast<size_t>(panNum))->setSequenceUnit(cnt, PanUnit(data));
}

std::vector<PanUnit> InstrumentsManager::getPanFMSequence(int panNum)
{
	return panFM_.at(static_cast<size_t>(panNum))->getSequence();
}

void InstrumentsManager::addPanFMLoop(int panNum, const InstrumentSequenceLoop& loop)
{
	panFM_.at(static_cast<size_t>(panNum))->addLoop(loop);
}

void InstrumentsManager::removePanFMLoop(int panNum, int begin, int end)
{
	panFM_.at(static_cast<size_t>(panNum))->removeLoop(begin, end);
}

void InstrumentsManager::changePanFMLoop(int panNum, int prevBegin, int prevEnd, const InstrumentSequenceLoop& loop)
{
	panFM_.at(static_cast<size_t>(panNum))->changeLoop(prevBegin, prevEnd, loop);
}

void InstrumentsManager::clearPanFMLoops(int panNum)
{
	panFM_.at(static_cast<size_t>(panNum))->clearLoops();
}

InstrumentSequenceLoopRoot InstrumentsManager::getPanFMLoopRoot(int panNum) const
{
	return panFM_.at(static_cast<size_t>(panNum))->getLoopRoot();
}

void InstrumentsManager::setPanFMRelease(int panNum, const InstrumentSequenceRelease& release)
{
	panFM_.at(static_cast<size_t>(panNum))->setRelease(release);
}

InstrumentSequenceRelease InstrumentsManager::getPanFMRelease(int panNum) const
{
	return panFM_.at(static_cast<size_t>(panNum))->getRelease();
}

PanIter InstrumentsManager::getPanFMIterator(int panNum) const
{
	return panFM_.at(static_cast<size_t>(panNum))->getIterator();
}

std::multiset<int> InstrumentsManager::getPanFMUsers(int panNum) const
{
	return panFM_.at(static_cast<size_t>(panNum))->getUserInstruments();
}

std::vector<int> InstrumentsManager::getPanFMEntriedIndices() const
{
	return utils::findIndicesIf(panFM_, IsUsedOrEdited<decltype(panFM_)>());
}

int InstrumentsManager::findFirstAssignablePanFM() const
{
	return findFirstAssignableProperty(panFM_, regardingUnedited_);
}

void InstrumentsManager::setInstrumentFMEnvelopeResetEnabled(int instNum, FMOperatorType op, bool enabled)
{
	std::dynamic_pointer_cast<InstrumentFM>(insts_.at(static_cast<size_t>(instNum)))->setEnvelopeResetEnabled(op, enabled);
}

bool InstrumentsManager::equalPropertiesFM(std::shared_ptr<AbstractInstrument> a, std::shared_ptr<AbstractInstrument> b) const
{
	auto aFm = std::dynamic_pointer_cast<InstrumentFM>(a);
	auto bFm = std::dynamic_pointer_cast<InstrumentFM>(b);

	if (*envFM_[aFm->getEnvelopeNumber()] != *envFM_[bFm->getEnvelopeNumber()])
		return false;
	if (aFm->getLFOEnabled() != bFm->getLFOEnabled())
		return false;
	if (aFm->getLFOEnabled() && *lfoFM_[aFm->getLFONumber()] != *lfoFM_[bFm->getLFONumber()])
		return false;
	for (auto& pair : opSeqFM_) {
		if (aFm->getOperatorSequenceEnabled(pair.first) != bFm->getOperatorSequenceEnabled(pair.first))
			return false;
		if (aFm->getOperatorSequenceEnabled(pair.first)
				&& *pair.second[aFm->getOperatorSequenceNumber(pair.first)] != *pair.second[bFm->getOperatorSequenceNumber(pair.first)])
			return false;
	}
	for (auto& type : FM_OP_TYPES) {
		if (aFm->getArpeggioEnabled(type) != bFm->getArpeggioEnabled(type))
			return false;
		if (aFm->getArpeggioEnabled(type)
				&& *arpFM_[aFm->getArpeggioNumber(type)] != *arpFM_[bFm->getArpeggioNumber(type)])
			return false;
		if (aFm->getPitchEnabled(type) != bFm->getPitchEnabled(type))
			return false;
		if (aFm->getPitchEnabled(type)
				&& *ptFM_[aFm->getPitchNumber(type)] != *ptFM_[bFm->getPitchNumber(type)])
			return false;
		if (aFm->getEnvelopeResetEnabled(type) != bFm->getEnvelopeResetEnabled(type))
			return false;
	}
	if (aFm->getPanEnabled()
			&& *panFM_[aFm->getPanNumber()] != *panFM_[bFm->getPanNumber()])
		return false;
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
	return std::dynamic_pointer_cast<InstrumentSSG>(insts_.at(static_cast<size_t>(instNum)))->getWaveformNumber();
}

void InstrumentsManager::addWaveformSSGSequenceData(int wfNum, const SSGWaveformUnit& data)
{
	wfSSG_.at(static_cast<size_t>(wfNum))->addSequenceUnit(data);
}

void InstrumentsManager::removeWaveformSSGSequenceData(int wfNum)
{
	wfSSG_.at(static_cast<size_t>(wfNum))->removeSequenceUnit();
}

void InstrumentsManager::setWaveformSSGSequenceData(int wfNum, int cnt, const SSGWaveformUnit& data)
{
	wfSSG_.at(static_cast<size_t>(wfNum))->setSequenceUnit(cnt, data);
}

std::vector<SSGWaveformUnit> InstrumentsManager::getWaveformSSGSequence(int wfNum)
{
	return wfSSG_.at(static_cast<size_t>(wfNum))->getSequence();
}

void InstrumentsManager::addWaveformSSGLoop(int wfNum, const InstrumentSequenceLoop& loop)
{
	wfSSG_.at(static_cast<size_t>(wfNum))->addLoop(loop);
}

void InstrumentsManager::removeWaveformSSGLoop(int wfNum, int begin, int end)
{
	wfSSG_.at(static_cast<size_t>(wfNum))->removeLoop(begin, end);
}

void InstrumentsManager::changeWaveformSSGLoop(int wfNum, int prevBegin, int prevEnd, const InstrumentSequenceLoop& loop)
{
	wfSSG_.at(static_cast<size_t>(wfNum))->changeLoop(prevBegin, prevEnd, loop);
}

void InstrumentsManager::clearWaveformSSGLoops(int wfNum)
{
	wfSSG_.at(static_cast<size_t>(wfNum))->clearLoops();
}

InstrumentSequenceLoopRoot InstrumentsManager::getWaveformSSGLoopRoot(int wfNum) const
{
	return wfSSG_.at(static_cast<size_t>(wfNum))->getLoopRoot();
}

void InstrumentsManager::setWaveformSSGRelease(int wfNum, const InstrumentSequenceRelease& release)
{
	wfSSG_.at(static_cast<size_t>(wfNum))->setRelease(release);
}

InstrumentSequenceRelease InstrumentsManager::getWaveformSSGRelease(int wfNum) const
{
	return wfSSG_.at(static_cast<size_t>(wfNum))->getRelease();
}

SSGWaveformIter InstrumentsManager::getWaveformSSGIterator(int wfNum) const
{
	return wfSSG_.at(static_cast<size_t>(wfNum))->getIterator();
}

std::multiset<int> InstrumentsManager::getWaveformSSGUsers(int wfNum) const
{
	return wfSSG_.at(static_cast<size_t>(wfNum))->getUserInstruments();
}

std::vector<int> InstrumentsManager::getWaveformSSGEntriedIndices() const
{
	return utils::findIndicesIf(wfSSG_, IsUsedOrEdited<decltype(wfSSG_)>());
}

int InstrumentsManager::findFirstAssignableWaveformSSG() const
{
	return findFirstAssignableProperty(wfSSG_, regardingUnedited_);
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
	return std::dynamic_pointer_cast<InstrumentSSG>(insts_.at(static_cast<size_t>(instNum)))->getToneNoiseNumber();
}

void InstrumentsManager::addToneNoiseSSGSequenceData(int tnNum, int data)
{
	tnSSG_.at(static_cast<size_t>(tnNum))->addSequenceUnit(SSGToneNoiseUnit(data));
}

void InstrumentsManager::removeToneNoiseSSGSequenceData(int tnNum)
{
	tnSSG_.at(static_cast<size_t>(tnNum))->removeSequenceUnit();
}

void InstrumentsManager::setToneNoiseSSGSequenceData(int tnNum, int cnt, int data)
{
	tnSSG_.at(static_cast<size_t>(tnNum))->setSequenceUnit(cnt, SSGToneNoiseUnit(data));
}

std::vector<SSGToneNoiseUnit> InstrumentsManager::getToneNoiseSSGSequence(int tnNum)
{
	return tnSSG_.at(static_cast<size_t>(tnNum))->getSequence();
}

void InstrumentsManager::addToneNoiseSSGLoop(int tnNum, const InstrumentSequenceLoop& loop)
{
	tnSSG_.at(static_cast<size_t>(tnNum))->addLoop(loop);
}

void InstrumentsManager::removeToneNoiseSSGLoop(int tnNum, int begin, int end)
{
	tnSSG_.at(static_cast<size_t>(tnNum))->removeLoop(begin, end);
}

void InstrumentsManager::changeToneNoiseSSGLoop(int tnNum, int prevBegin, int prevEnd, const InstrumentSequenceLoop& loop)
{
	tnSSG_.at(static_cast<size_t>(tnNum))->changeLoop(prevBegin, prevEnd, loop);
}

void InstrumentsManager::clearToneNoiseSSGLoops(int tnNum)
{
	tnSSG_.at(static_cast<size_t>(tnNum))->clearLoops();
}

InstrumentSequenceLoopRoot InstrumentsManager::getToneNoiseSSGLoopRoot(int tnNum) const
{
	return tnSSG_.at(static_cast<size_t>(tnNum))->getLoopRoot();
}

void InstrumentsManager::setToneNoiseSSGRelease(int tnNum, const InstrumentSequenceRelease& release)
{
	tnSSG_.at(static_cast<size_t>(tnNum))->setRelease(release);
}

InstrumentSequenceRelease InstrumentsManager::getToneNoiseSSGRelease(int tnNum) const
{
	return tnSSG_.at(static_cast<size_t>(tnNum))->getRelease();
}

SSGToneNoiseIter InstrumentsManager::getToneNoiseSSGIterator(int tnNum) const
{
	return tnSSG_.at(static_cast<size_t>(tnNum))->getIterator();
}

std::multiset<int> InstrumentsManager::getToneNoiseSSGUsers(int tnNum) const
{
	return tnSSG_.at(static_cast<size_t>(tnNum))->getUserInstruments();
}

std::vector<int> InstrumentsManager::getToneNoiseSSGEntriedIndices() const
{
	return utils::findIndicesIf(tnSSG_, IsUsedOrEdited<decltype(tnSSG_)>());
}

int InstrumentsManager::findFirstAssignableToneNoiseSSG() const
{
	return findFirstAssignableProperty(tnSSG_, regardingUnedited_);
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
	return std::dynamic_pointer_cast<InstrumentSSG>(insts_.at(static_cast<size_t>(instNum)))->getEnvelopeNumber();
}

void InstrumentsManager::addEnvelopeSSGSequenceData(int envNum, const SSGEnvelopeUnit& data)
{
	envSSG_.at(static_cast<size_t>(envNum))->addSequenceUnit(data);
}

void InstrumentsManager::removeEnvelopeSSGSequenceData(int envNum)
{
	envSSG_.at(static_cast<size_t>(envNum))->removeSequenceUnit();
}

void InstrumentsManager::setEnvelopeSSGSequenceData(int envNum, int cnt, const SSGEnvelopeUnit& data)
{
	envSSG_.at(static_cast<size_t>(envNum))->setSequenceUnit(cnt, data);
}

std::vector<SSGEnvelopeUnit> InstrumentsManager::getEnvelopeSSGSequence(int envNum)
{
	return envSSG_.at(static_cast<size_t>(envNum))->getSequence();
}

void InstrumentsManager::addEnvelopeSSGLoop(int envNum, const InstrumentSequenceLoop& loop)
{
	envSSG_.at(static_cast<size_t>(envNum))->addLoop(loop);
}

void InstrumentsManager::removeEnvelopeSSGLoop(int envNum, int begin, int end)
{
	envSSG_.at(static_cast<size_t>(envNum))->removeLoop(begin, end);
}

void InstrumentsManager::changeEnvelopeSSGLoop(int envNum, int prevBegin, int prevEnd, const InstrumentSequenceLoop& loop)
{
	envSSG_.at(static_cast<size_t>(envNum))->changeLoop(prevBegin, prevEnd, loop);
}

void InstrumentsManager::clearEnvelopeSSGLoops(int envNum)
{
	envSSG_.at(static_cast<size_t>(envNum))->clearLoops();
}

InstrumentSequenceLoopRoot InstrumentsManager::getEnvelopeSSGLoopRoot(int envNum) const
{
	return envSSG_.at(static_cast<size_t>(envNum))->getLoopRoot();
}

void InstrumentsManager::setEnvelopeSSGRelease(int envNum, const InstrumentSequenceRelease& release)
{
	envSSG_.at(static_cast<size_t>(envNum))->setRelease(release);
}

InstrumentSequenceRelease InstrumentsManager::getEnvelopeSSGRelease(int envNum) const
{
	return envSSG_.at(static_cast<size_t>(envNum))->getRelease();
}

SSGEnvelopeIter InstrumentsManager::getEnvelopeSSGIterator(int envNum) const
{
	return envSSG_.at(static_cast<size_t>(envNum))->getIterator();
}

std::multiset<int> InstrumentsManager::getEnvelopeSSGUsers(int envNum) const
{
	return envSSG_.at(static_cast<size_t>(envNum))->getUserInstruments();
}

std::vector<int> InstrumentsManager::getEnvelopeSSGEntriedIndices() const
{
	return utils::findIndicesIf(envSSG_, IsUsedOrEdited<decltype(envSSG_)>());
}

int InstrumentsManager::findFirstAssignableEnvelopeSSG() const
{
	return findFirstAssignableProperty(envSSG_, regardingUnedited_);
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
	return std::dynamic_pointer_cast<InstrumentSSG>(insts_.at(static_cast<size_t>(instNum)))->getArpeggioNumber();
}

void InstrumentsManager::setArpeggioSSGType(int arpNum, SequenceType type)
{
	arpSSG_.at(static_cast<size_t>(arpNum))->setType(type);
}

SequenceType InstrumentsManager::getArpeggioSSGType(int arpNum) const
{
	return arpSSG_.at(static_cast<size_t>(arpNum))->getType();
}

void InstrumentsManager::addArpeggioSSGSequenceData(int arpNum, int data)
{
	arpSSG_.at(static_cast<size_t>(arpNum))->addSequenceUnit(ArpeggioUnit(data));
}

void InstrumentsManager::removeArpeggioSSGSequenceData(int arpNum)
{
	arpSSG_.at(static_cast<size_t>(arpNum))->removeSequenceUnit();
}

void InstrumentsManager::setArpeggioSSGSequenceData(int arpNum, int cnt, int data)
{
	arpSSG_.at(static_cast<size_t>(arpNum))->setSequenceUnit(cnt, ArpeggioUnit(data));
}

std::vector<ArpeggioUnit> InstrumentsManager::getArpeggioSSGSequence(int arpNum)
{
	return arpSSG_.at(static_cast<size_t>(arpNum))->getSequence();
}

void InstrumentsManager::addArpeggioSSGLoop(int arpNum, const InstrumentSequenceLoop& loop)
{
	arpSSG_.at(static_cast<size_t>(arpNum))->addLoop(loop);
}

void InstrumentsManager::removeArpeggioSSGLoop(int arpNum, int begin, int end)
{
	arpSSG_.at(static_cast<size_t>(arpNum))->removeLoop(begin, end);
}

void InstrumentsManager::changeArpeggioSSGLoop(int arpNum, int prevBegin, int prevEnd, const InstrumentSequenceLoop& loop)
{
	arpSSG_.at(static_cast<size_t>(arpNum))->changeLoop(prevBegin, prevEnd, loop);
}

void InstrumentsManager::clearArpeggioSSGLoops(int arpNum)
{
	arpSSG_.at(static_cast<size_t>(arpNum))->clearLoops();
}

InstrumentSequenceLoopRoot InstrumentsManager::getArpeggioSSGLoopRoot(int arpNum) const
{
	return arpSSG_.at(static_cast<size_t>(arpNum))->getLoopRoot();
}

void InstrumentsManager::setArpeggioSSGRelease(int arpNum, const InstrumentSequenceRelease& release)
{
	arpSSG_.at(static_cast<size_t>(arpNum))->setRelease(release);
}

InstrumentSequenceRelease InstrumentsManager::getArpeggioSSGRelease(int arpNum) const
{
	return arpSSG_.at(static_cast<size_t>(arpNum))->getRelease();
}

ArpeggioIter InstrumentsManager::getArpeggioSSGIterator(int arpNum) const
{
	return arpSSG_.at(static_cast<size_t>(arpNum))->getIterator();
}

std::multiset<int> InstrumentsManager::getArpeggioSSGUsers(int arpNum) const
{
	return arpSSG_.at(static_cast<size_t>(arpNum))->getUserInstruments();
}

std::vector<int> InstrumentsManager::getArpeggioSSGEntriedIndices() const
{
	return utils::findIndicesIf(arpSSG_, IsUsedOrEdited<decltype(arpSSG_)>());
}

int InstrumentsManager::findFirstAssignableArpeggioSSG() const
{
	return findFirstAssignableProperty(arpSSG_, regardingUnedited_);
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
	return std::dynamic_pointer_cast<InstrumentSSG>(insts_.at(static_cast<size_t>(instNum)))->getPitchNumber();
}

void InstrumentsManager::setPitchSSGType(int ptNum, SequenceType type)
{
	ptSSG_.at(static_cast<size_t>(ptNum))->setType(type);
}

SequenceType InstrumentsManager::getPitchSSGType(int ptNum) const
{
	return ptSSG_.at(static_cast<size_t>(ptNum))->getType();
}

void InstrumentsManager::addPitchSSGSequenceData(int ptNum, int data)
{
	ptSSG_.at(static_cast<size_t>(ptNum))->addSequenceUnit(PitchUnit(data));
}

void InstrumentsManager::removePitchSSGSequenceData(int ptNum)
{
	ptSSG_.at(static_cast<size_t>(ptNum))->removeSequenceUnit();
}

void InstrumentsManager::setPitchSSGSequenceData(int ptNum, int cnt, int data)
{
	ptSSG_.at(static_cast<size_t>(ptNum))->setSequenceUnit(cnt, PitchUnit(data));
}

std::vector<PitchUnit> InstrumentsManager::getPitchSSGSequence(int ptNum)
{
	return ptSSG_.at(static_cast<size_t>(ptNum))->getSequence();
}

void InstrumentsManager::addPitchSSGLoop(int ptNum, const InstrumentSequenceLoop& loop)
{
	ptSSG_.at(static_cast<size_t>(ptNum))->addLoop(loop);
}

void InstrumentsManager::removePitchSSGLoop(int ptNum, int begin, int end)
{
	ptSSG_.at(static_cast<size_t>(ptNum))->removeLoop(begin, end);
}

void InstrumentsManager::changePitchSSGLoop(int ptNum, int prevBegin, int prevEnd, const InstrumentSequenceLoop& loop)
{
	ptSSG_.at(static_cast<size_t>(ptNum))->changeLoop(prevBegin, prevEnd, loop);
}

void InstrumentsManager::clearPitchSSGLoops(int ptNum)
{
	ptSSG_.at(static_cast<size_t>(ptNum))->clearLoops();
}

InstrumentSequenceLoopRoot InstrumentsManager::getPitchSSGLoopRoot(int ptNum) const
{
	return ptSSG_.at(static_cast<size_t>(ptNum))->getLoopRoot();
}

void InstrumentsManager::setPitchSSGRelease(int ptNum, const InstrumentSequenceRelease& release)
{
	ptSSG_.at(static_cast<size_t>(ptNum))->setRelease(release);
}

InstrumentSequenceRelease InstrumentsManager::getPitchSSGRelease(int ptNum) const
{
	return ptSSG_.at(static_cast<size_t>(ptNum))->getRelease();
}

PitchIter InstrumentsManager::getPitchSSGIterator(int ptNum) const
{
	return ptSSG_.at(static_cast<size_t>(ptNum))->getIterator();
}

std::multiset<int> InstrumentsManager::getPitchSSGUsers(int ptNum) const
{
	return ptSSG_.at(static_cast<size_t>(ptNum))->getUserInstruments();
}

std::vector<int> InstrumentsManager::getPitchSSGEntriedIndices() const
{
	return utils::findIndicesIf(ptSSG_, IsUsedOrEdited<decltype(ptSSG_)>());
}

int InstrumentsManager::findFirstAssignablePitchSSG() const
{
	return findFirstAssignableProperty(ptSSG_, regardingUnedited_);
}

bool InstrumentsManager::equalPropertiesSSG(std::shared_ptr<AbstractInstrument> a, std::shared_ptr<AbstractInstrument> b) const
{
	auto aSsg = std::dynamic_pointer_cast<InstrumentSSG>(a);
	auto bSsg = std::dynamic_pointer_cast<InstrumentSSG>(b);

	if (aSsg->getWaveformEnabled() != bSsg->getWaveformEnabled())
		return false;
	if (aSsg->getWaveformEnabled()
			&& *wfSSG_[aSsg->getWaveformNumber()] != *wfSSG_[bSsg->getWaveformNumber()])
		return false;
	if (aSsg->getToneNoiseEnabled() != bSsg->getToneNoiseEnabled())
		return false;
	if (aSsg->getToneNoiseEnabled()
			&& *tnSSG_[aSsg->getToneNoiseNumber()] != *tnSSG_[bSsg->getToneNoiseNumber()])
		return false;
	if (aSsg->getEnvelopeEnabled() != bSsg->getEnvelopeEnabled())
		return false;
	if (aSsg->getEnvelopeEnabled()
			&& *envSSG_[aSsg->getEnvelopeNumber()] != *envSSG_[bSsg->getEnvelopeNumber()])
		return false;
	if (aSsg->getArpeggioEnabled() != bSsg->getArpeggioEnabled())
		return false;
	if (aSsg->getArpeggioEnabled()
			&& *arpSSG_[aSsg->getArpeggioNumber()] != *arpSSG_[bSsg->getArpeggioNumber()])
		return false;
	if (aSsg->getPitchEnabled() != bSsg->getPitchEnabled())
		return false;
	if (aSsg->getPitchEnabled()
			&& *ptSSG_[aSsg->getPitchNumber()] != *ptSSG_[bSsg->getPitchNumber()])
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
	return std::dynamic_pointer_cast<InstrumentADPCM>(insts_.at(static_cast<size_t>(instNum)))->getSampleNumber();
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

bool InstrumentsManager::setSampleADPCMRepeatrange(int sampNum, const SampleRepeatRange& range)
{
	return sampADPCM_.at(static_cast<size_t>(sampNum))->setRepeatRange(range);
}

SampleRepeatFlag InstrumentsManager::getSampleADPCMRepeatFlag(int sampNum) const
{
	return sampADPCM_.at(static_cast<size_t>(sampNum))->getRepeatFlag();
}

SampleRepeatRange InstrumentsManager::getSampleADPCMRepeatRange(int sampNum) const
{
	return sampADPCM_.at(static_cast<size_t>(sampNum))->getRepeatRange();
}

void InstrumentsManager::storeSampleADPCMRawSample(int sampNum, const std::vector<uint8_t>& sample)
{
	sampADPCM_.at(static_cast<size_t>(sampNum))->storeSample(sample);
}

void InstrumentsManager::storeSampleADPCMRawSample(int sampNum, std::vector<uint8_t>&& sample)
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

std::multiset<int> InstrumentsManager::getSampleADPCMUsers(int sampNum) const
{
	return sampADPCM_.at(static_cast<size_t>(sampNum))->getUserInstruments();
}

std::vector<int> InstrumentsManager::getSampleADPCMEntriedIndices() const
{
	return utils::findIndicesIf(sampADPCM_, IsEdited<decltype(sampADPCM_)>());
}

std::vector<int> InstrumentsManager::getSampleADPCMValidIndices() const
{
	return utils::findIndicesIf(sampADPCM_, IsUsedOrEdited<decltype(sampADPCM_)>());
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
	return findFirstAssignableProperty(sampADPCM_, regardingUnedited_, startIndex);
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
	return std::dynamic_pointer_cast<InstrumentADPCM>(insts_.at(static_cast<size_t>(instNum)))->getEnvelopeNumber();
}

void InstrumentsManager::addEnvelopeADPCMSequenceData(int envNum, int data)
{
	envADPCM_.at(static_cast<size_t>(envNum))->addSequenceUnit(ADPCMEnvelopeUnit(data));
}

void InstrumentsManager::removeEnvelopeADPCMSequenceData(int envNum)
{
	envADPCM_.at(static_cast<size_t>(envNum))->removeSequenceUnit();
}

void InstrumentsManager::setEnvelopeADPCMSequenceData(int envNum, int cnt, int data)
{
	envADPCM_.at(static_cast<size_t>(envNum))->setSequenceUnit(cnt, ADPCMEnvelopeUnit(data));
}

std::vector<ADPCMEnvelopeUnit> InstrumentsManager::getEnvelopeADPCMSequence(int envNum)
{
	return envADPCM_.at(static_cast<size_t>(envNum))->getSequence();
}

void InstrumentsManager::addEnvelopeADPCMLoop(int envNum, const InstrumentSequenceLoop& loop)
{
	envADPCM_.at(static_cast<size_t>(envNum))->addLoop(loop);
}

void InstrumentsManager::removeEnvelopeADPCMLoop(int envNum, int begin, int end)
{
	envADPCM_.at(static_cast<size_t>(envNum))->removeLoop(begin, end);
}

void InstrumentsManager::changeEnvelopeADPCMLoop(int envNum, int prevBegin, int prevEnd, const InstrumentSequenceLoop& loop)
{
	envADPCM_.at(static_cast<size_t>(envNum))->changeLoop(prevBegin, prevEnd, loop);
}

void InstrumentsManager::clearEnvelopeADPCMLoops(int envNum)
{
	envADPCM_.at(static_cast<size_t>(envNum))->clearLoops();
}

InstrumentSequenceLoopRoot InstrumentsManager::getEnvelopeADPCMLoopRoot(int envNum) const
{
	return envADPCM_.at(static_cast<size_t>(envNum))->getLoopRoot();
}

void InstrumentsManager::setEnvelopeADPCMRelease(int envNum, const InstrumentSequenceRelease& release)
{
	envADPCM_.at(static_cast<size_t>(envNum))->setRelease(release);
}

InstrumentSequenceRelease InstrumentsManager::getEnvelopeADPCMRelease(int envNum) const
{
	return envADPCM_.at(static_cast<size_t>(envNum))->getRelease();
}

ADPCMEnvelopeIter InstrumentsManager::getEnvelopeADPCMIterator(int envNum) const
{
	return envADPCM_.at(static_cast<size_t>(envNum))->getIterator();
}

std::multiset<int> InstrumentsManager::getEnvelopeADPCMUsers(int envNum) const
{
	return envADPCM_.at(static_cast<size_t>(envNum))->getUserInstruments();
}

std::vector<int> InstrumentsManager::getEnvelopeADPCMEntriedIndices() const
{
	return utils::findIndicesIf(envADPCM_, IsUsedOrEdited<decltype(envADPCM_)>());
}

int InstrumentsManager::findFirstAssignableEnvelopeADPCM() const
{
	return findFirstAssignableProperty(envADPCM_, regardingUnedited_);
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
	return std::dynamic_pointer_cast<InstrumentADPCM>(insts_.at(static_cast<size_t>(instNum)))->getArpeggioNumber();
}

void InstrumentsManager::setArpeggioADPCMType(int arpNum, SequenceType type)
{
	arpADPCM_.at(static_cast<size_t>(arpNum))->setType(type);
}

SequenceType InstrumentsManager::getArpeggioADPCMType(int arpNum) const
{
	return arpADPCM_.at(static_cast<size_t>(arpNum))->getType();
}

void InstrumentsManager::addArpeggioADPCMSequenceData(int arpNum, int data)
{
	arpADPCM_.at(static_cast<size_t>(arpNum))->addSequenceUnit(ArpeggioUnit(data));
}

void InstrumentsManager::removeArpeggioADPCMSequenceData(int arpNum)
{
	arpADPCM_.at(static_cast<size_t>(arpNum))->removeSequenceUnit();
}

void InstrumentsManager::setArpeggioADPCMSequenceData(int arpNum, int cnt, int data)
{
	arpADPCM_.at(static_cast<size_t>(arpNum))->setSequenceUnit(cnt, ArpeggioUnit(data));
}

std::vector<ArpeggioUnit> InstrumentsManager::getArpeggioADPCMSequence(int arpNum)
{
	return arpADPCM_.at(static_cast<size_t>(arpNum))->getSequence();
}

void InstrumentsManager::addArpeggioADPCMLoop(int arpNum, const InstrumentSequenceLoop& loop)
{
	arpADPCM_.at(static_cast<size_t>(arpNum))->addLoop(loop);
}

void InstrumentsManager::removeArpeggioADPCMLoop(int arpNum, int begin, int end)
{
	arpADPCM_.at(static_cast<size_t>(arpNum))->removeLoop(begin, end);
}

void InstrumentsManager::changeArpeggioADPCMLoop(int arpNum, int prevBegin, int prevEnd, const InstrumentSequenceLoop& loop)
{
	arpADPCM_.at(static_cast<size_t>(arpNum))->changeLoop(prevBegin, prevEnd, loop);
}

void InstrumentsManager::clearArpeggioADPCMLoops(int arpNum)
{
	arpADPCM_.at(static_cast<size_t>(arpNum))->clearLoops();
}

InstrumentSequenceLoopRoot InstrumentsManager::getArpeggioADPCMLoopRoot(int arpNum) const
{
	return arpADPCM_.at(static_cast<size_t>(arpNum))->getLoopRoot();
}

void InstrumentsManager::setArpeggioADPCMRelease(int arpNum, const InstrumentSequenceRelease& release)
{
	arpADPCM_.at(static_cast<size_t>(arpNum))->setRelease(release);
}

InstrumentSequenceRelease InstrumentsManager::getArpeggioADPCMRelease(int arpNum) const
{
	return arpADPCM_.at(static_cast<size_t>(arpNum))->getRelease();
}

ArpeggioIter InstrumentsManager::getArpeggioADPCMIterator(int arpNum) const
{
	return arpADPCM_.at(static_cast<size_t>(arpNum))->getIterator();
}

std::multiset<int> InstrumentsManager::getArpeggioADPCMUsers(int arpNum) const
{
	return arpADPCM_.at(static_cast<size_t>(arpNum))->getUserInstruments();
}

std::vector<int> InstrumentsManager::getArpeggioADPCMEntriedIndices() const
{
	return utils::findIndicesIf(arpADPCM_, IsUsedOrEdited<decltype(arpADPCM_)>());
}

int InstrumentsManager::findFirstAssignableArpeggioADPCM() const
{
	return findFirstAssignableProperty(arpADPCM_, regardingUnedited_);
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
	return std::dynamic_pointer_cast<InstrumentADPCM>(insts_.at(static_cast<size_t>(instNum)))->getPitchNumber();
}

void InstrumentsManager::setPitchADPCMType(int ptNum, SequenceType type)
{
	ptADPCM_.at(static_cast<size_t>(ptNum))->setType(type);
}

SequenceType InstrumentsManager::getPitchADPCMType(int ptNum) const
{
	return ptADPCM_.at(static_cast<size_t>(ptNum))->getType();
}

void InstrumentsManager::addPitchADPCMSequenceData(int ptNum, int data)
{
	ptADPCM_.at(static_cast<size_t>(ptNum))->addSequenceUnit(PitchUnit(data));
}

void InstrumentsManager::removePitchADPCMSequenceData(int ptNum)
{
	ptADPCM_.at(static_cast<size_t>(ptNum))->removeSequenceUnit();
}

void InstrumentsManager::setPitchADPCMSequenceData(int ptNum, int cnt, int data)
{
	ptADPCM_.at(static_cast<size_t>(ptNum))->setSequenceUnit(cnt, PitchUnit(data));
}

std::vector<PitchUnit> InstrumentsManager::getPitchADPCMSequence(int ptNum)
{
	return ptADPCM_.at(static_cast<size_t>(ptNum))->getSequence();
}

void InstrumentsManager::addPitchADPCMLoop(int ptNum, const InstrumentSequenceLoop& loop)
{
	ptADPCM_.at(static_cast<size_t>(ptNum))->addLoop(loop);
}

void InstrumentsManager::removePitchADPCMLoop(int ptNum, int begin, int end)
{
	ptADPCM_.at(static_cast<size_t>(ptNum))->removeLoop(begin, end);
}

void InstrumentsManager::changePitchADPCMLoop(int ptNum, int prevBegin, int prevEnd, const InstrumentSequenceLoop& loop)
{
	ptADPCM_.at(static_cast<size_t>(ptNum))->changeLoop(prevBegin, prevEnd, loop);
}

void InstrumentsManager::clearPitchADPCMLoops(int ptNum)
{
	ptADPCM_.at(static_cast<size_t>(ptNum))->clearLoops();
}

InstrumentSequenceLoopRoot InstrumentsManager::getPitchADPCMLoopRoot(int ptNum) const
{
	return ptADPCM_.at(static_cast<size_t>(ptNum))->getLoopRoot();
}

void InstrumentsManager::setPitchADPCMRelease(int ptNum, const InstrumentSequenceRelease& release)
{
	ptADPCM_.at(static_cast<size_t>(ptNum))->setRelease(release);
}

InstrumentSequenceRelease InstrumentsManager::getPitchADPCMRelease(int ptNum) const
{
	return ptADPCM_.at(static_cast<size_t>(ptNum))->getRelease();
}

PitchIter InstrumentsManager::getPitchADPCMIterator(int ptNum) const
{
	return ptADPCM_.at(static_cast<size_t>(ptNum))->getIterator();
}

std::multiset<int> InstrumentsManager::getPitchADPCMUsers(int ptNum) const
{
	return ptADPCM_.at(static_cast<size_t>(ptNum))->getUserInstruments();
}

std::vector<int> InstrumentsManager::getPitchADPCMEntriedIndices() const
{
	return utils::findIndicesIf(ptADPCM_, IsUsedOrEdited<decltype(ptADPCM_)>());
}

int InstrumentsManager::findFirstAssignablePitchADPCM() const
{
	return findFirstAssignableProperty(ptADPCM_, regardingUnedited_);
}

void InstrumentsManager::setInstrumentADPCMPanEnabled(int instNum, bool enabled)
{
	auto adpcm = std::dynamic_pointer_cast<InstrumentADPCM>(insts_.at(static_cast<size_t>(instNum)));
	adpcm->setPanEnabled(enabled);
	if (enabled)
		panADPCM_.at(static_cast<size_t>(adpcm->getPanNumber()))->registerUserInstrument(instNum);
	else
		panADPCM_.at(static_cast<size_t>(adpcm->getPanNumber()))->deregisterUserInstrument(instNum);
}

bool InstrumentsManager::getInstrumentADPCMPanEnabled(int instNum) const
{
	return std::dynamic_pointer_cast<InstrumentADPCM>(insts_.at(static_cast<size_t>(instNum)))->getPanEnabled();
}

void InstrumentsManager::setInstrumentADPCMPan(int instNum, int panNum)
{
	auto adpcm = std::dynamic_pointer_cast<InstrumentADPCM>(insts_.at(static_cast<size_t>(instNum)));
	if (adpcm->getPanEnabled()) {
		panADPCM_.at(static_cast<size_t>(adpcm->getPanNumber()))->deregisterUserInstrument(instNum);
		panADPCM_.at(static_cast<size_t>(panNum))->registerUserInstrument(instNum);
	}
	adpcm->setPanNumber(panNum);
}

int InstrumentsManager::getInstrumentADPCMPan(int instNum)
{
	return std::dynamic_pointer_cast<InstrumentADPCM>(insts_.at(static_cast<size_t>(instNum)))->getPanNumber();
}

void InstrumentsManager::addPanADPCMSequenceData(int panNum, int data)
{
	panADPCM_.at(static_cast<size_t>(panNum))->addSequenceUnit(PanUnit(data));
}

void InstrumentsManager::removePanADPCMSequenceData(int panNum)
{
	panADPCM_.at(static_cast<size_t>(panNum))->removeSequenceUnit();
}

void InstrumentsManager::setPanADPCMSequenceData(int panNum, int cnt, int data)
{
	panADPCM_.at(static_cast<size_t>(panNum))->setSequenceUnit(cnt, PanUnit(data));
}

std::vector<PanUnit> InstrumentsManager::getPanADPCMSequence(int panNum)
{
	return panADPCM_.at(static_cast<size_t>(panNum))->getSequence();
}

void InstrumentsManager::addPanADPCMLoop(int panNum, const InstrumentSequenceLoop& loop)
{
	panADPCM_.at(static_cast<size_t>(panNum))->addLoop(loop);
}

void InstrumentsManager::removePanADPCMLoop(int panNum, int begin, int end)
{
	panADPCM_.at(static_cast<size_t>(panNum))->removeLoop(begin, end);
}

void InstrumentsManager::changePanADPCMLoop(int panNum, int prevBegin, int prevEnd, const InstrumentSequenceLoop& loop)
{
	panADPCM_.at(static_cast<size_t>(panNum))->changeLoop(prevBegin, prevEnd, loop);
}

void InstrumentsManager::clearPanADPCMLoops(int panNum)
{
	panADPCM_.at(static_cast<size_t>(panNum))->clearLoops();
}

InstrumentSequenceLoopRoot InstrumentsManager::getPanADPCMLoopRoot(int panNum) const
{
	return panADPCM_.at(static_cast<size_t>(panNum))->getLoopRoot();
}

void InstrumentsManager::setPanADPCMRelease(int panNum, const InstrumentSequenceRelease& release)
{
	panADPCM_.at(static_cast<size_t>(panNum))->setRelease(release);
}

InstrumentSequenceRelease InstrumentsManager::getPanADPCMRelease(int panNum) const
{
	return panADPCM_.at(static_cast<size_t>(panNum))->getRelease();
}

PanIter InstrumentsManager::getPanADPCMIterator(int panNum) const
{
	return panADPCM_.at(static_cast<size_t>(panNum))->getIterator();
}

std::multiset<int> InstrumentsManager::getPanADPCMUsers(int panNum) const
{
	return panADPCM_.at(static_cast<size_t>(panNum))->getUserInstruments();
}

std::vector<int> InstrumentsManager::getPanADPCMEntriedIndices() const
{
	return utils::findIndicesIf(panADPCM_, IsUsedOrEdited<decltype(panADPCM_)>());
}

int InstrumentsManager::findFirstAssignablePanADPCM() const
{
	return findFirstAssignableProperty(panADPCM_, regardingUnedited_);
}

bool InstrumentsManager::equalPropertiesADPCM(std::shared_ptr<AbstractInstrument> a, std::shared_ptr<AbstractInstrument> b) const
{
	auto aAdpcm = std::dynamic_pointer_cast<InstrumentADPCM>(a);
	auto bAdpcm = std::dynamic_pointer_cast<InstrumentADPCM>(b);

	if (*sampADPCM_[aAdpcm->getSampleNumber()] != *sampADPCM_[bAdpcm->getSampleNumber()])
		return false;
	if (aAdpcm->getEnvelopeEnabled() != bAdpcm->getEnvelopeEnabled())
		return false;
	if (aAdpcm->getEnvelopeEnabled()
			&& *envADPCM_[aAdpcm->getEnvelopeNumber()] != *envADPCM_[bAdpcm->getEnvelopeNumber()])
		return false;
	if (aAdpcm->getArpeggioEnabled() != bAdpcm->getArpeggioEnabled())
		return false;
	if (aAdpcm->getArpeggioEnabled()
			&& *arpADPCM_[aAdpcm->getArpeggioNumber()] != *arpADPCM_[bAdpcm->getArpeggioNumber()])
		return false;
	if (aAdpcm->getPitchEnabled() != bAdpcm->getPitchEnabled())
		return false;
	if (aAdpcm->getPitchEnabled()
			&& *ptADPCM_[aAdpcm->getPitchNumber()] != *ptADPCM_[bAdpcm->getPitchNumber()])
		return false;
	if (aAdpcm->getPanEnabled()
			&& *panADPCM_[aAdpcm->getPanNumber()] != *panADPCM_[bAdpcm->getPanNumber()])
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
	return std::dynamic_pointer_cast<InstrumentDrumkit>(insts_.at(static_cast<size_t>(instNum)))->getSampleNumber(key);
}

void InstrumentsManager::setInstrumentDrumkitPitch(int instNum, int key, int pitch)
{
	std::dynamic_pointer_cast<InstrumentDrumkit>(insts_.at(static_cast<size_t>(instNum)))->setPitch(key, pitch);
}

void InstrumentsManager::setInstrumentDrumkitPan(int instNum, int key, int pan)
{
	std::dynamic_pointer_cast<InstrumentDrumkit>(insts_.at(static_cast<size_t>(instNum)))->setPan(key, pan);
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
	if (!std::includes(aKeys.cbegin(), aKeys.cend(), bKeys.cbegin(), bKeys.cend())) return false;

	for (const int& key : aKeys) {
		if (*sampADPCM_[aKit->getSampleNumber(key)] != *sampADPCM_[bKit->getSampleNumber(key)])
			return false;
		if (aKit->getPitch(key) != bKit->getPitch(key))
			return false;
		if (aKit->getPan(key) != bKit->getPan(key))
			return false;
	}

	return true;
}
