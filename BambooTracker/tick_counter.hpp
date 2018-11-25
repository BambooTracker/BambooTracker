#pragma once

#include <cstdint>
#include <cstddef>
#include <vector>

class TickCounter
{
public:
	TickCounter();
	void setInterruptRate(uint32_t rate);
	void setTempo(int tempo);
	void setSpeed(int speed);
	void setGroove(std::vector<int> seq);
	void setGrooveEnebled(bool enabled);
	void setPlayState(bool isPlaySong);
	int countUp();
	void resetCount();

private:
	bool isPlaySong_;
	int tempo_;
	int tickRate_;
	std::vector<int> grooves_;
	int nextGroovePos_;

	size_t defStepSize_;
	size_t restTickToNextStep_;

	float tickDif_;
	float tickDifSum_;

	void updateTickDIf();
	void resetRest();
};
