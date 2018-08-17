#include "step.hpp"

Step::Step()
	: noteNum_(-1),
	  instNum_(-1),
	  vol_(-1),
	  effID_("--"),
	  effVal_(-1)
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

std::string Step::getEffectID() const
{
	return effID_;
}

void Step::setEffectID(std::string str)
{
	effID_ = str;
}

int Step::getEffectValue() const
{
	return effVal_;
}

void Step::setEffectValue(int v)
{
	effVal_ = v;
}
