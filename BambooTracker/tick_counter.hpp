#pragma once

#include <cstdint>
#include <cstddef>
#include <vector>

enum class GrooveTrigger
{
	ValidByGlobal,
	ValidByLocal,
	Invalid
};

class TickCounter
{
public:
	TickCounter();
	void setInterruptRate(uint32_t rate);
	void setTempo(int tempo);
	int getTempo() const;
	void setSpeed(int speed);
	int getSpeed() const;
	void setGroove(std::vector<int> seq);
	void setGrooveTrigger(GrooveTrigger trigger);
	bool getGrooveEnabled() const;
	void setPlayState(bool isPlaySong);
	int countUp();
	void resetCount();

private:
	bool isPlaySong_;
	int tempo_;
	int tickRate_;
	std::vector<int> grooves_;
	int nextGroovePos_;

	int defStepSize_;
	int restTickToNextStep_;

	float tickDif_;
	float tickDifSum_;

	void updateTickDIf();
	void resetRest();
};
