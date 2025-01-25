/*
 * SPDX-FileCopyrightText: 2018 Rerrah
 * SPDX-License-Identifier: MIT
 */

#include "interpolate_pattern_command.hpp"
#include "pattern_command_utils.hpp"

namespace
{
inline int interp(int a, int b, size_t t, int div)
{
	return a + (b - a) * static_cast<int>(t) / div;
}
}

InterpolatePatternCommand::InterpolatePatternCommand(
		std::weak_ptr<Module> mod, int songNum, int beginTrack, int beginColumn,
		int beginOrder, int beginStep, int endTrack, int endColumn, int endStep)
	: AbstractCommand(CommandId::InterpolatePattern),
	  mod_(mod),
	  song_(songNum),
	  bTrack_(beginTrack),
	  bCol_(beginColumn),
	  order_(beginOrder),
	  bStep_(beginStep),
	  eStep_(endStep)
{
	auto& song = mod.lock()->getSong(songNum);
	std::size_t h = static_cast<size_t>(endStep - beginStep + 1);
	std::size_t w = command_utils::calculateColumnSize(beginTrack, beginColumn, endTrack, endColumn);
	prevCells_ = command_utils::getPreviousCells(song, w, h, beginTrack, beginColumn, beginOrder, beginStep);
}

bool InterpolatePatternCommand::redo()
{
	auto& song = mod_.lock()->getSong(song_);
	int div = static_cast<int>(prevCells_.rowSize()) - 1;
	if (!div) div = 1;

	int trackIndex = bTrack_;
	int columnIndex = bCol_;

	for (std::size_t i = 0; i < prevCells_.columnSize(); ++i) {
		int stepIndex = bStep_;

		for (std::size_t j = 0; j < prevCells_.rowSize(); ++j) {
			Pattern& pattern = command_utils::getPattern(song, trackIndex, order_);
			Step& firstStep = pattern.getStep(bStep_);
			Step& lastStep = pattern.getStep(eStep_);

			switch (columnIndex) {
			case 0: {
				if (firstStep.hasGeneralNote() && lastStep.hasGeneralNote()) {
					pattern.getStep(stepIndex).setNoteNumber(
						interp(firstStep.getNoteNumber(), lastStep.getNoteNumber(), j, div));
				}
				break;
			}

			case 1: {
				if (firstStep.hasInstrument() && lastStep.hasInstrument()) {
					pattern.getStep(stepIndex).setInstrumentNumber(
						interp(firstStep.getInstrumentNumber(), lastStep.getInstrumentNumber(), j, div));
				}
				break;
			}

			case 2: {
				if (firstStep.hasVolume() && lastStep.hasVolume()) {
					pattern.getStep(stepIndex).setVolume(
						interp(firstStep.getVolume(), lastStep.getVolume(), j, div));
				}
				break;
			}

			default: {
				int effectColumnIndex = columnIndex - 3;
				int effectNumber = effectColumnIndex / 2;
				if (effectColumnIndex % 2) {
					// Effect value column.
					if (firstStep.hasEffectValue(effectNumber) && lastStep.hasEffectValue(effectNumber)) {
						pattern.getStep(stepIndex).setEffectValue(
							effectNumber, interp(firstStep.getEffectValue(effectNumber), lastStep.getEffectValue(effectNumber), j, div));
					}
				}
				else {
					// Effect ID column.
					std::string firstId = firstStep.getEffectId(effectNumber);
					std::string lastId = lastStep.getEffectId(effectNumber);
					if (firstId == lastId) {
						pattern.getStep(stepIndex).setEffectId(effectNumber, firstId);
					}
				}
				break;
			}
			}

			++stepIndex;
		}

		++columnIndex;
		trackIndex += (columnIndex / Step::N_COLUMN);
		columnIndex %= Step::N_COLUMN;
	}

	return true;
}

bool InterpolatePatternCommand::undo()
{
	try {
		command_utils::restorePattern(mod_.lock()->getSong(song_), prevCells_, bTrack_, bCol_, order_, bStep_);
		return true;
	}
	catch (...) {
		return false;
	}
}
