#include "deep_clone_instrument_command.hpp"

DeepCloneInstrumentCommand::DeepCloneInstrumentCommand(std::weak_ptr<InstrumentsManager> manager, int num, int refNum)
	: manager_(manager), cloneInstNum_(num), refInstNum_(refNum)
{
}

void DeepCloneInstrumentCommand::redo()
{
	manager_.lock()->deepCloneInstrument(cloneInstNum_, refInstNum_);
}

void DeepCloneInstrumentCommand::undo()
{
	manager_.lock()->removeInstrument(cloneInstNum_);
}

CommandId DeepCloneInstrumentCommand::getID() const
{
	return CommandId::DeepCloneInstrument;
}
