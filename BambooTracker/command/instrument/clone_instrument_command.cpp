#include "clone_instrument_command.hpp"

cloneInstrumentCommand::cloneInstrumentCommand(std::weak_ptr<InstrumentsManager> manager, int num, int refNum)
	: manager_(manager), cloneInstNum_(num), refInstNum_(refNum)
{
}

void cloneInstrumentCommand::redo()
{
	manager_.lock()->cloneInstrument(cloneInstNum_, refInstNum_);
}

void cloneInstrumentCommand::undo()
{
	manager_.lock()->removeInstrument(cloneInstNum_);
}

int cloneInstrumentCommand::getID() const
{
	return 0x13;
}
