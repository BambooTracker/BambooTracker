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

#include "clone_order_command.hpp"

CloneOrderCommand::CloneOrderCommand(std::weak_ptr<Module> mod, int songNum, int orderNum)
	: mod_(mod),
	  song_(songNum),
	  order_(orderNum)
{
}

void CloneOrderCommand::redo()
{
	auto& sng = mod_.lock()->getSong(song_);
	sng.insertOrderBelow(order_);
	for (auto& t : sng.getTrackAttributes()) {
		auto& track = sng.getTrack(t.number);
		// Set previous pattern to avoid leaving unused pattern
		track.registerPatternToOrder(order_ + 1, track.getPatternFromOrderNumber(order_).getNumber());
		track.registerPatternToOrder(order_ + 1, track.clonePattern(track.getOrderData(order_).patten));
	}
}

void CloneOrderCommand::undo()
{
	auto& sng = mod_.lock()->getSong(song_);
	for (auto& t : sng.getTrackAttributes()) {
		auto& p = sng.getTrack(t.number).getPatternFromOrderNumber(order_ + 1);
		if (p.getUsedCount() == 1) p.clear();
	}
	sng.deleteOrder(order_ + 1);
}

CommandId CloneOrderCommand::getID() const
{
	return CommandId::CloneOrder;
}
