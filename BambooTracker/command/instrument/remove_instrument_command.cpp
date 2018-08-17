#include "remove_instrument_command.hpp"
#include <utility>

RemoveInstrumentCommand::RemoveInstrumentCommand(InstrumentsManager &manager, int number) :
	manager_(manager),
	number_(number)
{}

void RemoveInstrumentCommand::redo()
{
	inst_ = manager_.removeInstrument(number_);
}

void RemoveInstrumentCommand::undo()
{
	manager_.addInstrument(std::move(inst_));
}

int RemoveInstrumentCommand::getID() const
{
	return 0x11;
}
