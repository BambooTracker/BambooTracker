#include "erase_cells_in_pattern_command.hpp"
#include "pattern_command_utils.hpp"

EraseCellsInPatternCommand::EraseCellsInPatternCommand(std::weak_ptr<Module> mod,
													   int songNum, int beginTrack, int beginColmn,
													   int beginOrder, int beginStep,
													   int endTrack, int endColumn, int endStep)
	: mod_(mod),
	  song_(songNum),
	  bTrack_(beginTrack),
	  bCol_(beginColmn),
	  order_(beginOrder),
	  bStep_(beginStep)
{
	auto& song = mod.lock()->getSong(songNum);
	size_t h = static_cast<size_t>(endStep - beginStep + 1);
	size_t w = calculateColumnSize(beginTrack, beginColmn, endTrack, endColumn);
	prevCells_ = getPreviousCells(song, w, h, beginTrack, beginColmn, beginOrder, beginStep);
}

void EraseCellsInPatternCommand::redo()
{
	auto& sng = mod_.lock()->getSong(song_);

	int s = bStep_;
	for (size_t i = 0; i < prevCells_.size(); ++i) {
		int t = bTrack_;
		int c = bCol_;
		for (size_t j = 0; j < prevCells_.at(i).size(); ++j) {
			Step& st = sng.getTrack(t).getPatternFromOrderNumber(order_).getStep(s);
			switch (c) {
			case 0:		st.setNoteNumber(-1);		break;
			case 1:		st.setInstrumentNumber(-1);	break;
			case 2:		st.setVolume(-1);			break;
			case 3:		st.setEffectID(0, "--");	break;
			case 4:		st.setEffectValue(0, -1);	break;
			case 5:		st.setEffectID(1, "--");	break;
			case 6:		st.setEffectValue(1, -1);	break;
			case 7:		st.setEffectID(2, "--");	break;
			case 8:		st.setEffectValue(2, -1);	break;
			case 9:		st.setEffectID(3, "--");	break;
			case 10:	st.setEffectValue(3, -1);	break;
			}

			t += (++c / 11);
			c %= 11;
		}

		++s;
	}
}

void EraseCellsInPatternCommand::undo()
{
	restorePattern(mod_.lock()->getSong(song_), prevCells_, bTrack_, bCol_, order_, bStep_);
}

CommandId EraseCellsInPatternCommand::getID() const
{
	return CommandId::EraseCellsInPattern;
}
