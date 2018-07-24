#include "add_instrument_command.hpp"

AddInstrumentCommand::AddInstrumentCommand(InstrumentsManager &manager, int num, SoundSource source, std::string name) :
	manager_(manager),
	num_(num),
	source_(source),
	name_(name)
{}

void AddInstrumentCommand::redo()
{
	manager_.addInstrument(num_, source_, name_);
}

void AddInstrumentCommand::undo()
{
	manager_.removeInstrument(num_);
}

int AddInstrumentCommand::getID() const
{
	return 0;
}
