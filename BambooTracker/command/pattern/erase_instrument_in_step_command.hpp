/*
 * SPDX-FileCopyrightText: 2018 Rerrah
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <memory>
#include "../abstract_command.hpp"
#include "module.hpp"

class EraseInstrumentInStepCommand final : public AbstractCommand
{
public:
	EraseInstrumentInStepCommand(std::weak_ptr<Module> mod, int songNum, int trackNum,
								 int orderNum, int stepNum);
	bool redo() override;
	bool undo() override;

private:
	std::weak_ptr<Module> mod_;
	int song_, track_, order_, step_;
	int prevInst_;
};
