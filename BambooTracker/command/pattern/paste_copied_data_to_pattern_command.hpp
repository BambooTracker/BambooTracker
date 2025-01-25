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

class PasteCopiedDataToPatternCommand final : public AbstractCommand
{
public:
	PasteCopiedDataToPatternCommand(
			std::weak_ptr<Module> mod, int songNum, int beginTrack, int beginColmn,
			int beginOrder, int beginStep, const Vector2d<std::string>& cells);
	bool redo() override;
	bool undo() override;

private:
	std::weak_ptr<Module> mod_;
	int song_, track_, col_, order_, step_;
	Vector2d<std::string> cells_, prevCells_;
};
