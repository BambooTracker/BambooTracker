/*
 * SPDX-FileCopyrightText: 2020 Rerrah
 * SPDX-License-Identifier: MIT
 */

#include "transpose_note_in_pattern_command.hpp"
#include "pattern_command_utils.hpp"
#include "note.hpp"
#include "utils.hpp"

TransposeNoteInPatternCommand::TransposeNoteInPatternCommand(
		std::weak_ptr<Module> mod, int songNum, int beginTrack,
		int beginOrder, int beginStep, int endTrack, int endStep, int semitone)
	: AbstractCommand(CommandId::TransposeNoteInPattern),
	  mod_(mod),
	  song_(songNum),
	  bTrack_(beginTrack),
	  order_(beginOrder),
	  bStep_(beginStep),
	  eTrack_(endTrack),
	  eStep_(endStep),
	  semitone_(semitone)
{
	auto& sng = mod.lock()->getSong(songNum);

	for (int step = beginStep; step <= endStep; ++step) {
		for (int track = beginTrack; track <= endTrack; ++track) {
			Step& st = command_utils::getStep(sng, track, beginOrder, step);
			if (st.hasGeneralNote()) prevKeys_.push_back(st.getNoteNumber());
		}
	}
}

bool TransposeNoteInPatternCommand::redo()
{
	auto& sng = mod_.lock()->getSong(song_);

	for (int step = bStep_; step <= eStep_; ++step) {
		for (int track = bTrack_; track <= eTrack_; ++track) {
			Step& st = command_utils::getStep(sng, track, order_, step);
			int n = st.getNoteNumber();
			if (st.hasGeneralNote()) {
				st.setNoteNumber(utils::clamp(n + semitone_, 0, Note::NOTE_NUMBER_RANGE - 1));
			}
		}
	}
	return true;
}

bool TransposeNoteInPatternCommand::undo()
{
	auto& sng = mod_.lock()->getSong(song_);

	size_t i = 0;
	for (int step = bStep_; step <= eStep_; ++step) {
		for (int track = bTrack_; track <= eTrack_; ++track) {
			Step& st = command_utils::getStep(sng, track, order_, step);
			if (st.hasGeneralNote()) st.setNoteNumber(prevKeys_.at(i++));
		}
	}
	return true;
}
