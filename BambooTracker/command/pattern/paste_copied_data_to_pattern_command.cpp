/*
 * SPDX-FileCopyrightText: 2018 Rerrah
 * SPDX-License-Identifier: MIT
 */

#include "paste_copied_data_to_pattern_command.hpp"
#include "pattern_command_utils.hpp"

PasteCopiedDataToPatternCommand::PasteCopiedDataToPatternCommand(
		std::weak_ptr<Module> mod, int songNum, int beginTrack, int beginColmn,
		int beginOrder, int beginStep, const Vector2d<std::string>& cells)
	: AbstractCommand(CommandId::PasteCopiedDataToPattern),
	  mod_(mod),
	  song_(songNum),
	  track_(beginTrack),
	  col_(beginColmn),
	  order_(beginOrder),
	  step_(beginStep),
	  cells_(cells)
{
	auto& song = mod.lock()->getSong(songNum);
	prevCells_ = command_utils::getPreviousCells(song, cells.columnSize(), cells.rowSize(),
												 beginTrack, beginColmn, beginOrder, beginStep);
}

bool PasteCopiedDataToPatternCommand::redo()
{
	try {
		command_utils::restorePattern(mod_.lock()->getSong(song_), cells_, track_, col_, order_, step_);
		return true;
	}
	catch (...) {
		return false;
	}
}

bool PasteCopiedDataToPatternCommand::undo()
{
	try {
		command_utils::restorePattern(mod_.lock()->getSong(song_), prevCells_, track_, col_, order_, step_);
		return true;
	}
	catch (...) {
		return false;
	}
}
