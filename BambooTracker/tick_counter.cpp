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

#include "tick_counter.hpp"

TickCounter::TickCounter() :
	isPlaySong_(false),
	tempo_(150),    // Dummy set
	tickRate_(60),	// NTSC
	nextGroovePos_(-1),
	defStepSize_(6),	// Dummy set
	curStepSize_(6),	// Dummy set
	restTickToNextStep_(0),
	tickDiff_(0.f),
	tickDiffSum_(0.f),
	prevTickDiffSum_(0.f)
{
	updateTickDifference();
}

void TickCounter::setInterruptRate(uint32_t rate)
{
	tickRate_ = static_cast<int>(rate);
	updateTickDifference();
}

void TickCounter::setTempo(int tempo)
{
	tempo_ = tempo;
	updateTickDifference();
	resetRest();
}

int TickCounter::getTempo() const noexcept
{
	return tempo_;
}

void TickCounter::setSpeed(int speed)
{
	defStepSize_ = speed;
	updateTickDifference();
	// Changing speed does not reset tempo, so don't reset tickDiffSum_ either.
	resetRest();
}

int TickCounter::getSpeed() const noexcept
{
	return defStepSize_;
}

void TickCounter::setGroove(const std::vector<int>& seq)
{
	grooves_ = seq;
	if (nextGroovePos_ != -1) nextGroovePos_ = 0;
}

void TickCounter::setGrooveState(GrooveState state)
{
	switch (state) {
	case GrooveState::ValidByGlobal:
		nextGroovePos_ = 0;
		if (isPlaySong_) {
			curStepSize_ = grooves_.at(0);
			restTickToNextStep_ = curStepSize_ - 1;
		}
		break;
	case GrooveState::ValidByLocal:
		nextGroovePos_ = 0;
		resetRest();
		break;
	case GrooveState::Invalid:
		nextGroovePos_ = -1;
		resetRest();
		break;
	}

	// When enabling groove (which disables tempo), reset tempo accumulator.
	if (state == GrooveState::ValidByGlobal || state == GrooveState::ValidByLocal) {
		tickDiffSum_ = 0.f;
		prevTickDiffSum_ = 0.f;
	}
}

bool TickCounter::getGrooveEnabled() const noexcept
{
	return (nextGroovePos_ != -1);
}

void TickCounter::setPlayState(bool isPlaySong) noexcept
{
	isPlaySong_ = isPlaySong;
}

int TickCounter::countUp()
{
	prevTickDiffSum_ = tickDiffSum_;
	if (isPlaySong_) {
		int ret = restTickToNextStep_;

		if (!restTickToNextStep_) {  // When head of step, calculate real step size
			resetRest();
		}
		else {
			--restTickToNextStep_;	 // Count down to next step
		}

		return ret;
	}
	else {
		return -1;
	}
}

void TickCounter::updateTickDifference()
{
	float strictTicksPerStepByBpm = 10.0f * tickRate_ * defStepSize_ / (tempo_ << 2);
	tickDiff_ = strictTicksPerStepByBpm - static_cast<float>(defStepSize_);
}

void TickCounter::resetCount()
{
	restTickToNextStep_ = 0;
	tickDiffSum_ = 0;
	prevTickDiffSum_ = 0;
}

// MUST be idempotent when no groove is active.
void TickCounter::resetRest()
{
	if (nextGroovePos_ == -1) {
		tickDiffSum_ = prevTickDiffSum_ + tickDiff_;
		int castedTickDifSum = static_cast<int>(tickDiffSum_);
		restTickToNextStep_ = defStepSize_ + castedTickDifSum;
		tickDiffSum_ -= castedTickDifSum;
	}
	else {
		restTickToNextStep_ = grooves_.at(static_cast<size_t>(nextGroovePos_));
		nextGroovePos_ = (nextGroovePos_ + 1) % static_cast<int>(grooves_.size());
	}
	curStepSize_ = restTickToNextStep_;

	// Ensure each row lasts for at least 1 tick, and that we can subtract 1 safely.
	if (restTickToNextStep_ < 1) curStepSize_ = restTickToNextStep_ = 1;

	// If resetRest() is called in response to the tracker processing a row or Fxx/Oxx event,
	// subtract 1 so countUp() will remain on the row for (speed - 1) subsequent ticks.
	if (isPlaySong_) {
		restTickToNextStep_--;
	}
}

int TickCounter::getCountsInCurrentStep() const noexcept
{
	return curStepSize_;
}
