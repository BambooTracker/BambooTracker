/*
 * SPDX-FileCopyrightText: 2018 Rerrah
 * SPDX-License-Identifier: MIT
 */

#include "expand_pattern_command.hpp"
#include "pattern_command_utils.hpp"

ExpandPatternCommand::ExpandPatternCommand(
		std::weak_ptr<Module> mod, int songNum, int beginTrack, int beginColumn,
		int beginOrder, int beginStep, int endTrack, int endColumn, int endStep)
	: AbstractCommand(CommandId::ExpandPattern),
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

bool ExpandPatternCommand::redo()
{
	auto& song = mod_.lock()->getSong(song_);

	int stepIndex = bStep_;
	for (std::size_t i = 0; i < prevCells_.rowSize(); ++i) {
		int trackIndex = bTrack_;
		int columnIndex = bCol_;

		for (std::size_t j = 0; j < prevCells_.columnSize(); ++j) {
			Step& step = command_utils::getStep(song, trackIndex, order_, stepIndex);

			switch (columnIndex) {
			case 0: {
				if (i % 2) {
					step.clearNoteNumber();
				}
				else {
					step.setNoteNumber(std::stoi(prevCells_.at(i / 2).at(j)));
				}
				break;
			}

			case 1: {
				if (i % 2) {
					step.clearInstrumentNumber();
				}
				else {
					step.setInstrumentNumber(std::stoi(prevCells_.at(i / 2).at(j)));
				}
				break;
			}

			case 2: {
				if (i % 2) {
					step.clearVolume();
				}
				else {
					step.setVolume(std::stoi(prevCells_.at(i / 2).at(j)));
				}
				break;
			}

			default: {
				int effectColumnIndex = columnIndex - 3;
				int effectNumber = effectColumnIndex / 2;
				if (effectColumnIndex % 2) {
					// Effect value column.
					if (i % 2) {
						step.clearEffectValue(effectNumber);
					}
					else {
						step.setEffectValue(effectNumber, std::stoi(prevCells_.at(i / 2).at(j)));
					}
				}
				else {
					// Effect ID column.
					if (i % 2) {
						step.clearEffectId(effectNumber);
					}
					else {
						step.setEffectId(effectNumber, prevCells_.at(i / 2).at(j));
					}
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

bool ExpandPatternCommand::undo()
{
	try {
		command_utils::restorePattern(mod_.lock()->getSong(song_), prevCells_, bTrack_, bCol_, order_, bStep_);
		return true;
	}
	catch (...) {
		return false;
	}
}
