#include "paste_overwrite_copied_data_to_pattern_command.hpp"

PasteOverwriteCopiedDataToPatternCommand::PasteOverwriteCopiedDataToPatternCommand(std::weak_ptr<Module> mod, int songNum, int beginTrack, int beginColmn,
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
	for (size_t i = 0; i < cells.size(); ++i) {
		prevCells_.emplace_back();
		int t = beginTrack;
		int c = beginColmn;
		for (size_t j = 0; j < cells.at(i).size(); ++j) {
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
							sng.getTrack(t).getPatternFromOrderNumber(beginOrder).getStep(s).getEffectID(0));
				break;
			case 4:
				prevCells_.at(i).push_back(std::to_string(
											   sng.getTrack(t).getPatternFromOrderNumber(beginOrder).getStep(s).getEffectValue(0)));
				break;
			case 5:
				prevCells_.at(i).push_back(
							sng.getTrack(t).getPatternFromOrderNumber(beginOrder).getStep(s).getEffectID(1));
				break;
			case 6:
				prevCells_.at(i).push_back(std::to_string(
											   sng.getTrack(t).getPatternFromOrderNumber(beginOrder).getStep(s).getEffectValue(1)));
				break;
			case 7:
				prevCells_.at(i).push_back(
							sng.getTrack(t).getPatternFromOrderNumber(beginOrder).getStep(s).getEffectID(2));
				break;
			case 8:
				prevCells_.at(i).push_back(std::to_string(
											   sng.getTrack(t).getPatternFromOrderNumber(beginOrder).getStep(s).getEffectValue(2)));
				break;
			case 9:
				prevCells_.at(i).push_back(
							sng.getTrack(t).getPatternFromOrderNumber(beginOrder).getStep(s).getEffectID(3));
				break;
			case 10:
				prevCells_.at(i).push_back(std::to_string(
											   sng.getTrack(t).getPatternFromOrderNumber(beginOrder).getStep(s).getEffectValue(3)));
				break;
			}

			++c;
			t += (c / 11);
			c %= 11;
		}
		++s;
	}
}

void PasteOverwriteCopiedDataToPatternCommand::redo()
{
	auto& sng = mod_.lock()->getSong(song_);

	int s = step_;
	for (size_t i = 0; i < cells_.size(); ++i) {
		int t = track_;
		int c = col_;
		for (size_t j = 0; j < cells_.at(i).size(); ++j) {
			switch (c) {
			case 0:
			{
				int n = std::stoi(cells_.at(i).at(j));
				if (n != -1)
					sng.getTrack(t).getPatternFromOrderNumber(order_).getStep(s).setNoteNumber(n);
				break;
			}
			case 1:
			{
				int n = std::stoi(cells_.at(i).at(j));
				if (n != -1)
					sng.getTrack(t).getPatternFromOrderNumber(order_).getStep(s).setInstrumentNumber(n);
				break;
			}
			case 2:
			{
				int vol = std::stoi(cells_.at(i).at(j));
				if (vol != -1)
					sng.getTrack(t).getPatternFromOrderNumber(order_).getStep(s).setVolume(vol);
				break;
			}
			case 3:
			{
				std::string id = cells_.at(i).at(j);
				if (id != "--")
					sng.getTrack(t).getPatternFromOrderNumber(order_).getStep(s).setEffectID(0, id);
				break;
			}
			case 4:
			{
				int val = std::stoi(cells_.at(i).at(j));
				if (val != -1)
					sng.getTrack(t).getPatternFromOrderNumber(order_).getStep(s).setEffectValue(0, val);
				break;
			}
			case 5:
			{
				std::string id = cells_.at(i).at(j);
				if (id != "--")
					sng.getTrack(t).getPatternFromOrderNumber(order_).getStep(s).setEffectID(1, id);
				break;
			}
			case 6:
			{
				int val = std::stoi(cells_.at(i).at(j));
				if (val != -1)
					sng.getTrack(t).getPatternFromOrderNumber(order_).getStep(s).setEffectValue(1, val);
				break;
			}
			case 7:
			{
				std::string id = cells_.at(i).at(j);
				if (id != "--")
					sng.getTrack(t).getPatternFromOrderNumber(order_).getStep(s).setEffectID(2, id);
				break;
			}
			case 8:
			{
				int val = std::stoi(cells_.at(i).at(j));
				if (val != -1)
					sng.getTrack(t).getPatternFromOrderNumber(order_).getStep(s).setEffectValue(2, val);
				break;
			}
			case 9:
			{
				std::string id = cells_.at(i).at(j);
				if (id != "--")
					sng.getTrack(t).getPatternFromOrderNumber(order_).getStep(s).setEffectID(3, id);
				break;
			}
			case 10:
			{
				int val = std::stoi(cells_.at(i).at(j));
				if (val != -1)
					sng.getTrack(t).getPatternFromOrderNumber(order_).getStep(s).setEffectValue(3, val);
				break;
			}
			}

			++c;
			t += (c / 11);
			c %= 11;
		}

		++s;
	}
}

void PasteOverwriteCopiedDataToPatternCommand::undo()
{
	auto& sng = mod_.lock()->getSong(song_);

	int s = step_;
	for (size_t i = 0; i < prevCells_.size(); ++i) {
		int t = track_;
		int c = col_;
		for (size_t j = 0; j < prevCells_.at(i).size(); ++j) {
			switch (c) {
			case 0:
				sng.getTrack(t).getPatternFromOrderNumber(order_).getStep(s)
						.setNoteNumber(std::stoi(prevCells_.at(i).at(j)));
				break;
			case 1:
				sng.getTrack(t).getPatternFromOrderNumber(order_).getStep(s)
						.setInstrumentNumber(std::stoi(prevCells_.at(i).at(j)));
				break;
			case 2:
				sng.getTrack(t).getPatternFromOrderNumber(order_).getStep(s)
						.setVolume(std::stoi(prevCells_.at(i).at(j)));
				break;
			case 3:
				sng.getTrack(t).getPatternFromOrderNumber(order_).getStep(s)
						.setEffectID(0, prevCells_.at(i).at(j));
				break;
			case 4:
				sng.getTrack(t).getPatternFromOrderNumber(order_).getStep(s)
						.setEffectValue(0, std::stoi(prevCells_.at(i).at(j)));
				break;
			case 5:
				sng.getTrack(t).getPatternFromOrderNumber(order_).getStep(s)
						.setEffectID(1, prevCells_.at(i).at(j));
				break;
			case 6:
				sng.getTrack(t).getPatternFromOrderNumber(order_).getStep(s)
						.setEffectValue(1, std::stoi(prevCells_.at(i).at(j)));
				break;
			case 7:
				sng.getTrack(t).getPatternFromOrderNumber(order_).getStep(s)
						.setEffectID(2, prevCells_.at(i).at(j));
				break;
			case 8:
				sng.getTrack(t).getPatternFromOrderNumber(order_).getStep(s)
						.setEffectValue(2, std::stoi(prevCells_.at(i).at(j)));
				break;
			case 9:
				sng.getTrack(t).getPatternFromOrderNumber(order_).getStep(s)
						.setEffectID(3, prevCells_.at(i).at(j));
				break;
			case 10:
				sng.getTrack(t).getPatternFromOrderNumber(order_).getStep(s)
						.setEffectValue(3, std::stoi(prevCells_.at(i).at(j)));
				break;
			}

			++c;
			t += (c / 11);
			c %= 11;
		}

		++s;
	}
}

CommandId PasteOverwriteCopiedDataToPatternCommand::getID() const
{
	return CommandId::PasteOverwriteCopiedDataToPattern;
}
