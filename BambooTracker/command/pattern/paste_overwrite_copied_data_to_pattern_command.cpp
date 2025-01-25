/*
 * SPDX-FileCopyrightText: 2018 Rerrah
 * SPDX-License-Identifier: MIT
 */

#include "paste_overwrite_copied_data_to_pattern_command.hpp"
#include "pattern_command_utils.hpp"

PasteOverwriteCopiedDataToPatternCommand::PasteOverwriteCopiedDataToPatternCommand(
	std::weak_ptr<Module> mod, int songNum, int beginTrack, int beginColumn,
		int beginOrder, int beginStep, const Vector2d<std::string>& cells)
	: AbstractCommand(CommandId::PasteOverwriteCopiedDataToPattern),
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

bool PasteOverwriteCopiedDataToPatternCommand::redo()
{
	auto& sng = mod_.lock()->getSong(song_);

	int stepIndex = step_;
	int orderIndex = order_;

	for (const auto& row : cells_) {
		int trackIndex = track_;
		int columnIndex = col_;

		for (const std::string& cell : row) {
			if (static_cast<std::size_t>(stepIndex) >= sng.getTrack(trackIndex).getPatternFromOrderNumber(orderIndex).getSize()) {
				if (static_cast<std::size_t>(++orderIndex) < sng.getTrack(trackIndex).getOrderSize()) {
					stepIndex = 0;
				}
				else {
					return true;
				}
			}

			Step& step = command_utils::getStep(sng, trackIndex, orderIndex, stepIndex);
			switch (columnIndex) {
			case 0: {
				int n = std::stoi(cell);
				if (!Step::testEmptyNote(n)) {
					step.setNoteNumber(n);
				}
				break;
			}

			case 1: {
				int n = std::stoi(cell);
				if (!Step::testEmptyInstrument(n)) {
					step.setInstrumentNumber(n);
				}
				break;
			}

			case 2: {
				int volume = std::stoi(cell);
				if (!Step::testEmptyVolume(volume)) {
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
					if (!Step::testEmptyEffectValue(value)) {
						step.setEffectValue(effectNumber, value);
					}
				}
				else {
					// Effect ID column.
					if (!Step::testEmptyEffectId(cell)) {
						step.setEffectId(effectNumber, cell);
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

bool PasteOverwriteCopiedDataToPatternCommand::undo()
{
	try {
		command_utils::restorePattern(mod_.lock()->getSong(song_), prevCells_, track_, col_, order_, step_);
		return true;
	}
	catch (...) {
		return false;
	}
}
