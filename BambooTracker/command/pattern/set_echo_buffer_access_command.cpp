/*
 * SPDX-FileCopyrightText: 2018 Rerrah
 * SPDX-License-Identifier: MIT
 */

#include "set_echo_buffer_access_command.hpp"
#include "pattern_command_utils.hpp"

SetEchoBufferAccessCommand::SetEchoBufferAccessCommand(
		std::weak_ptr<Module> mod, int songNum, int trackNum, int orderNum, int stepNum, int bufNum)
	: AbstractCommand(CommandId::SetEchoBufferAccess),
	  mod_(mod),
	  song_(songNum),
	  track_(trackNum),
	  order_(orderNum),
	  step_(stepNum),
	  buf_(bufNum)
{
	prevNote_ = command_utils::getStep(mod, songNum, trackNum, orderNum, stepNum).getNoteNumber();
}

bool SetEchoBufferAccessCommand::redo()
{
	command_utils::getStep(mod_, song_, track_, order_, step_).setEchoBuffer(buf_);
	return true;
}

bool SetEchoBufferAccessCommand::undo()
{
	command_utils::getStep(mod_, song_, track_, order_, step_).setNoteNumber(prevNote_);
	return true;
}
