#include "erase_step_command.hpp"

EraseStepCommand::EraseStepCommand(std::weak_ptr<Module> mod, int songNum, int trackNum, int orderNum, int stepNum)
	: mod_(mod),
	  song_(songNum),
	  track_(trackNum),
	  order_(orderNum),
	  step_(stepNum)
{
	auto& st = mod_.lock()->getSong(songNum).getTrack(trackNum).getPatternFromOrderNumber(orderNum).getStep(stepNum);
	prevNote_ = st.getNoteNumber();
	prevInst_ = st.getInstrumentNumber();
	prevVol_ = st.getVolume();
	for (int i = 0; i < 4; ++i) {
		prevEffID_[i] = st.getEffectID(i);
		prevEffVal_[i] = st.getEffectValue(i);
	}
}

void EraseStepCommand::redo()
{
	auto& st = mod_.lock()->getSong(song_).getTrack(track_).getPatternFromOrderNumber(order_).getStep(step_);
	st.setNoteNumber(-1);
	st.setInstrumentNumber(-1);
	st.setVolume(-1);
	for (int i = 0; i < 4; ++i){
		st.setEffectID(i, "--");
		st.setEffectValue(i, -1);
	}
}

void EraseStepCommand::undo()
{
	auto& st = mod_.lock()->getSong(song_).getTrack(track_).getPatternFromOrderNumber(order_).getStep(step_);
	st.setNoteNumber(prevNote_);
	st.setInstrumentNumber(prevInst_);
	st.setVolume(prevVol_);
	for (int i = 0; i < 4; ++i) {
		st.setEffectID(i, prevEffID_[i]);
		st.setEffectValue(i, prevEffVal_[i]);
	}
}

CommandId EraseStepCommand::getID() const
{
	return CommandId::EraseStep;
}
