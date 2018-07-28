#include "change_instrument_name_command.hpp"

ChangeInstrumentNameCommand::ChangeInstrumentNameCommand(InstrumentsManager &manager, int num, std::string name) :
	manager_(manager), instNum_(num), newName_(name)
{
	oldName_ = manager_.getInstrumentName(instNum_);
}

void ChangeInstrumentNameCommand::redo()
{
	manager_.setInstrumentName(instNum_, newName_);
}


void ChangeInstrumentNameCommand::undo()
{
	manager_.setInstrumentName(instNum_, oldName_);
}

int ChangeInstrumentNameCommand::getID() const
{
	return 2;
}
