#include "add_instrument_command.hpp"
#include "instrument_fm.hpp"

AddInstrumentCommand::AddInstrumentCommand(InstrumentsManager &manager, int num, SoundSource source, std::string name) :
	manager_(manager),
	num_(num),
	source_(source),
	name_(name),
	inst_(InstrumentFM(0, u8"dummy"))
{}

void AddInstrumentCommand::invoke()
{
	inst_ = manager_.addInstrument(num_, source_, name_);
}

void AddInstrumentCommand::undo()
{
	manager_.removeInstrument(inst_.getNumber());
}

void AddInstrumentCommand::redo()
{
	manager_.addInstrument(inst_);
}
