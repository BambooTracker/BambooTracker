#include "add_instrument_command.hpp"
#include "instrument_fm.hpp"

AddInstrumentCommand::AddInstrumentCommand(InstrumentsManager &manager, int num, SoundSource source, std::string name) :
	manager_(manager),
	num_(num),
	source_(source),
	name_(name)
{}

void AddInstrumentCommand::invoke()
{
	manager_.addInstrument(num_, source_, name_);
}

void AddInstrumentCommand::undo()
{
	manager_.removeInstrument(num_);
}

void AddInstrumentCommand::redo()
{
	invoke();
}
