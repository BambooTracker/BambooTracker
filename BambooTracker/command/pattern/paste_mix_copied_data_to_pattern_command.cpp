/*
 * SPDX-FileCopyrightText: 2018 Rerrah
 * SPDX-License-Identifier: MIT
 */

#include "paste_mix_copied_data_to_pattern_command.hpp"
#include "pattern_command_utils.hpp"

PasteMixCopiedDataToPatternCommand::PasteMixCopiedDataToPatternCommand(
	std::weak_ptr<Module> mod, int songNum, int beginTrack, int beginColumn,
		int beginOrder, int beginStep, const Vector2d<std::string>& cells)
	: AbstractCommand(CommandId::PasteMixCopiedDataToPattern),
	  mod_(mod),
	  song_(songNum),
	  track_(beginTrack),
	  col_(beginColumn),
	  order_(beginOrder),
	  step_(beginStep),
	  cells_(cells)
{
	auto& song = mod.lock()->getSong(songNum);
	prevCells_ = command_utils::getPreviousCells(song, cells.columnSize(), cells.rowSize(),
												 beginTrack, beginColumn, beginOrder, beginStep);
}

bool PasteMixCopiedDataToPatternCommand::redo()
{
	try {
		auto& song = mod_.lock()->getSong(song_);

		int stepIndex = step_;
		int orderIndex = order_;

		for (const auto& row : cells_) {
			int trackIndex = track_;
			int columnIndex = col_;

			for (const std::string& cell : row) {
				if (static_cast<std::size_t>(stepIndex) >= song.getTrack(trackIndex).getPatternFromOrderNumber(orderIndex).getSize()) {
					if (static_cast<std::size_t>(++orderIndex) < song.getTrack(trackIndex).getOrderSize()) {
						stepIndex = 0;
					}
					else {
						return true;
					}
				}

				Step& step = command_utils::getStep(song, trackIndex, orderIndex, stepIndex);
				switch (columnIndex) {
				case 0: {
					int n = std::stoi(cell);
					if (!Step::testEmptyNote(n) && step.isEmptyNote()) {
						step.setNoteNumber(n);
					}
					break;
				}

				case 1: {
					int n = std::stoi(cell);
					if (!Step::testEmptyInstrument(n) && !step.hasInstrument()) {
						step.setInstrumentNumber(n);
					}
					break;
				}

				case 2: {
					int volume = std::stoi(cell);
					if (!Step::testEmptyVolume(volume) && !step.hasVolume()) {
						step.setVolume(volume);
					}
					break;
				}

				default: {
					int effectColumnIndex = columnIndex - 3;
					int effectNumber = effectColumnIndex / 2;
					if (effectColumnIndex % 2) {
						// Effect value column.
						int value = std::stoi(cell);
						if (!Step::testEmptyEffectValue(value) && !step.hasEffectValue(effectNumber)) {
							step.setEffectValue(effectNumber, value);
						}
					}
					else {
						// Effect ID column.
						if (!Step::testEmptyEffectId(cell) && !step.hasEffectId(effectNumber)) {
							step.setEffectId(effectNumber, cell);
						}
					}
					break;
				}
				}

				++columnIndex;
				trackIndex += (columnIndex / Step::N_COLUMN);
				columnIndex %= Step::N_COLUMN;
			}

			++stepIndex;
		}

		return true;
	}
	catch (...) {
		return false;
	}
}

bool PasteMixCopiedDataToPatternCommand::undo()
{
	try {
		command_utils::restorePattern(mod_.lock()->getSong(song_), prevCells_, track_, col_, order_, step_);
		return true;
	}
	catch (...) {
		return false;
	}
}
