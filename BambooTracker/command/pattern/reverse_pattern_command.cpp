#include "reverse_pattern_command.hpp"
#include "pattern_command_utils.hpp"

ReversePatternCommand::ReversePatternCommand(std::weak_ptr<Module> mod,
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

void ReversePatternCommand::redo()
{
	auto& sng = mod_.lock()->getSong(song_);

	size_t l = prevCells_.size() - 1;
	int s = bStep_;
	for (size_t i = 0; i < prevCells_.size(); ++i) {
		int t = bTrack_;
		int c = bCol_;
		for (size_t j = 0; j < prevCells_.at(i).size(); ++j) {
			Step& st = sng.getTrack(t).getPatternFromOrderNumber(order_).getStep(s);
			switch (c) {
			case 0:		st.setNoteNumber(std::stoi(prevCells_.at(l - i).at(j)));		break;
			case 1:		st.setInstrumentNumber(std::stoi(prevCells_.at(l - i).at(j)));	break;
			case 2:		st.setVolume(std::stoi(prevCells_.at(l - i).at(j)));			break;
			case 3:		st.setEffectID(0, prevCells_.at(l - i).at(j));					break;
			case 4:		st.setEffectValue(0, std::stoi(prevCells_.at(l - i).at(j)));	break;
			case 5:		st.setEffectID(1, prevCells_.at(l - i).at(j));					break;
			case 6:		st.setEffectValue(1, std::stoi(prevCells_.at(l - i).at(j)));	break;
			case 7:		st.setEffectID(2, prevCells_.at(l - i).at(j));					break;
			case 8:		st.setEffectValue(2, std::stoi(prevCells_.at(l - i).at(j)));	break;
			case 9:		st.setEffectID(3, prevCells_.at(l - i).at(j));					break;
			case 10:	st.setEffectValue(3, std::stoi(prevCells_.at(l - i).at(j)));	break;
			}

			t += (++c / 11);
			c %= 11;
		}

		++s;
	}
}

void ReversePatternCommand::undo()
{
	restorePattern(mod_.lock()->getSong(song_), prevCells_, bTrack_, bCol_, order_, bStep_);
}

CommandId ReversePatternCommand::getID() const
{
	return CommandId::ReversePattern;
}
