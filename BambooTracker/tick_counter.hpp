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

enum class GrooveState
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
	int getTempo() const noexcept;
	void setSpeed(int speed);
	int getSpeed() const noexcept;
	void setGroove(const std::vector<int>& seq);
	void setGrooveState(GrooveState state);
	bool getGrooveEnabled() const noexcept;
	void setPlayState(bool isPlaySong) noexcept;
	/// Reuturn:
	///		-1: not tick or step
	///		 0: head of step
	///		0<: rest tick count to next step
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

	float tickDiff_;
	float tickDiffSum_;

	void updateTickDifference();
	void resetRest();
};
