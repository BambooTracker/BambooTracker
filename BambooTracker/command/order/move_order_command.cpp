#include "move_order_command.hpp"

MoveOrderCommand::MoveOrderCommand(std::weak_ptr<Module> mod, int songNum, int orderNum, bool isUp)
	: mod_(mod),
	  song_(songNum),
	  order_(orderNum),
	  isUp_(isUp)
{
}

void MoveOrderCommand::redo()
{
	swap();
}

void MoveOrderCommand::undo()
{
	swap();
}

int MoveOrderCommand::getID() const
{
	return 0x45;
}

void MoveOrderCommand::swap()
{
	auto& sng = mod_.lock()->getSong(song_);
	if (isUp_) sng.swapOrder(order_ - 1, order_);
	else sng.swapOrder(order_, order_ + 1);
}
