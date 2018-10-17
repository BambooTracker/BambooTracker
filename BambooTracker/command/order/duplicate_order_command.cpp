#include "duplicate_order_command.hpp"

DuplicateOrderCommand::DuplicateOrderCommand(std::weak_ptr<Module> mod, int songNum, int orderNum)
	: mod_(mod),
	  song_(songNum),
	  order_(orderNum)
{
}

void DuplicateOrderCommand::redo()
{
	auto& sng = mod_.lock()->getSong(song_);
	sng.insertOrderBelow(order_);
	for (auto& t : sng.getTrackAttributes()) {
		auto& track = sng.getTrack(t.number);
		track.registerPatternToOrder(order_ + 1, track.getOrderData(order_).patten);
	}
}

void DuplicateOrderCommand::undo()
{
	mod_.lock()->getSong(song_).deleteOrder(order_ + 1);
}

int DuplicateOrderCommand::getID() const
{
	return 0x44;
}
