/*
 * SPDX-FileCopyrightText: 2020 Rerrah
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <memory>
#include <vector>
#include "../abstract_command.hpp"
#include "module.hpp"

class TransposeNoteInPatternCommand final : public AbstractCommand
{
public:
	TransposeNoteInPatternCommand(std::weak_ptr<Module> mod, int songNum,
								  int beginTrack, int beginOrder, int beginStep,
								  int endTrack, int endStep, int semitone);
	bool redo() override;
	bool undo() override;

private:
	std::weak_ptr<Module> mod_;
	int song_;
	int bTrack_, order_, bStep_;
	int eTrack_, eStep_;
	int semitone_;
	std::vector<int> prevKeys_;
};
