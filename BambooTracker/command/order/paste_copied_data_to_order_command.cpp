/*
 * SPDX-FileCopyrightText: 2018 Rerrah
 * SPDX-License-Identifier: MIT
 */

#include "paste_copied_data_to_order_command.hpp"
#include "track.hpp"

PasteCopiedDataToOrderCommand::PasteCopiedDataToOrderCommand(std::weak_ptr<Module> mod, int songNum, int beginTrack, int beginOrder,
		const Vector2d<int>& cells)
	: AbstractCommand(CommandId::PasteCopiedDataToOrder),
	  mod_(mod),
	  song_(songNum),
	  track_(beginTrack),
	  order_(beginOrder),
	  cells_(cells),
	  prevCells_(cells.shape())
{
	const auto& sng = mod.lock()->getSong(songNum);

	for (std::size_t i = 0; i < cells.rowSize(); ++i) {
		const std::vector<OrderInfo> odrs = sng.getOrderData(beginOrder + static_cast<int>(i));
		for (std::size_t j = 0; j < cells.columnSize(); ++j) {
			prevCells_[i][j] = odrs.at(static_cast<std::size_t>(beginTrack) + j).patten;
		}
	}
}

bool PasteCopiedDataToOrderCommand::redo()
{
	try {
		setCells(cells_);
		return true;
	}
	catch (...) {
		return false;
	}
}

bool PasteCopiedDataToOrderCommand::undo()
{
	try {
		setCells(prevCells_);
		return true;
	}
	catch (...) {
		return false;
	}
}

void PasteCopiedDataToOrderCommand::setCells(const Vector2d<int>& cells)
{
	auto& sng = mod_.lock()->getSong(song_);

	for (std::size_t i = 0; i < cells.rowSize(); ++i) {
		for (std::size_t j = 0; j < cells.columnSize(); ++j) {
			sng.getTrack(track_ + static_cast<int>(j))
					.registerPatternToOrder(order_ + static_cast<int>(i), cells[i][j]);
		}
	}
}
