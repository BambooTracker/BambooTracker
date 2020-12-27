/*
 * Copyright (C) 2018-2020 Rerrah
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

#include "paste_overwrite_copied_data_to_pattern_command.hpp"
#include "pattern_command_utils.hpp"

PasteOverwriteCopiedDataToPatternCommand::PasteOverwriteCopiedDataToPatternCommand(
		std::weak_ptr<Module> mod, int songNum, int beginTrack, int beginColumn,
		int beginOrder, int beginStep, std::vector<std::vector<std::string>> cells)
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
	prevCells_ = command_utils::getPreviousCells(song, cells.front().size(), cells.size(),
												 beginTrack, beginColumn, beginOrder, beginStep);
}

void PasteOverwriteCopiedDataToPatternCommand::redo()
{
	auto& sng = mod_.lock()->getSong(song_);

	int s = step_;
	for (const auto& row : cells_) {
		int t = track_;
		int c = col_;
		for (const std::string& cell : row) {
			Step& st = command_utils::getStep(sng, t, order_, s);
			switch (c) {
			case 0:
			{
				int n = std::stoi(cell);
				if (n != -1) st.setNoteNumber(n);
				break;
			}
			case 1:
			{
				int n = std::stoi(cell);
				if (n != -1) st.setInstrumentNumber(n);
				break;
			}
			case 2:
			{
				int vol = std::stoi(cell);
				if (vol != -1) st.setVolume(vol);
				break;
			}
			case 3:
			{
				if (cell != "--") st.setEffectID(0, cell);
				break;
			}
			case 4:
			{
				int val = std::stoi(cell);
				if (val != -1) st.setEffectValue(0, val);
				break;
			}
			case 5:
			{
				if (cell != "--") st.setEffectID(1, cell);
				break;
			}
			case 6:
			{
				int val = std::stoi(cell);
				if (val != -1) st.setEffectValue(1, val);
				break;
			}
			case 7:
			{
				if (cell != "--") st.setEffectID(2, cell);
				break;
			}
			case 8:
			{
				int val = std::stoi(cell);
				if (val != -1) st.setEffectValue(2, val);
				break;
			}
			case 9:
			{
				if (cell != "--") st.setEffectID(3, cell);
				break;
			}
			case 10:
			{
				int val = std::stoi(cell);
				if (val != -1) st.setEffectValue(3, val);
				break;
			}
			}

			t += (++c / 11);
			c %= 11;
		}

		++s;
	}
}

void PasteOverwriteCopiedDataToPatternCommand::undo()
{
	command_utils::restorePattern(mod_.lock()->getSong(song_), prevCells_, track_, col_, order_, step_);
}
