/*
 * Copyright (C) 2018-2020 Rerrah
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#include "effect_iterator.hpp"
#include "pitch_converter.hpp"
#include <cstddef>

ArpeggioEffectIterator::ArpeggioEffectIterator(int second, int third)
	: pos_(2),
	  started_(false),
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

	if (started_) {
		pos_ = (pos_ + 1) % 3;
	}
	else {
		started_ = true;
	}

	return pos_;
}

int ArpeggioEffectIterator::front()
{
	pos_ = 0;
	started_ = true;
	return 0;
}

int ArpeggioEffectIterator::end()
{
	pos_ = -1;
	started_ = false;
	return -1;
}

/****************************************/
WavingEffectIterator::WavingEffectIterator(int period, int depth)
	: started_(false)
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

	if (started_) {
		pos_ = (pos_ + 1) % static_cast<int>(seq_.size());
	}
	else {
		started_ = true;
	}

	return pos_;
}

int WavingEffectIterator::front()
{
	pos_ = 0;
	started_ = true;
	return 0;
}

int WavingEffectIterator::end()
{
	pos_ = -1;
	started_ = false;
	return -1;
}

/****************************************/
NoteSlideEffectIterator::NoteSlideEffectIterator(int speed, int seminote)
	: started_(false)
{
	int d = seminote * PitchConverter::SEMINOTE_PITCH;
	if (speed) {
		int prev = 0;
		for (int i = 0; i <= speed; ++i) {
			int dif = d * i / speed - prev;
			seq_.push_back(dif);
			prev += dif;
		}
	}
	else {
		seq_.push_back(d);
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

	if (started_) {
		return (++pos_ < static_cast<int>(seq_.size())) ? pos_ : -1;
	}
	else {
		started_ = true;
		return pos_;
	}
}

int NoteSlideEffectIterator::front()
{
	pos_ = 0;
	started_ = true;
	return 0;
}

int NoteSlideEffectIterator::end()
{
	pos_ = -1;
	started_ = false;
	return -1;
}
