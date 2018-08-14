#include "erase_note_in_step_command.hpp"

EraseNoteInStepCommand::EraseNoteInStepCommand(std::weak_ptr<Module> mod, int songNum, int trackNum, int orderNum, int stepNum)
	: mod_(mod),
	  song_(songNum),
	  track_(trackNum),
	  order_(orderNum),
	  step_(stepNum)
{
	prevNote_ = mod_.lock()->getSong(songNum).getTrack(trackNum).getPatternFromOrderNumber(orderNum)
				.getStep(stepNum).getNoteNumber();
}

void EraseNoteInStepCommand::redo()
{
	mod_.lock()->getSong(song_).getTrack(track_).getPatternFromOrderNumber(order_)
			.getStep(step_).setNoteNumber(-1);
}

void EraseNoteInStepCommand::undo()
{
	mod_.lock()->getSong(song_).getTrack(track_).getPatternFromOrderNumber(order_)
			.getStep(step_).setNoteNumber(prevNote_);
}

int EraseNoteInStepCommand::getID() const
{
	return 0x25;
}
