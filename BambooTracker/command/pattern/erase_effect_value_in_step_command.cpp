#include "erase_effect_value_in_step_command.hpp"

EraseEffectValueInStepCommand::EraseEffectValueInStepCommand(std::weak_ptr<Module> mod, int songNum, int trackNum, int orderNum, int stepNum)
	: mod_(mod),
	  song_(songNum),
	  track_(trackNum),
	  order_(orderNum),
	  step_(stepNum)
{
	prevVal_ = mod_.lock()->getSong(songNum).getTrack(trackNum).getPatternFromOrderNumber(orderNum)
			   .getStep(stepNum).getEffectValue();
}

void EraseEffectValueInStepCommand::redo()
{
	mod_.lock()->getSong(song_).getTrack(track_).getPatternFromOrderNumber(order_)
			.getStep(step_).setEffectValue(-1);
}

void EraseEffectValueInStepCommand::undo()
{
	mod_.lock()->getSong(song_).getTrack(track_).getPatternFromOrderNumber(order_)
			.getStep(step_).setEffectValue(prevVal_);
}

int EraseEffectValueInStepCommand::getID() const
{
	return 0x2b;
}
