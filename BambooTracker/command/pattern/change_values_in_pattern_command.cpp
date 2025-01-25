/*
 * SPDX-FileCopyrightText: 2020 Rerrah
 * SPDX-License-Identifier: MIT
 */

#include "change_values_in_pattern_command.hpp"
#include "pattern_command_utils.hpp"
#include "bamboo_tracker_defs.hpp"
#include "utils.hpp"

ChangeValuesInPatternCommand::ChangeValuesInPatternCommand(
		std::weak_ptr<Module> mod, int songNum, int beginTrack, int beginColumn, int beginOrder,
		int beginStep, int endTrack, int endColumn, int endStep, int value, bool isFMReversed)
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
			Step& st = command_utils::getStep(sng, track, beginOrder, step);
			switch (col) {
			case 1:
				if (st.hasInstrument()) vals.push_back(st.getInstrumentNumber());
				break;
			case 2:
				if (st.hasVolume()) vals.push_back(st.getVolume());
				break;
			default:
			{
				if (col) {
					int ec = col - 3;
					int ei = ec / 2;
					if (ec % 2 && st.hasEffectValue(ei)) vals.push_back(st.getEffectValue(ei));
				}
				break;
			}
			}
			if (track == endTrack && col == endColumn) break;
			track += (++col / Step::N_COLUMN);
			col %= Step::N_COLUMN;
		}
		prevVals_.push_back(vals);
	}
}

bool ChangeValuesInPatternCommand::redo()
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
				if (st.hasInstrument())
					st.setInstrumentNumber(utils::clamp(diff_ + *valit++, 0, 127));
				break;
			case 2:
				if (st.hasVolume()) {
					int d = (tr.getAttribute().source == SoundSource::FM && fmReverse_) ? -diff_
																						: diff_;
					st.setVolume(utils::clamp(d + *valit++, 0, 255));
				}
				break;
			default:
			{
				if (col) {
					int ec = col - 3;
					int ei = ec / 2;
					if (ec % 2 && st.hasEffectValue(ei))
						st.setEffectValue(ei, utils::clamp(diff_ + *valit++, 0, 255));
				}
				break;
			}
			}
			if (track == eTrack_ && col == eCol_) break;
			track += (++col / Step::N_COLUMN);
			col %= Step::N_COLUMN;
		}
	}
	return true;
}

bool ChangeValuesInPatternCommand::undo()
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
				if (st.hasInstrument()) st.setInstrumentNumber(*valit++);
				break;
			case 2:
				if (st.hasVolume()) st.setVolume(*valit++);
				break;
			default:
			{
				if (col) {
					int ec = col - 3;
					int ei = ec / 2;
					if (ec % 2 && st.hasEffectValue(ei)) st.setEffectValue(ei, *valit++);
				}
				break;
			}
			}
			if (track == eTrack_ && col == eCol_) break;
			track += (++col / Step::N_COLUMN);
			col %= Step::N_COLUMN;
		}
	}
	return true;
}
