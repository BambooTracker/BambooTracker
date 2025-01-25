/*
 * SPDX-FileCopyrightText: 2018 Rerrah
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <memory>
#include <vector>
#include "../abstract_command.hpp"
#include "module.hpp"

class ReplaceInstrumentInPatternCommand final : public AbstractCommand
{
public:
	ReplaceInstrumentInPatternCommand(std::weak_ptr<Module> mod, int songNum,
									  int beginTrack, int beginOrder, int beginStep,
									  int endTrack, int endStep, int newInst);
	bool redo() override;
	bool undo() override;

private:
	std::weak_ptr<Module> mod_;
	int song_;
	int bTrack_, order_, bStep_;
	int eTrack_, eStep_;
	int inst_;
	std::vector<int> prevInsts_;
};
