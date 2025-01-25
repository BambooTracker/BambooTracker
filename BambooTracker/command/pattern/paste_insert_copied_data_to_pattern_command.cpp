/*
 * SPDX-FileCopyrightText: 2020 Rerrah
 * SPDX-License-Identifier: MIT
 */

#include "paste_insert_copied_data_to_pattern_command.hpp"
#include <algorithm>
#include <iterator>
#include "pattern_command_utils.hpp"

PasteInsertCopiedDataToPatternCommand::PasteInsertCopiedDataToPatternCommand(
	std::weak_ptr<Module> mod, int songNum, int beginTrack, int beginColumn,
		int beginOrder, int beginStep, const Vector2d<std::string>& cells)
	: AbstractCommand(CommandId::PasteInsertCopiedDataToPattern),
	  mod_(mod),
	  song_(songNum),
	  track_(beginTrack),
	  col_(beginColumn),
	  order_(beginOrder),
	  step_(beginStep)
{
	auto& song = mod.lock()->getSong(songNum);
	size_t newStepSize = song.getTrack(track_).getPatternFromOrderNumber(order_).getSize() - step_;
	prevCells_ = command_utils::getPreviousCells(song, cells.columnSize(), newStepSize,
												 beginTrack, beginColumn, beginOrder, beginStep);

	Vector2d<std::string> newCells(newStepSize, cells.columnSize());
	std::size_t shiftedRowIndex = std::min(cells.rowSize(), newStepSize);
	std::copy_n(cells.begin(), shiftedRowIndex, newCells.begin());
	std::copy_n(prevCells_.begin(), newStepSize - shiftedRowIndex, newCells.begin() + shiftedRowIndex);
	cells_ = newCells;
}

bool PasteInsertCopiedDataToPatternCommand::redo()
{
	try {
		command_utils::restorePattern(mod_.lock()->getSong(song_), cells_, track_, col_, order_, step_);
		return true;
	}
	catch (...) {
		return false;
	}

}

bool PasteInsertCopiedDataToPatternCommand::undo()
{
	try {
		command_utils::restorePattern(mod_.lock()->getSong(song_), prevCells_, track_, col_, order_, step_);
		return true;
	}
	catch (...) {
		return false;
	}
}
