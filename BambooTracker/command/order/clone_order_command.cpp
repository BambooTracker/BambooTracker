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

int CloneOrderCommand::getID() const
{
	return 0x47;
}
