#include "shrink_pattern_qt_command.hpp"

ShrinkPatternQtCommand::ShrinkPatternQtCommand(PatternEditorPanel* panel, QUndoCommand* parent)
	: QUndoCommand(parent),
	  panel_(panel)
{
}

void ShrinkPatternQtCommand::redo()
{
	panel_->update();
}

void ShrinkPatternQtCommand::undo()
{
	panel_->update();
}

int ShrinkPatternQtCommand::id() const
{
	return 0x35;
}
