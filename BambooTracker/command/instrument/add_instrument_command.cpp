#include "add_instrument_command.hpp"
#include <utility>

AddInstrumentCommand::AddInstrumentCommand(std::weak_ptr<InstrumentsManager> manager, int num, SoundSource source, std::string name)
	: manager_(manager),
	  num_(num),
	  source_(source),
	  name_(name)
{}

AddInstrumentCommand::AddInstrumentCommand(std::weak_ptr<InstrumentsManager> manager, std::unique_ptr<AbstractInstrument> inst)
	: manager_(manager),
	  inst_(std::move(inst))
{
	num_ = inst_->getNumber();
}

void AddInstrumentCommand::redo()
{
	if (inst_) manager_.lock()->addInstrument(inst_->clone());
	else manager_.lock()->addInstrument(num_, source_, name_);
}

void AddInstrumentCommand::undo()
{
	manager_.lock()->removeInstrument(num_);
}

CommandId AddInstrumentCommand::getID() const
{
	return CommandId::AddInstrument;
}
