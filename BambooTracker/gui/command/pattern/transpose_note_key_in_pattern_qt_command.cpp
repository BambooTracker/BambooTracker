#include "transpose_note_key_in_pattern_qt_command.hpp"
#include "command_id.hpp"

TransposeNoteKeyInPatternQtCommand::TransposeNoteKeyInPatternQtCommand(PatternEditorPanel* panel, QUndoCommand* parent)
	: QUndoCommand(parent),
	  panel_(panel)
{
}

void TransposeNoteKeyInPatternQtCommand::redo()
{
	panel_->redrawByPatternChanged();
}

void TransposeNoteKeyInPatternQtCommand::undo()
{
	panel_->redrawByPatternChanged();
}

int TransposeNoteKeyInPatternQtCommand::id() const
{
	return CommandId::TransposeNoteKeyInPattern;
}
