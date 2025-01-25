/*
 * SPDX-FileCopyrightText: 2018 Rerrah
 * SPDX-License-Identifier: MIT
 */

#include "replace_instrument_in_pattern_command.hpp"
#include "pattern_command_utils.hpp"

ReplaceInstrumentInPatternCommand::ReplaceInstrumentInPatternCommand(
		std::weak_ptr<Module> mod, int songNum, int beginTrack,
		int beginOrder, int beginStep, int endTrack, int endStep, int newInst)
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

bool ReplaceInstrumentInPatternCommand::redo()
{
	auto& sng = mod_.lock()->getSong(song_);

	for (int step = bStep_; step <= eStep_; ++step) {
		for (int track = bTrack_; track <= eTrack_; ++track) {
			Step& st = command_utils::getStep(sng, track, order_, step);
			if (st.hasInstrument()) st.setInstrumentNumber(inst_);
		}
	}
	return true;
}

bool ReplaceInstrumentInPatternCommand::undo()
{
	auto& sng = mod_.lock()->getSong(song_);

	size_t i = 0;
	for (int step = bStep_; step <= eStep_; ++step) {
		for (int track = bTrack_; track <= eTrack_; ++track) {
			Step& st = command_utils::getStep(sng, track, order_, step);
			if (st.hasInstrument()) st.setInstrumentNumber(prevInsts_.at(i++));
		}
	}
	return true;
}
