#include "set_effect_value_to_step_command.hpp"

SetEffectValueToStepCommand::SetEffectValueToStepCommand(std::weak_ptr<Module> mod, int songNum, int trackNum,
														 int orderNum, int stepNum, int n, int value,
														 EffectDisplayControl ctrl, bool secondEntry)
	: mod_(mod),
	  song_(songNum),
	  track_(trackNum),
	  order_(orderNum),
	  step_(stepNum),
	  n_(n),
	  val_(value),
	  ctrl_(ctrl),
	  isSecond_(secondEntry)
{
	prevVal_ = mod_.lock()->getSong(songNum).getTrack(trackNum).getPatternFromOrderNumber(orderNum)
				 .getStep(stepNum).getEffectValue(n);
}

void SetEffectValueToStepCommand::redo()
{
	int value;
	switch (ctrl_) {
	default:
	case EffectDisplayControl::Unset:
		value = val_;
		break;
	case EffectDisplayControl::ReverseFMVolumeDelay:
		value = (val_ < 0x80) ? (0x7f - val_) : val_;
		break;
	case EffectDisplayControl::ReverseFMBrightness:
		value = (val_ > 0) ? (0xff - val_ + 1) : val_;
		break;
	}
	mod_.lock()->getSong(song_).getTrack(track_).getPatternFromOrderNumber(order_)
			.getStep(step_).setEffectValue(n_, value);
}

void SetEffectValueToStepCommand::undo()
{
	mod_.lock()->getSong(song_).getTrack(track_).getPatternFromOrderNumber(order_)
			.getStep(step_).setEffectValue(n_, prevVal_);
	isSecond_ = true;	// Forced complete
}

CommandId SetEffectValueToStepCommand::getID() const
{
	return CommandId::SetEffectValueToStep;
}

bool SetEffectValueToStepCommand::mergeWith(const AbstractCommand* other)
{
	if (other->getID() == getID() && !isSecond_) {
		auto com = dynamic_cast<const SetEffectValueToStepCommand*>(other);
		if (com->getSong() == song_ && com->getTrack() == track_
				&& com->getOrder() == order_ && com->getStep() == step_ && com->getN() == n_
				&& com->isSecondEntry()) {
			val_ = (val_ << 4) + com->getEffectValue();
			redo();
			isSecond_ = true;
			return true;
		}
	}

	isSecond_ = true;
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

bool SetEffectValueToStepCommand::isSecondEntry() const
{
	return isSecond_;
}

int SetEffectValueToStepCommand::getEffectValue() const
{
	return val_;
}
