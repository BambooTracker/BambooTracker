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

#include "instrument.hpp"
#include "instruments_manager.hpp"
#include "utils.hpp"

AbstractInstrument::AbstractInstrument(int number, SoundSource src, InstrumentType type, const std::string& name, InstrumentsManager* owner)
	: owner_(owner),
	  number_(number),
	  name_(name),
	  sndSrc_(src),
	  instType_(type)
{
}

bool AbstractInstrument::isRegisteredWithManager() const
{
	return (this == owner_->getInstrumentSharedPtr(number_).get());
}

/****************************************/

InstrumentFM::InstrumentFM(int number, const std::string& name, InstrumentsManager* owner) :
	AbstractInstrument(number, SoundSource::FM, InstrumentType::FM, name, owner),
	envNum_(0),
	lfoEnabled_(false),
	lfoNum_(0),
	panEnabled_(false),
	panNum_(0)
{
	opSeqEnabled_ = {
		{ FMEnvelopeParameter::AL,	false },
		{ FMEnvelopeParameter::FB,	false },
		{ FMEnvelopeParameter::AR1,	false },
		{ FMEnvelopeParameter::DR1,	false },
		{ FMEnvelopeParameter::SR1,	false },
		{ FMEnvelopeParameter::RR1,	false },
		{ FMEnvelopeParameter::SL1,	false },
		{ FMEnvelopeParameter::TL1,	false },
		{ FMEnvelopeParameter::KS1,	false },
		{ FMEnvelopeParameter::ML1,	false },
		{ FMEnvelopeParameter::DT1,	false },
		{ FMEnvelopeParameter::AR2,	false },
		{ FMEnvelopeParameter::DR2,	false },
		{ FMEnvelopeParameter::SR2,	false },
		{ FMEnvelopeParameter::RR2,	false },
		{ FMEnvelopeParameter::SL2,	false },
		{ FMEnvelopeParameter::TL2,	false },
		{ FMEnvelopeParameter::KS2,	false },
		{ FMEnvelopeParameter::ML2,	false },
		{ FMEnvelopeParameter::DT2,	false },
		{ FMEnvelopeParameter::AR3,	false },
		{ FMEnvelopeParameter::DR3,	false },
		{ FMEnvelopeParameter::SR3,	false },
		{ FMEnvelopeParameter::RR3,	false },
		{ FMEnvelopeParameter::SL3,	false },
		{ FMEnvelopeParameter::TL3,	false },
		{ FMEnvelopeParameter::KS3,	false },
		{ FMEnvelopeParameter::ML3,	false },
		{ FMEnvelopeParameter::DT3,	false },
		{ FMEnvelopeParameter::AR4,	false },
		{ FMEnvelopeParameter::DR4,	false },
		{ FMEnvelopeParameter::SR4,	false },
		{ FMEnvelopeParameter::RR4,	false },
		{ FMEnvelopeParameter::SL4,	false },
		{ FMEnvelopeParameter::TL4,	false },
		{ FMEnvelopeParameter::KS4,	false },
		{ FMEnvelopeParameter::ML4,	false },
		{ FMEnvelopeParameter::DT4,	false }
	};
	opSeqNum_ = {
		{ FMEnvelopeParameter::AL,	0 },
		{ FMEnvelopeParameter::FB,	0 },
		{ FMEnvelopeParameter::AR1,	0 },
		{ FMEnvelopeParameter::DR1,	0 },
		{ FMEnvelopeParameter::SR1,	0 },
		{ FMEnvelopeParameter::RR1,	0 },
		{ FMEnvelopeParameter::SL1,	0 },
		{ FMEnvelopeParameter::TL1,	0 },
		{ FMEnvelopeParameter::KS1,	0 },
		{ FMEnvelopeParameter::ML1,	0 },
		{ FMEnvelopeParameter::DT1,	0 },
		{ FMEnvelopeParameter::AR2,	0 },
		{ FMEnvelopeParameter::DR2,	0 },
		{ FMEnvelopeParameter::SR2,	0 },
		{ FMEnvelopeParameter::RR2,	0 },
		{ FMEnvelopeParameter::SL2,	0 },
		{ FMEnvelopeParameter::TL2,	0 },
		{ FMEnvelopeParameter::KS2,	0 },
		{ FMEnvelopeParameter::ML2,	0 },
		{ FMEnvelopeParameter::DT2,	0 },
		{ FMEnvelopeParameter::AR3,	0 },
		{ FMEnvelopeParameter::DR3,	0 },
		{ FMEnvelopeParameter::SR3,	0 },
		{ FMEnvelopeParameter::RR3,	0 },
		{ FMEnvelopeParameter::SL3,	0 },
		{ FMEnvelopeParameter::TL3,	0 },
		{ FMEnvelopeParameter::KS3,	0 },
		{ FMEnvelopeParameter::ML3,	0 },
		{ FMEnvelopeParameter::DT3,	0 },
		{ FMEnvelopeParameter::AR4,	0 },
		{ FMEnvelopeParameter::DR4,	0 },
		{ FMEnvelopeParameter::SR4,	0 },
		{ FMEnvelopeParameter::RR4,	0 },
		{ FMEnvelopeParameter::SL4,	0 },
		{ FMEnvelopeParameter::TL4,	0 },
		{ FMEnvelopeParameter::KS4,	0 },
		{ FMEnvelopeParameter::ML4,	0 },
		{ FMEnvelopeParameter::DT4,	0 }
	};
	arpEnabled_ = {
		{ FMOperatorType::All, false },
		{ FMOperatorType::Op1, false },
		{ FMOperatorType::Op2, false },
		{ FMOperatorType::Op3, false },
		{ FMOperatorType::Op4, false }
	};
	arpNum_ = {
		{ FMOperatorType::All, 0 },
		{ FMOperatorType::Op1, 0 },
		{ FMOperatorType::Op2, 0 },
		{ FMOperatorType::Op3, 0 },
		{ FMOperatorType::Op4, 0 }
	};
	ptEnabled_ = {
		{ FMOperatorType::All, false },
		{ FMOperatorType::Op1, false },
		{ FMOperatorType::Op2, false },
		{ FMOperatorType::Op3, false },
		{ FMOperatorType::Op4, false }
	};
	ptNum_ = {
		{ FMOperatorType::All, 0 },
		{ FMOperatorType::Op1, 0 },
		{ FMOperatorType::Op2, 0 },
		{ FMOperatorType::Op3, 0 },
		{ FMOperatorType::Op4, 0 }
	};
	envResetEnabled_ = {
		{ FMOperatorType::All, false },
		{ FMOperatorType::Op1, false },
		{ FMOperatorType::Op2, false },
		{ FMOperatorType::Op3, false },
		{ FMOperatorType::Op4, false }
	};
}

AbstractInstrument* InstrumentFM::clone()
{
	return new InstrumentFM(*this);
}

int InstrumentFM::getEnvelopeParameter(FMEnvelopeParameter param) const
{
	return owner_->getEnvelopeFMParameter(envNum_, param);
}

bool InstrumentFM::getOperatorEnabled(int n) const
{
	return owner_->getEnvelopeFMOperatorEnabled(envNum_, n);
}

int InstrumentFM::getLFOParameter(FMLFOParameter param) const
{
	return owner_->getLFOFMparameter(lfoNum_, param);
}

void InstrumentFM::setOperatorSequenceEnabled(FMEnvelopeParameter param, bool enabled)
{
	opSeqEnabled_.at(param) = enabled;
}

bool InstrumentFM::getOperatorSequenceEnabled(FMEnvelopeParameter param) const
{
	return opSeqEnabled_.at(param);
}

void InstrumentFM::setOperatorSequenceNumber(FMEnvelopeParameter param, int n)
{
	opSeqNum_.at(param) = n;
}

int InstrumentFM::getOperatorSequenceNumber(FMEnvelopeParameter param) const
{
	return opSeqNum_.at(param);
}

std::vector<FMOperatorSequenceUnit> InstrumentFM::getOperatorSequenceSequence(FMEnvelopeParameter param) const
{
	return owner_->getOperatorSequenceFMSequence(param, opSeqNum_.at(param));
}

InstrumentSequenceLoopRoot InstrumentFM::getOperatorSequenceLoopRoot(FMEnvelopeParameter param) const
{
	return owner_->getOperatorSequenceFMLoopRoot(param, opSeqNum_.at(param));
}

InstrumentSequenceRelease InstrumentFM::getOperatorSequenceRelease(FMEnvelopeParameter param) const
{
	return owner_->getOperatorSequenceFMRelease(param, opSeqNum_.at(param));
}

FMOperatorSequenceIter InstrumentFM::getOperatorSequenceSequenceIterator(FMEnvelopeParameter param) const
{
	return owner_->getOperatorSequenceFMIterator(param, opSeqNum_.at(param));
}

void InstrumentFM::setArpeggioEnabled(FMOperatorType op, bool enabled)
{
	arpEnabled_.at(op) = enabled;
}

bool InstrumentFM::getArpeggioEnabled(FMOperatorType op) const
{
	return arpEnabled_.at(op);
}

void InstrumentFM::setArpeggioNumber(FMOperatorType op, int n)
{
	arpNum_.at(op) = n;
}

int InstrumentFM::getArpeggioNumber(FMOperatorType op) const
{
	return arpNum_.at(op);
}

SequenceType InstrumentFM::getArpeggioType(FMOperatorType op) const
{
	return owner_->getArpeggioFMType(arpNum_.at(op));
}

std::vector<ArpeggioUnit> InstrumentFM::getArpeggioSequence(FMOperatorType op) const
{
	return owner_->getArpeggioFMSequence(arpNum_.at(op));
}

InstrumentSequenceLoopRoot InstrumentFM::getArpeggioLoopRoot(FMOperatorType op) const
{
	return owner_->getArpeggioFMLoopRoot(arpNum_.at(op));
}

InstrumentSequenceRelease InstrumentFM::getArpeggioRelease(FMOperatorType op) const
{
	return owner_->getArpeggioFMRelease(arpNum_.at(op));
}

ArpeggioIter InstrumentFM::getArpeggioSequenceIterator(FMOperatorType op) const
{
	return owner_->getArpeggioFMIterator(arpNum_.at(op));
}

void InstrumentFM::setPitchEnabled(FMOperatorType op, bool enabled)
{
	ptEnabled_.at(op) = enabled;
}

bool InstrumentFM::getPitchEnabled(FMOperatorType op) const
{
	return ptEnabled_.at(op);
}

void InstrumentFM::setPitchNumber(FMOperatorType op, int n)
{
	ptNum_.at(op) = n;
}

int InstrumentFM::getPitchNumber(FMOperatorType op) const
{
	return ptNum_.at(op);
}

SequenceType InstrumentFM::getPitchType(FMOperatorType op) const
{
	return owner_->getPitchFMType(ptNum_.at(op));
}

std::vector<PitchUnit> InstrumentFM::getPitchSequence(FMOperatorType op) const
{
	return owner_->getPitchFMSequence(ptNum_.at(op));
}

InstrumentSequenceLoopRoot InstrumentFM::getPitchLoopRoot(FMOperatorType op) const
{
	return owner_->getPitchFMLoopRoot(ptNum_.at(op));
}

InstrumentSequenceRelease InstrumentFM::getPitchRelease(FMOperatorType op) const
{
	return owner_->getPitchFMRelease(ptNum_.at(op));
}

PitchIter InstrumentFM::getPitchSequenceIterator(FMOperatorType op) const
{
	return owner_->getPitchFMIterator(ptNum_.at(op));
}

void InstrumentFM::setPanEnabled(bool enabled)
{
	panEnabled_ = enabled;
}

bool InstrumentFM::getPanEnabled() const
{
	return panEnabled_;
}

void InstrumentFM::setPanNumber(int n)
{
	panNum_ = n;
}

int InstrumentFM::getPanNumber() const
{
	return panNum_;
}

std::vector<PanUnit> InstrumentFM::getPanSequence() const
{
	return owner_->getPanFMSequence(panNum_);
}

InstrumentSequenceLoopRoot InstrumentFM::getPanLoopRoot() const
{
	return owner_->getPanFMLoopRoot(panNum_);
}

InstrumentSequenceRelease InstrumentFM::getPanRelease() const
{
	return owner_->getPanFMRelease(panNum_);
}

PanIter InstrumentFM::getPanSequenceIterator() const
{
	return owner_->getPanFMIterator(panNum_);
}

void InstrumentFM::setEnvelopeResetEnabled(FMOperatorType op, bool enabled)
{
	envResetEnabled_.at(op) = enabled;
}

bool InstrumentFM::getEnvelopeResetEnabled(FMOperatorType op) const
{
	return envResetEnabled_.at(op);
}

/****************************************/

InstrumentSSG::InstrumentSSG(int number, const std::string& name, InstrumentsManager* owner)
	: AbstractInstrument(number, SoundSource::SSG, InstrumentType::SSG, name, owner),
	  wfEnabled_(false),
	  wfNum_(0),
	  tnEnabled_(false),
	  tnNum_(0),
	  envEnabled_(false),
	  envNum_(0),
	  arpEnabled_(false),
	  arpNum_(0),
	  ptEnabled_(false),
	  ptNum_(0)
{
}

AbstractInstrument* InstrumentSSG::clone()
{
	return new InstrumentSSG(*this);
}

std::vector<SSGWaveformUnit> InstrumentSSG::getWaveformSequence() const
{
	return owner_->getWaveformSSGSequence(wfNum_);
}

InstrumentSequenceLoopRoot InstrumentSSG::getWaveformLoopRoot() const
{
	return owner_->getWaveformSSGLoopRoot(wfNum_);
}

InstrumentSequenceRelease InstrumentSSG::getWaveformRelease() const
{
	return owner_->getWaveformSSGRelease(wfNum_);
}

SSGWaveformIter InstrumentSSG::getWaveformSequenceIterator() const
{
	return owner_->getWaveformSSGIterator(wfNum_);
}

std::vector<SSGToneNoiseUnit> InstrumentSSG::getToneNoiseSequence() const
{
	return owner_->getToneNoiseSSGSequence(tnNum_);
}

InstrumentSequenceLoopRoot InstrumentSSG::getToneNoiseLoopRoot() const
{
	return owner_->getToneNoiseSSGLoopRoot(tnNum_);
}

InstrumentSequenceRelease InstrumentSSG::getToneNoiseRelease() const
{
	return owner_->getToneNoiseSSGRelease(tnNum_);
}

SSGToneNoiseIter InstrumentSSG::getToneNoiseSequenceIterator() const
{
	return owner_->getToneNoiseSSGIterator(tnNum_);
}

std::vector<SSGEnvelopeUnit> InstrumentSSG::getEnvelopeSequence() const
{
	return owner_->getEnvelopeSSGSequence(envNum_);
}

InstrumentSequenceLoopRoot InstrumentSSG::getEnvelopeLoopRoot() const
{
	return owner_->getEnvelopeSSGLoopRoot(envNum_);
}

InstrumentSequenceRelease InstrumentSSG::getEnvelopeRelease() const
{
	return owner_->getEnvelopeSSGRelease(envNum_);
}

SSGEnvelopeIter InstrumentSSG::getEnvelopeSequenceIterator() const
{
	return owner_->getEnvelopeSSGIterator(envNum_);
}

SequenceType InstrumentSSG::getArpeggioType() const
{
	return owner_->getArpeggioSSGType(arpNum_);
}

std::vector<ArpeggioUnit> InstrumentSSG::getArpeggioSequence() const
{
	return owner_->getArpeggioSSGSequence(arpNum_);
}

InstrumentSequenceLoopRoot InstrumentSSG::getArpeggioLoopRoot() const
{
	return owner_->getArpeggioSSGLoopRoot(arpNum_);
}

InstrumentSequenceRelease InstrumentSSG::getArpeggioRelease() const
{
	return owner_->getArpeggioSSGRelease(arpNum_);
}

ArpeggioIter InstrumentSSG::getArpeggioSequenceIterator() const
{
	return owner_->getArpeggioSSGIterator(arpNum_);
}

SequenceType InstrumentSSG::getPitchType() const
{
	return owner_->getPitchSSGType(ptNum_);
}

std::vector<PitchUnit> InstrumentSSG::getPitchSequence() const
{
	return owner_->getPitchSSGSequence(ptNum_);
}

InstrumentSequenceLoopRoot InstrumentSSG::getPitchLoopRoot() const
{
	return owner_->getPitchSSGLoopRoot(ptNum_);
}

InstrumentSequenceRelease InstrumentSSG::getPitchRelease() const
{
	return owner_->getPitchSSGRelease(ptNum_);
}

PitchIter InstrumentSSG::getPitchSequenceIterator() const
{
	return owner_->getPitchSSGIterator(ptNum_);
}

/****************************************/

InstrumentADPCM::InstrumentADPCM(int number, const std::string& name, InstrumentsManager* owner)
	: AbstractInstrument(number, SoundSource::ADPCM, InstrumentType::ADPCM, name, owner),
	  sampNum_(0),
	  envEnabled_(false),
	  envNum_(0),
	  arpEnabled_(false),
	  arpNum_(0),
	  ptEnabled_(false),
	  ptNum_(0),
	  panEnabled_(false),
	  panNum_(0)
{
}

AbstractInstrument* InstrumentADPCM::clone()
{
	return new InstrumentADPCM(*this);
}

int InstrumentADPCM::getSampleRootKeyNumber() const
{
	return owner_->getSampleADPCMRootKeyNumber(sampNum_);
}

int InstrumentADPCM::getSampleRootDeltaN() const
{
	return owner_->getSampleADPCMRootDeltaN(sampNum_);
}

bool InstrumentADPCM::isSampleRepeatable() const
{
	return owner_->isSampleADPCMRepeatable(sampNum_);
}

SampleRepeatFlag InstrumentADPCM::getSampleRepeatFlag() const
{
	return owner_->getSampleADPCMRepeatFlag(sampNum_);
}

SampleRepeatRange InstrumentADPCM::getSampleRepeatRange() const
{
	return owner_->getSampleADPCMRepeatRange(sampNum_);
}

std::vector<uint8_t> InstrumentADPCM::getRawSample() const
{
	return owner_->getSampleADPCMRawSample(sampNum_);
}

size_t InstrumentADPCM::getSampleStartAddress() const
{
	return owner_->getSampleADPCMStartAddress(sampNum_);
}

size_t InstrumentADPCM::getSampleStopAddress() const
{
	return owner_->getSampleADPCMStopAddress(sampNum_);
}

std::vector<ADPCMEnvelopeUnit> InstrumentADPCM::getEnvelopeSequence() const
{
	return owner_->getEnvelopeADPCMSequence(envNum_);
}

InstrumentSequenceLoopRoot InstrumentADPCM::getEnvelopeLoopRoot() const
{
	return owner_->getEnvelopeADPCMLoopRoot(envNum_);
}

InstrumentSequenceRelease InstrumentADPCM::getEnvelopeRelease() const
{
	return owner_->getEnvelopeADPCMRelease(envNum_);
}

ADPCMEnvelopeIter InstrumentADPCM::getEnvelopeSequenceIterator() const
{
	return owner_->getEnvelopeADPCMIterator(envNum_);
}

SequenceType InstrumentADPCM::getArpeggioType() const
{
	return owner_->getArpeggioADPCMType(arpNum_);
}

std::vector<ArpeggioUnit> InstrumentADPCM::getArpeggioSequence() const
{
	return owner_->getArpeggioADPCMSequence(arpNum_);
}

InstrumentSequenceLoopRoot InstrumentADPCM::getArpeggioLoopRoot() const
{
	return owner_->getArpeggioADPCMLoopRoot(arpNum_);
}

InstrumentSequenceRelease InstrumentADPCM::getArpeggioRelease() const
{
	return owner_->getArpeggioADPCMRelease(arpNum_);
}

ArpeggioIter InstrumentADPCM::getArpeggioSequenceIterator() const
{
	return owner_->getArpeggioADPCMIterator(arpNum_);
}

SequenceType InstrumentADPCM::getPitchType() const
{
	return owner_->getPitchADPCMType(ptNum_);
}

std::vector<PitchUnit> InstrumentADPCM::getPitchSequence() const
{
	return owner_->getPitchADPCMSequence(ptNum_);
}

InstrumentSequenceLoopRoot InstrumentADPCM::getPitchLoopRoot() const
{
	return owner_->getPitchADPCMLoopRoot(ptNum_);
}

InstrumentSequenceRelease InstrumentADPCM::getPitchRelease() const
{
	return owner_->getPitchADPCMRelease(ptNum_);
}

PitchIter InstrumentADPCM::getPitchSequenceIterator() const
{
	return owner_->getPitchADPCMIterator(ptNum_);
}

std::vector<PanUnit> InstrumentADPCM::getPanSequence() const
{
	return owner_->getPanADPCMSequence(panNum_);
}

InstrumentSequenceLoopRoot InstrumentADPCM::getPanLoopRoot() const
{
	return owner_->getPanADPCMLoopRoot(panNum_);
}

InstrumentSequenceRelease InstrumentADPCM::getPanRelease() const
{
	return owner_->getPanADPCMRelease(panNum_);
}

PanIter InstrumentADPCM::getPanSequenceIterator() const
{
	return owner_->getPanADPCMIterator(panNum_);
}

/****************************************/

InstrumentDrumkit::InstrumentDrumkit(int number, const std::string& name, InstrumentsManager* owner)
	: AbstractInstrument(number, SoundSource::ADPCM, InstrumentType::Drumkit, name, owner)
{
}

AbstractInstrument* InstrumentDrumkit::clone()
{
	return new InstrumentDrumkit(*this);
}

std::vector<int> InstrumentDrumkit::getAssignedKeys() const
{
	return utils::getMapKeys(kit_);
}

void InstrumentDrumkit::setSampleEnabled(int key, bool enabled)
{
	if (enabled) kit_[key] = { 0, 0, PanType::CENTER };
	else kit_.erase(key);
}

bool InstrumentDrumkit::getSampleEnabled(int key) const
{
	return kit_.count(key);
}

void InstrumentDrumkit::setSampleNumber(int key, int n)
{
	if (kit_.count(key)) kit_.at(key).sampNum = n;
}

int InstrumentDrumkit::getSampleNumber(int key) const
{
	return kit_.at(key).sampNum;
}

int InstrumentDrumkit::getSampleRootKeyNumber(int key) const
{
	return owner_->getSampleADPCMRootKeyNumber(kit_.at(key).sampNum);
}

int InstrumentDrumkit::getSampleRootDeltaN(int key) const
{
	return owner_->getSampleADPCMRootDeltaN(kit_.at(key).sampNum);
}

bool InstrumentDrumkit::isSampleRepeatable(int key) const
{
	return owner_->isSampleADPCMRepeatable(kit_.at(key).sampNum);
}

SampleRepeatFlag InstrumentDrumkit::getSampleRepeatFlag(int key) const
{
	return owner_->getSampleADPCMRepeatFlag(kit_.at(key).sampNum);
}

SampleRepeatRange InstrumentDrumkit::getSampleRepeatRange(int key) const
{
	return owner_->getSampleADPCMRepeatRange(kit_.at(key).sampNum);
}

std::vector<uint8_t> InstrumentDrumkit::getRawSample(int key) const
{
	return owner_->getSampleADPCMRawSample(kit_.at(key).sampNum);
}

size_t InstrumentDrumkit::getSampleStartAddress(int key) const
{
	return owner_->getSampleADPCMStartAddress(kit_.at(key).sampNum);
}

size_t InstrumentDrumkit::getSampleStopAddress(int key) const
{
	return owner_->getSampleADPCMStopAddress(kit_.at(key).sampNum);
}

void InstrumentDrumkit::setPitch(int key, int pitch)
{
	if (kit_.count(key)) kit_.at(key).pitch = pitch;
}

int InstrumentDrumkit::getPitch(int key) const
{
	return kit_.at(key).pitch;
}

void InstrumentDrumkit::setPan(int key, int pan)
{
	if (kit_.count(key)) kit_.at(key).pan = pan;
}

int InstrumentDrumkit::getPan(int key) const
{
	return kit_.at(key).pan;
}
