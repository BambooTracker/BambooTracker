#include "instrument.hpp"
#include <utility>

AbstractInstrument::AbstractInstrument(int number, SoundSource source, std::string name, InstrumentsManager* owner)
	: owner_(owner),
	  number_(number),
	  name_(name),
	  source_(source)
{}

int AbstractInstrument::getNumber() const
{
	return number_;
}

void AbstractInstrument::setNumber(int n)
{
	number_ = n;
}

SoundSource AbstractInstrument::getSoundSource() const
{
	return source_;
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
	AbstractInstrument(number, SoundSource::FM, name, owner),
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

std::unique_ptr<AbstractInstrument> InstrumentFM::clone()
{
	std::unique_ptr<InstrumentFM> c = std::make_unique<InstrumentFM>(number_, name_, owner_);
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
	return std::move(c);
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

std::vector<CommandSequenceUnit> InstrumentFM::getOperatorSequenceSequence(FMEnvelopeParameter param) const
{
	return owner_->getOperatorSequenceFMSequence(param, opSeqNum_.at(param));
}

std::vector<Loop> InstrumentFM::getOperatorSequenceLoops(FMEnvelopeParameter param) const
{
	return owner_->getOperatorSequenceFMLoops(param, opSeqNum_.at(param));
}

Release InstrumentFM::getOperatorSequenceRelease(FMEnvelopeParameter param) const
{
	return owner_->getOperatorSequenceFMRelease(param, opSeqNum_.at(param));
}

std::unique_ptr<CommandSequence::Iterator> InstrumentFM::getOperatorSequenceSequenceIterator(FMEnvelopeParameter param) const
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

int InstrumentFM::getArpeggioType(FMOperatorType op) const
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

int InstrumentFM::getPitchType(FMOperatorType op) const
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
	: AbstractInstrument(number, SoundSource::SSG, name, owner),
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

std::unique_ptr<AbstractInstrument> InstrumentSSG::clone()
{
	std::unique_ptr<InstrumentSSG> c = std::make_unique<InstrumentSSG>(number_, name_, owner_);
	c->setWaveFormEnabled(wfEnabled_);
	c->setWaveFormNumber(wfNum_);
	c->setToneNoiseEnabled(tnEnabled_);
	c->setToneNoiseNumber(tnNum_);
	c->setEnvelopeEnabled(envEnabled_);
	c->setEnvelopeNumber(envNum_);
	c->setArpeggioEnabled(arpEnabled_);
	c->setArpeggioNumber(arpNum_);
	c->setPitchEnabled(ptEnabled_);
	c->setPitchNumber(ptNum_);
	return std::move(c);
}

void InstrumentSSG::setWaveFormEnabled(bool enabled)
{
	wfEnabled_ = enabled;
}

bool InstrumentSSG::getWaveFormEnabled() const
{
	return wfEnabled_;
}

void InstrumentSSG::setWaveFormNumber(int n)
{
	wfNum_ = n;
}

int InstrumentSSG::getWaveFormNumber() const
{
	return wfNum_;
}

std::vector<CommandSequenceUnit> InstrumentSSG::getWaveFormSequence() const
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

int InstrumentSSG::getArpeggioType() const
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

int InstrumentSSG::getPitchType() const
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
