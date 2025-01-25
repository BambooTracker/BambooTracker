/*
 * SPDX-FileCopyrightText: 2018 Rerrah
 * SPDX-License-Identifier: MIT
 */

#include "clone_patterns_command.hpp"

ClonePatternsCommand::ClonePatternsCommand(std::weak_ptr<Module> mod, int songNum,
										   int beginOrder, int beginTrack, int endOrder, int endTrack)
	: AbstractCommand(CommandId::ClonePatterns),
	  mod_(mod),
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
						mod_.lock()->getSong(songNum).getTrack(t).getOrderInfo(o));
		}
	}
}

bool ClonePatternsCommand::redo()
{
	auto& sng = mod_.lock()->getSong(song_);
	for (int o = bOrder_; o <= eOrder_; ++o) {
		for (int t = bTrack_; t <= eTrack_; ++t) {
			auto& track = sng.getTrack(t);
			track.registerPatternToOrder(o, track.clonePattern(track.getOrderInfo(o).patten));
		}
	}
	return true;
}

bool ClonePatternsCommand::undo()
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
	return true;
}
