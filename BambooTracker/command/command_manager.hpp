/*
 * SPDX-FileCopyrightText: 2018 Rerrah
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <deque>
#include <memory>
#include "abstract_command.hpp"

class CommandManager
{
public:
	using CommandIPtr = std::unique_ptr<AbstractCommand>;

	bool invoke(CommandIPtr command);
	bool undo();
	bool redo();
	void clear();

private:
	std::deque<CommandIPtr> undoStack_, redoStack_;
};
