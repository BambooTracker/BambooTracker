#include "replace_instrument_in_pattern_command.hpp"

ReplaceInstrumentInPatternCommand::ReplaceInstrumentInPatternCommand(std::weak_ptr<Module> mod,
																	 int songNum, int beginTrack,
																	 int beginOrder, int beginStep,
																	 int endTrack, int endStep, int newInst)
	: mod_(mod),
	  song_(songNum),
	  bTrack_(beginTrack),
	  order_(beginOrder),
	  bStep_(beginStep),
	  eTrack_(endTrack),
	  eStep_(endStep),
	  inst_(newInst)
{
	auto& sng = mod.lock()->getSong(songNum);

	for (int step = beginStep; step <= endStep; ++step) {
		for (int track = beginTrack; track <= endTrack; ++track) {
			int n = sng.getTrack(track).getPatternFromOrderNumber(beginOrder)
					.getStep(step).getInstrumentNumber();
			if (n > -1) prevInsts_.push_back(n);
		}
	}
}

void ReplaceInstrumentInPatternCommand::redo()
{
	auto& sng = mod_.lock()->getSong(song_);

	for (int step = bStep_; step <= eStep_; ++step) {
		for (int track = bTrack_; track <= eTrack_; ++track) {
			auto& s = sng.getTrack(track).getPatternFromOrderNumber(order_).getStep(step);
			int n = s.getInstrumentNumber();
			if (n > -1) s.setInstrumentNumber(inst_);
		}
	}
}

void ReplaceInstrumentInPatternCommand::undo()
{
	auto& sng = mod_.lock()->getSong(song_);

	size_t i = 0;
	for (int step = bStep_; step <= eStep_; ++step) {
		for (int track = bTrack_; track <= eTrack_; ++track) {
			auto& s = sng.getTrack(track).getPatternFromOrderNumber(order_).getStep(step);
			if (s.getInstrumentNumber() > -1) s.setInstrumentNumber(prevInsts_.at(i));
		}
	}
}

CommandId ReplaceInstrumentInPatternCommand::getID() const
{
	return CommandId::ReplaceInstrumentInPattern;
}
