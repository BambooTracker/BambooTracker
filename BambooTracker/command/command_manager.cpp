/*
 * SPDX-FileCopyrightText: 2018 Rerrah
 * SPDX-License-Identifier: MIT
 */

#include "command_manager.hpp"
#include <utility>

bool CommandManager::invoke(CommandIPtr command)
{
	redoStack_.clear();

	if (!undoStack_.empty() && undoStack_.back()->mergeWith(command.get())) {
		return true;
	}

	redoStack_.push_back(std::move(command));

	return redo();
}

bool CommandManager::undo()
{
	if (undoStack_.empty()) return true;

	CommandIPtr command = std::move(undoStack_.back());
	if (!command) return false;

	if (command->undo()) {
		// Push and pop history stacks.
		undoStack_.pop_back();
		redoStack_.push_back(std::move(command));
		return true;
	}
	else {
		// Rollback.
		command->redo();
		return false;
	}
}

bool CommandManager::redo()
{
	if (redoStack_.empty()) return true;

	CommandIPtr command = std::move(redoStack_.back());
	if (!command) return false;

	if (command->redo()) {
		// Push and pop history stacks.
		redoStack_.pop_back();
		undoStack_.push_back(std::move(command));
		return true;
	}
	else {
		// Rollback.
		command->undo();
		return false;
	}
}

void CommandManager::clear()
{
	redoStack_.clear();
	undoStack_.clear();
}
