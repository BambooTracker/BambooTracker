/*
 * SPDX-FileCopyrightText: 2018 Rerrah
 * SPDX-License-Identifier: MIT
 */

#include "erase_step_command.hpp"
#include "pattern_command_utils.hpp"

EraseStepCommand::EraseStepCommand(
		std::weak_ptr<Module> mod, int songNum, int trackNum, int orderNum, int stepNum)
	: AbstractCommand(CommandId::EraseStep),
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

bool EraseStepCommand::redo()
{
	command_utils::getStep(mod_, song_, track_, order_, step_).clear();
	return true;
}

bool EraseStepCommand::undo()
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
