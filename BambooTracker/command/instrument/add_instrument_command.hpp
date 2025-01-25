/*
 * SPDX-FileCopyrightText: 2018 Rerrah
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <memory>
#include <string>
#include "../abstract_command.hpp"
#include "instrument.hpp"
#include "instruments_manager.hpp"

class AddInstrumentCommand final : public AbstractCommand
{
public:
	AddInstrumentCommand(std::weak_ptr<InstrumentsManager> manager,
						 int num, InstrumentType type, const std::string& name);
	AddInstrumentCommand(std::weak_ptr<InstrumentsManager> manager,
						 std::unique_ptr<AbstractInstrument> inst);
	bool redo() override;
	bool undo() override;

private:
	std::weak_ptr<InstrumentsManager> manager_;
	int num_;
	InstrumentType type_;
	std::string name_;
	std::unique_ptr<AbstractInstrument> inst_;
};
