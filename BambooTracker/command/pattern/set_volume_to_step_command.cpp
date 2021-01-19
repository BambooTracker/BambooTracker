/*
 * Copyright (C) 2018-2021 Rerrah
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

void SetVolumeToStepCommand::redo()
{
	int volume = isFMReversed_ ? effect_utils::reverseFmVolume(vol_) : vol_;
	command_utils::getStep(mod_, song_, track_, order_, step_).setVolume(volume);
}

void SetVolumeToStepCommand::undo()
{
	command_utils::getStep(mod_, song_, track_, order_, step_).setVolume(prevVol_);
	isSecondEntry_ = true;	// Forced complete
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
