#include "increase_note_octave_in_pattern_qt_command.hpp"
#include "command_id.hpp"

IncreaseNoteOctaveInPatternQtCommand::IncreaseNoteOctaveInPatternQtCommand(PatternEditorPanel* panel, QUndoCommand* parent)
	: QUndoCommand(parent),
	  panel_(panel)
{
}

void IncreaseNoteOctaveInPatternQtCommand::redo()
{
	panel_->update();
}

void IncreaseNoteOctaveInPatternQtCommand::undo()
{
	panel_->update();
}

int IncreaseNoteOctaveInPatternQtCommand::id() const
{
	return CommandId::IncreaseNoteOctaveInPattern;
}
