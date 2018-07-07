#include "remove_instrument_command.hpp"
#include <utility>
#include "instrument_fm.hpp"

RemoveInstrumentCommand::RemoveInstrumentCommand(InstrumentsManager &manager, int number) :
	manager_(manager),
	number_(number)
{}

void RemoveInstrumentCommand::invoke()
{
	inst_ = manager_.removeInstrument(number_);
}

void RemoveInstrumentCommand::undo()
{
	manager_.addInstrument(std::move(inst_));
}

void RemoveInstrumentCommand::redo()
{
	invoke();
}
