#pragma once

#include <stack>
#include <memory>
#include "abstruct_command.hpp"

class CommandManager
{
public:
	using CommandIPtr = std::unique_ptr<AbstructCommand>;

	CommandManager();
	void invoke(CommandIPtr command);
	void undo();
	void redo();

private:
	std::stack<CommandIPtr> undoStack_;
	std::stack<CommandIPtr> redoStack_;
};
