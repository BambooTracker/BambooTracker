/*
 * SPDX-FileCopyrightText: 2018 Rerrah
 * SPDX-License-Identifier: MIT
 */

#include "move_order_command.hpp"

MoveOrderCommand::MoveOrderCommand(std::weak_ptr<Module> mod, int songNum, int orderNum, bool isUp)
	: AbstractCommand(CommandId::MoveOrder), mod_(mod), song_(songNum), order_(orderNum), isUp_(isUp)
{
}

bool MoveOrderCommand::redo()
{
	swap();
	return true;
}

bool MoveOrderCommand::undo()
{
	swap();
	return true;
}

void MoveOrderCommand::swap()
{
	auto& sng = mod_.lock()->getSong(song_);
	if (isUp_) sng.swapOrder(order_ - 1, order_);
	else sng.swapOrder(order_, order_ + 1);
}
