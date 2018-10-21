#include "change_instrument_name_command.hpp"

ChangeInstrumentNameCommand::ChangeInstrumentNameCommand(std::weak_ptr<InstrumentsManager> manager, int num, std::string name) :
	manager_(manager), instNum_(num), newName_(name)
{
	oldName_ = manager_.lock()->getInstrumentName(instNum_);
}

void ChangeInstrumentNameCommand::redo()
{
	manager_.lock()->setInstrumentName(instNum_, newName_);
}


void ChangeInstrumentNameCommand::undo()
{
	manager_.lock()->setInstrumentName(instNum_, oldName_);
}

int ChangeInstrumentNameCommand::getID() const
{
	return 0x12;
}
