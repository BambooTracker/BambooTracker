#include "insert_order_below_command.hpp"

InsertOrderBelowCommand::InsertOrderBelowCommand(std::weak_ptr<Module> mod, int songNum, int orderNum)
	: mod_(mod),
	  song_(songNum),
	  order_(orderNum)
{
}

void InsertOrderBelowCommand::redo()
{
	mod_.lock()->getSong(song_).insertOrderBelow(order_);
}

void InsertOrderBelowCommand::undo()
{
	mod_.lock()->getSong(song_).deleteOrder(order_ + 1);
}

int InsertOrderBelowCommand::getID() const
{
	return 0x41;
}
