#include "set_volume_to_step_command.hpp"
#include "misc.hpp"

SetVolumeToStepCommand::SetVolumeToStepCommand(std::weak_ptr<Module> mod, int songNum, int trackNum, int orderNum, int stepNum, int volume, bool isFMReversed)
	: mod_(mod),
	  song_(songNum),
	  track_(trackNum),
	  order_(orderNum),
	  step_(stepNum),
	  vol_(volume),
	  isComplete_(false),
	  isFMReserved_(isFMReversed)
{
	prevVol_ = mod_.lock()->getSong(songNum).getTrack(trackNum)
			   .getPatternFromOrderNumber(orderNum).getStep(stepNum).getVolume();
}

void SetVolumeToStepCommand::redo()
{
	int volume = (isFMReserved_ && vol_ < 0x80) ? (0x7f - vol_) : vol_;
	mod_.lock()->getSong(song_).getTrack(track_).getPatternFromOrderNumber(order_)
			.getStep(step_).setVolume(volume);
}

void SetVolumeToStepCommand::undo()
{
	mod_.lock()->getSong(song_).getTrack(track_).getPatternFromOrderNumber(order_)
			.getStep(step_).setVolume(prevVol_);
}

int SetVolumeToStepCommand::getID() const
{
	return 0x25;
}

bool SetVolumeToStepCommand::mergeWith(const AbstractCommand* other)
{
	if (other->getID() == getID() && !isComplete_) {
		auto com = dynamic_cast<const SetVolumeToStepCommand*>(other);
		if (com->getSong() == song_ && com->getTrack() == track_
				&& com->getOrder() == order_ && com->getStep() == step_) {
			vol_ = (vol_ << 4) + com->getVol();
			redo();
			isComplete_ = true;
			return true;
		}
	}

	isComplete_ = true;
	return false;
}

int SetVolumeToStepCommand::getSong() const
{
	return song_;
}

int SetVolumeToStepCommand::getTrack() const
{
	return track_;
}

int SetVolumeToStepCommand::getOrder() const
{
	return order_;
}

int SetVolumeToStepCommand::getStep() const
{
	return step_;
}

int SetVolumeToStepCommand::getVol() const
{
	return vol_;
}
