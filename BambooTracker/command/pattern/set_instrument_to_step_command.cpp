#include "set_instrument_to_step_command.hpp"

SetInstrumentToStepCommand::SetInstrumentToStepCommand(std::weak_ptr<Module> mod, int songNum, int trackNum, int orderNum, int stepNum, int instNum, bool secondEntry)
	: mod_(mod),
	  song_(songNum),
	  track_(trackNum),
	  order_(orderNum),
	  step_(stepNum),
	  inst_(instNum),
	  isSecond_(secondEntry)
{
	prevInst_ = mod_.lock()->getSong(songNum).getTrack(trackNum).getPatternFromOrderNumber(orderNum)
				.getStep(stepNum).getInstrumentNumber();
}

void SetInstrumentToStepCommand::redo()
{
	mod_.lock()->getSong(song_).getTrack(track_).getPatternFromOrderNumber(order_)
			.getStep(step_).setInstrumentNumber(inst_);
}

void SetInstrumentToStepCommand::undo()
{
	mod_.lock()->getSong(song_).getTrack(track_).getPatternFromOrderNumber(order_)
			.getStep(step_).setInstrumentNumber(prevInst_);
	isSecond_ = true;	// Forced complete
}

CommandId SetInstrumentToStepCommand::getID() const
{
	return CommandId::SetInstrumentInStep;
}

bool SetInstrumentToStepCommand::mergeWith(const AbstractCommand* other)
{
	if (other->getID() == getID() && !isSecond_) {
		auto com = dynamic_cast<const SetInstrumentToStepCommand*>(other);
		if (com->getSong() == song_ && com->getTrack() == track_
				&& com->getOrder() == order_ && com->getStep() == step_
				&& com->isSecondEntry()) {
			inst_ = (inst_ << 4) + com->getInst();
			redo();
			isSecond_ = true;
			return true;
		}
	}

	isSecond_ = true;
	return false;
}

int SetInstrumentToStepCommand::getSong() const
{
	return song_;
}

int SetInstrumentToStepCommand::getTrack() const
{
	return track_;
}

int SetInstrumentToStepCommand::getOrder() const
{
	return order_;
}

int SetInstrumentToStepCommand::getStep() const
{
	return step_;
}

bool SetInstrumentToStepCommand::isSecondEntry() const
{
	return isSecond_;
}

int SetInstrumentToStepCommand::getInst() const
{
	return inst_;
}
