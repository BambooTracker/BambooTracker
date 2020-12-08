/*
 * Copyright (C) 2020 Rerrah
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
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

std::vector<std::vector<std::string>> getPreviousCells(Song& song, size_t w, size_t h, int beginTrack,
													   int beginColumn, int beginOrder, int beginStep)
{
	std::vector<std::vector<std::string>> cells(h);
	int s = beginStep;
	for (size_t i = 0; i < h; ++i) {
		int t = beginTrack;
		int c = beginColumn;
		cells[i].resize(w);
		for (size_t j = 0; j < w; ++j) {
			Step& st = song.getTrack(t).getPatternFromOrderNumber(beginOrder).getStep(s);
			std::string val;
			switch (c) {
			case 0:		val = std::to_string(st.getNoteNumber());		break;
			case 1:		val = std::to_string(st.getInstrumentNumber());	break;
			case 2:		val = std::to_string(st.getVolume());			break;
			default:
			{
				int ec = c - 3;
				int ei = ec / 2;
				if (ec % 2) {	// Value
					val = std::to_string(st.getEffectValue(ei));
				}
				else {	// ID
					val = st.getEffectId(ei);
				}
				break;
			}
			}
			cells[i][j] = val;

			t += (++c / Step::N_COLUMN);
			c %= 11;
		}
		++s;
	}
	return cells;
}

void restorePattern(Song& song, const std::vector<std::vector<std::string>>& cells, int beginTrack,
					int beginColumn, int beginOrder, int beginStep)
{
	int s = beginStep;
	for (const auto& row : cells) {
		int t = beginTrack;
		int c = beginColumn;
		for (const std::string& cell : row) {
			Step& st = song.getTrack(t).getPatternFromOrderNumber(beginOrder).getStep(s);
			switch (c) {
			case 0:		st.setNoteNumber(std::stoi(cell));			break;
			case 1:		st.setInstrumentNumber(std::stoi(cell));	break;
			case 2:		st.setVolume(std::stoi(cell));				break;
			default:
			{
				int ec = c - 3;
				int ei = ec / 2;
				if (ec % 2) {	// Value
					st.setEffectValue(ei, std::stoi(cell));
				}
				else {	// ID
					st.setEffectId(ei, cell);
				}
				break;
			}
			}

			t += (++c / Step::N_COLUMN);
			c %= Step::N_COLUMN;
		}
		++s;
	}
}
}
