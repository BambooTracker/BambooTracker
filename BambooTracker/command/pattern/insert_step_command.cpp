/*
 * SPDX-FileCopyrightText: 2018 Rerrah
 * SPDX-License-Identifier: MIT
 */

#include "insert_step_command.hpp"
#include "pattern_command_utils.hpp"

InsertStepCommand::InsertStepCommand(
		std::weak_ptr<Module> mod, int songNum, int trackNum, int orderNum, int stepNum)
	: AbstractCommand(CommandId::InsertStep),
	  mod_(mod),
	  song_(songNum),
	  track_(trackNum),
	  order_(orderNum),
	  step_(stepNum)
{
}

bool InsertStepCommand::redo()
{
	command_utils::getPattern(mod_, song_, track_, order_).insertStep(step_);
	return true;
}

bool InsertStepCommand::undo()
{
	command_utils::getPattern(mod_, song_, track_, order_).deletePreviousStep(step_ + 1);
	return true;
}
