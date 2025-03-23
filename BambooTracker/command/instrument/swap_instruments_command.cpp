/*
 * SPDX-FileCopyrightText: 2020 Rerrah
 * SPDX-License-Identifier: MIT
 */

#include "swap_instruments_command.hpp"

SwapInstrumentsCommand::SwapInstrumentsCommand(std::weak_ptr<InstrumentsManager> manager,
											   std::weak_ptr<Module> mod, int inst1, int inst2,
											   bool patternChange)
	: AbstractCommand(CommandId::SwapInstruments),
	  manager_(manager),
	  mod_(mod),
	  inst1Num_(inst1),
	  inst2Num_(inst2),
	  patternChange_(patternChange)
{
}

bool SwapInstrumentsCommand::redo()
{
	manager_.lock()->swapInstruments(inst1Num_, inst2Num_);
	if (patternChange_) swapInstrumentsInPatterns();
	return true;
}

bool SwapInstrumentsCommand::undo()
{
	manager_.lock()->swapInstruments(inst1Num_, inst2Num_);
	if (patternChange_) swapInstrumentsInPatterns();
	return true;
}

void SwapInstrumentsCommand::swapInstrumentsInPatterns()
{
	// OPTIMIZE: It is too slow!
	for (size_t n = 0; n < mod_.lock()->getSongCount(); ++n) {
		Song& song = mod_.lock()->getSong(static_cast<int>(n));
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
}
