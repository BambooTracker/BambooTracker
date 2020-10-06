/*
 * Copyright (C) 2018-2019 Rerrah
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

#include "set_pattern_to_order_command.hpp"

SetPatternToOrderCommand::SetPatternToOrderCommand(std::weak_ptr<Module> mod, int songNum, int trackNum, int orderNum, int patternNum, bool secondEntry)
	: mod_(mod),
	  song_(songNum),
	  track_(trackNum),
	  order_(orderNum),
	  pattern_(patternNum),
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
			pattern_ = (pattern_ << 4) + com->getPattern();
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
