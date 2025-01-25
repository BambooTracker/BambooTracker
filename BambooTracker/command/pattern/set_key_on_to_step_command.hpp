/*
 * SPDX-FileCopyrightText: 2018 Rerrah
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <memory>
#include "../abstract_command.hpp"
#include "module.hpp"

class SetKeyOnToStepCommand final : public AbstractCommand
{
public:
	SetKeyOnToStepCommand(
			std::weak_ptr<Module> mod, int songNum, int trackNum, int orderNum, int stepNum,
			int noteNum, bool instMask, int instNum, bool volMask, int vol, bool isFMReversed);
	bool redo() override;
	bool undo() override;

private:
	std::weak_ptr<Module> mod_;
	int song_, track_, order_, step_, note_, inst_, vol_;
	int prevNote_, prevInst_, prevVol_;
	bool instMask_, volMask_;
	bool isFMReversed_;
};
