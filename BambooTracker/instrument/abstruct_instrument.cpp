#include "abstruct_instrument.hpp"

AbstructInstrument::AbstructInstrument(int number, SoundSource source, std::string name) :
	number_(number), source_(source), name_(name) {}

int AbstructInstrument::getNumber() const
{
	return number_;
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
