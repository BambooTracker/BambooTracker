#include "reverse_pattern_qt_command.hpp"

ReversePatternQtCommand::ReversePatternQtCommand(PatternEditorPanel* panel, QUndoCommand* parent)
	: QUndoCommand(parent),
	  panel_(panel)
{
}

void ReversePatternQtCommand::redo()
{
	panel_->update();
}

void ReversePatternQtCommand::undo()
{
	panel_->update();
}

int ReversePatternQtCommand::id() const
{
	return 0x38;
}
