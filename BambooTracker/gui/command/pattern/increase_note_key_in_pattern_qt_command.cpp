#include "increase_note_key_in_pattern_qt_command.hpp"
#include "command_id.hpp"

IncreaseNoteKeyInPatternQtCommand::IncreaseNoteKeyInPatternQtCommand(PatternEditorPanel* panel, QUndoCommand* parent)
	: QUndoCommand(parent),
	  panel_(panel)
{
}

void IncreaseNoteKeyInPatternQtCommand::redo()
{
	panel_->redrawByPatternChanged();
}

void IncreaseNoteKeyInPatternQtCommand::undo()
{
	panel_->redrawByPatternChanged();
}

int IncreaseNoteKeyInPatternQtCommand::id() const
{
	return CommandId::IncreaseNoteKeyInPattern;
}
