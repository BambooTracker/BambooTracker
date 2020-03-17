#include "paste_copied_data_to_pattern_command.hpp"
#include "pattern_command_utils.hpp"

PasteCopiedDataToPatternCommand::PasteCopiedDataToPatternCommand(std::weak_ptr<Module> mod, int songNum, int beginTrack, int beginColmn,
																 int beginOrder, int beginStep,
																 std::vector<std::vector<std::string>> cells)
	: mod_(mod),
	  song_(songNum),
	  track_(beginTrack),
	  col_(beginColmn),
	  order_(beginOrder),
	  step_(beginStep),
	  cells_(cells)
{
	auto& song = mod.lock()->getSong(songNum);
	prevCells_ = getPreviousCells(song, cells.front().size(), cells.size(),
								  beginTrack, beginColmn, beginOrder, beginStep);
}

void PasteCopiedDataToPatternCommand::redo()
{
	restorePattern(mod_.lock()->getSong(song_), cells_, track_, col_, order_, step_);
}

void PasteCopiedDataToPatternCommand::undo()
{
	restorePattern(mod_.lock()->getSong(song_), prevCells_, track_, col_, order_, step_);
}

CommandId PasteCopiedDataToPatternCommand::getID() const
{
	return CommandId::PasteCopiedDataToPattern;
}
