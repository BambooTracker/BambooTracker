#include "transpose_note_in_pattern_qt_command.hpp"
#include "command_id.hpp"

TransposeNoteInPatternQtCommand::TransposeNoteInPatternQtCommand(PatternEditorPanel* panel, QUndoCommand* parent)
	: QUndoCommand(parent),
	  panel_(panel)
{
}

void TransposeNoteInPatternQtCommand::redo()
{
	panel_->redrawByPatternChanged();
}

void TransposeNoteInPatternQtCommand::undo()
{
	panel_->redrawByPatternChanged();
}

int TransposeNoteInPatternQtCommand::id() const
{
	return CommandId::TransposeNoteInPattern;
}
