#pragma once

#include <cstdint>
#include <cstddef>

class TickCounter
{
public:
	TickCounter();
	void setChipRate(uint32_t rate);
	void setInterruptRate(uint32_t rate);
	void setTempo(int tempo);
	void setStepSize(size_t size);
	void setPlayState(bool isPlaySong);
	int countUp();

private:
	size_t rate_;
	bool isPlaySong_;
	size_t tempo_;

	size_t tickRate_;
	size_t tickIntrCount_;

	size_t specificTicksPerStep_;
	size_t executingTicksPerStep_;
	size_t tickCount_;
	float strictTicksPerStepByBpm_;
	float tickDifSum_;
};
