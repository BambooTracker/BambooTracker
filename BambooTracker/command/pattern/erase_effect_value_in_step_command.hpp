/*
 * SPDX-FileCopyrightText: 2018 Rerrah
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <memory>
#include "../abstract_command.hpp"
#include "module.hpp"

class EraseEffectValueInStepCommand final : public AbstractCommand
{
public:
	EraseEffectValueInStepCommand(std::weak_ptr<Module> mod, int songNum, int trackNum,
								  int orderNum, int stepNum, int n);
	bool redo() override;
	bool undo() override;

private:
	std::weak_ptr<Module> mod_;
	int song_, track_, order_, step_, n_;
	int prevVal_;
};
