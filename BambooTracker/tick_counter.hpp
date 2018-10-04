#pragma once

#include <cstdint>
#include <cstddef>

class TickCounter
{
public:
	TickCounter();
	void setInterruptRate(uint32_t rate);
	void setTempo(int tempo);
	void setSpeed(int speed);
	void setPlayState(bool isPlaySong);
	int countUp();
	void resetCount();

private:
	bool isPlaySong_;
	size_t tempo_;
	size_t tickRate_;

	size_t defStepSize_;
	size_t restTickToNextStep_;

	float tickDif_;
	float tickDifSum_;

	void updateTickDIf();
	void resetRest();
};
