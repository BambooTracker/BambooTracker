#include "decrease_note_key_in_pattern_qt_command.hpp"
#include "command_id.hpp"

DecreaseNoteKeyInPatternQtCommand::DecreaseNoteKeyInPatternQtCommand(PatternEditorPanel* panel, QUndoCommand* parent)
	: QUndoCommand(parent),
	  panel_(panel)
{
}

void DecreaseNoteKeyInPatternQtCommand::redo()
{
	panel_->redrawByPatternChanged();
}

void DecreaseNoteKeyInPatternQtCommand::undo()
{
	panel_->redrawByPatternChanged();
}

int DecreaseNoteKeyInPatternQtCommand::id() const
{
	return CommandId::DecreaseNoteKeyInPattern;
}
