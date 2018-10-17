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
			prevOdrs_.at(o - beginOrder).push_back(
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
			track.registerPatternToOrder(o, prevOdrs_.at(o - bOrder_).at(t - bTrack_).patten);
		}
	}
}

int ClonePatternsCommand::getID() const
{
	return 0x46;
}
