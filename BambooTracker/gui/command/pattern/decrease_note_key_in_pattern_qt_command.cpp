#include "decrease_note_key_in_pattern_qt_command.hpp"
#include "command_id.hpp"

DecreaseNoteKeyInPatternQtCommand::DecreaseNoteKeyInPatternQtCommand(PatternEditorPanel* panel, QUndoCommand* parent)
	: QUndoCommand(parent),
	  panel_(panel)
{
}

void DecreaseNoteKeyInPatternQtCommand::redo()
{
	panel_->update();
}

void DecreaseNoteKeyInPatternQtCommand::undo()
{
	panel_->update();
}

int DecreaseNoteKeyInPatternQtCommand::id() const
{
	return CommandId::DecreaseNoteKeyInPattern;
}
