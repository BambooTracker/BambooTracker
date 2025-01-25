/*
 * SPDX-FileCopyrightText: 2018 Rerrah
 * SPDX-License-Identifier: MIT
 */

#include "add_instrument_command.hpp"
#include <utility>

AddInstrumentCommand::AddInstrumentCommand(std::weak_ptr<InstrumentsManager> manager,
										   int num, InstrumentType type, const std::string& name)
	: AbstractCommand(CommandId::AddInstrument),
	  manager_(manager),
	  num_(num),
	  type_(type),
	  name_(name)
{
}

AddInstrumentCommand::AddInstrumentCommand(std::weak_ptr<InstrumentsManager> manager,
										   std::unique_ptr<AbstractInstrument> inst)
	: AbstractCommand(CommandId::AddInstrument),
	  manager_(manager),
	  num_(inst->getNumber()),
	  inst_(std::move(inst))
{
}

bool AddInstrumentCommand::redo()
{
	if (inst_) manager_.lock()->addInstrument(inst_->clone());
	else manager_.lock()->addInstrument(num_, type_, name_);
	return true;
}

bool AddInstrumentCommand::undo()
{
	manager_.lock()->removeInstrument(num_);
	return true;
}
