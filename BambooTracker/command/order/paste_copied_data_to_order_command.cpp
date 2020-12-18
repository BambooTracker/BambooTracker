/*
 * Copyright (C) 2018-2020 Rerrah
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#include "paste_copied_data_to_order_command.hpp"
#include "track.hpp"

PasteCopiedDataToOrderCommand::PasteCopiedDataToOrderCommand(std::weak_ptr<Module> mod, int songNum, int beginTrack, int beginOrder,
															 std::vector<std::vector<std::string>> cells)
	: AbstractCommand(CommandId::PasteCopiedDataToOrder),
	  mod_(mod),
	  song_(songNum),
	  track_(beginTrack),
	  order_(beginOrder),
	  cells_(cells)
{
	auto& sng = mod.lock()->getSong(songNum);
	for (size_t i = 0; i < cells.size(); ++i) {
		prevCells_.emplace_back();
		std::vector<OrderInfo> odrs = sng.getOrderData(beginOrder + static_cast<int>(i));
		for (size_t j = 0; j < cells.at(i).size(); ++j) {
			prevCells_.at(i).push_back(std::to_string(odrs.at(static_cast<size_t>(beginTrack) + j).patten));
		}
	}
}

void PasteCopiedDataToOrderCommand::redo()
{
	setCells(cells_);
}

void PasteCopiedDataToOrderCommand::undo()
{
	setCells(prevCells_);
}

void PasteCopiedDataToOrderCommand::setCells(std::vector<std::vector<std::string>>& cells)
{
	auto& sng = mod_.lock()->getSong(song_);

	for (size_t i = 0; i < cells.size(); ++i) {
		for (size_t j = 0; j < cells.at(i).size(); ++j) {
			sng.getTrack(track_ + static_cast<int>(j))
					.registerPatternToOrder(order_ + static_cast<int>(i), std::stoi(cells.at(i).at(j)));
		}
	}
}
