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

#include "paste_insert_copied_data_to_pattern_command.hpp"
#include <algorithm>
#include "pattern_command_utils.hpp"

PasteInsertCopiedDataToPatternCommand::PasteInsertCopiedDataToPatternCommand(std::weak_ptr<Module> mod, int songNum, int beginTrack, int beginColumn,
		int beginOrder, int beginStep, const std::vector<std::vector<std::string> >& cells)
	: AbstractCommand(CommandId::PasteInsertCopiedDataToPattern),
	  mod_(mod),
	  song_(songNum),
	  track_(beginTrack),
	  col_(beginColumn),
	  order_(beginOrder),
	  step_(beginStep),
	  cells_(cells)
{
	auto& song = mod.lock()->getSong(songNum);
	size_t newStepSize = song.getTrack(track_).getPatternFromOrderNumber(order_).getSize() - step_;
	prevCells_ = command_utils::getPreviousCells(song, cells.front().size(), newStepSize,
												 beginTrack, beginColumn, beginOrder, beginStep);
	if (cells.size() < newStepSize) {
		std::copy(prevCells_.begin(), prevCells_.end() - cells.size(), std::back_inserter(cells_));
	}
}

void PasteInsertCopiedDataToPatternCommand::redo()
{
	command_utils::restorePattern(mod_.lock()->getSong(song_), cells_, track_, col_, order_, step_);
}

void PasteInsertCopiedDataToPatternCommand::undo()
{
	command_utils::restorePattern(mod_.lock()->getSong(song_), prevCells_, track_, col_, order_, step_);
}
