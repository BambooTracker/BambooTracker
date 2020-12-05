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

#include "set_effect_id_to_step_command.hpp"
#include "pattern_command_utils.hpp"

SetEffectIDToStepCommand::SetEffectIDToStepCommand(std::weak_ptr<Module> mod, int songNum,
												   int trackNum, int orderNum, int stepNum,
												   int n, std::string id, bool fillValue00,
												   bool secondEntry)
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
	prevEffID_ = step.getEffectID(n);
	filledValue00_ = fillValue00 && (step.getEffectValue(n) == -1);
}

void SetEffectIDToStepCommand::redo()
{
	std::string str = isSecondEntry_ ? effID_ : ("0" + effID_);
	Step& step = command_utils::getStep(mod_, song_, track_, order_, step_);
	step.setEffectID(n_, str);
	if (filledValue00_) step.setEffectValue(n_, 0);
}

void SetEffectIDToStepCommand::undo()
{
	Step& step = command_utils::getStep(mod_, song_, track_, order_, step_);
	step.setEffectID(n_, prevEffID_);
	if (filledValue00_) step.setEffectValue(n_, -1);

	if (!isSecondEntry_) {	// Forced complete
		effID_ = "0" + effID_;
		isSecondEntry_ = true;
	}
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
