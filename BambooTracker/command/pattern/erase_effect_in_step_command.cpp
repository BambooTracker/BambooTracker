#include "erase_effect_in_step_command.hpp"

EraseEffectInStepCommand::EraseEffectInStepCommand(std::weak_ptr<Module> mod, int songNum, int trackNum, int orderNum, int stepNum, int n)
	: mod_(mod),
	  song_(songNum),
	  track_(trackNum),
	  order_(orderNum),
	  step_(stepNum),
	  n_(n)
{
	auto& st = mod_.lock()->getSong(songNum).getTrack(trackNum).getPatternFromOrderNumber(orderNum).getStep(stepNum);
	prevEffID_ = st.getEffectID(n);
	prevEffVal_ = st.getEffectValue(n);
}

void EraseEffectInStepCommand::redo()
{
	auto& st = mod_.lock()->getSong(song_).getTrack(track_).getPatternFromOrderNumber(order_).getStep(step_);
	st.setEffectID(n_, "--");
	st.setEffectValue(n_, -1);
}

void EraseEffectInStepCommand::undo()
{
	auto& st = mod_.lock()->getSong(song_).getTrack(track_).getPatternFromOrderNumber(order_).getStep(step_);
	st.setEffectID(n_, prevEffID_);
	st.setEffectValue(n_, prevEffVal_);
}

CommandId EraseEffectInStepCommand::getID() const
{
	return CommandId::EraseEffectInStep;
}
