#include "set_effect_id_to_step_command.hpp"

SetEffectIDToStepCommand::SetEffectIDToStepCommand(std::weak_ptr<Module> mod, int songNum, int trackNum, int orderNum, int stepNum, std::string id)
	: mod_(mod),
	  song_(songNum),
	  track_(trackNum),
	  order_(orderNum),
	  step_(stepNum),
	  effID_(id),
	  isComplete_(false)
{
	prevEffID_ = mod_.lock()->getSong(songNum).getTrack(trackNum).getPatternFromOrderNumber(orderNum)
				 .getStep(stepNum).getEffectID();
}

void SetEffectIDToStepCommand::redo()
{
	std::string str = isComplete_ ? effID_ : ("0" + effID_);
	mod_.lock()->getSong(song_).getTrack(track_).getPatternFromOrderNumber(order_)
					.getStep(step_).setEffectID(str);
}

void SetEffectIDToStepCommand::undo()
{
	mod_.lock()->getSong(song_).getTrack(track_).getPatternFromOrderNumber(order_)
					.getStep(step_).setEffectID(prevEffID_);
}

int SetEffectIDToStepCommand::getID() const
{
	return 0x27;
}

bool SetEffectIDToStepCommand::mergeWith(const AbstructCommand* other)
{
	if (other->getID() == getID() && !isComplete_) {
		auto com = dynamic_cast<const SetEffectIDToStepCommand*>(other);
		if (com->getSong() == song_ && com->getTrack() == track_
				&& com->getOrder() == order_ && com->getStep() == step_) {
			effID_ += com->getEffectID();
			isComplete_ = true;
			redo();
			return true;
		}
	}

	isComplete_ = true;
	return false;
}

int SetEffectIDToStepCommand::getSong() const
{
	return song_;
}

int SetEffectIDToStepCommand::getTrack() const
{
	return track_;
}

int SetEffectIDToStepCommand::getOrder() const
{
	return order_;
}

int SetEffectIDToStepCommand::getStep() const
{
	return step_;
}

std::string SetEffectIDToStepCommand::getEffectID() const
{
	return effID_;
}
