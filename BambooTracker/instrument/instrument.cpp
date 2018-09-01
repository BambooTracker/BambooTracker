#include "instrument.hpp"
#include <regex>

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

int InstrumentFM::getEnvelopeParameter(FMParameter param) const
{
	return owner_->getEnvelopeFMParameter(envNum_, param);
}

bool InstrumentFM::getOperatorEnable(int n) const
{
	return owner_->getEnvelopeFMOperatorEnable(envNum_, n);
}

void InstrumentFM::setEnvelopeResetEnabled(bool enabled)
{
	envResetEnabled_ = enabled;
}

bool InstrumentFM::getEnvelopeResetEnabled() const
{
	return envResetEnabled_;
}

/****************************************/

InstrumentSSG::InstrumentSSG(int number, std::string name, InstrumentsManager* owner) :
	AbstructInstrument(number, SoundSource::SSG, name, owner)
{
}

std::unique_ptr<AbstructInstrument> InstrumentSSG::clone()
{
	return std::unique_ptr<AbstructInstrument>(std::make_unique<InstrumentSSG>(*this));
}
