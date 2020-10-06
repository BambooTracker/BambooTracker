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

#include "clone_patterns_command.hpp"

ClonePatternsCommand::ClonePatternsCommand(std::weak_ptr<Module> mod, int songNum,
										   int beginOrder, int beginTrack, int endOrder, int endTrack)
	: mod_(mod),
	  song_(songNum),
	  bOrder_(beginOrder),
	  bTrack_(beginTrack),
	  eOrder_(endOrder),
	  eTrack_(endTrack)
{
	for (int o = beginOrder; o <= endOrder; ++o) {
		prevOdrs_.emplace_back();
		for (int t = beginTrack; t <= endTrack; ++t) {
			prevOdrs_.at(static_cast<size_t>(o - beginOrder)).push_back(
						mod_.lock()->getSong(songNum).getTrack(t).getOrderData(o));
		}
	}
}

void ClonePatternsCommand::redo()
{
	auto& sng = mod_.lock()->getSong(song_);
	for (int o = bOrder_; o <= eOrder_; ++o) {
		for (int t = bTrack_; t <= eTrack_; ++t) {
			auto& track = sng.getTrack(t);
			track.registerPatternToOrder(o, track.clonePattern(track.getOrderData(o).patten));
		}
	}
}

void ClonePatternsCommand::undo()
{
	auto& sng = mod_.lock()->getSong(song_);
	for (int o = bOrder_; o <= eOrder_; ++o) {
		for (int t = bTrack_; t <= eTrack_; ++t) {
			auto& track = sng.getTrack(t);
			auto& p = track.getPatternFromOrderNumber(o);
			if (p.getUsedCount() == 1) p.clear();
			track.registerPatternToOrder(
						o,
						prevOdrs_.at(static_cast<size_t>(o - bOrder_)).at(static_cast<size_t>(t - bTrack_)).patten);
		}
	}
}

CommandId ClonePatternsCommand::getID() const
{
	return CommandId::ClonePatterns;
}
