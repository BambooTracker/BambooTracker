#include "delete_order_command.hpp"

DeleteOrderCommand::DeleteOrderCommand(std::weak_ptr<Module> mod, int songNum, int orderNum)
	: mod_(mod),
	  song_(songNum),
	  order_(orderNum)
{
	prevOdr_ = mod_.lock()->getSong(songNum).getOrderData(orderNum);
}

void DeleteOrderCommand::redo()
{
	mod_.lock()->getSong(song_).deleteOrder(order_);
}

void DeleteOrderCommand::undo()
{
	auto& sng = mod_.lock()->getSong(song_);
	sng.insertOrderBelow(order_ - 1);
	for (const auto& t : prevOdr_) {
		sng.getTrack(t.trackAttribute.number).registerPatternToOrder(t.order, t.patten);
	}
}

CommandId DeleteOrderCommand::getID() const
{
	return CommandId::DeleteOrder;
}
