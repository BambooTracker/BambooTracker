/*
 * SPDX-FileCopyrightText: 2018 Rerrah
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <memory>
#include <string>
#include "../abstract_command.hpp"
#include "module.hpp"

class DeletePreviousStepCommand final : public AbstractCommand
{
public:
	DeletePreviousStepCommand(std::weak_ptr<Module> mod, int songNum, int trackNum,
							  int orderNum, int stepNum);
	bool redo() override;
	bool undo() override;

private:
	std::weak_ptr<Module> mod_;
	int song_, track_, order_, step_;
	int prevNote_, prevInst_, prevVol_;
	Step::PlainEffect prevEff_[Step::N_EFFECT];
};
