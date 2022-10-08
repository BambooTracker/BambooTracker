/*
 * Copyright (C) 2018-2021 Rerrah
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

#include "pattern.hpp"
#include <algorithm>
#include "effect.hpp"
#include "note.hpp"
#include "utils.hpp"

namespace
{
constexpr size_t MAX_STEP_SIZE = 256;
}

Pattern::Pattern(int n, size_t defSize)
	: num_(n), size_(defSize), steps_(defSize), usedCnt_(0)
{
}

Pattern::Pattern(int n, size_t size, const std::vector<Step>& steps)
	: num_(n), size_(size), steps_(steps), usedCnt_(0)
{
}

Step& Pattern::getStep(int n)
{
	return steps_.at(static_cast<size_t>(n));
}

size_t Pattern::getSize() const
{
	for (size_t i = 0; i < size_; ++i) {
		for (int j = 0; j < Step::N_EFFECT; ++j) {
			if (!steps_[i].hasEffectValue(j)) continue;
			// "SoundSource::FM" is dummy, these effects are not related with sound source
			switch (effect_utils::validateEffectId(SoundSource::FM, steps_[i].getEffectId(j))) {
			case EffectType::PositionJump:
			case EffectType::SongEnd:
			case EffectType::PatternBreak:
				return i + 1;
			default:
				break;
			}
		}
	}
	return size_;
}

void Pattern::changeSize(size_t size)
{
	if (size && size <= MAX_STEP_SIZE) {
		size_ = size;
		if (steps_.size() < size) steps_.resize(size);
	}
}

void Pattern::insertStep(int n)
{
	if (n < static_cast<int>(size_))
		steps_.emplace(steps_.begin() + n);
}

void Pattern::deletePreviousStep(int n)
{
	if (!n) return;

	steps_.erase(steps_.begin() + n - 1);
	if (steps_.size() < size_)
		steps_.resize(size_);
}

bool Pattern::hasEvent() const
{
	auto endIt = steps_.cbegin() + static_cast<int>(size_);
	return std::any_of(steps_.cbegin(), endIt,
					   [](const Step& step) { return step.hasEvent(); });
}

std::vector<int> Pattern::getEditedStepIndices() const
{
	auto endIt = steps_.cbegin() + static_cast<int>(size_);
	return utils::findIndicesIf(steps_.cbegin(), endIt,
								[](const Step& step) { return step.hasEvent(); });
}

std::set<int> Pattern::getRegisteredInstruments() const
{
	std::set<int> set;
	for (size_t i = 0; i < size_; ++i) {
		const Step& step = steps_.at(i);
		if (step.hasInstrument()) set.insert(step.getInstrumentNumber());
	}
	return set;
}

Pattern Pattern::clone(int asNumber)
{
	return Pattern(asNumber, size_, steps_);
}

void Pattern::transpose(int semitones, const std::vector<int>& excludeInsts)
{
	for (size_t i = 0; i < size_; ++i) {
		Step& step = steps_.at(i);
		int note = step.getNoteNumber();
		if (step.hasGeneralNote() && std::none_of(excludeInsts.begin(), excludeInsts.end(),
									  [a = step.getInstrumentNumber()](int b) { return a == b; })) {
			step.setNoteNumber(utils::clamp(note + semitones, 0, Note::NOTE_NUMBER_RANGE - 1));
		}
	}
}

void Pattern::clear()
{
	steps_ = std::vector<Step>(size_);
}
