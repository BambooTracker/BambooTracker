#include "shrink_pattern_qt_command.hpp"
#include "command_id.hpp"

ShrinkPatternQtCommand::ShrinkPatternQtCommand(PatternEditorPanel* panel, QUndoCommand* parent)
	: QUndoCommand(parent),
	  panel_(panel)
{
}

void ShrinkPatternQtCommand::redo()
{
	panel_->redrawByPatternChanged(true);
}

void ShrinkPatternQtCommand::undo()
{
	panel_->redrawByPatternChanged(true);
}

int ShrinkPatternQtCommand::id() const
{
	return CommandId::ShrinkPattern;
}
