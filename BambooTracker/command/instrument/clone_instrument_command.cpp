/*
 * SPDX-FileCopyrightText: 2018 Rerrah
 * SPDX-License-Identifier: MIT
 */

#include "clone_instrument_command.hpp"

cloneInstrumentCommand::cloneInstrumentCommand(std::weak_ptr<InstrumentsManager> manager,
											   int num, int refNum)
	: AbstractCommand(CommandId::CloneInstrument),
	  manager_(manager),
	  cloneInstNum_(num),
	  refInstNum_(refNum)
{
}

bool cloneInstrumentCommand::redo()
{
	manager_.lock()->cloneInstrument(cloneInstNum_, refInstNum_);
	return true;
}

bool cloneInstrumentCommand::undo()
{
	manager_.lock()->removeInstrument(cloneInstNum_);
	return true;
}
