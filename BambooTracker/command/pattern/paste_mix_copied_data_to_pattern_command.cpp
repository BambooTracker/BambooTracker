#include "paste_mix_copied_data_to_pattern_command.hpp"
#include "pattern_command_utils.hpp"

PasteMixCopiedDataToPatternCommand::PasteMixCopiedDataToPatternCommand(std::weak_ptr<Module> mod, int songNum,
																	   int beginTrack, int beginColumn,
																	   int beginOrder, int beginStep,
																	   std::vector<std::vector<std::string>> cells)
	: mod_(mod),
	  song_(songNum),
	  track_(beginTrack),
	  col_(beginColumn),
	  order_(beginOrder),
	  step_(beginStep),
	  cells_(cells)
{
	auto& song = mod.lock()->getSong(songNum);
	prevCells_ = getPreviousCells(song, cells.front().size(), cells.size(),
								  beginTrack, beginColumn, beginOrder, beginStep);
}

void PasteMixCopiedDataToPatternCommand::redo()
{
	auto& sng = mod_.lock()->getSong(song_);

	int s = step_;
	for (size_t i = 0; i < cells_.size(); ++i) {
		int t = track_;
		int c = col_;
		for (size_t j = 0; j < cells_.at(i).size(); ++j) {
			auto& step = sng.getTrack(t).getPatternFromOrderNumber(order_).getStep(s);
			switch (c) {
			case 0:
			{
				int n = std::stoi(cells_.at(i).at(j));
				if (n != -1 && step.getNoteNumber() == -1) step.setNoteNumber(n);
				break;
			}
			case 1:
			{
				int n = std::stoi(cells_.at(i).at(j));
				if (n != -1 && step.getInstrumentNumber() == -1) step.setInstrumentNumber(n);
				break;
			}
			case 2:
			{
				int vol = std::stoi(cells_.at(i).at(j));
				if (vol != -1 && step.getVolume() == -1) step.setVolume(vol);
				break;
			}
			case 3:
			{
				std::string id = cells_.at(i).at(j);
				if (id != "--" && step.getEffectID(0) == "--") step.setEffectID(0, id);
				break;
			}
			case 4:
			{
				int val = std::stoi(cells_.at(i).at(j));
				if (val != -1 && step.getEffectValue(0) == -1) step.setEffectValue(0, val);
				break;
			}
			case 5:
			{
				std::string id = cells_.at(i).at(j);
				if (id != "--" && step.getEffectID(1) == "--") step.setEffectID(1, id);
				break;
			}
			case 6:
			{
				int val = std::stoi(cells_.at(i).at(j));
				if (val != -1 && step.getEffectValue(1) == -1) step.setEffectValue(1, val);
				break;
			}
			case 7:
			{
				std::string id = cells_.at(i).at(j);
				if (id != "--" && step.getEffectID(2) == "--") step.setEffectID(2, id);
				break;
			}
			case 8:
			{
				int val = std::stoi(cells_.at(i).at(j));
				if (val != -1 && step.getEffectValue(2) == -1) step.setEffectValue(2, val);
				break;
			}
			case 9:
			{
				std::string id = cells_.at(i).at(j);
				if (id != "--" && step.getEffectID(3) == "--") step.setEffectID(3, id);
				break;
			}
			case 10:
			{
				int val = std::stoi(cells_.at(i).at(j));
				if (val != -1 && step.getEffectValue(3) == -1) step.setEffectValue(3, val);
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

void PasteMixCopiedDataToPatternCommand::undo()
{
	restorePattern(mod_.lock()->getSong(song_), prevCells_, track_, col_, order_, step_);
}

CommandId PasteMixCopiedDataToPatternCommand::getID() const
{
	return CommandId::PasteMixCopiedDataToPattern;
}
