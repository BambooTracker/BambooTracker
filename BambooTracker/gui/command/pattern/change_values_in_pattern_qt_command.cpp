#include "change_values_in_pattern_qt_command.hpp"

ChangeValuesInPatternQtCommand::ChangeValuesInPatternQtCommand(PatternEditorPanel* panel, QUndoCommand* parent)
	: QUndoCommand(parent),
	  panel_(panel)
{
}

void ChangeValuesInPatternQtCommand::redo()
{
	panel_->redrawByPatternChanged(true);
}

void ChangeValuesInPatternQtCommand::undo()
{
	panel_->redrawByPatternChanged(true);
}

int ChangeValuesInPatternQtCommand::id() const
{
	return CommandId::ChangeValuesInPattern;
}
