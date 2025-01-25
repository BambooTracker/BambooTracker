/*
 * SPDX-FileCopyrightText: 2018 Rerrah
 * SPDX-License-Identifier: MIT
 */

#include "erase_instrument_in_step_command.hpp"
#include "pattern_command_utils.hpp"

EraseInstrumentInStepCommand::EraseInstrumentInStepCommand(
		std::weak_ptr<Module> mod, int songNum, int trackNum, int orderNum, int stepNum)
	: AbstractCommand(CommandId::EraseInstrumentInStep),
	  mod_(mod),
	  song_(songNum),
	  track_(trackNum),
	  order_(orderNum),
	  step_(stepNum)
{
	prevInst_ = command_utils::getStep(mod, songNum, trackNum, orderNum, stepNum)
				.getInstrumentNumber();
}

bool EraseInstrumentInStepCommand::redo()
{
	command_utils::getStep(mod_, song_, track_, order_, step_).clearInstrumentNumber();
	return true;
}

bool EraseInstrumentInStepCommand::undo()
{
	command_utils::getStep(mod_, song_, track_, order_, step_).setInstrumentNumber(prevInst_);
	return true;
}
