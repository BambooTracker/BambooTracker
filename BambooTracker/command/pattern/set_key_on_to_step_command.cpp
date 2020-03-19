#include "set_key_on_to_step_command.hpp"

SetKeyOnToStepCommand::SetKeyOnToStepCommand(std::weak_ptr<Module> mod, int songNum, int trackNum, int orderNum,
											 int stepNum, int noteNum, bool instMask, int instNum, bool volMask, int vol)
	: mod_(mod),
	  song_(songNum),
	  track_(trackNum),
	  order_(orderNum),
	  step_(stepNum),
	  note_(noteNum),
	  inst_(instNum),
	  vol_(vol),
	  instMask_(instMask),
	  volMask_(volMask)
{
	auto& st = mod_.lock()->getSong(songNum).getTrack(trackNum).getPatternFromOrderNumber(orderNum).getStep(stepNum);
	prevNote_ = st.getNoteNumber();
	if (!instMask) prevInst_ = st.getInstrumentNumber();
	if (!volMask) prevVol_ = st.getVolume();
}

void SetKeyOnToStepCommand::redo()
{
	auto& st = mod_.lock()->getSong(song_).getTrack(track_).getPatternFromOrderNumber(order_).getStep(step_);
	st.setNoteNumber(note_);
	if (!instMask_) st.setInstrumentNumber(inst_);
	if (!volMask_) st.setVolume(vol_);
}

void SetKeyOnToStepCommand::undo()
{
	auto& st = mod_.lock()->getSong(song_).getTrack(track_).getPatternFromOrderNumber(order_).getStep(step_);
	st.setNoteNumber(prevNote_);
	if (!instMask_) st.setInstrumentNumber(prevInst_);
	if (!volMask_) st.setVolume(prevVol_);
}

CommandId SetKeyOnToStepCommand::getID() const
{
	return CommandId::SetKeyOnToStep;
}
