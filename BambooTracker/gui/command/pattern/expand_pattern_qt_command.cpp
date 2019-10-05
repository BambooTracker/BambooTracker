#include "expand_pattern_qt_command.hpp"
#include "command_id.hpp"

ExpandPatternQtCommand::ExpandPatternQtCommand(PatternEditorPanel* panel, QUndoCommand* parent)
	: QUndoCommand(parent),
	  panel_(panel)
{
}

void ExpandPatternQtCommand::redo()
{
	panel_->redrawByPatternChanged();
}

void ExpandPatternQtCommand::undo()
{
	panel_->redrawByPatternChanged();
}

int ExpandPatternQtCommand::id() const
{
	return CommandId::ExpandPattern;
}
