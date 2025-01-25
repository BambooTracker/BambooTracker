/*
 * SPDX-FileCopyrightText: 2018 Rerrah
 * SPDX-License-Identifier: MIT
 */

#include "erase_effect_in_step_command.hpp"
#include "pattern_command_utils.hpp"

EraseEffectInStepCommand::EraseEffectInStepCommand(
		std::weak_ptr<Module> mod, int songNum, int trackNum, int orderNum, int stepNum, int n)
	: AbstractCommand(CommandId::EraseEffectInStep),
	  mod_(mod),
	  song_(songNum),
	  track_(trackNum),
	  order_(orderNum),
	  step_(stepNum),
	  n_(n)
{
	prevEff_ = command_utils::getStep(mod, songNum, trackNum, orderNum, stepNum).getEffect(n);
}

bool EraseEffectInStepCommand::redo()
{
	command_utils::getStep(mod_, song_, track_, order_, step_).clearEffect(n_);
	return true;
}

bool EraseEffectInStepCommand::undo()
{
	command_utils::getStep(mod_, song_, track_, order_, step_).setEffect(n_, prevEff_);
	return true;
}
