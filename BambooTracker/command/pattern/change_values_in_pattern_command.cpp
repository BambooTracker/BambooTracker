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

#include "change_values_in_pattern_command.hpp"
#include "pattern_command_utils.hpp"
#include "misc.hpp"

ChangeValuesInPatternCommand::ChangeValuesInPatternCommand(std::weak_ptr<Module> mod, int songNum, int beginTrack,
														   int beginColumn, int beginOrder, int beginStep,
														   int endTrack, int endColumn, int endStep, int value, bool isFMReversed)
	: AbstractCommand(CommandId::ChangeValuesInPattern),
	  mod_(mod),
	  song_(songNum),
	  bTrack_(beginTrack),
	  bCol_(beginColumn),
	  order_(beginOrder),
	  bStep_(beginStep),
	  eTrack_(endTrack),
	  eCol_(endColumn),
	  eStep_(endStep),
	  diff_(value),
	  fmReverse_(isFMReversed)
{
	auto& sng = mod.lock()->getSong(songNum);

	for (int step = beginStep; step <= endStep; ++step) {
		int track = beginTrack;
		int col = beginColumn;
		std::vector<int> vals;
		while (true) {
			int val;
			Step& st = command_utils::getStep(sng, track, beginOrder, step);
			switch (col) {
			case 1:		val = st.getInstrumentNumber();	break;
			case 2:		val = st.getVolume();			break;
			case 4:		val = st.getEffectValue(0);		break;
			case 6:		val = st.getEffectValue(1);		break;
			case 8:		val = st.getEffectValue(2);		break;
			case 10:	val = st.getEffectValue(3);		break;
			default:	val = -1;						break;
			}
			if (val > -1) vals.push_back(val);
			if (track == endTrack && col == endColumn) break;
			track += (++col / 11);
			col %= 11;
		}
		prevVals_.push_back(vals);
	}
}

void ChangeValuesInPatternCommand::redo()
{
	auto& sng = mod_.lock()->getSong(song_);
	auto it = prevVals_.begin();
	for (int step = bStep_; step <= eStep_; ++step, ++it) {
		int track = bTrack_;
		int col = bCol_;
		auto valit = it->begin();
		while (true) {
			Track& tr = sng.getTrack(track);
			Step& st = tr.getPatternFromOrderNumber(order_).getStep(step);
			switch (col) {
			case 1:
				if (st.getInstrumentNumber() > -1) st.setInstrumentNumber(clamp(diff_ + *valit++, 0, 127));
				break;
			case 2:
				if (st.getVolume() > -1) {
					int d = (tr.getAttribute().source == SoundSource::FM && fmReverse_) ? -diff_ : diff_;
					st.setVolume(clamp(d + *valit++, 0, 255));
				}
				break;
			case 4:
				if (st.getEffectValue(0) > -1) st.setEffectValue(0, clamp(diff_ + *valit++, 0, 255));
				break;
			case 6:
				if (st.getEffectValue(1) > -1) st.setEffectValue(1, clamp(diff_ + *valit++, 0, 255));
				break;
			case 8:
				if (st.getEffectValue(2) > -1) st.setEffectValue(2, clamp(diff_ + *valit++, 0, 255));
				break;
			case 10:
				if (st.getEffectValue(3) > -1) st.setEffectValue(3, clamp(diff_ + *valit++, 0, 255));
				break;
			default:
				break;
			}
			if (track == eTrack_ && col == eCol_) break;
			track += (++col / 11);
			col %= 11;
		}
	}
}

void ChangeValuesInPatternCommand::undo()
{
	auto& sng = mod_.lock()->getSong(song_);
	auto it = prevVals_.begin();
	for (int step = bStep_; step <= eStep_; ++step, ++it) {
		int track = bTrack_;
		int col = bCol_;
		auto valit = it->begin();
		while (true) {
			Step& st = command_utils::getStep(sng, track, order_, step);
			switch (col) {
			case 1:
				if (st.getInstrumentNumber() > -1) st.setInstrumentNumber(*valit++);
				break;
			case 2:
				if (st.getVolume() > -1) st.setVolume(*valit++);
				break;
			case 4:
				if (st.getEffectValue(0) > -1) st.setEffectValue(0, *valit++);
				break;
			case 6:
				if (st.getEffectValue(1) > -1) st.setEffectValue(1, *valit++);
				break;
			case 8:
				if (st.getEffectValue(2) > -1) st.setEffectValue(2, *valit++);
				break;
			case 10:
				if (st.getEffectValue(3) > -1) st.setEffectValue(3, *valit++);
				break;
			default:
				break;
			}
			if (track == eTrack_ && col == eCol_) break;
			track += (++col / 11);
			col %= 11;
		}
	}
}
