/*
 * SPDX-FileCopyrightText: 2018 Rerrah
 * SPDX-License-Identifier: MIT
 */

#include "set_effect_value_to_step_command.hpp"
#include "pattern_command_utils.hpp"
#include "effect.hpp"

SetEffectValueToStepCommand::SetEffectValueToStepCommand(
		std::weak_ptr<Module> mod, int songNum, int trackNum, int orderNum, int stepNum,
		int n, int value, EffectDisplayControl ctrl, bool secondEntry)
	: AbstractCommand(CommandId::SetEffectValueToStep),
	  mod_(mod),
	  song_(songNum),
	  track_(trackNum),
	  order_(orderNum),
	  step_(stepNum),
	  n_(n),
	  val_(value),
	  ctrl_(ctrl),
	  isSecondEntry_(secondEntry)
{
	prevVal_ = command_utils::getStep(mod, songNum, trackNum, orderNum, stepNum).getEffectValue(n);
}

bool SetEffectValueToStepCommand::redo()
{
	int value;
	switch (ctrl_) {
	default:
	case EffectDisplayControl::Unset:
		value = val_;
		break;
	case EffectDisplayControl::ReverseFMVolumeDelay:
		value = effect_utils::reverseFmVolume(val_);
		break;
	case EffectDisplayControl::ReverseFMBrightness:
		value = effect_utils::reverseFmBrightness(val_);
		break;
	}
	command_utils::getStep(mod_, song_, track_, order_, step_).setEffectValue(n_, value);
	return true;
}

bool SetEffectValueToStepCommand::undo()
{
	command_utils::getStep(mod_, song_, track_, order_, step_).setEffectValue(n_, prevVal_);
	isSecondEntry_ = true;	// Forced complete
	return true;
}

bool SetEffectValueToStepCommand::mergeWith(const AbstractCommand* other)
{
	if (other->getID() == getID() && !isSecondEntry_) {
		auto com = dynamic_cast<const SetEffectValueToStepCommand*>(other);
		if (com->song_ == song_ && com->track_ == track_
				&& com->order_ == order_ && com->step_ == step_ && com->n_ == n_
				&& com->isSecondEntry_) {
			val_ = (val_ << 4) + com->val_;
			redo();
			isSecondEntry_ = true;
			return true;
		}
	}

	isSecondEntry_ = true;
	return false;
}
