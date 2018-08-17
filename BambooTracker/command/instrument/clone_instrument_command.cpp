#include "clone_instrument_command.hpp"

CloneInstrumentCommand::CloneInstrumentCommand(InstrumentsManager& manager, int num, int refNum) :
	manager_(manager), cloneInstNum_(num), refInstNum_(refNum)
{
}

void CloneInstrumentCommand::redo()
{
	manager_.cloneInstrument(cloneInstNum_, refInstNum_);
}

void CloneInstrumentCommand::undo()
{
	manager_.removeInstrument(cloneInstNum_);
}

int CloneInstrumentCommand::getID() const
{
	return 0x14;
}
