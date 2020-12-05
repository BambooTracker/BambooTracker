/*
 * Copyright (C) 2020 Rerrah
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

#include "swap_instruments_command.hpp"

SwapInstrumentsCommand::SwapInstrumentsCommand(std::weak_ptr<InstrumentsManager> manager,
											   std::weak_ptr<Module> mod, int inst1, int inst2,
											   int song, bool patternChange)
	: AbstractCommand(CommandId::SwapInstruments),
	  manager_(manager),
	  mod_(mod),
	  inst1Num_(inst1),
	  inst2Num_(inst2),
	  songNum_(song),
	  patternChange_(patternChange)
{
}

void SwapInstrumentsCommand::redo()
{
	manager_.lock()->swapInstruments(inst1Num_, inst2Num_);
	if (patternChange_) swapInstrumentsInPatterns();
}

void SwapInstrumentsCommand::undo()
{
	manager_.lock()->swapInstruments(inst1Num_, inst2Num_);
	if (patternChange_) swapInstrumentsInPatterns();
}

void SwapInstrumentsCommand::swapInstrumentsInPatterns()
{
	// NOTE: Is it better to execute this as the method of Song to use global replace action?
	// Too slow...
	Song& song = mod_.lock()->getSong(songNum_);
	for (const auto& attrib : song.getStyle().trackAttribs) {
		Track& track = song.getTrack(attrib.number);
		for (int i = 0; i < 256; ++i) {	// Used track size
			Pattern& pat = track.getPattern(i);
			for (size_t j = 0; j < pat.getSize(); ++j) {
				Step& step = pat.getStep(static_cast<int>(j));
				if (step.getInstrumentNumber() == inst1Num_) step.setInstrumentNumber(inst2Num_);
				else if (step.getInstrumentNumber() == inst2Num_) step.setInstrumentNumber(inst1Num_);
			}
		}
	}
}
