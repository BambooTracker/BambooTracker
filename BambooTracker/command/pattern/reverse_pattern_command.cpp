/*
 * SPDX-FileCopyrightText: 2018 Rerrah
 * SPDX-License-Identifier: MIT
 */

#include "reverse_pattern_command.hpp"
#include "pattern_command_utils.hpp"

ReversePatternCommand::ReversePatternCommand(
		std::weak_ptr<Module> mod, int songNum, int beginTrack, int beginColumn,
		int beginOrder, int beginStep, int endTrack, int endColumn, int endStep)
	: AbstractCommand(CommandId::ReversePattern),
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

bool ReversePatternCommand::redo()
{
	auto& song = mod_.lock()->getSong(song_);

	std::size_t lastRowIndex = prevCells_.rowSize() - 1;
	int stepIndex = bStep_;

	for (std::size_t i = 0; i < prevCells_.rowSize(); ++i) {
		int trackIndex = bTrack_;
		int columnIndex = bCol_;

		for (std::size_t j = 0; j < prevCells_.columnSize(); ++j) {
			Step& step = command_utils::getStep(song, trackIndex, order_, stepIndex);

			switch (columnIndex) {
			case 0:
				step.setNoteNumber(std::stoi(prevCells_.at(lastRowIndex - i).at(j)));
				break;

			case 1:
				step.setInstrumentNumber(std::stoi(prevCells_.at(lastRowIndex - i).at(j)));
				break;

			case 2:
				step.setVolume(std::stoi(prevCells_.at(lastRowIndex - i).at(j)));
				break;

			default: {
				int effectColumnIndex = columnIndex - 3;
				int effectNumber = effectColumnIndex / 2;
				if (effectColumnIndex % 2) {
					// Effect value column.
					step.setEffectValue(effectNumber, std::stoi(prevCells_.at(lastRowIndex - i).at(j)));
				}
				else {
					// Effect ID column.
					step.setEffectId(effectNumber, prevCells_.at(lastRowIndex - i).at(j));
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

bool ReversePatternCommand::undo()
{
	try {
		command_utils::restorePattern(mod_.lock()->getSong(song_), prevCells_, bTrack_, bCol_, order_, bStep_);
		return true;
	}
	catch (...) {
		return false;
	}
}
