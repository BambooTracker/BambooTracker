#include "remove_instrument_command.hpp"
#include "instrument_fm.hpp"

RemoveInstrumentCommand::RemoveInstrumentCommand(InstrumentsManager &manager, int number) :
	manager_(manager),
	number_(number),
	inst_(InstrumentFM(0, u8"dummy"))
{}

void RemoveInstrumentCommand::invoke()
{
	remove();
}

void RemoveInstrumentCommand::undo()
{
	manager_.addInstrument(inst_);
}

void RemoveInstrumentCommand::redo()
{
	remove();
}

void RemoveInstrumentCommand::remove()
{
	inst_ = manager_.removeInstrument(number_);
}
