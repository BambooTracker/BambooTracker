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

#include "paste_mix_copied_data_to_pattern_command.hpp"
#include "pattern_command_utils.hpp"

PasteMixCopiedDataToPatternCommand::PasteMixCopiedDataToPatternCommand(std::weak_ptr<Module> mod, int songNum,
																	   int beginTrack, int beginColumn,
																	   int beginOrder, int beginStep,
																	   const std::vector<std::vector<std::string> >& cells)
	: AbstractCommand(CommandId::PasteMixCopiedDataToPattern),
	  mod_(mod),
	  song_(songNum),
	  track_(beginTrack),
	  col_(beginColumn),
	  order_(beginOrder),
	  step_(beginStep),
	  cells_(cells)
{
	auto& song = mod.lock()->getSong(songNum);
	prevCells_ = command_utils::getPreviousCells(song, cells.front().size(), cells.size(),
												 beginTrack, beginColumn, beginOrder, beginStep);
}

void PasteMixCopiedDataToPatternCommand::redo()
{
	auto& sng = mod_.lock()->getSong(song_);

	int s = step_;
	for (const auto& row : cells_) {
		int t = track_;
		int c = col_;
		for (const std::string& cell : row) {
			Step& step = command_utils::getStep(sng, t, order_, s);
			switch (c) {
			case 0:
			{
				int n = std::stoi(cell);
				if (!Step::testEmptyNote(n) && step.isEmptyNote()) step.setNoteNumber(n);
				break;
			}
			case 1:
			{
				int n = std::stoi(cell);
				if (!Step::testEmptyInstrument(n) && !step.hasInstrument()) step.setInstrumentNumber(n);
				break;
			}
			case 2:
			{
				int vol = std::stoi(cell);
				if (!Step::testEmptyVolume(vol) && !step.hasVolume()) step.setVolume(vol);
				break;
			}
			default:
			{
				int ec = c - 3;
				int ei = ec / 2;
				if (ec % 2) {	// Value
					int val = std::stoi(cell);
					if (!Step::testEmptyEffectValue(val) && !step.hasEffectValue(ei)) step.setEffectValue(ei, val);
				}
				else {	// ID
					if (!Step::testEmptyEffectId(cell) && !step.hasEffectId(ei)) step.setEffectId(ei, cell);
				}
				break;
			}
			}

			++c;
			t += (c / Step::N_COLUMN);
			c %= Step::N_COLUMN;
		}

		++s;
	}
}

void PasteMixCopiedDataToPatternCommand::undo()
{
	command_utils::restorePattern(mod_.lock()->getSong(song_), prevCells_, track_, col_, order_, step_);
}
