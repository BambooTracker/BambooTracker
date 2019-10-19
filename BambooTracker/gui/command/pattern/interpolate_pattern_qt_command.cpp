#include "interpolate_pattern_qt_command.hpp"
#include "command_id.hpp"

InterpolatePatternQtCommand::InterpolatePatternQtCommand(PatternEditorPanel* panel, QUndoCommand* parent)
	: QUndoCommand(parent),
	  panel_(panel)
{
}

void InterpolatePatternQtCommand::redo()
{
	panel_->redrawByPatternChanged();
}

void InterpolatePatternQtCommand::undo()
{
	panel_->redrawByPatternChanged();
}

int InterpolatePatternQtCommand::id() const
{
	return CommandId::InterpolatePattern;
}
