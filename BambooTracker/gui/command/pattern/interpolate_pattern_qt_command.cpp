#include "interpolate_pattern_qt_command.hpp"

InterpolatePatternQtCommand::InterpolatePatternQtCommand(PatternEditorPanel* panel, QUndoCommand* parent)
	: QUndoCommand(parent),
	  panel_(panel)
{
}

void InterpolatePatternQtCommand::redo()
{
	panel_->update();
}

void InterpolatePatternQtCommand::undo()
{
	panel_->update();
}

int InterpolatePatternQtCommand::id() const
{
	return 0x36;
}
