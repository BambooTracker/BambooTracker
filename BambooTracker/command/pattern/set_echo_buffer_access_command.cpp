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

void SetEchoBufferAccessCommand::redo()
{
	command_utils::getStep(mod_, song_, track_, order_, step_).setNoteNumber(-buf_ - 3);
}

void SetEchoBufferAccessCommand::undo()
{
	command_utils::getStep(mod_, song_, track_, order_, step_).setNoteNumber(prevNote_);
}
