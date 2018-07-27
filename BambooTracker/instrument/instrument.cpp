#include "instrument.hpp"

AbstructInstrument::AbstructInstrument(int number, SoundSource source, std::string name, InstrumentsManager* owner) :
	owner_(owner), number_(number), source_(source), name_(name) {}

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

/****************************************/

InstrumentFM::InstrumentFM(int number, std::string name, InstrumentsManager* owner) :
	AbstructInstrument(number, SoundSource::FM, name, owner),
	envNum_(0)
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
	return owner_->getFMEnvelopeParameter(envNum_, param);
}

bool InstrumentFM::getOperatorEnable(int n) const
{
	return owner_->getFMOperatorEnable(envNum_, n);
}

/****************************************/

InstrumentPSG::InstrumentPSG(int number, std::string name, InstrumentsManager* owner) :
	AbstructInstrument(number, SoundSource::PSG, name, owner)
{
}

std::unique_ptr<AbstructInstrument> InstrumentPSG::clone()
{
	return std::unique_ptr<AbstructInstrument>(std::make_unique<InstrumentPSG>(*this));
}
