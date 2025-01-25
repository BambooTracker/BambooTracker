/*
 * SPDX-FileCopyrightText: 2018 Rerrah
 * SPDX-License-Identifier: MIT
 */

#include "set_key_on_to_step_command.hpp"
#include "pattern_command_utils.hpp"
#include "effect.hpp"

SetKeyOnToStepCommand::SetKeyOnToStepCommand(
		std::weak_ptr<Module> mod, int songNum, int trackNum, int orderNum, int stepNum,
		int noteNum, bool instMask, int instNum, bool volMask, int vol, bool isFMReversed)
	: AbstractCommand(CommandId::SetKeyOnToStep),
	  mod_(mod),
	  song_(songNum),
	  track_(trackNum),
	  order_(orderNum),
	  step_(stepNum),
	  note_(noteNum),
	  inst_(instNum),
	  vol_(vol),
	  instMask_(instMask),
	  volMask_(volMask),
	  isFMReversed_(isFMReversed)
{
	Step& st = command_utils::getStep(mod, songNum, trackNum, orderNum, stepNum);
	prevNote_ = st.getNoteNumber();
	if (!instMask) prevInst_ = st.getInstrumentNumber();
	if (!volMask) prevVol_ = st.getVolume();
}

bool SetKeyOnToStepCommand::redo()
{
	Step& st = command_utils::getStep(mod_, song_, track_, order_, step_);
	st.setNoteNumber(note_);
	if (!instMask_) st.setInstrumentNumber(inst_);
	if (!volMask_) st.setVolume(isFMReversed_ ? effect_utils::reverseFmVolume(vol_) : vol_);
	return true;
}

bool SetKeyOnToStepCommand::undo()
{
	Step& st = command_utils::getStep(mod_, song_, track_, order_, step_);
	st.setNoteNumber(prevNote_);
	if (!instMask_) st.setInstrumentNumber(prevInst_);
	if (!volMask_) st.setVolume(prevVol_);
	return true;
}
