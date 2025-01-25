/*
 * SPDX-FileCopyrightText: 2018 Rerrah
 * SPDX-License-Identifier: MIT
 */

#include "deep_clone_instrument_command.hpp"

DeepCloneInstrumentCommand::DeepCloneInstrumentCommand(std::weak_ptr<InstrumentsManager> manager,
													   int num, int refNum)
	: AbstractCommand(CommandId::DeepCloneInstrument),
	  manager_(manager),
	  cloneInstNum_(num),
	  refInstNum_(refNum)
{
}

bool DeepCloneInstrumentCommand::redo()
{
	manager_.lock()->deepCloneInstrument(cloneInstNum_, refInstNum_);
	return true;
}

bool DeepCloneInstrumentCommand::undo()
{
	manager_.lock()->removeInstrument(cloneInstNum_);
	return true;
}
