/*
 * SPDX-FileCopyrightText: 2018 Rerrah
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <memory>
#include <string>
#include "../abstract_command.hpp"
#include "module.hpp"
#include "vector_2d.hpp"

class ShrinkPatternCommand final : public AbstractCommand
{
public:
	ShrinkPatternCommand(std::weak_ptr<Module> mod, int songNum,
						 int beginTrack, int beginColumn, int beginOrder, int beginStep,
						 int endTrack, int endColumn, int endStep);
	bool redo() override;
	bool undo() override;

private:
	std::weak_ptr<Module> mod_;
	int song_, bTrack_, bCol_, order_, bStep_;
	int eStep_;
	Vector2d<std::string> prevCells_;
};
