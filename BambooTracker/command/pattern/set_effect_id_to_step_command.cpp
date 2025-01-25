/*
 * SPDX-FileCopyrightText: 2018 Rerrah
 * SPDX-License-Identifier: MIT
 */

#include "set_effect_id_to_step_command.hpp"
#include "pattern_command_utils.hpp"

SetEffectIDToStepCommand::SetEffectIDToStepCommand(
		std::weak_ptr<Module> mod, int songNum, int trackNum, int orderNum, int stepNum,
		int n, std::string id, bool fillValue00, bool secondEntry)
	: AbstractCommand(CommandId::SetEffectIDToStep),
	  mod_(mod),
	  song_(songNum),
	  track_(trackNum),
	  order_(orderNum),
	  step_(stepNum),
	  n_(n),
	  effID_(id),
	  isSecondEntry_(secondEntry)
{
	Step& step = command_utils::getStep(mod, songNum, trackNum, orderNum, stepNum);
	prevEffID_ = step.getEffectId(n);
	filledValue00_ = fillValue00 && !step.hasEffectValue(n);
}

bool SetEffectIDToStepCommand::redo()
{
	std::string str = isSecondEntry_ ? effID_ : ("0" + effID_);
	Step& step = command_utils::getStep(mod_, song_, track_, order_, step_);
	step.setEffectId(n_, str);
	if (filledValue00_) step.setEffectValue(n_, 0);
	return true;
}

bool SetEffectIDToStepCommand::undo()
{
	Step& step = command_utils::getStep(mod_, song_, track_, order_, step_);
	step.setEffectId(n_, prevEffID_);
	if (filledValue00_) step.clearEffectValue(n_);

	if (!isSecondEntry_) {	// Forced complete
		effID_ = "0" + effID_;
		isSecondEntry_ = true;
	}
	return true;
}

bool SetEffectIDToStepCommand::mergeWith(const AbstractCommand* other)
{
	if (other->getID() == getID() && !isSecondEntry_) {
		auto com = dynamic_cast<const SetEffectIDToStepCommand*>(other);
		if (com->song_ == song_ && com->track_ == track_
				&& com->order_ == order_ && com->step_ == step_ && com->n_ == n_
				&& com->isSecondEntry_) {
			effID_ = effID_ + com->effID_;
			isSecondEntry_ = true;
			redo();
			return true;
		}
	}

	// Enterd only 1 character
	if (!isSecondEntry_) {
		effID_ = "0" + effID_;
		isSecondEntry_ = true;
	}

	return false;
}
