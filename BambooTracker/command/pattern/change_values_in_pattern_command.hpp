/*
 * SPDX-FileCopyrightText: 2020 Rerrah
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <memory>
#include <vector>
#include "../abstract_command.hpp"
#include "module.hpp"

class ChangeValuesInPatternCommand final : public AbstractCommand
{
public:
	ChangeValuesInPatternCommand(std::weak_ptr<Module> mod, int songNum,
								 int beginTrack, int beginColumn, int beginOrder, int beginStep,
								 int endTrack, int endColumn, int endStep, int value, bool isFMReversed);
	bool redo() override;
	bool undo() override;

private:
	std::weak_ptr<Module> mod_;
	int song_;
	int bTrack_, bCol_, order_, bStep_;
	int eTrack_, eCol_, eStep_;
	int diff_;
	bool fmReverse_;
	std::vector<std::vector<int>> prevVals_;
};
