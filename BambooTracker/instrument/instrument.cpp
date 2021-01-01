/*
 * Copyright (C) 2018-2020 Rerrah
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
#include <algorithm>

AbstractInstrument::AbstractInstrument(int number, std::string name, InstrumentsManager* owner)
	: owner_(owner),
	  number_(number),
	  name_(name)
{}

int AbstractInstrument::getNumber() const
{
	return number_;
}

void AbstractInstrument::setNumber(int n)
{
	number_ = n;
}

std::string AbstractInstrument::getName() const
{
	return name_;
}

void AbstractInstrument::setName(std::string name)
{
	name_ = name;
}

bool AbstractInstrument::isRegisteredWithManager() const
{
	return (this == owner_->getInstrumentSharedPtr(number_).get());
}

/****************************************/

InstrumentFM::InstrumentFM(int number, std::string name, InstrumentsManager* owner) :
	AbstractInstrument(number, name, owner),
	envNum_(0),
	lfoEnabled_(false),
	lfoNum_(0)
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

SoundSource InstrumentFM::getSoundSource() const
{
	return SoundSource::FM;
}

InstrumentType InstrumentFM::getType() const
{
	return InstrumentType::FM;
}

AbstractInstrument* InstrumentFM::clone()
{
	auto c = new InstrumentFM(number_, name_, owner_);
	c->setEnvelopeNumber(envNum_);
	c->setLFOEnabled(lfoEnabled_);
	c->setLFONumber(lfoNum_);
	for (auto pair : opSeqEnabled_)	{
		c->setOperatorSequenceEnabled(pair.first, pair.second);
		c->setOperatorSequenceNumber(pair.first, opSeqNum_.at(pair.first));
	}
	for (auto pair : arpEnabled_) {
		c->setArpeggioEnabled(pair.first, pair.second);
		c->setArpeggioNumber(pair.first, arpNum_.at(pair.first));
		c->setPitchEnabled(pair.first, ptEnabled_.at(pair.first));
		c->setPitchNumber(pair.first, ptNum_.at(pair.first));
		c->setEnvelopeResetEnabled(pair.first, envResetEnabled_.at(pair.first));
	}
	return c;
}

void InstrumentFM::setEnvelopeNumber(int n)
{
	envNum_ = n;
}

int InstrumentFM::getEnvelopeNumber() const
{
	return envNum_;
}

int InstrumentFM::getEnvelopeParameter(FMEnvelopeParameter param) const
{
	return owner_->getEnvelopeFMParameter(envNum_, param);
}

bool InstrumentFM::getOperatorEnabled(int n) const
{
	return owner_->getEnvelopeFMOperatorEnabled(envNum_, n);
}

void InstrumentFM::setLFOEnabled(bool enabled)
{
	lfoEnabled_ = enabled;
}

bool InstrumentFM::getLFOEnabled() const
{
	return lfoEnabled_;
}

void InstrumentFM::setLFONumber(int n)
{
	lfoNum_ = n;
}

int InstrumentFM::getLFONumber() const
{
	return lfoNum_;
}

int InstrumentFM::getLFOParameter(FMLFOParameter param) const
{
	return owner_->getLFOFMparameter(lfoNum_, param);
}

void InstrumentFM::setEnvelopeResetEnabled(FMOperatorType op, bool enabled)
{
	envResetEnabled_.at(op) = enabled;
}

bool InstrumentFM::getEnvelopeResetEnabled(FMOperatorType op) const
{
	return envResetEnabled_.at(op);
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

std::vector<CommandSequenceUnit> InstrumentFM::getArpeggioSequence(FMOperatorType op) const
{
	return owner_->getArpeggioFMSequence(arpNum_.at(op));
}

std::vector<Loop> InstrumentFM::getArpeggioLoops(FMOperatorType op) const
{
	return owner_->getArpeggioFMLoops(arpNum_.at(op));
}

Release InstrumentFM::getArpeggioRelease(FMOperatorType op) const
{
	return owner_->getArpeggioFMRelease(arpNum_.at(op));
}

std::unique_ptr<CommandSequence::Iterator> InstrumentFM::getArpeggioSequenceIterator(FMOperatorType op) const
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

std::vector<CommandSequenceUnit> InstrumentFM::getPitchSequence(FMOperatorType op) const
{
	return owner_->getPitchFMSequence(ptNum_.at(op));
}

std::vector<Loop> InstrumentFM::getPitchLoops(FMOperatorType op) const
{
	return owner_->getPitchFMLoops(ptNum_.at(op));
}

Release InstrumentFM::getPitchRelease(FMOperatorType op) const
{
	return owner_->getPitchFMRelease(ptNum_.at(op));
}

std::unique_ptr<CommandSequence::Iterator> InstrumentFM::getPitchSequenceIterator(FMOperatorType op) const
{
	return owner_->getPitchFMIterator(ptNum_.at(op));
}

/****************************************/

InstrumentSSG::InstrumentSSG(int number, std::string name, InstrumentsManager* owner)
	: AbstractInstrument(number, name, owner),
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

SoundSource InstrumentSSG::getSoundSource() const
{
	return SoundSource::SSG;
}

InstrumentType InstrumentSSG::getType() const
{
	return InstrumentType::SSG;
}

AbstractInstrument* InstrumentSSG::clone()
{
	auto c = new InstrumentSSG(number_, name_, owner_);
	c->setWaveformEnabled(wfEnabled_);
	c->setWaveformNumber(wfNum_);
	c->setToneNoiseEnabled(tnEnabled_);
	c->setToneNoiseNumber(tnNum_);
	c->setEnvelopeEnabled(envEnabled_);
	c->setEnvelopeNumber(envNum_);
	c->setArpeggioEnabled(arpEnabled_);
	c->setArpeggioNumber(arpNum_);
	c->setPitchEnabled(ptEnabled_);
	c->setPitchNumber(ptNum_);
	return c;
}

void InstrumentSSG::setWaveformEnabled(bool enabled)
{
	wfEnabled_ = enabled;
}

bool InstrumentSSG::getWaveformEnabled() const
{
	return wfEnabled_;
}

void InstrumentSSG::setWaveformNumber(int n)
{
	wfNum_ = n;
}

int InstrumentSSG::getWaveformNumber() const
{
	return wfNum_;
}

std::vector<CommandSequenceUnit> InstrumentSSG::getWaveformSequence() const
{
	return owner_->getWaveformSSGSequence(wfNum_);
}

std::vector<Loop> InstrumentSSG::getWaveformLoops() const
{
	return owner_->getWaveformSSGLoops(wfNum_);
}

Release InstrumentSSG::getWaveformRelease() const
{
	return owner_->getWaveformSSGRelease(wfNum_);
}

std::unique_ptr<CommandSequence::Iterator> InstrumentSSG::getWaveformSequenceIterator() const
{
	return owner_->getWaveformSSGIterator(wfNum_);
}

void InstrumentSSG::setToneNoiseEnabled(bool enabled)
{
	tnEnabled_ = enabled;
}

bool InstrumentSSG::getToneNoiseEnabled() const
{
	return tnEnabled_;
}

void InstrumentSSG::setToneNoiseNumber(int n)
{
	tnNum_ = n;
}

int InstrumentSSG::getToneNoiseNumber() const
{
	return tnNum_;
}

std::vector<CommandSequenceUnit> InstrumentSSG::getToneNoiseSequence() const
{
	return owner_->getToneNoiseSSGSequence(tnNum_);
}

std::vector<Loop> InstrumentSSG::getToneNoiseLoops() const
{
	return owner_->getToneNoiseSSGLoops(tnNum_);
}

Release InstrumentSSG::getToneNoiseRelease() const
{
	return owner_->getToneNoiseSSGRelease(tnNum_);
}

std::unique_ptr<CommandSequence::Iterator> InstrumentSSG::getToneNoiseSequenceIterator() const
{
	return owner_->getToneNoiseSSGIterator(tnNum_);
}

void InstrumentSSG::setEnvelopeEnabled(bool enabled)
{
	envEnabled_ = enabled;
}

bool InstrumentSSG::getEnvelopeEnabled() const
{
	return envEnabled_;
}

void InstrumentSSG::setEnvelopeNumber(int n)
{
	envNum_ = n;
}

int InstrumentSSG::getEnvelopeNumber() const
{
	return envNum_;
}

std::vector<CommandSequenceUnit> InstrumentSSG::getEnvelopeSequence() const
{
	return owner_->getEnvelopeSSGSequence(envNum_);
}

std::vector<Loop> InstrumentSSG::getEnvelopeLoops() const
{
	return owner_->getEnvelopeSSGLoops(envNum_);
}

Release InstrumentSSG::getEnvelopeRelease() const
{
	return owner_->getEnvelopeSSGRelease(envNum_);
}

std::unique_ptr<CommandSequence::Iterator> InstrumentSSG::getEnvelopeSequenceIterator() const
{
	return owner_->getEnvelopeSSGIterator(envNum_);
}

void InstrumentSSG::setArpeggioEnabled(bool enabled)
{
	arpEnabled_ = enabled;
}

bool InstrumentSSG::getArpeggioEnabled() const
{
	return arpEnabled_;
}

void InstrumentSSG::setArpeggioNumber(int n)
{
	arpNum_ = n;
}

int InstrumentSSG::getArpeggioNumber() const
{
	return arpNum_;
}

SequenceType InstrumentSSG::getArpeggioType() const
{
	return owner_->getArpeggioSSGType(arpNum_);
}

std::vector<CommandSequenceUnit> InstrumentSSG::getArpeggioSequence() const
{
	return owner_->getArpeggioSSGSequence(arpNum_);
}

std::vector<Loop> InstrumentSSG::getArpeggioLoops() const
{
	return owner_->getArpeggioSSGLoops(arpNum_);
}

Release InstrumentSSG::getArpeggioRelease() const
{
	return owner_->getArpeggioSSGRelease(arpNum_);
}

std::unique_ptr<CommandSequence::Iterator> InstrumentSSG::getArpeggioSequenceIterator() const
{
	return owner_->getArpeggioSSGIterator(arpNum_);
}

void InstrumentSSG::setPitchEnabled(bool enabled)
{
	ptEnabled_ = enabled;
}

bool InstrumentSSG::getPitchEnabled() const
{
	return ptEnabled_;
}

void InstrumentSSG::setPitchNumber(int n)
{
	ptNum_ = n;
}

int InstrumentSSG::getPitchNumber() const
{
	return ptNum_;
}

SequenceType InstrumentSSG::getPitchType() const
{
	return owner_->getPitchSSGType(ptNum_);
}

std::vector<CommandSequenceUnit> InstrumentSSG::getPitchSequence() const
{
	return owner_->getPitchSSGSequence(ptNum_);
}

std::vector<Loop> InstrumentSSG::getPitchLoops() const
{
	return owner_->getPitchSSGLoops(ptNum_);
}

Release InstrumentSSG::getPitchRelease() const
{
	return owner_->getPitchSSGRelease(ptNum_);
}

std::unique_ptr<CommandSequence::Iterator> InstrumentSSG::getPitchSequenceIterator() const
{
	return owner_->getPitchSSGIterator(ptNum_);
}

/****************************************/

InstrumentADPCM::InstrumentADPCM(int number, std::string name, InstrumentsManager* owner)
	: AbstractInstrument(number, name, owner),
	  sampNum_(0),
	  envEnabled_(false),
	  envNum_(0),
	  arpEnabled_(false),
	  arpNum_(0),
	  ptEnabled_(false),
	  ptNum_(0)
{
}

SoundSource InstrumentADPCM::getSoundSource() const
{
	return SoundSource::ADPCM;
}

InstrumentType InstrumentADPCM::getType() const
{
	return InstrumentType::ADPCM;
}

AbstractInstrument* InstrumentADPCM::clone()
{
	auto c = new InstrumentADPCM(number_, name_, owner_);
	c->setSampleNumber(sampNum_);
	c->setEnvelopeEnabled(envEnabled_);
	c->setEnvelopeNumber(envNum_);
	c->setArpeggioEnabled(arpEnabled_);
	c->setArpeggioNumber(arpNum_);
	c->setPitchEnabled(ptEnabled_);
	c->setPitchNumber(ptNum_);
	return c;
}

void InstrumentADPCM::setSampleNumber(int n)
{
	sampNum_ = n;
}

int InstrumentADPCM::getSampleNumber() const
{
	return sampNum_;
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

void InstrumentADPCM::setEnvelopeEnabled(bool enabled)
{
	envEnabled_ = enabled;
}

bool InstrumentADPCM::getEnvelopeEnabled() const
{
	return envEnabled_;
}

void InstrumentADPCM::setEnvelopeNumber(int n)
{
	envNum_ = n;
}

int InstrumentADPCM::getEnvelopeNumber() const
{
	return envNum_;
}

std::vector<CommandSequenceUnit> InstrumentADPCM::getEnvelopeSequence() const
{
	return owner_->getEnvelopeADPCMSequence(envNum_);
}

std::vector<Loop> InstrumentADPCM::getEnvelopeLoops() const
{
	return owner_->getEnvelopeADPCMLoops(envNum_);
}

Release InstrumentADPCM::getEnvelopeRelease() const
{
	return owner_->getEnvelopeADPCMRelease(envNum_);
}

std::unique_ptr<CommandSequence::Iterator> InstrumentADPCM::getEnvelopeSequenceIterator() const
{
	return owner_->getEnvelopeADPCMIterator(envNum_);
}

void InstrumentADPCM::setArpeggioEnabled(bool enabled)
{
	arpEnabled_ = enabled;
}

bool InstrumentADPCM::getArpeggioEnabled() const
{
	return arpEnabled_;
}

void InstrumentADPCM::setArpeggioNumber(int n)
{
	arpNum_ = n;
}

int InstrumentADPCM::getArpeggioNumber() const
{
	return arpNum_;
}

SequenceType InstrumentADPCM::getArpeggioType() const
{
	return owner_->getArpeggioADPCMType(arpNum_);
}

std::vector<CommandSequenceUnit> InstrumentADPCM::getArpeggioSequence() const
{
	return owner_->getArpeggioADPCMSequence(arpNum_);
}

std::vector<Loop> InstrumentADPCM::getArpeggioLoops() const
{
	return owner_->getArpeggioADPCMLoops(arpNum_);
}

Release InstrumentADPCM::getArpeggioRelease() const
{
	return owner_->getArpeggioADPCMRelease(arpNum_);
}

std::unique_ptr<CommandSequence::Iterator> InstrumentADPCM::getArpeggioSequenceIterator() const
{
	return owner_->getArpeggioADPCMIterator(arpNum_);
}

void InstrumentADPCM::setPitchEnabled(bool enabled)
{
	ptEnabled_ = enabled;
}

bool InstrumentADPCM::getPitchEnabled() const
{
	return ptEnabled_;
}

void InstrumentADPCM::setPitchNumber(int n)
{
	ptNum_ = n;
}

int InstrumentADPCM::getPitchNumber() const
{
	return ptNum_;
}

SequenceType InstrumentADPCM::getPitchType() const
{
	return owner_->getPitchADPCMType(ptNum_);
}

std::vector<CommandSequenceUnit> InstrumentADPCM::getPitchSequence() const
{
	return owner_->getPitchADPCMSequence(ptNum_);
}

std::vector<Loop> InstrumentADPCM::getPitchLoops() const
{
	return owner_->getPitchADPCMLoops(ptNum_);
}

Release InstrumentADPCM::getPitchRelease() const
{
	return owner_->getPitchADPCMRelease(ptNum_);
}

std::unique_ptr<CommandSequence::Iterator> InstrumentADPCM::getPitchSequenceIterator() const
{
	return owner_->getPitchADPCMIterator(ptNum_);
}

/****************************************/

InstrumentDrumkit::InstrumentDrumkit(int number, std::string name, InstrumentsManager* owner)
	: AbstractInstrument(number, name, owner)
{
}

SoundSource InstrumentDrumkit::getSoundSource() const
{
	return SoundSource::ADPCM;
}

InstrumentType InstrumentDrumkit::getType() const
{
	return InstrumentType::Drumkit;
}

AbstractInstrument* InstrumentDrumkit::clone()
{
	auto c = new InstrumentDrumkit(number_, name_, owner_);

	for (const auto& pair : kit_) {
		c->setSampleEnabled(pair.first, true);
		c->setSampleNumber(pair.first, pair.second.sampNum);
		c->setPitch(pair.first, pair.second.pitch);
	}
	return c;
}

std::vector<int> InstrumentDrumkit::getAssignedKeys() const
{
	std::vector<int> keys(kit_.size());
	std::transform(kit_.begin(), kit_.end(), keys.begin(), [](const auto& pair) { return pair.first; });
	return keys;
}

void InstrumentDrumkit::setSampleEnabled(int key, bool enabled)
{
	if (enabled) kit_[key] = { 0, 0 };
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
