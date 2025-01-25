/*
 * SPDX-FileCopyrightText: 2018 Rerrah
 * SPDX-License-Identifier: MIT
 */

#include "erase_cells_in_pattern_command.hpp"
#include "pattern_command_utils.hpp"

EraseCellsInPatternCommand::EraseCellsInPatternCommand(
		std::weak_ptr<Module> mod, int songNum, int beginTrack, int beginColumn,
		int beginOrder, int beginStep, int endTrack, int endColumn, int endStep)
	: AbstractCommand(CommandId::EraseCellsInPattern),
	  mod_(mod),
	  song_(songNum),
	  bTrack_(beginTrack),
	  bCol_(beginColumn),
	  order_(beginOrder),
	  bStep_(beginStep)
{
	auto& song = mod.lock()->getSong(songNum);
	std::size_t h = static_cast<size_t>(endStep - beginStep + 1);
	std::size_t w = command_utils::calculateColumnSize(beginTrack, beginColumn, endTrack, endColumn);
	prevCells_ = command_utils::getPreviousCells(song, w, h, beginTrack, beginColumn, beginOrder, beginStep);
}

bool EraseCellsInPatternCommand::redo()
{
	auto& sng = mod_.lock()->getSong(song_);

	int stepIndex = bStep_;

	for (std::size_t i = 0; i < prevCells_.rowSize(); ++i) {
		int trackIndex = bTrack_;
		int columnIndex = bCol_;

		for (std::size_t j = 0; j < prevCells_.columnSize(); ++j) {
			Step& st = command_utils::getStep(sng, trackIndex, order_, stepIndex);

			switch (columnIndex) {
			case 0:
				st.clearNoteNumber();
				break;

			case 1:
				st.clearInstrumentNumber();
				break;

			case 2:
				st.clearVolume();
				break;

			default: {
				int effectColumnIndex = columnIndex - 3;
				int effectNumber = effectColumnIndex / 2;
				if (effectColumnIndex % 2) {
					// Effect value column.
					st.clearEffectValue(effectNumber);
				}
				else {
					// Effect ID column.
					st.clearEffectId(effectNumber);
				}
				break;
			}
			}

			trackIndex += (++columnIndex / Step::N_COLUMN);
			columnIndex %= Step::N_COLUMN;
		}

		++stepIndex;
	}

	return true;
}

bool EraseCellsInPatternCommand::undo()
{
	try {
		command_utils::restorePattern(mod_.lock()->getSong(song_), prevCells_, bTrack_, bCol_, order_, bStep_);
		return true;
	}
	catch (...) {
		return false;
	}
}
