/*
 * Copyright (C) 2018-2022 Rerrah
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
#include "note.hpp"
#include <cstddef>
#include "utils.hpp"

namespace
{
const InstrumentSequenceBaseUnit ARP_CENTER(Note::DEFAULT_NOTE_NUM);
}

ArpeggioEffectIterator::ArpeggioEffectIterator(int second, int third)
	: SequenceIteratorInterface<InstrumentSequenceBaseUnit>(2),
	  started_(false),
	  second_(second + ARP_CENTER.data),
	  third_(third + ARP_CENTER.data)
{
}

InstrumentSequenceBaseUnit ArpeggioEffectIterator::data() const noexcept
{
	switch (pos_) {
	case 0:	return ARP_CENTER;
	case 1:	return second_;
	case 2:	return third_;
	default:	return InstrumentSequenceBaseUnit();
	}
}

int ArpeggioEffectIterator::next()
{
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
	pos_ = END_SEQ_POS;
	started_ = false;
	return END_SEQ_POS;
}

WavingEffectIterator::WavingEffectIterator(int period, int depth)
	: started_(false)
{
	for (int i = 0; i <= period; ++i) {
		seq_.emplace_back(i * depth);
	}
	for (size_t i = static_cast<size_t>(period - 1); i > 0; --i) {
		seq_.push_back(seq_.at(i));
	}
	size_t p2 = static_cast<size_t>(period) << 1;
	for (size_t i = 0; i < p2; ++i) {
		seq_.emplace_back(-seq_.at(i).data);
	}

	pos_ = static_cast<int>(seq_.size()) - 1;
}

InstrumentSequenceBaseUnit WavingEffectIterator::data() const
{
	return (hasEnded() ? InstrumentSequenceBaseUnit() : seq_.at(static_cast<size_t>(pos_)));
}

int WavingEffectIterator::next()
{
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
	pos_ = END_SEQ_POS;
	started_ = false;
	return END_SEQ_POS;
}

NoteSlideEffectIterator::NoteSlideEffectIterator(int speed, int seminote)
	: SequenceIteratorInterface<InstrumentSequenceBaseUnit>(0),
	  started_(false)
{
	int d = seminote * Note::SEMINOTE_PITCH;
	if (speed) {
		int prev = 0;
		for (int i = 0; i <= speed; ++i) {
			int dif = d * i / speed - prev;
			seq_.emplace_back(dif);
			prev += dif;
		}
	}
	else {
		seq_.emplace_back(d);
	}
}

InstrumentSequenceBaseUnit NoteSlideEffectIterator::data() const
{
	return (hasEnded() ? InstrumentSequenceBaseUnit() : seq_.at(static_cast<size_t>(pos_)));
}

int NoteSlideEffectIterator::next()
{
	if (started_ && !hasEnded()) {
		if (++pos_ >= static_cast<int>(seq_.size())) pos_ = END_SEQ_POS;
	}
	else {
		started_ = true;
	}
	return pos_;
}

int NoteSlideEffectIterator::front()
{
	pos_ = 0;
	started_ = true;
	return 0;
}

int NoteSlideEffectIterator::end()
{
	pos_ = END_SEQ_POS;
	started_ = false;
	return END_SEQ_POS;
}

XVolumeSlideEffectIterator::XVolumeSlideEffectIterator(int factor, int cycleCount)
	: SequenceIteratorInterface<InstrumentSequenceBaseUnit>(END_SEQ_POS),
	  mem_(0),
	  sum_(0),
	  factor_(factor),
	  cycleCount_(cycleCount)
{
}

InstrumentSequenceBaseUnit XVolumeSlideEffectIterator::data() const noexcept
{
	return InstrumentSequenceBaseUnit(mem_);
}

int XVolumeSlideEffectIterator::next()
{
	if (hasEnded()) {
		pos_ = 0;
		mem_ = 0;
		sum_ = 0;
	}
	else {
		int prevSum = std::exchange(sum_, ++pos_ * factor_ / cycleCount_);
		mem_ = sum_ - prevSum;
		pos_ %= cycleCount_;
		sum_ %= factor_;
	}

	return pos_;
}

int XVolumeSlideEffectIterator::front()
{
	pos_ = 0;
	mem_ = 0;
	sum_ = 0;
	return 0;
}

int XVolumeSlideEffectIterator::end()
{
	pos_ = END_SEQ_POS;
	mem_ = 0;
	sum_ = 0;
	return pos_;
}
