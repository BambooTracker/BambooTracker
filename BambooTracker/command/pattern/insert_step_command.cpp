#include "insert_step_command.hpp"

InsertStepCommand::InsertStepCommand(std::weak_ptr<Module> mod, int songNum, int trackNum, int orderNum, int stepNum)
	: mod_(mod),
	  song_(songNum),
	  track_(trackNum),
	  order_(orderNum),
	  step_(stepNum)
{
}

void InsertStepCommand::redo()
{
	mod_.lock()->getSong(song_).getTrack(track_).getPatternFromOrderNumber(order_).insertStep(step_);
}

void InsertStepCommand::undo()
{
	mod_.lock()->getSong(song_).getTrack(track_).getPatternFromOrderNumber(order_).deletePreviousStep(step_ + 1);
}

int InsertStepCommand::getID() const
{
	return 0x2b;
}
