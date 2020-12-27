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

#include "set_instrument_to_step_command.hpp"
#include "pattern_command_utils.hpp"

SetInstrumentToStepCommand::SetInstrumentToStepCommand(std::weak_ptr<Module> mod, int songNum, int trackNum, int orderNum, int stepNum, int instNum, bool secondEntry)
	: AbstractCommand(CommandId::SetInstrumentInStep),
	  mod_(mod),
	  song_(songNum),
	  track_(trackNum),
	  order_(orderNum),
	  step_(stepNum),
	  inst_(instNum),
	  prevInst_(command_utils::getStep(mod, songNum, trackNum, orderNum, stepNum).getInstrumentNumber()),
	  isSecondEntry_(secondEntry)
{
}

void SetInstrumentToStepCommand::redo()
{
	command_utils::getStep(mod_, song_, track_, order_, step_).setInstrumentNumber(inst_);
}

void SetInstrumentToStepCommand::undo()
{
	command_utils::getStep(mod_, song_, track_, order_, step_).setInstrumentNumber(prevInst_);
	isSecondEntry_ = true;	// Forced complete
}

bool SetInstrumentToStepCommand::mergeWith(const AbstractCommand* other)
{
	if (other->getID() == getID() && !isSecondEntry_) {
		auto com = dynamic_cast<const SetInstrumentToStepCommand*>(other);
		if (com->song_ == song_ && com->track_ == track_
				&& com->order_ == order_ && com->step_ == step_
				&& com->isSecondEntry_) {
			inst_ = (inst_ << 4) + com->inst_;
			redo();
			isSecondEntry_ = true;
			return true;
		}
	}

	isSecondEntry_ = true;
	return false;
}
