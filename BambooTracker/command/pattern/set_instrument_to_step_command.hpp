/*
 * SPDX-FileCopyrightText: 2018 Rerrah
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <memory>
#include "../abstract_command.hpp"
#include "module.hpp"

class SetInstrumentToStepCommand final : public AbstractCommand
{
public:
	SetInstrumentToStepCommand(std::weak_ptr<Module> mod, int songNum, int trackNum,
							   int orderNum, int stepNum, int instNum, bool secondEntry);
	bool redo() override;
	bool undo() override;
	bool mergeWith(const AbstractCommand* other) override;

private:
	std::weak_ptr<Module> mod_;
	const int song_, track_, order_, step_;
	int inst_;
	const int prevInst_;
	bool isSecondEntry_;
};
