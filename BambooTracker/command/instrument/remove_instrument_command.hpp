/*
 * SPDX-FileCopyrightText: 2018 Rerrah
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <memory>
#include "../abstract_command.hpp"
#include "instruments_manager.hpp"
#include "instrument.hpp"

class RemoveInstrumentCommand final : public AbstractCommand
{
public:
	RemoveInstrumentCommand(std::weak_ptr<InstrumentsManager> manager, int number);
	bool redo() override;
	bool undo() override;

private:
	std::weak_ptr<InstrumentsManager> manager_;
	int number_;
	std::unique_ptr<AbstractInstrument> inst_;
};
