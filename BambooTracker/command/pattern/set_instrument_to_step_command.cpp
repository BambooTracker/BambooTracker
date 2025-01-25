/*
 * SPDX-FileCopyrightText: 2018 Rerrah
 * SPDX-License-Identifier: MIT
 */

#include "set_instrument_to_step_command.hpp"
#include "pattern_command_utils.hpp"

SetInstrumentToStepCommand::SetInstrumentToStepCommand(
		std::weak_ptr<Module> mod, int songNum, int trackNum, int orderNum,
		int stepNum, int instNum, bool secondEntry)
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

bool SetInstrumentToStepCommand::redo()
{
	command_utils::getStep(mod_, song_, track_, order_, step_).setInstrumentNumber(inst_);
	return true;
}

bool SetInstrumentToStepCommand::undo()
{
	command_utils::getStep(mod_, song_, track_, order_, step_).setInstrumentNumber(prevInst_);
	isSecondEntry_ = true;	// Forced complete
	return true;
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
