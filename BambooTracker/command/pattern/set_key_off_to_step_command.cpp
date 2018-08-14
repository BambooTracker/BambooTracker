#include "set_key_off_to_step_command.hpp"

SetKeyOffToStepCommand::SetKeyOffToStepCommand(std::weak_ptr<Module> mod, int songNum, int trackNum, int orderNum, int stepNum)
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
	prevEff_ = st.getEffectString();
}

void SetKeyOffToStepCommand::redo()
{
	auto& st = mod_.lock()->getSong(song_).getTrack(track_).getPatternFromOrderNumber(order_).getStep(step_);
	st.setNoteNumber(-3);
	st.setInstrumentNumber(-1);
	st.setVolume(-1);
	st.setEffectString(u8"---");
}

void SetKeyOffToStepCommand::undo()
{
	auto& st = mod_.lock()->getSong(song_).getTrack(track_).getPatternFromOrderNumber(order_).getStep(step_);
	st.setNoteNumber(prevNote_);
	st.setInstrumentNumber(prevInst_);
	st.setVolume(prevVol_);
	st.setEffectString(prevEff_);
}

int SetKeyOffToStepCommand::getID() const
{
	return 0x23;
}
