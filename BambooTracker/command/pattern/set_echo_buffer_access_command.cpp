#include "set_echo_buffer_access_command.hpp"

SetEchoBufferAccessCommand::SetEchoBufferAccessCommand(std::weak_ptr<Module> mod, int songNum, int trackNum, int orderNum, int stepNum, int bufNum)
	: mod_(mod),
	  song_(songNum),
	  track_(trackNum),
	  order_(orderNum),
	  step_(stepNum),
	  buf_(bufNum)
{
	prevNote_ = mod_.lock()->getSong(songNum).getTrack(trackNum).getPatternFromOrderNumber(orderNum)
				.getStep(stepNum).getNoteNumber();
}

void SetEchoBufferAccessCommand::redo()
{
	mod_.lock()->getSong(song_).getTrack(track_).getPatternFromOrderNumber(order_)
			.getStep(step_).setNoteNumber(-buf_ - 3);
}

void SetEchoBufferAccessCommand::undo()
{
	mod_.lock()->getSong(song_).getTrack(track_).getPatternFromOrderNumber(order_)
			.getStep(step_).setNoteNumber(prevNote_);
}

int SetEchoBufferAccessCommand::getID() const
{
	return 0x36;
}
