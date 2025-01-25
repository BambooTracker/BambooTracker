/*
 * SPDX-FileCopyrightText: 2018 Rerrah
 * SPDX-License-Identifier: MIT
 */

#include "erase_effect_value_in_step_command.hpp"
#include "pattern_command_utils.hpp"

EraseEffectValueInStepCommand::EraseEffectValueInStepCommand(
		std::weak_ptr<Module> mod, int songNum, int trackNum, int orderNum, int stepNum, int n)
	: AbstractCommand(CommandId::EraseEffectValueInStep),
	  mod_(mod),
	  song_(songNum),
	  track_(trackNum),
	  order_(orderNum),
	  step_(stepNum),
	  n_(n)
{
	prevVal_ = command_utils::getStep(mod, songNum, trackNum, orderNum, stepNum).getEffectValue(n);
}

bool EraseEffectValueInStepCommand::redo()
{
	command_utils::getStep(mod_, song_, track_, order_, step_).clearEffectValue(n_);
	return true;
}

bool EraseEffectValueInStepCommand::undo()
{
	command_utils::getStep(mod_, song_, track_, order_, step_).setEffectValue(n_, prevVal_);
	return true;
}
