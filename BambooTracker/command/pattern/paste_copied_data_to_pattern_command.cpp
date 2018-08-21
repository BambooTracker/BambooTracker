#include "paste_copied_data_to_pattern_command.hpp"

PasteCopiedDataToPatternCommand::PasteCopiedDataToPatternCommand(std::weak_ptr<Module> mod, int songNum, int beginTrack, int beginColmn,
																 int beginOrder, int beginStep,
																 std::vector<std::vector<std::string>> cells)
	: mod_(mod),
	  song_(songNum),
	  track_(beginTrack),
	  col_(beginColmn),
	  order_(beginOrder),
	  step_(beginStep),
	  cells_(cells)
{
	auto& sng = mod.lock()->getSong(songNum);
	int s = beginStep;
	for (int i = 0; i < cells.size(); ++i) {
		prevCells_.emplace_back();
		int t = beginTrack;
		int c = beginColmn;
		for (int j = 0; j < cells.at(i).size(); ++j) {
			switch (c) {
			case 0:
				prevCells_.at(i).push_back(std::to_string(
							sng.getTrack(t).getPatternFromOrderNumber(beginOrder).getStep(s).getNoteNumber()));
				break;
			case 1:
				prevCells_.at(i).push_back(std::to_string(
							sng.getTrack(t).getPatternFromOrderNumber(beginOrder).getStep(s).getInstrumentNumber()));
				break;
			case 2:
				prevCells_.at(i).push_back(std::to_string(
							sng.getTrack(t).getPatternFromOrderNumber(beginOrder).getStep(s).getVolume()));
				break;
			case 3:
				prevCells_.at(i).push_back(
							sng.getTrack(t).getPatternFromOrderNumber(beginOrder).getStep(s).getEffectID());
				break;
			case 4:
				prevCells_.at(i).push_back(std::to_string(
							sng.getTrack(t).getPatternFromOrderNumber(beginOrder).getStep(s).getEffectValue()));
				break;
			}

			++c;
			t = c / 5;
			c %= 5;
		}
		++s;
	}
}

void PasteCopiedDataToPatternCommand::redo()
{
	setCells(cells_);
}

void PasteCopiedDataToPatternCommand::undo()
{
	setCells(prevCells_);
}

int PasteCopiedDataToPatternCommand::getID() const
{
	return 0x2d;
}

void PasteCopiedDataToPatternCommand::setCells(std::vector<std::vector<std::string>>& cells)
{
	auto& sng = mod_.lock()->getSong(song_);

	int s = step_;
	for (int i = 0; i < cells.size(); ++i) {
		int t = track_;
		int c = col_;
		for (int j = 0; j < cells.at(i).size(); ++j) {
			switch (c) {
			case 0:
				sng.getTrack(t).getPatternFromOrderNumber(order_).getStep(s)
						.setNoteNumber(std::stoi(cells.at(i).at(j)));
				break;
			case 1:
				sng.getTrack(t).getPatternFromOrderNumber(order_).getStep(s)
						.setInstrumentNumber(std::stoi(cells.at(i).at(j)));
				break;
			case 2:
				sng.getTrack(t).getPatternFromOrderNumber(order_).getStep(s)
						.setVolume(std::stoi(cells.at(i).at(j)));
				break;
			case 3:
				sng.getTrack(t).getPatternFromOrderNumber(order_).getStep(s)
						.setEffectID(cells.at(i).at(j));
				break;
			case 4:
				sng.getTrack(t).getPatternFromOrderNumber(order_).getStep(s)
						.setEffectValue(std::stoi(cells.at(i).at(j)));
				break;
			}

			++c;
			t += (c / 5);
			c %= 5;
		}

		++s;
	}
}
