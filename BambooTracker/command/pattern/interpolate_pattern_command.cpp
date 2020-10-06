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

InterpolatePatternCommand::InterpolatePatternCommand(std::weak_ptr<Module> mod,
													 int songNum, int beginTrack, int beginColumn,
													 int beginOrder, int beginStep,
													 int endTrack, int endColumn, int endStep)
	: mod_(mod),
	  song_(songNum),
	  bTrack_(beginTrack),
	  bCol_(beginColumn),
	  order_(beginOrder),
	  bStep_(beginStep),
	  eStep_(endStep)
{
	auto& song = mod.lock()->getSong(songNum);
	size_t h = static_cast<size_t>(endStep - beginStep + 1);
	size_t w = calculateColumnSize(beginTrack, beginColumn, endTrack, endColumn);
	prevCells_ = getPreviousCells(song, w, h, beginTrack, beginColumn, beginOrder, beginStep);
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
			switch (c) {
			case 0:
			{
				auto& pattern = sng.getTrack(t).getPatternFromOrderNumber(order_);
				int a = pattern.getStep(bStep_).getNoteNumber();
				int b = pattern.getStep(eStep_).getNoteNumber();
				if (a > -1 && b > -1)
					pattern.getStep(s).setNoteNumber(interp(a, b, j, div));
				break;
			}
			case 1:
			{
				auto& pattern = sng.getTrack(t).getPatternFromOrderNumber(order_);
				int a = pattern.getStep(bStep_).getInstrumentNumber();
				int b = pattern.getStep(eStep_).getInstrumentNumber();
				if (a > -1 && b > -1)
					pattern.getStep(s).setInstrumentNumber(interp(a, b, j, div));
				break;
			}
			case 2:
			{
				auto& pattern = sng.getTrack(t).getPatternFromOrderNumber(order_);
				int a = pattern.getStep(bStep_).getVolume();
				int b = pattern.getStep(eStep_).getVolume();
				if (a > -1 && b > -1)
					pattern.getStep(s).setVolume(interp(a, b, j, div));
				break;
			}
			case 3:
			{
				auto& pattern = sng.getTrack(t).getPatternFromOrderNumber(order_);
				std::string a = pattern.getStep(bStep_).getEffectID(0);
				std::string b = pattern.getStep(eStep_).getEffectID(0);
				if (a == b)
					sng.getTrack(t).getPatternFromOrderNumber(order_)
							.getStep(s).setEffectID(0, a);
				break;
			}
			case 4:
			{
				auto& pattern = sng.getTrack(t).getPatternFromOrderNumber(order_);
				int a = pattern.getStep(bStep_).getEffectValue(0);
				int b = pattern.getStep(eStep_).getEffectValue(0);
				if (a > -1 && b > -1)
					pattern.getStep(s).setEffectValue(0, interp(a, b, j, div));
				break;
			}
			case 5:
			{
				auto& pattern = sng.getTrack(t).getPatternFromOrderNumber(order_);
				std::string a = pattern.getStep(bStep_).getEffectID(1);
				std::string b = pattern.getStep(eStep_).getEffectID(1);
				if (a == b)
					sng.getTrack(t).getPatternFromOrderNumber(order_)
							.getStep(s).setEffectID(1, a);
				break;
			}
			case 6:
			{
				auto& pattern = sng.getTrack(t).getPatternFromOrderNumber(order_);
				int a = pattern.getStep(bStep_).getEffectValue(1);
				int b = pattern.getStep(eStep_).getEffectValue(1);
				if (a > -1 && b > -1)
					pattern.getStep(s).setEffectValue(1, interp(a, b, j, div));
				break;
			}
			case 7:
			{
				auto& pattern = sng.getTrack(t).getPatternFromOrderNumber(order_);
				std::string a = pattern.getStep(bStep_).getEffectID(2);
				std::string b = pattern.getStep(eStep_).getEffectID(2);
				if (a == b)
					sng.getTrack(t).getPatternFromOrderNumber(order_)
							.getStep(s).setEffectID(2, a);
				break;
			}
			case 8:
			{
				auto& pattern = sng.getTrack(t).getPatternFromOrderNumber(order_);
				int a = pattern.getStep(bStep_).getEffectValue(2);
				int b = pattern.getStep(eStep_).getEffectValue(2);
				if (a > -1 && b > -1)
					pattern.getStep(s).setEffectValue(2, interp(a, b, j, div));
				break;
			}
			case 9:
			{
				auto& pattern = sng.getTrack(t).getPatternFromOrderNumber(order_);
				std::string a = pattern.getStep(bStep_).getEffectID(3);
				std::string b = pattern.getStep(eStep_).getEffectID(3);
				if (a == b)
					sng.getTrack(t).getPatternFromOrderNumber(order_)
							.getStep(s).setEffectID(3, a);
				break;
			}
			case 10:
			{
				auto& pattern = sng.getTrack(t).getPatternFromOrderNumber(order_);
				int a = pattern.getStep(bStep_).getEffectValue(3);
				int b = pattern.getStep(eStep_).getEffectValue(3);
				if (a > -1 && b > -1)
					pattern.getStep(s).setEffectValue(3, interp(a, b, j, div));
				break;
			}
			}

			++s;
		}
		++c;
		t += (c / 11);
		c %= 11;
	}
}

void InterpolatePatternCommand::undo()
{
	restorePattern(mod_.lock()->getSong(song_), prevCells_, bTrack_, bCol_, order_, bStep_);
}

CommandId InterpolatePatternCommand::getID() const
{
	return CommandId::InterpolatePattern;
}
