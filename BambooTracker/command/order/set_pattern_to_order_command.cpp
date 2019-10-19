#include "set_pattern_to_order_command.hpp"

SetPatternToOrderCommand::SetPatternToOrderCommand(std::weak_ptr<Module> mod, int songNum, int trackNum, int orderNum, int patternNum, bool secondEntry)
	: mod_(mod),
	  song_(songNum),
	  track_(trackNum),
	  order_(orderNum),
	  pattern_(patternNum << 4),
	  isSecond_(secondEntry)
{
	prevPattern_ = mod_.lock()->getSong(songNum).getTrack(trackNum)
				   .getPatternFromOrderNumber(orderNum).getNumber();
}

void SetPatternToOrderCommand::redo()
{
	mod_.lock()->getSong(song_).getTrack(track_).registerPatternToOrder(order_, pattern_);
}

void SetPatternToOrderCommand::undo()
{
	mod_.lock()->getSong(song_).getTrack(track_).registerPatternToOrder(order_, prevPattern_);
	isSecond_ = true;	// Forced complete
}

CommandId SetPatternToOrderCommand::getID() const
{
	return CommandId::SetPatternToOrder;
}

bool SetPatternToOrderCommand::mergeWith(const AbstractCommand* other)
{
	if (other->getID() == getID() && !isSecond_) {
		auto com = dynamic_cast<const SetPatternToOrderCommand*>(other);
		if (com->getSong() == song_ && com->getTrack() == track_
				&& com->getOrder() == order_ && com->isSecondEntry()) {
			pattern_ += (com->getPattern() >> 4);
			redo();
			isSecond_ = true;
			return true;
		}
	}

	isSecond_ = true;
	return false;
}

int SetPatternToOrderCommand::getSong() const
{
	return song_;
}

int SetPatternToOrderCommand::getTrack() const
{
	return track_;
}

int SetPatternToOrderCommand::getOrder() const
{
	return order_;
}

bool SetPatternToOrderCommand::isSecondEntry() const
{
	return isSecond_;
}

int SetPatternToOrderCommand::getPattern() const
{
	return pattern_;
}
