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

#include "interpolate_pattern_command.hpp"
#include "pattern_command_utils.hpp"

namespace
{
inline int interp(int a, int b, size_t t, int div)
{
	return a + (b - a) * static_cast<int>(t) / div;
}
}

InterpolatePatternCommand::InterpolatePatternCommand(std::weak_ptr<Module> mod,
													 int songNum, int beginTrack, int beginColumn,
													 int beginOrder, int beginStep,
													 int endTrack, int endColumn, int endStep)
	: AbstractCommand(CommandId::InterpolatePattern),
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

void InterpolatePatternCommand::redo()
{
	auto& sng = mod_.lock()->getSong(song_);
	int div = static_cast<int>(prevCells_.size()) - 1;
	if (!div) div = 1;

	int t = bTrack_;
	int c = bCol_;
	for (size_t i = 0; i < prevCells_.front().size(); ++i) {
		int s = bStep_;
		for (size_t j = 0; j < prevCells_.size(); ++j) {
			Pattern& pattern = command_utils::getPattern(sng, t, order_);
			Step& sa = pattern.getStep(bStep_);
			Step& sb = pattern.getStep(eStep_);
			switch (c) {
			case 0:
			{
				if (sa.hasGeneralNote() && sb.hasGeneralNote())
					pattern.getStep(s).setNoteNumber(
								interp(sa.getNoteNumber(), sb.getNoteNumber(), j, div));
				break;
			}
			case 1:
			{
				if (sa.hasInstrument() && sb.hasInstrument())
					pattern.getStep(s).setInstrumentNumber(interp(sa.getInstrumentNumber(), sb.getInstrumentNumber(), j, div));
				break;
			}
			case 2:
			{
				if (sa.hasVolume() && sb.hasVolume())
					pattern.getStep(s).setVolume(interp(sa.getVolume(), sb.getVolume(), j, div));
				break;
			}
			default:
			{
				int ec = c - 3;
				int ei = ec / 2;
				if (ec % 2) {	// Value
					if (sa.hasEffectValue(ei) && sb.hasEffectValue(ei))
						pattern.getStep(s).setEffectValue(ei, interp(sa.getEffectValue(ei), sb.getEffectValue(ei), j, div));
				}
				else {	// ID
					std::string a = sa.getEffectId(ei);
					std::string b = sb.getEffectId(ei);
					if (a == b)
						pattern.getStep(s).setEffectId(ei, a);
				}
				break;
			}
			}

			++s;
		}
		++c;
		t += (c / Step::N_COLUMN);
		c %= Step::N_COLUMN;
	}
}

void InterpolatePatternCommand::undo()
{
	command_utils::restorePattern(mod_.lock()->getSong(song_), prevCells_, bTrack_, bCol_, order_, bStep_);
}
