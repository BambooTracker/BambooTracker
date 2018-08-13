#include "step.hpp"

Step::Step()
	: noteNum_(-1), instNum_(-1), vol_(-1), effStr_(u8"---")
{
}

int Step::getNoteNumber() const
{
	return noteNum_;
}

void Step::setNoteNumber(int num)
{
	noteNum_ = num;
}

int Step::getInstrumentNumber() const
{
	return instNum_;
}

void Step::setInstrumentNumber(int num)
{
	instNum_ = num;
}

int Step::getVolume() const
{
	return vol_;
}

void Step::setVolume(int volume)
{
	vol_ = volume;
}

std::string Step::getEffectString() const
{
	return effStr_;
}

void Step::setEffectString(std::string str)
{
	effStr_ = str;
}
