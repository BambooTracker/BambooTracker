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
