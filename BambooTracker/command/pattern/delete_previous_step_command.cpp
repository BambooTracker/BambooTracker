/*
 * Copyright (C) 2018-2019 Rerrah
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
	for (int i = 0; i < 4; ++i) {
		prevEffID_[i] = st.getEffectID(i);
		prevEffVal_[i] = st.getEffectValue(i);
	}
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
	for (int i = 0; i < 4; ++i) {
		st.setEffectID(i, prevEffID_[i]);
		st.setEffectValue(i, prevEffVal_[i]);
	}
}

CommandId DeletePreviousStepCommand::getID() const
{
	return CommandId::DeletePreviousStep;
}
