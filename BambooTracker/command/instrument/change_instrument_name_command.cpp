/*
 * SPDX-FileCopyrightText: 2018 Rerrah
 * SPDX-License-Identifier: MIT
 */

#include "change_instrument_name_command.hpp"

ChangeInstrumentNameCommand::ChangeInstrumentNameCommand(std::weak_ptr<InstrumentsManager> manager,
														 int num, const std::string& name)
	: AbstractCommand(CommandId::ChangeInstrumentName),
	  manager_(manager),
	  instNum_(num),
	  newName_(name)
{
	oldName_ = manager_.lock()->getInstrumentName(instNum_);
}

bool ChangeInstrumentNameCommand::redo()
{
	manager_.lock()->setInstrumentName(instNum_, newName_);
	return true;
}

bool ChangeInstrumentNameCommand::undo()
{
	manager_.lock()->setInstrumentName(instNum_, oldName_);
	return true;
}
