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

#include "shrink_pattern_command.hpp"
#include "pattern_command_utils.hpp"

ShrinkPatternCommand::ShrinkPatternCommand(
		std::weak_ptr<Module> mod, int songNum, int beginTrack, int beginColumn,
		int beginOrder, int beginStep, int endTrack, int endColumn, int endStep)
	: AbstractCommand(CommandId::ShrinkPattern),
	  mod_(mod),
	  song_(songNum),
	  bTrack_(beginTrack),
	  bCol_(beginColumn),
	  order_(beginOrder),
	  bStep_(beginStep),
	  eStep_(endStep)
{
	auto& song = mod.lock()->getSong(songNum);
	size_t h = static_cast<size_t>(endStep - beginStep + 1);
	size_t w = command_utils::calculateColumnSize(beginTrack, beginColumn, endTrack, endColumn);
	prevCells_ = command_utils::getPreviousCells(song, w, h, beginTrack, beginColumn, beginOrder, beginStep);
}

void ShrinkPatternCommand::redo()
{
	auto& sng = mod_.lock()->getSong(song_);

	int s = bStep_;
	for (size_t i = 0; i < prevCells_.size(); i += 2) {
		int t = bTrack_;
		int c = bCol_;
		for (size_t j = 0; j < prevCells_.at(i).size(); ++j) {
			Step& st = command_utils::getStep(sng, t, order_, s);
			switch (c) {
			case 0:		st.setNoteNumber(std::stoi(prevCells_.at(i).at(j)));		break;
			case 1:		st.setInstrumentNumber(std::stoi(prevCells_.at(i).at(j)));	break;
			case 2:		st.setVolume(std::stoi(prevCells_.at(i).at(j)));			break;
			default:
			{
				int ec = c - 3;
				int ei = ec / 2;
				if (ec % 2) st.setEffectValue(ei, std::stoi(prevCells_.at(i).at(j)));
				else st.setEffectId(ei, prevCells_.at(i).at(j));
				break;
			}
			}

			t += (++c / Step::N_COLUMN);
			c %= Step::N_COLUMN;
		}

		++s;
	}

	for (; s <= eStep_; ++s) {
		int t = bTrack_;
		int c = bCol_;
		for (size_t j = 0; j < prevCells_.at(0).size(); ++j) {
			Step& st = command_utils::getStep(sng, t, order_, s);
			switch (c) {
			case 0:		st.clearNoteNumber();		break;
			case 1:		st.clearInstrumentNumber();	break;
			case 2:		st.clearVolume();			break;
			default:
			{
				int ec = c - 3;
				int ei = ec / 2;
				if (ec % 2) st.clearEffectValue(ei);
				else st.clearEffectId(ei);
				break;
			}
			}

			t += (++c / Step::N_COLUMN);
			c %= Step::N_COLUMN;
		}
	}
}

void ShrinkPatternCommand::undo()
{
	command_utils::restorePattern(mod_.lock()->getSong(song_), prevCells_, bTrack_, bCol_, order_, bStep_);
}
