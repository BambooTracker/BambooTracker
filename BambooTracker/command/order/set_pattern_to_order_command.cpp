/*
 * Copyright (C) 2018-2020 Rerrah
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

void SetPatternToOrderCommand::redo()
{
	mod_.lock()->getSong(song_).getTrack(track_).registerPatternToOrder(order_, pattern_);
}

void SetPatternToOrderCommand::undo()
{
	mod_.lock()->getSong(song_).getTrack(track_).registerPatternToOrder(order_, prevPattern_);
	isSecondEntry_ = true;	// Forced complete
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
