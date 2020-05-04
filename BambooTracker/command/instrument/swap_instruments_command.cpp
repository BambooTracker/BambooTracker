#include "swap_instruments_command.hpp"

SwapInstrumentsCommand::SwapInstrumentsCommand(std::weak_ptr<InstrumentsManager> manager,
											   std::weak_ptr<Module> mod, int inst1, int inst2,
											   int song, bool patternChange)
	: manager_(manager),
	  mod_(mod),
	  inst1Num_(inst1),
	  inst2Num_(inst2),
	  songNum_(song),
	  patternChange_(patternChange)
{
}

void SwapInstrumentsCommand::redo()
{
	manager_.lock()->swapInstruments(inst1Num_, inst2Num_);
	if (patternChange_) swapInstrumentsInPatterns();
}

void SwapInstrumentsCommand::undo()
{
	manager_.lock()->swapInstruments(inst1Num_, inst2Num_);
	if (patternChange_) swapInstrumentsInPatterns();
}

CommandId SwapInstrumentsCommand::getID() const
{
	return CommandId::SwapInstruments;
}

void SwapInstrumentsCommand::swapInstrumentsInPatterns()
{
	// NOTE: Is it better to execute this as the method of Song to use global replace action?
	// Too slow...
	Song& song = mod_.lock()->getSong(songNum_);
	for (const auto& attrib : song.getStyle().trackAttribs) {
		Track& track = song.getTrack(attrib.number);
		for (int i = 0; i < 256; ++i) {	// Used track size
			Pattern& pat = track.getPattern(i);
			for (size_t j = 0; j < pat.getSize(); ++j) {
				Step& step = pat.getStep(static_cast<int>(j));
				if (step.getInstrumentNumber() == inst1Num_) step.setInstrumentNumber(inst2Num_);
				else if (step.getInstrumentNumber() == inst2Num_) step.setInstrumentNumber(inst1Num_);
			}
		}
	}
}
