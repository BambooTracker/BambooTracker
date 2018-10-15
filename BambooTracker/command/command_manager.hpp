#pragma once

#include <stack>
#include <memory>
#include "abstract_command.hpp"

class CommandManager
{
public:
	using CommandIPtr = std::unique_ptr<AbstractCommand>;

	CommandManager();
	void invoke(CommandIPtr command);
	void undo();
	void redo();
	void clear();

private:
	std::stack<CommandIPtr> undoStack_;
	std::stack<CommandIPtr> redoStack_;
};
