/*
 * Copyright (C) 2018-2020 Rerrah
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#include "set_key_on_to_step_command.hpp"
#include "pattern_command_utils.hpp"

SetKeyOnToStepCommand::SetKeyOnToStepCommand(std::weak_ptr<Module> mod, int songNum, int trackNum, int orderNum,
											 int stepNum, int noteNum, bool instMask, int instNum, bool volMask, int vol, bool isFMReversed)
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
	  isFMReserved_(isFMReversed)
{
	Step& st = command_utils::getStep(mod, songNum, trackNum, orderNum, stepNum);
	prevNote_ = st.getNoteNumber();
	if (!instMask) prevInst_ = st.getInstrumentNumber();
	if (!volMask) prevVol_ = st.getVolume();
}

void SetKeyOnToStepCommand::redo()
{
	Step& st = command_utils::getStep(mod_, song_, track_, order_, step_);
	st.setNoteNumber(note_);
	if (!instMask_) st.setInstrumentNumber(inst_);
	if (!volMask_) st.setVolume((isFMReserved_ && vol_ < 0x80) ? (0x7f - vol_) : vol_);
}

void SetKeyOnToStepCommand::undo()
{
	Step& st = command_utils::getStep(mod_, song_, track_, order_, step_);
	st.setNoteNumber(prevNote_);
	if (!instMask_) st.setInstrumentNumber(prevInst_);
	if (!volMask_) st.setVolume(prevVol_);
}
