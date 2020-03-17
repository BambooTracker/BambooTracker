#include "expand_pattern_command.hpp"
#include "pattern_command_utils.hpp"

ExpandPatternCommand::ExpandPatternCommand(std::weak_ptr<Module> mod,
										   int songNum, int beginTrack, int beginColumn,
										   int beginOrder, int beginStep,
										   int endTrack, int endColumn, int endStep)
	: mod_(mod),
	  song_(songNum),
	  bTrack_(beginTrack),
	  bCol_(beginColumn),
	  order_(beginOrder),
	  bStep_(beginStep)
{
	auto& song = mod.lock()->getSong(songNum);
	size_t h = static_cast<size_t>(endStep - beginStep + 1);
	size_t w = calculateColumnSize(beginTrack, beginColumn, endTrack, endColumn);
	prevCells_ = getPreviousCells(song, w, h, beginTrack, beginColumn, beginOrder, beginStep);
}

void ExpandPatternCommand::redo()
{
	auto& sng = mod_.lock()->getSong(song_);

	int s = bStep_;
	for (size_t i = 0; i < prevCells_.size(); ++i) {
		int t = bTrack_;
		int c = bCol_;
		for (size_t j = 0; j < prevCells_.at(i).size(); ++j) {
			Step& st = sng.getTrack(t).getPatternFromOrderNumber(order_).getStep(s);
			switch (c) {
			case 0:
			{
				int n = (i % 2) ? -1 : std::stoi(prevCells_.at(i / 2).at(j));
				st.setNoteNumber(n);
				break;
			}
			case 1:
			{
				int n = (i % 2) ? -1 : std::stoi(prevCells_.at(i / 2).at(j));
				st.setInstrumentNumber(n);
				break;
			}
			case 2:
			{
				int v = (i % 2) ? -1 : std::stoi(prevCells_.at(i / 2).at(j));
				st.setVolume(v);
				break;
			}
			case 3:
			{
				std::string id = (i % 2) ? "--" : prevCells_.at(i / 2).at(j);
				st.setEffectID(0, id);
				break;
			}
			case 4:
			{
				int v = (i % 2) ? -1 : std::stoi(prevCells_.at(i / 2).at(j));
				st.setEffectValue(0, v);
				break;
			}
			case 5:
			{
				std::string id = (i % 2) ? "--" : prevCells_.at(i / 2).at(j);
				st.setEffectID(1, id);
				break;
			}
			case 6:
			{
				int v = (i % 2) ? -1 : std::stoi(prevCells_.at(i / 2).at(j));
				st.setEffectValue(1, v);
				break;
			}
			case 7:
			{
				std::string id = (i % 2) ? "--" : prevCells_.at(i / 2).at(j);
				st.setEffectID(2, id);
				break;
			}
			case 8:
			{
				int v = (i % 2) ? -1 : std::stoi(prevCells_.at(i / 2).at(j));
				st.setEffectValue(2, v);
				break;
			}
			case 9:
			{
				std::string id = (i % 2) ? "--" : prevCells_.at(i / 2).at(j);
				st.setEffectID(3, id);
				break;
			}
			case 10:
			{
				int v = (i % 2) ? -1 : std::stoi(prevCells_.at(i / 2).at(j));
				st.setEffectValue(3, v);
				break;
			}
			}

			t += (++c / 11);
			c %= 11;
		}

		++s;
	}
}

void ExpandPatternCommand::undo()
{
	restorePattern(mod_.lock()->getSong(song_), prevCells_, bTrack_, bCol_, order_, bStep_);
}

CommandId ExpandPatternCommand::getID() const
{
	return CommandId::ExpandPattern;
}
