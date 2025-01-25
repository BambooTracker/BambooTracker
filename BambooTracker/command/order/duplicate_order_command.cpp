/*
 * SPDX-FileCopyrightText: 2018 Rerrah
 * SPDX-License-Identifier: MIT
 */

#include "duplicate_order_command.hpp"

DuplicateOrderCommand::DuplicateOrderCommand(std::weak_ptr<Module> mod, int songNum, int orderNum)
	: AbstractCommand(CommandId::DuplicateOrder), mod_(mod), song_(songNum), order_(orderNum)
{
}

bool DuplicateOrderCommand::redo()
{
	auto& sng = mod_.lock()->getSong(song_);
	sng.insertOrderBelow(order_);
	for (auto& t : sng.getTrackAttributes()) {
		auto& track = sng.getTrack(t.number);
		track.registerPatternToOrder(order_ + 1, track.getOrderInfo(order_).patten);
	}
	return true;
}

bool DuplicateOrderCommand::undo()
{
	mod_.lock()->getSong(song_).deleteOrder(order_ + 1);
	return true;
}
