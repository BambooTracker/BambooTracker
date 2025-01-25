/*
 * SPDX-FileCopyrightText: 2018 Rerrah
 * SPDX-License-Identifier: MIT
 */

#include "insert_order_below_command.hpp"

InsertOrderBelowCommand::InsertOrderBelowCommand(std::weak_ptr<Module> mod, int songNum, int orderNum)
	: AbstractCommand(CommandId::InsertOrderBelow), mod_(mod), song_(songNum), order_(orderNum)
{
}

bool InsertOrderBelowCommand::redo()
{
	mod_.lock()->getSong(song_).insertOrderBelow(order_);
	return true;
}

bool InsertOrderBelowCommand::undo()
{
	mod_.lock()->getSong(song_).deleteOrder(order_ + 1);
	return true;
}
