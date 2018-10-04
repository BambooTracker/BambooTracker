#include "tick_counter.hpp"

TickCounter::TickCounter() :
	isPlaySong_(false),
	tempo_(150),    // Dummy set
	tickRate_(60),	// NTSC
	defStepSize_(6)    // Dummy set
{
	updateTickDIf();
}

void TickCounter::setInterruptRate(uint32_t rate)
{
	tickRate_ = rate;
	updateTickDIf();
}

void TickCounter::setTempo(int tempo)
{
	tempo_ = tempo;
	updateTickDIf();
	tickDifSum_ = 0;
	resetRest();
}

void TickCounter::setSpeed(int speed)
{
	defStepSize_ = speed;
	updateTickDIf();
	tickDifSum_ = 0;
	resetRest();
}

void TickCounter::setPlayState(bool isPlaySong)
{
	isPlaySong_ = isPlaySong;
}

/// Reuturn:
///		-1: not tick or step
///		 0: head of step
///		0<: rest tick count to next step
int TickCounter::countUp()
{
	int ret;

	if (isPlaySong_) {
		ret = restTickToNextStep_;

		if (!restTickToNextStep_) {  // When head of step, calculate real step size
			resetRest();
		}

		--restTickToNextStep_;   // Count down to next step
	}
	else {
		ret = -1;
	}

	return ret;
}

void TickCounter::updateTickDIf()
{
	float strictTicksPerStepByBpm = 10.0 * tickRate_ * defStepSize_ / (tempo_ << 2);
	tickDif_ = strictTicksPerStepByBpm - static_cast<float>(defStepSize_);
}

void TickCounter::resetCount()
{
	restTickToNextStep_ = 0;
	tickDifSum_ = 0;
}

void TickCounter::resetRest()
{
	tickDifSum_ += tickDif_;
	int castedTickDifSum = static_cast<int>(tickDifSum_);
	restTickToNextStep_ = defStepSize_ + castedTickDifSum;
	tickDifSum_ -= castedTickDifSum;
}
