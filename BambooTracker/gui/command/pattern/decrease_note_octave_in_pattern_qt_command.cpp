#include "decrease_note_octave_in_pattern_qt_command.hpp"
#include "command_id.hpp"

DecreaseNoteOctaveInPatternQtCommand::DecreaseNoteOctaveInPatternQtCommand(PatternEditorPanel* panel, QUndoCommand* parent)
	: QUndoCommand(parent),
	  panel_(panel)
{
}

void DecreaseNoteOctaveInPatternQtCommand::redo()
{
	panel_->redrawByPatternChanged();
}

void DecreaseNoteOctaveInPatternQtCommand::undo()
{
	panel_->redrawByPatternChanged();
}

int DecreaseNoteOctaveInPatternQtCommand::id() const
{
	return CommandId::DecreaseNoteOctaveInPattern;
}
