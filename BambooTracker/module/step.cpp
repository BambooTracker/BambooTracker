#include "step.hpp"

Step::Step()
	: noteNum_(-1),
	  instNum_(-1),
	  vol_(-1)
{
	for (size_t i = 0; i < 4; ++i) {
		effID_[i] = "--";
		effVal_[i] = -1;
	}
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

std::string Step::getEffectID(int n) const
{
	return effID_[n];
}

void Step::setEffectID(int n, std::string str)
{
	effID_[n] = str;
}

int Step::getEffectValue(int n) const
{
	return effVal_[n];
}

void Step::setEffectValue(int n, int v)
{
	effVal_[n] = v;
}

int Step::checkEffectID(std::string str) const
{
	for (int i = 0; i < 4; ++i) {
		if (effID_[i] == str && effVal_[i] != -1) return i;
	}
	return -1;
}
