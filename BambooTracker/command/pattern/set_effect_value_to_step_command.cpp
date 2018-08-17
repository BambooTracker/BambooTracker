#include "set_effect_value_to_step_command.hpp"

SetEffectValueToStepCommand::SetEffectValueToStepCommand(std::weak_ptr<Module> mod, int songNum, int trackNum, int orderNum, int stepNum, int value)
	: mod_(mod),
	  song_(songNum),
	  track_(trackNum),
	  order_(orderNum),
	  step_(stepNum),
	  val_(value),
	  isComplete_(false)
{
	prevVal_ = mod_.lock()->getSong(songNum).getTrack(trackNum).getPatternFromOrderNumber(orderNum)
				 .getStep(stepNum).getEffectValue();
}

void SetEffectValueToStepCommand::redo()
{
	mod_.lock()->getSong(song_).getTrack(track_).getPatternFromOrderNumber(order_)
			.getStep(step_).setEffectValue(val_);
}

void SetEffectValueToStepCommand::undo()
{
	mod_.lock()->getSong(song_).getTrack(track_).getPatternFromOrderNumber(order_)
			.getStep(step_).setEffectValue(prevVal_);
}

int SetEffectValueToStepCommand::getID() const
{
	return 0x2a;
}

bool SetEffectValueToStepCommand::mergeWith(const AbstructCommand* other)
{
	if (other->getID() == getID() && !isComplete_) {
		auto com = dynamic_cast<const SetEffectValueToStepCommand*>(other);
		if (com->getSong() == song_ && com->getTrack() == track_
				&& com->getOrder() == order_ && com->getStep() == step_) {
			val_ = (val_ << 4) + com->getEffectValue();
			redo();
			isComplete_ = true;
			return true;
		}
	}

	isComplete_ = true;
	return false;
}

int SetEffectValueToStepCommand::getSong() const
{
	return song_;
}

int SetEffectValueToStepCommand::getTrack() const
{
	return track_;
}

int SetEffectValueToStepCommand::getOrder() const
{
	return order_;
}

int SetEffectValueToStepCommand::getStep() const
{
	return step_;
}

int SetEffectValueToStepCommand::getEffectValue() const
{
	return val_;
}
