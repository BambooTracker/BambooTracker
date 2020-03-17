#include "change_values_in_pattern_command.hpp"
#include "misc.hpp"

ChangeValuesInPatternCommand::ChangeValuesInPatternCommand(std::weak_ptr<Module> mod, int songNum, int beginTrack,
														   int beginColumn, int beginOrder, int beginStep,
														   int endTrack, int endColumn, int endStep, int value, bool isFMReversed)
	: mod_(mod),
	  song_(songNum),
	  bTrack_(beginTrack),
	  bCol_(beginColumn),
	  order_(beginOrder),
	  bStep_(beginStep),
	  eTrack_(endTrack),
	  eCol_(endColumn),
	  eStep_(endStep),
	  diff_(value),
	  fmReverse_(isFMReversed)
{
	auto& sng = mod.lock()->getSong(songNum);

	for (int step = beginStep; step <= endStep; ++step) {
		int track = beginTrack;
		int col = beginColumn;
		std::vector<int> vals;
		while (true) {
			int val;
			Step& st = sng.getTrack(track).getPatternFromOrderNumber(beginOrder).getStep(step);
			switch (col) {
			case 1:		val = st.getInstrumentNumber();	break;
			case 2:		val = st.getVolume();			break;
			case 4:		val = st.getEffectValue(0);		break;
			case 6:		val = st.getEffectValue(1);		break;
			case 8:		val = st.getEffectValue(2);		break;
			case 10:	val = st.getEffectValue(3);		break;
			default:	val = -1;						break;
			}
			if (val > -1) vals.push_back(val);
			if (track == endTrack && col == endColumn) break;
			track += (++col / 11);
			col %= 11;
		}
		prevVals_.push_back(vals);
	}
}

void ChangeValuesInPatternCommand::redo()
{
	auto& sng = mod_.lock()->getSong(song_);
	auto it = prevVals_.begin();
	for (int step = bStep_; step <= eStep_; ++step, ++it) {
		int track = bTrack_;
		int col = bCol_;
		auto valit = it->begin();
		while (true) {
			Track& tr = sng.getTrack(track);
			Step& st = tr.getPatternFromOrderNumber(order_).getStep(step);
			switch (col) {
			case 1:
				if (st.getInstrumentNumber() > -1) st.setInstrumentNumber(clamp(diff_ + *valit++, 0, 127));
				break;
			case 2:
				if (st.getVolume() > -1) {
					int d = (tr.getAttribute().source == SoundSource::FM && fmReverse_) ? -diff_ : diff_;
					st.setVolume(clamp(d + *valit++, 0, 255));
				}
				break;
			case 4:
				if (st.getEffectValue(0) > -1) st.setEffectValue(0, clamp(diff_ + *valit++, 0, 255));
				break;
			case 6:
				if (st.getEffectValue(1) > -1) st.setEffectValue(1, clamp(diff_ + *valit++, 0, 255));
				break;
			case 8:
				if (st.getEffectValue(2) > -1) st.setEffectValue(2, clamp(diff_ + *valit++, 0, 255));
				break;
			case 10:
				if (st.getEffectValue(3) > -1) st.setEffectValue(3, clamp(diff_ + *valit++, 0, 255));
				break;
			default:
				break;
			}
			if (track == eTrack_ && col == eCol_) break;
			track += (++col / 11);
			col %= 11;
		}
	}
}

void ChangeValuesInPatternCommand::undo()
{
	auto& sng = mod_.lock()->getSong(song_);
	auto it = prevVals_.begin();
	for (int step = bStep_; step <= eStep_; ++step, ++it) {
		int track = bTrack_;
		int col = bCol_;
		auto valit = it->begin();
		while (true) {
			Step& st = sng.getTrack(track).getPatternFromOrderNumber(order_).getStep(step);
			switch (col) {
			case 1:
				if (st.getInstrumentNumber() > -1) st.setInstrumentNumber(*valit++);
				break;
			case 2:
				if (st.getVolume() > -1) st.setVolume(*valit++);
				break;
			case 4:
				if (st.getEffectValue(0) > -1) st.setEffectValue(0, *valit++);
				break;
			case 6:
				if (st.getEffectValue(1) > -1) st.setEffectValue(1, *valit++);
				break;
			case 8:
				if (st.getEffectValue(2) > -1) st.setEffectValue(2, *valit++);
				break;
			case 10:
				if (st.getEffectValue(3) > -1) st.setEffectValue(3, *valit++);
				break;
			default:
				break;
			}
			if (track == eTrack_ && col == eCol_) break;
			track += (++col / 11);
			col %= 11;
		}
	}
}

CommandId ChangeValuesInPatternCommand::getID() const
{
	return CommandId::ChangeValuesInPattern;
}
