#include "effect_iterator.hpp"

ArpeggioEffectIterator::ArpeggioEffectIterator(int second, int third)
	: pos_(2),
	  second_(second + 48),
	  third_(third + 48)
{
}

int ArpeggioEffectIterator::getPosition() const
{
	return pos_;
}

int ArpeggioEffectIterator::getSequenceType() const
{
	return 0;
}

int ArpeggioEffectIterator::getCommandType() const
{
	switch (pos_) {
	case 0:	return 48;
	case 1:	return second_;
	case 2:	return third_;
	default:	return -1;
	}
}

int ArpeggioEffectIterator::getCommandData() const
{
	return -1;
}

int ArpeggioEffectIterator::next(bool isReleaseBegin)
{
	pos_ = (pos_ + 1) % 3;
	return pos_;
}
int ArpeggioEffectIterator::front()
{
	pos_ = 0;
	return 0;
}
