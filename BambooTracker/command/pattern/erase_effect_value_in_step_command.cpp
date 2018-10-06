#include "erase_effect_value_in_step_command.hpp"

EraseEffectValueInStepCommand::EraseEffectValueInStepCommand(std::weak_ptr<Module> mod, int songNum, int trackNum, int orderNum, int stepNum, int n)
	: mod_(mod),
	  song_(songNum),
	  track_(trackNum),
	  order_(orderNum),
	  step_(stepNum),
	  n_(n)
{
	prevVal_ = mod_.lock()->getSong(songNum).getTrack(trackNum).getPatternFromOrderNumber(orderNum)
			   .getStep(stepNum).getEffectValue(n);
}

void EraseEffectValueInStepCommand::redo()
{
	mod_.lock()->getSong(song_).getTrack(track_).getPatternFromOrderNumber(order_)
			.getStep(step_).setEffectValue(n_, -1);
}

void EraseEffectValueInStepCommand::undo()
{
	mod_.lock()->getSong(song_).getTrack(track_).getPatternFromOrderNumber(order_)
			.getStep(step_).setEffectValue(n_, prevVal_);
}

int EraseEffectValueInStepCommand::getID() const
{
	return 0x2a;
}
