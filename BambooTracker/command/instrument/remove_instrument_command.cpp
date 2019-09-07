#include "remove_instrument_command.hpp"
#include <utility>

RemoveInstrumentCommand::RemoveInstrumentCommand(std::weak_ptr<InstrumentsManager> manager, int number) :
	manager_(manager),
	number_(number)
{}

void RemoveInstrumentCommand::redo()
{
	inst_ = manager_.lock()->removeInstrument(number_);
}

void RemoveInstrumentCommand::undo()
{
	manager_.lock()->addInstrument(std::move(inst_));
}

CommandId RemoveInstrumentCommand::getID() const
{
	return CommandId::RemoveInstrument;
}
