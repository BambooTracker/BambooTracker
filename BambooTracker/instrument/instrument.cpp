#include "instrument.hpp"

AbstructInstrument::AbstructInstrument(int number, SoundSource source, std::string name, InstrumentsManager* owner)
	: owner_(owner),
	  name_(name),
	  number_(number),
	  source_(source),
	  gateCount_(0)
{}

int AbstructInstrument::getNumber() const
{
	return number_;
}

void AbstructInstrument::setNumber(int n)
{
	number_ = n;
}

SoundSource AbstructInstrument::getSoundSource() const
{
	return source_;
}

std::string AbstructInstrument::getName() const
{
	return name_;
}

void AbstructInstrument::setName(std::string name)
{
	name_ = name;
}

void AbstructInstrument::setGateCount(int count)
{
	gateCount_ = count;
}

int AbstructInstrument::getGateCount() const
{
	return gateCount_;
}

/****************************************/

InstrumentFM::InstrumentFM(int number, std::string name, InstrumentsManager* owner) :
	AbstructInstrument(number, SoundSource::FM, name, owner),
	envNum_(0),
	lfoNum_(-1),
	arpNum_(-1),
	ptNum_(-1),
	envResetEnabled_(true)
{
}

std::unique_ptr<AbstructInstrument> InstrumentFM::clone()
{
	return std::unique_ptr<InstrumentFM>(std::make_unique<InstrumentFM>(*this));
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

void InstrumentFM::setLFONumber(int n)
{
	lfoNum_ = n;
}

int InstrumentFM::getLFONumber() const
{
	return lfoNum_;
}

int InstrumentFM::getLFOParameter(FMLFOParamter param) const
{
	return owner_->getLFOFMparameter(lfoNum_, param);
}

void InstrumentFM::setEnvelopeResetEnabled(bool enabled)
{
	envResetEnabled_ = enabled;
}

bool InstrumentFM::getEnvelopeResetEnabled() const
{
	return envResetEnabled_;
}

void InstrumentFM::setArpeggioNumber(int n)
{
	arpNum_ = n;
}

int InstrumentFM::getArpeggioNumber() const
{
	return arpNum_;
}

int InstrumentFM::getArpeggioType() const
{
	return owner_->getArpeggioFMType(arpNum_);
}

std::vector<CommandInSequence> InstrumentFM::getArpeggioSequence() const
{
	return owner_->getArpeggioFMSequence(arpNum_);
}

std::vector<Loop> InstrumentFM::getArpeggioLoops() const
{
	return owner_->getArpeggioFMLoops(arpNum_);
}

Release InstrumentFM::getArpeggioRelease() const
{
	return owner_->getArpeggioFMRelease(arpNum_);
}

std::unique_ptr<CommandSequence::Iterator> InstrumentFM::getArpeggioSequenceIterator() const
{
	return owner_->getArpeggioFMIterator(arpNum_);
}

void InstrumentFM::setPitchNumber(int n)
{
	ptNum_ = n;
}

int InstrumentFM::getPitchNumber() const
{
	return ptNum_;
}

int InstrumentFM::getPitchType() const
{
	return owner_->getPitchFMType(ptNum_);
}

std::vector<CommandInSequence> InstrumentFM::getPitchSequence() const
{
	return owner_->getPitchFMSequence(ptNum_);
}

std::vector<Loop> InstrumentFM::getPitchLoops() const
{
	return owner_->getPitchFMLoops(ptNum_);
}

Release InstrumentFM::getPitchRelease() const
{
	return owner_->getPitchFMRelease(ptNum_);
}

std::unique_ptr<CommandSequence::Iterator> InstrumentFM::getPitchSequenceIterator() const
{
	return owner_->getPitchFMIterator(ptNum_);
}

/****************************************/

InstrumentSSG::InstrumentSSG(int number, std::string name, InstrumentsManager* owner)
	: AbstructInstrument(number, SoundSource::SSG, name, owner),
	  wfNum_(-1),
	  tnNum_(-1),
	  envNum_(-1),
	  arpNum_(-1),
	  ptNum_(-1)
{
}

std::unique_ptr<AbstructInstrument> InstrumentSSG::clone()
{
	return std::unique_ptr<AbstructInstrument>(std::make_unique<InstrumentSSG>(*this));
}

void InstrumentSSG::setWaveFormNumber(int n)
{
	wfNum_ = n;
}

int InstrumentSSG::getWaveFormNumber() const
{
	return wfNum_;
}

std::vector<CommandInSequence> InstrumentSSG::getWaveFormSequence() const
{
	return owner_->getWaveFormSSGSequence(wfNum_);
}

std::vector<Loop> InstrumentSSG::getWaveFormLoops() const
{
	return owner_->getWaveFormSSGLoops(wfNum_);
}

Release InstrumentSSG::getWaveFormRelease() const
{
	return owner_->getWaveFormSSGRelease(wfNum_);
}

std::unique_ptr<CommandSequence::Iterator> InstrumentSSG::getWaveFormSequenceIterator() const
{
	return owner_->getWaveFormSSGIterator(wfNum_);
}

void InstrumentSSG::setToneNoiseNumber(int n)
{
	tnNum_ = n;
}

int InstrumentSSG::getToneNoiseNumber() const
{
	return tnNum_;
}

std::vector<CommandInSequence> InstrumentSSG::getToneNoiseSequence() const
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

void InstrumentSSG::setEnvelopeNumber(int n)
{
	envNum_ = n;
}

int InstrumentSSG::getEnvelopeNumber() const
{
	return envNum_;
}

std::vector<CommandInSequence> InstrumentSSG::getEnvelopeSequence() const
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

void InstrumentSSG::setArpeggioNumber(int n)
{
	arpNum_ = n;
}

int InstrumentSSG::getArpeggioNumber() const
{
	return arpNum_;
}

int InstrumentSSG::getArpeggioType() const
{
	return owner_->getArpeggioSSGType(arpNum_);
}

std::vector<CommandInSequence> InstrumentSSG::getArpeggioSequence() const
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

void InstrumentSSG::setPitchNumber(int n)
{
	ptNum_ = n;
}

int InstrumentSSG::getPitchNumber() const
{
	return ptNum_;
}

int InstrumentSSG::getPitchType() const
{
	return owner_->getPitchSSGType(ptNum_);
}

std::vector<CommandInSequence> InstrumentSSG::getPitchSequence() const
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
