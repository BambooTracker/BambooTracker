#include "set_effect_value_to_step_command.hpp"

SetEffectValueToStepCommand::SetEffectValueToStepCommand(std::weak_ptr<Module> mod, int songNum, int trackNum,
														 int orderNum, int stepNum, int n, int value,
														 bool isFMReversed)
	: mod_(mod),
	  song_(songNum),
	  track_(trackNum),
	  order_(orderNum),
	  step_(stepNum),
	  n_(n),
	  val_(value),
	  isComplete_(false),
	  isFMReserved_(isFMReversed)
{
	prevVal_ = mod_.lock()->getSong(songNum).getTrack(trackNum).getPatternFromOrderNumber(orderNum)
				 .getStep(stepNum).getEffectValue(n);
}

void SetEffectValueToStepCommand::redo()
{
	int value = (isFMReserved_ && val_ < 0x80) ? (0x7f - val_) : val_;	// For effect Mxyy of FM
	mod_.lock()->getSong(song_).getTrack(track_).getPatternFromOrderNumber(order_)
			.getStep(step_).setEffectValue(n_, value);
}

void SetEffectValueToStepCommand::undo()
{
	mod_.lock()->getSong(song_).getTrack(track_).getPatternFromOrderNumber(order_)
			.getStep(step_).setEffectValue(n_, prevVal_);
}

CommandId SetEffectValueToStepCommand::getID() const
{
	return CommandId::SetEffectValueToStep;
}

bool SetEffectValueToStepCommand::mergeWith(const AbstractCommand* other)
{
	if (other->getID() == getID() && !isComplete_) {
		auto com = dynamic_cast<const SetEffectValueToStepCommand*>(other);
		if (com->getSong() == song_ && com->getTrack() == track_
				&& com->getOrder() == order_ && com->getStep() == step_ && com->getN() == n_) {
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

int SetEffectValueToStepCommand::getN() const
{
	return n_;
}

int SetEffectValueToStepCommand::getEffectValue() const
{
	return val_;
}
