#include "add_instrument_command.hpp"

AddInstrumentCommand::AddInstrumentCommand(std::weak_ptr<InstrumentsManager> manager, int num, SoundSource source, std::string name) :
	manager_(manager),
	num_(num),
	source_(source),
	name_(name)
{}

void AddInstrumentCommand::redo()
{
	manager_.lock()->addInstrument(num_, source_, name_);
}

void AddInstrumentCommand::undo()
{
	manager_.lock()->removeInstrument(num_);
}

int AddInstrumentCommand::getID() const
{
	return 0x10;
}
