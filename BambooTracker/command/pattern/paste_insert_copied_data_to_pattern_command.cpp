#include "paste_insert_copied_data_to_pattern_command.hpp"
#include <algorithm>
#include "pattern_command_utils.hpp"

PasteInsertCopiedDataToPatternCommand::PasteInsertCopiedDataToPatternCommand(
		std::weak_ptr<Module> mod, int songNum, int beginTrack, int beginColumn,
		int beginOrder, int beginStep, std::vector<std::vector<std::string>> cells)
	: mod_(mod),
	  song_(songNum),
	  track_(beginTrack),
	  col_(beginColumn),
	  order_(beginOrder),
	  step_(beginStep),
	  cells_(cells)
{
	auto& song = mod.lock()->getSong(songNum);
	size_t newStepSize = song.getTrack(track_).getPatternFromOrderNumber(order_).getSize() - step_;
	prevCells_ = getPreviousCells(song, cells.front().size(), newStepSize,
								  beginTrack, beginColumn, beginOrder, beginStep);
	if (cells.size() < newStepSize) {
		std::copy(prevCells_.begin(), prevCells_.end() - cells.size(), std::back_inserter(cells_));
	}
}

void PasteInsertCopiedDataToPatternCommand::redo()
{
	restorePattern(mod_.lock()->getSong(song_), cells_, track_, col_, order_, step_);
}

void PasteInsertCopiedDataToPatternCommand::undo()
{
	restorePattern(mod_.lock()->getSong(song_), prevCells_, track_, col_, order_, step_);
}

CommandId PasteInsertCopiedDataToPatternCommand::getID() const
{
	return CommandId::PasteInsertCopiedDataToPattern;
}
