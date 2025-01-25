/*
 * SPDX-FileCopyrightText: 2018 Rerrah
 * SPDX-License-Identifier: MIT
 */

#include "set_volume_to_step_command.hpp"
#include "pattern_command_utils.hpp"
#include "effect.hpp"

SetVolumeToStepCommand::SetVolumeToStepCommand(
		std::weak_ptr<Module> mod, int songNum, int trackNum, int orderNum,
		int stepNum, int volume, bool isFMReversed, bool secondEntry)
	: AbstractCommand(CommandId::SetVolumeToStep),
	  mod_(mod),
	  song_(songNum),
	  track_(trackNum),
	  order_(orderNum),
	  step_(stepNum),
	  vol_(volume),
	  prevVol_(command_utils::getStep(mod, songNum, trackNum, orderNum, stepNum).getVolume()),
	  isFMReversed_(isFMReversed),
	  isSecondEntry_(secondEntry)
{
}

bool SetVolumeToStepCommand::redo()
{
	int volume = isFMReversed_ ? effect_utils::reverseFmVolume(vol_) : vol_;
	command_utils::getStep(mod_, song_, track_, order_, step_).setVolume(volume);
	return true;
}

bool SetVolumeToStepCommand::undo()
{
	command_utils::getStep(mod_, song_, track_, order_, step_).setVolume(prevVol_);
	isSecondEntry_ = true;	// Forced complete
	return true;
}

bool SetVolumeToStepCommand::mergeWith(const AbstractCommand* other)
{
	if (other->getID() == getID() && !isSecondEntry_) {
		auto com = dynamic_cast<const SetVolumeToStepCommand*>(other);
		if (com->song_ == song_ && com->track_ == track_
				&& com->order_ == order_ && com->step_ == step_
				&& com->isSecondEntry_) {
			vol_ = (vol_ << 4) + com->vol_;
			redo();
			isSecondEntry_ = true;
			return true;
		}
	}

	isSecondEntry_ = true;
	return false;
}
