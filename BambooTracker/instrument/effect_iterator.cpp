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

/****************************************/
VibratoEffectIterator::VibratoEffectIterator(int period, int depth)
{
	for (int i = 0; i <= period; ++i) {
		seq_.push_back(i * depth);
	}
	for (int i = period - 1; i > 0; --i) {
		seq_.push_back(seq_.at(i));
	}
	int p2 = period << 1;
	for (int i = 0; i < p2; ++i) {
		seq_.push_back(-seq_.at(i));
	}

	pos_ = seq_.size() - 1;
}

int VibratoEffectIterator::getPosition() const
{
	return pos_;
}

int VibratoEffectIterator::getSequenceType() const
{
	return 0;
}

int VibratoEffectIterator::getCommandType() const
{
	return seq_.at(pos_);
}

int VibratoEffectIterator::getCommandData() const
{
	return -1;
}

int VibratoEffectIterator::next(bool isReleaseBegin)
{
	pos_ = (pos_ + 1) % seq_.size();
	return pos_;
}
int VibratoEffectIterator::front()
{
	pos_ = 0;
	return 0;
}
