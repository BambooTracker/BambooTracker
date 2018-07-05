#pragma once

#include <stack>
#include <memory>
#include "command_interface.hpp"

class CommandManager
{
public:
	using CommandIPtr = std::unique_ptr<CommandInterface>;

	CommandManager();
	void invoke(CommandIPtr command);
	void undo();
	void redo();

private:
	std::stack<CommandIPtr> undoStack_;
	std::stack<CommandIPtr> redoStack_;
};
