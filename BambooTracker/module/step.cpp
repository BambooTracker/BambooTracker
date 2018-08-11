#include "step.hpp"

Step::Step()
	: noteNum_(-1), instNum_(-1), vol_(-1), effStr_("---")
{
}

int Step::getNoteNumber() const
{
	return noteNum_;
}

int Step::getInstrumentNumber() const
{
	return instNum_;
}

int Step::getVolume() const
{
	return vol_;
}

std::string Step::getEffectString() const
{
	return effStr_;
}
