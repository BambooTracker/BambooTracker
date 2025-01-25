/*
 * SPDX-FileCopyrightText: 2018 Rerrah
 * SPDX-License-Identifier: MIT
 */

#include "delete_order_command.hpp"

DeleteOrderCommand::DeleteOrderCommand(std::weak_ptr<Module> mod, int songNum, int orderNum)
	: AbstractCommand(CommandId::DeleteOrder), mod_(mod), song_(songNum), order_(orderNum)
{
	prevOdr_ = mod_.lock()->getSong(songNum).getOrderData(orderNum);
}

bool DeleteOrderCommand::redo()
{
	mod_.lock()->getSong(song_).deleteOrder(order_);
	return true;
}

bool DeleteOrderCommand::undo()
{
	auto& sng = mod_.lock()->getSong(song_);
	sng.insertOrderBelow(order_ - 1);
	for (const auto& t : prevOdr_) {
		sng.getTrack(t.trackAttribute.number).registerPatternToOrder(t.order, t.patten);
	}
	return true;
}
