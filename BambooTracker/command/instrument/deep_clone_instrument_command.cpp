#include "deep_clone_instrument_command.hpp"

DeepCloneInstrumentCommand::DeepCloneInstrumentCommand(InstrumentsManager& manager, int num, int refNum)
	: manager_(manager), cloneInstNum_(num), refInstNum_(refNum)
{
}

void DeepCloneInstrumentCommand::redo()
{
	manager_.deepCloneInstrument(cloneInstNum_, refInstNum_);
}

void DeepCloneInstrumentCommand::undo()
{
	manager_.removeInstrument(cloneInstNum_);
}

int DeepCloneInstrumentCommand::getID() const
{
	return 0x14;
}
