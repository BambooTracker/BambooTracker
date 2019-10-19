#include "clone_patterns_qt_command.hpp"
#include "command_id.hpp"

ClonePatternsQtCommand::ClonePatternsQtCommand(OrderListPanel* panel, QUndoCommand* parent)
	: QUndoCommand(parent),
	  panel_(panel)
{
}

void ClonePatternsQtCommand::redo()
{
	panel_->onOrderEdited();
	panel_->redrawByPatternChanged();
}

void ClonePatternsQtCommand::undo()
{
	panel_->onOrderEdited();
	panel_->redrawByPatternChanged();
}

int ClonePatternsQtCommand::id() const
{
	return CommandId::ClonePatterns;
}
