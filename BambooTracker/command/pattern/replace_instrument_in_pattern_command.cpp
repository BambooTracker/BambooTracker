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

#include "replace_instrument_in_pattern_command.hpp"
#include "pattern_command_utils.hpp"

ReplaceInstrumentInPatternCommand::ReplaceInstrumentInPatternCommand(std::weak_ptr<Module> mod,
																	 int songNum, int beginTrack,
																	 int beginOrder, int beginStep,
																	 int endTrack, int endStep, int newInst)
	: AbstractCommand(CommandId::ReplaceInstrumentInPattern),
	  mod_(mod),
	  song_(songNum),
	  bTrack_(beginTrack),
	  order_(beginOrder),
	  bStep_(beginStep),
	  eTrack_(endTrack),
	  eStep_(endStep),
	  inst_(newInst)
{
	auto& sng = mod.lock()->getSong(songNum);

	for (int step = beginStep; step <= endStep; ++step) {
		for (int track = beginTrack; track <= endTrack; ++track) {
			Step& st = command_utils::getStep(sng, track, beginOrder, step);
			if (st.hasInstrument()) prevInsts_.push_back(st.getInstrumentNumber());
		}
	}
}

void ReplaceInstrumentInPatternCommand::redo()
{
	auto& sng = mod_.lock()->getSong(song_);

	for (int step = bStep_; step <= eStep_; ++step) {
		for (int track = bTrack_; track <= eTrack_; ++track) {
			Step& st = command_utils::getStep(sng, track, order_, step);
			if (st.hasInstrument()) st.setInstrumentNumber(inst_);
		}
	}
}

void ReplaceInstrumentInPatternCommand::undo()
{
	auto& sng = mod_.lock()->getSong(song_);

	size_t i = 0;
	for (int step = bStep_; step <= eStep_; ++step) {
		for (int track = bTrack_; track <= eTrack_; ++track) {
			Step& st = command_utils::getStep(sng, track, order_, step);
			if (st.hasInstrument()) st.setInstrumentNumber(prevInsts_.at(i));
		}
	}
}
