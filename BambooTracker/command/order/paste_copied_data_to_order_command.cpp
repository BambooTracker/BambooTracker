#include "paste_copied_data_to_order_command.hpp"
#include "track.hpp"

PasteCopiedDataToOrderCommand::PasteCopiedDataToOrderCommand(std::weak_ptr<Module> mod, int songNum, int beginTrack, int beginOrder,
															 std::vector<std::vector<std::string>> cells)
	: mod_(mod),
	  song_(songNum),
	  track_(beginTrack),
	  order_(beginOrder),
	  cells_(cells)
{
	auto& sng = mod.lock()->getSong(songNum);
	for (size_t i = 0; i < cells.size(); ++i) {
		prevCells_.emplace_back();
		std::vector<OrderData> odrs = sng.getOrderData(beginOrder + i);
		for (size_t j = 0; j < cells.at(i).size(); ++j) {
			prevCells_.at(i).push_back(std::to_string(odrs.at(beginTrack + j).patten));
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

int PasteCopiedDataToOrderCommand::getID() const
{
	return 0x43;
}

void PasteCopiedDataToOrderCommand::setCells(std::vector<std::vector<std::string>>& cells)
{
	auto& sng = mod_.lock()->getSong(song_);

	for (size_t i = 0; i < cells.size(); ++i) {
		for (size_t j = 0; j < cells.at(i).size(); ++j) {
			sng.getTrack(track_ + j).registerPatternToOrder(order_ + i, std::stoi(cells.at(i).at(j)));
		}
	}
}
