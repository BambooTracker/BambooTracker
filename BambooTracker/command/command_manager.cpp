#include "command_manager.hpp"
#include <utility>

CommandManager::CommandManager() {}

void CommandManager::invoke(CommandIPtr command)
{
	command->invoke();
	redoStack_ = std::stack<CommandIPtr>();
	undoStack_.push(std::move(command));
}

void CommandManager::undo()
{
	if (undoStack_.empty()) return;
	CommandIPtr command = std::move(undoStack_.top());
	command->undo();
	undoStack_.pop();
	redoStack_.push(std::move(command));
}

void CommandManager::redo()
{
	if (redoStack_.empty()) return;
	CommandIPtr command = std::move(redoStack_.top());
	command->redo();
	redoStack_.pop();
	undoStack_.push(std::move(command));
}
