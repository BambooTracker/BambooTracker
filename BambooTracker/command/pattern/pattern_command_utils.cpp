/*
 * SPDX-FileCopyrightText: 2020 Rerrah
 * SPDX-License-Identifier: MIT
 */

#include "pattern_command_utils.hpp"

namespace command_utils
{
size_t calculateColumnSize(int beginTrack, int beginColumn, int endTrack, int endColumn)
{
	constexpr int WCOL = Step::N_COLUMN - 1;
	int w = 0;
	int tr = endTrack;
	int cl = endColumn;
	while (true) {
		if (tr == beginTrack) {
			w += (cl - beginColumn + 1);
			break;
		}
		else {
			w += (cl + 1);
			cl = WCOL;
			--tr;
		}
	}

	return static_cast<size_t>(w);
}

Vector2d<std::string> getPreviousCells(Song& song, std::size_t w, std::size_t h, int beginTrack,
									   int beginColumn, int beginOrder, int beginStep)
{
	Vector2d<std::string> cells(h, w);
	int stepIndex = beginStep;
	for (std::size_t i = 0; i < h; ++i) {
		int trackIndex = beginTrack;
		int columnIndex = beginColumn;

		for (std::size_t j = 0; j < w; ++j) {
			if (static_cast<std::size_t>(stepIndex) >= song.getTrack(trackIndex).getPatternFromOrderNumber(beginOrder).getSize()) {
				if (static_cast<std::size_t>(++beginOrder) < song.getTrack(trackIndex).getOrderSize()) {
					stepIndex = 0;
				}
				else {
					return cells;
				}
			}

			Step& step = song.getTrack(trackIndex).getPatternFromOrderNumber(beginOrder).getStep(stepIndex);
			std::string value;
			switch (columnIndex) {
			case 0:
				value = std::to_string(step.getNoteNumber());
				break;

			case 1:
				value = std::to_string(step.getInstrumentNumber());
				break;

			case 2:
				value = std::to_string(step.getVolume());
				break;

			default: {
				int effectColumnIndex = columnIndex - 3;
				int effectNumber = effectColumnIndex / 2;
				if (effectColumnIndex % 2) {
					// Effect value column.
					value = std::to_string(step.getEffectValue(effectNumber));
				}
				else {
					// Effect ID column.
					value = step.getEffectId(effectNumber);
				}
				break;
			}
			}
			cells[i][j] = value;

			trackIndex += (++columnIndex / Step::N_COLUMN);
			columnIndex %= Step::N_COLUMN;
		}

		++stepIndex;
	}

	return cells;
}

void restorePattern(Song& song, const Vector2d<std::string>& cells, int beginTrack,
					int beginColumn, int beginOrder, int beginStep)
{
	int stepIndex = beginStep;

	for (const auto& row : cells) {
		int trackIndex = beginTrack;
		int columnIndex = beginColumn;

		for (const std::string& cell : row) {
			if (static_cast<std::size_t>(stepIndex) >= song.getTrack(trackIndex).getPatternFromOrderNumber(beginOrder).getSize()) {
				if (static_cast<std::size_t>(++beginOrder) < song.getTrack(trackIndex).getOrderSize()) {
					stepIndex = 0;
				}
				else {
					return;
				}
			}

			Step& step = song.getTrack(trackIndex).getPatternFromOrderNumber(beginOrder).getStep(stepIndex);
			switch (columnIndex) {
			case 0:
				step.setNoteNumber(std::stoi(cell));
				break;

			case 1:
				step.setInstrumentNumber(std::stoi(cell));
				break;

			case 2:
				step.setVolume(std::stoi(cell));
				break;

			default: {
				int effectColumnIndex = columnIndex - 3;
				int effectNumber = effectColumnIndex / 2;
				if (effectColumnIndex % 2) {
					// Effect value column.
					step.setEffectValue(effectNumber, std::stoi(cell));
				}
				else {
					// Effect ID column.
					step.setEffectId(effectNumber, cell);
				}
				break;
			}
			}

			trackIndex += (++columnIndex / Step::N_COLUMN);
			columnIndex %= Step::N_COLUMN;
		}

		++stepIndex;
	}
}
}
