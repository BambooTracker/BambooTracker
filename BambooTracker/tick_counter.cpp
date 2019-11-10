#include "tick_counter.hpp"

TickCounter::TickCounter() :
	isPlaySong_(false),
	tempo_(150),    // Dummy set
	tickRate_(60),	// NTSC
	nextGroovePos_(-1),
	defStepSize_(6)    // Dummy set
{
	updateTickDIf();
}

void TickCounter::setInterruptRate(uint32_t rate)
{
	tickRate_ = static_cast<int>(rate);
	updateTickDIf();
}

void TickCounter::setTempo(int tempo)
{
	tempo_ = tempo;
	updateTickDIf();
	tickDifSum_ = 0;
	resetRest();
}

int TickCounter::getTempo() const
{
	return tempo_;
}

void TickCounter::setSpeed(int speed)
{
	defStepSize_ = speed;
	updateTickDIf();
	tickDifSum_ = 0;
	resetRest();
}

int TickCounter::getSpeed() const
{
	return defStepSize_;
}

void TickCounter::setGroove(std::vector<int> seq)
{
	grooves_ = seq;
	if (nextGroovePos_ != -1) nextGroovePos_ = 0;
}

void TickCounter::setGrooveTrigger(GrooveTrigger trigger)
{
	switch (trigger) {
	case GrooveTrigger::ValidByGlobal:
		nextGroovePos_ = static_cast<int>(grooves_.size()) - 1;
		resetRest();
		break;
	case GrooveTrigger::ValidByLocal:
		nextGroovePos_ = 0;
		resetRest();
		--restTickToNextStep_;	// Count down by step head
		break;
	case GrooveTrigger::Invalid:
		nextGroovePos_ = -1;
		resetRest();
		break;
	}
}

bool TickCounter::getGrooveEnabled() const
{
	return (nextGroovePos_ != -1);
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
	if (isPlaySong_) {
		int ret = restTickToNextStep_;

		if (!restTickToNextStep_) {  // When head of step, calculate real step size
			resetRest();
		}

		--restTickToNextStep_;   // Count down to next step

		return ret;
	}
	else {
		return -1;
	}
}

void TickCounter::updateTickDIf()
{
	float strictTicksPerStepByBpm = 10.0f * tickRate_ * defStepSize_ / (tempo_ << 2);
	tickDif_ = strictTicksPerStepByBpm - static_cast<float>(defStepSize_);
}

void TickCounter::resetCount()
{
	restTickToNextStep_ = 0;
	tickDifSum_ = 0;
}

void TickCounter::resetRest()
{
	if (nextGroovePos_ == -1) {
		tickDifSum_ += tickDif_;
		int castedTickDifSum = static_cast<int>(tickDifSum_);
		restTickToNextStep_ = defStepSize_ + castedTickDifSum;
		if (restTickToNextStep_ < 1) restTickToNextStep_ = 1;	// Prevent wait count changing to 0 (freeze)
		tickDifSum_ -= castedTickDifSum;
	}
	else {
		restTickToNextStep_ = grooves_.at(static_cast<size_t>(nextGroovePos_));
		nextGroovePos_ = (nextGroovePos_ + 1) % static_cast<int>(grooves_.size());
	}
}
