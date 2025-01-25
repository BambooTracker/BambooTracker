/*
 * SPDX-FileCopyrightText: 2018 Rerrah
 * SPDX-License-Identifier: MIT
 */

#include "set_pattern_to_order_command.hpp"

SetPatternToOrderCommand::SetPatternToOrderCommand(std::weak_ptr<Module> mod, int songNum,
												   int trackNum, int orderNum, int patternNum,
												   bool secondEntry)
	: AbstractCommand(CommandId::SetPatternToOrder),
	  mod_(mod),
	  song_(songNum),
	  track_(trackNum),
	  order_(orderNum),
	  pattern_(patternNum),
	  prevPattern_(mod_.lock()->getSong(songNum).getTrack(trackNum)
				   .getPatternFromOrderNumber(orderNum).getNumber()),
	  isSecondEntry_(secondEntry)
{
}

bool SetPatternToOrderCommand::redo()
{
	mod_.lock()->getSong(song_).getTrack(track_).registerPatternToOrder(order_, pattern_);
	return true;
}

bool SetPatternToOrderCommand::undo()
{
	mod_.lock()->getSong(song_).getTrack(track_).registerPatternToOrder(order_, prevPattern_);
	isSecondEntry_ = true;	// Forced complete
	return true;
}

bool SetPatternToOrderCommand::mergeWith(const AbstractCommand* other)
{
	if (other->getID() == getID() && !isSecondEntry_) {
		auto com = dynamic_cast<const SetPatternToOrderCommand*>(other);
		if (com->song_ == song_ && com->track_ == track_
				&& com->order_ == order_ && com->isSecondEntry_) {
			pattern_ = (pattern_ << 4) + com->pattern_;
			redo();
			isSecondEntry_ = true;
			return true;
		}
	}

	isSecondEntry_ = true;
	return false;
}
