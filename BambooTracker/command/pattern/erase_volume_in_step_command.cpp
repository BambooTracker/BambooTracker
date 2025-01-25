/*
 * SPDX-FileCopyrightText: 2018 Rerrah
 * SPDX-License-Identifier: MIT
 */

#include "erase_volume_in_step_command.hpp"
#include "pattern_command_utils.hpp"

EraseVolumeInStepCommand::EraseVolumeInStepCommand(
		std::weak_ptr<Module> mod, int songNum, int trackNum, int orderNum, int stepNum)
	: AbstractCommand(CommandId::EraseVolumeInStep),
	  mod_(mod),
	  song_(songNum),
	  track_(trackNum),
	  order_(orderNum),
	  step_(stepNum)
{
	prevVol_ = command_utils::getStep(mod, songNum, trackNum, orderNum, stepNum).getVolume();
}

bool EraseVolumeInStepCommand::redo()
{
	command_utils::getStep(mod_, song_, track_, order_, step_).clearVolume();
	return true;
}

bool EraseVolumeInStepCommand::undo()
{
	command_utils::getStep(mod_, song_, track_, order_, step_).setVolume(prevVol_);
	return true;
}
