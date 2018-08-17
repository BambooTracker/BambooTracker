#include "erase_effect_in_step_command.hpp"

EraseEffectInStepCommand::EraseEffectInStepCommand(std::weak_ptr<Module> mod, int songNum, int trackNum, int orderNum, int stepNum)
	: mod_(mod),
	  song_(songNum),
	  track_(trackNum),
	  order_(orderNum),
	  step_(stepNum)
{
	auto& st = mod_.lock()->getSong(songNum).getTrack(trackNum).getPatternFromOrderNumber(orderNum).getStep(stepNum);
	prevEffID_ = st.getEffectID();
	prevEffVal_ = st.getEffectValue();
}

void EraseEffectInStepCommand::redo()
{
	auto& st = mod_.lock()->getSong(song_).getTrack(track_).getPatternFromOrderNumber(order_).getStep(step_);
	st.setEffectID("--");
	st.setEffectValue(-1);
}

void EraseEffectInStepCommand::undo()
{
	auto& st = mod_.lock()->getSong(song_).getTrack(track_).getPatternFromOrderNumber(order_).getStep(step_);
	st.setEffectID(prevEffID_);
	st.setEffectValue(prevEffVal_);
}

int EraseEffectInStepCommand::getID() const
{
	return 0x28;
}
