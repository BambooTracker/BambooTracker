/*
 * SPDX-FileCopyrightText: 2018 Rerrah
 * SPDX-License-Identifier: MIT
 */

#include "set_key_off_to_step_command.hpp"
#include "pattern_command_utils.hpp"

SetKeyOffToStepCommand::SetKeyOffToStepCommand(
		std::weak_ptr<Module> mod, int songNum, int trackNum, int orderNum, int stepNum)
	: AbstractCommand(CommandId::SetKeyOffToStep),
	  mod_(mod),
	  song_(songNum),
	  track_(trackNum),
	  order_(orderNum),
	  step_(stepNum)
{
	Step& st = command_utils::getStep(mod, songNum, trackNum, orderNum, stepNum);
	prevNote_ = st.getNoteNumber();
	prevInst_ = st.getInstrumentNumber();
	prevVol_ = st.getVolume();
	for (int i = 0; i < Step::N_EFFECT; ++i) {
		prevEff_[i] = st.getEffect(i);
	}
}

bool SetKeyOffToStepCommand::redo()
{
	Step& st = command_utils::getStep(mod_, song_, track_, order_, step_);
	st.setKeyOff();
	st.clearInstrumentNumber();
	st.clearVolume();
	for (int i = 0; i < Step::N_EFFECT; ++i) {
		st.clearEffect(i);
	}
	return true;
}

bool SetKeyOffToStepCommand::undo()
{
	Step& st = command_utils::getStep(mod_, song_, track_, order_, step_);
	st.setNoteNumber(prevNote_);
	st.setInstrumentNumber(prevInst_);
	st.setVolume(prevVol_);
	for (int i = 0; i < Step::N_EFFECT; ++i) {
		st.setEffect(i, prevEff_[i]);
	}
	return true;
}
