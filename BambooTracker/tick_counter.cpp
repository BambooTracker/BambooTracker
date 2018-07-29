#include "tick_counter.hpp"

TickCounter::TickCounter() :
	isPlaySong_(false),
	tickRate_(60)	// NTSC
{
}

void TickCounter::setChipRate(uint32_t rate)
{
	rate_ = rate;
	tickIntrCount_ = rate / tickRate_;
}

void TickCounter::setInterruptRate(uint32_t rate)
{
	tickRate_ = rate;
	tickIntrCount_ = rate_ / rate;
}

void TickCounter::setTempo(int tempo)
{
	tempo_ = tempo;
}

void TickCounter::setStepSize(size_t size)
{
	specificTicksPerStep_ = size;
}

void TickCounter::setPlayState(bool isPlaySong)
{
	isPlaySong_ = isPlaySong;
}

/// Reuturn:
///		-1: not tick or step
///		 0: step
///		0<: rest tick count to next step
int TickCounter::countUp()
{
	int ret = 0;

	if (isPlaySong_) {
		if (executingTicksPerStep_) {   //  Read by tick
		}
		else {  // Read by step (first tick in step)

			// Dummy set reading speed
			specificTicksPerStep_ = 6;
			tempo_ = 150;

			// Calculate executing ticks in step
			{
				strictTicksPerStepByBpm_ = 10.0 * tickRate_ * specificTicksPerStep_ / (tempo_ << 2);
				tickDifSum_ += strictTicksPerStepByBpm_ - static_cast<float>(specificTicksPerStep_);
				int castedTickDifSum = static_cast<int>(tickDifSum_);
				executingTicksPerStep_ = specificTicksPerStep_ + castedTickDifSum;
				tickDifSum_ -= castedTickDifSum;
			}
		}
		ret = executingTicksPerStep_;

		--executingTicksPerStep_;   // Count down to next step
	}
	else {
		ret = -1;
	}

	return ret;
}
