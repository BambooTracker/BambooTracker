#include "erase_instrument_in_step_command.hpp"

EraseInstrumentInStepCommand::EraseInstrumentInStepCommand(std::weak_ptr<Module> mod, int songNum, int trackNum, int orderNum, int stepNum)
	: mod_(mod),
	  song_(songNum),
	  track_(trackNum),
	  order_(orderNum),
	  step_(stepNum)
{
	prevInst_ = mod_.lock()->getSong(songNum).getTrack(trackNum).getPatternFromOrderNumber(orderNum)
				.getStep(stepNum).getInstrumentNumber();
}

void EraseInstrumentInStepCommand::redo()
{
	mod_.lock()->getSong(song_).getTrack(track_).getPatternFromOrderNumber(order_)
					.getStep(step_).setInstrumentNumber(-1);
}

void EraseInstrumentInStepCommand::undo()
{
	mod_.lock()->getSong(song_).getTrack(track_).getPatternFromOrderNumber(order_)
					.getStep(step_).setInstrumentNumber(prevInst_);
}

int EraseInstrumentInStepCommand::getID() const
{
	return 0x25;
}
