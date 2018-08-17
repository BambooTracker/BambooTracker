#include "erase_volume_in_step_command.hpp"

EraseVolumeInStepCommand::EraseVolumeInStepCommand(std::weak_ptr<Module> mod, int songNum, int trackNum, int orderNum, int stepNum)
	: mod_(mod),
	  song_(songNum),
	  track_(trackNum),
	  order_(orderNum),
	  step_(stepNum)
{
	prevVol_ = mod_.lock()->getSong(songNum).getTrack(trackNum).getPatternFromOrderNumber(orderNum)
				.getStep(stepNum).getVolume();
}

void EraseVolumeInStepCommand::redo()
{
	mod_.lock()->getSong(song_).getTrack(track_).getPatternFromOrderNumber(order_)
					.getStep(step_).setVolume(-1);
}

void EraseVolumeInStepCommand::undo()
{
	mod_.lock()->getSong(song_).getTrack(track_).getPatternFromOrderNumber(order_)
					.getStep(step_).setVolume(prevVol_);
}

int EraseVolumeInStepCommand::getID() const
{
	return 0x26;
}
