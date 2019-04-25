#include "effect_iterator.hpp"
#include <cstddef>

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
	(void)isReleaseBegin;
	pos_ = (pos_ + 1) % 3;
	return pos_;
}

int ArpeggioEffectIterator::front()
{
	pos_ = 0;
	return 0;
}

int ArpeggioEffectIterator::end()
{
	pos_ = -1;
	return -1;
}

/****************************************/
WavingEffectIterator::WavingEffectIterator(int period, int depth)
{
	for (int i = 0; i <= period; ++i) {
		seq_.push_back(i * depth);
	}
	for (size_t i = static_cast<size_t>(period - 1); i > 0; --i) {
		seq_.push_back(seq_.at(i));
	}
	size_t p2 = static_cast<size_t>(period) << 1;
	for (size_t i = 0; i < p2; ++i) {
		seq_.push_back(-seq_.at(i));
	}

	pos_ = static_cast<int>(seq_.size()) - 1;
}

int WavingEffectIterator::getPosition() const
{
	return pos_;
}

int WavingEffectIterator::getSequenceType() const
{
	return 0;
}

int WavingEffectIterator::getCommandType() const
{
	return seq_.at(static_cast<size_t>(pos_));
}

int WavingEffectIterator::getCommandData() const
{
	return -1;
}

int WavingEffectIterator::next(bool isReleaseBegin)
{
	(void)isReleaseBegin;
	pos_ = (pos_ + 1) % static_cast<int>(seq_.size());
	return pos_;
}

int WavingEffectIterator::front()
{
	pos_ = 0;
	return 0;
}

int WavingEffectIterator::end()
{
	pos_ = -1;
	return -1;
}

/****************************************/
NoteSlideEffectIterator::NoteSlideEffectIterator(int speed, int seminote)
{
	int d = seminote * 32;
	int prev = 0;
	for (int i = 0; i <= speed; ++i) {
		int dif = d * i / speed - prev;
		seq_.push_back(dif);
		prev += dif;
	}
	pos_ = 0;
}

int NoteSlideEffectIterator::getPosition() const
{
	return pos_;
}

int NoteSlideEffectIterator::getSequenceType() const
{
	return 0;
}

int NoteSlideEffectIterator::getCommandType() const
{
	return seq_.at(static_cast<size_t>(pos_));
}

int NoteSlideEffectIterator::getCommandData() const
{
	return -1;
}

int NoteSlideEffectIterator::next(bool isReleaseBegin)
{
	(void)isReleaseBegin;
	return (++pos_ < static_cast<int>(seq_.size())) ? pos_ : -1;
}

int NoteSlideEffectIterator::front()
{
	pos_ = 0;
	return 0;
}

int NoteSlideEffectIterator::end()
{
	pos_ = -1;
	return -1;
}
