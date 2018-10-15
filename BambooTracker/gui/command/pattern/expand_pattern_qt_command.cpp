#include "expand_pattern_qt_command.hpp"

ExpandPatternQtCommand::ExpandPatternQtCommand(PatternEditorPanel* panel, QUndoCommand* parent)
	: QUndoCommand(parent),
	  panel_(panel)
{
}

void ExpandPatternQtCommand::redo()
{
	panel_->update();
}

void ExpandPatternQtCommand::undo()
{
	panel_->update();
}

int ExpandPatternQtCommand::id() const
{
	return 0x34;
}
