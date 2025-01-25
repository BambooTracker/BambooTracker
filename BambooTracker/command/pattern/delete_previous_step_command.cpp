/*
 * SPDX-FileCopyrightText: 2018 Rerrah
 * SPDX-License-Identifier: MIT
 */

#include "delete_previous_step_command.hpp"
#include "pattern_command_utils.hpp"

DeletePreviousStepCommand::DeletePreviousStepCommand(
		std::weak_ptr<Module> mod, int songNum, int trackNum, int orderNum, int stepNum)
	: AbstractCommand(CommandId::DeletePreviousStep),
	  mod_(mod),
	  song_(songNum),
	  track_(trackNum),
	  order_(orderNum),
	  step_(stepNum)
{
	Step& st = command_utils::getStep(mod, songNum, trackNum, orderNum, stepNum - 1);
	prevNote_ = st.getNoteNumber();
	prevInst_ = st.getInstrumentNumber();
	prevVol_ = st.getVolume();
	for (int i = 0; i < Step::N_EFFECT; ++i) {
		prevEff_[i] = st.getEffect(i);
	}
}

bool DeletePreviousStepCommand::redo()
{
	command_utils::getPattern(mod_, song_, track_, order_).deletePreviousStep(step_);
	return true;
}

bool DeletePreviousStepCommand::undo()
{
	auto& pt = command_utils::getPattern(mod_, song_, track_, order_);
	pt.insertStep(step_ - 1);	// Insert previous step
	auto& st = pt.getStep(step_ - 1);
	st.setNoteNumber(prevNote_);
	st.setInstrumentNumber(prevInst_);
	st.setVolume(prevVol_);
	for (int i = 0; i < Step::N_EFFECT; ++i) {
		st.setEffect(i, prevEff_[i]);
	}
	return true;
}
