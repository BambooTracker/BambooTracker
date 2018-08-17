#include "delete_previous_step_command.hpp"

DeletePreviousStepCommand::DeletePreviousStepCommand(std::weak_ptr<Module> mod, int songNum, int trackNum, int orderNum, int stepNum)
	: mod_(mod),
	  song_(songNum),
	  track_(trackNum),
	  order_(orderNum),
	  step_(stepNum)
{
	auto& st = mod_.lock()->getSong(songNum).getTrack(trackNum)
			   .getPatternFromOrderNumber(orderNum).getStep(stepNum - 1);
	prevNote_ = st.getNoteNumber();
	prevInst_ = st.getInstrumentNumber();
	prevVol_ = st.getVolume();
	prevEffID_ = st.getEffectID();
	prevEffVal_ = st.getEffectValue();
}

void DeletePreviousStepCommand::redo()
{
	mod_.lock()->getSong(song_).getTrack(track_)
			.getPatternFromOrderNumber(order_).deletePreviousStep(step_);
}

void DeletePreviousStepCommand::undo()
{
	auto& pt =  mod_.lock()->getSong(song_).getTrack(track_).getPatternFromOrderNumber(order_);
	pt.insertStep(step_ - 1);	// Insert previous step
	auto& st = pt.getStep(step_ - 1);
	st.setNoteNumber(prevNote_);
	st.setInstrumentNumber(prevInst_);
	st.setVolume(prevVol_);
	st.setEffectID(prevEffID_);
	st.setEffectValue(prevEffVal_);
}

int DeletePreviousStepCommand::getID() const
{
	return 0x2d;
}
