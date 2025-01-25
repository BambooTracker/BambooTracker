/*
 * SPDX-FileCopyrightText: 2018 Rerrah
 * SPDX-License-Identifier: MIT
 */

#include "remove_instrument_command.hpp"
#include <utility>

RemoveInstrumentCommand::RemoveInstrumentCommand(std::weak_ptr<InstrumentsManager> manager,
												 int number)
	: AbstractCommand(CommandId::RemoveInstrument), manager_(manager), number_(number)
{
}

bool RemoveInstrumentCommand::redo()
{
	inst_ = manager_.lock()->removeInstrument(number_);
	return true;
}

bool RemoveInstrumentCommand::undo()
{
	manager_.lock()->addInstrument(inst_.release());
	return true;
}
