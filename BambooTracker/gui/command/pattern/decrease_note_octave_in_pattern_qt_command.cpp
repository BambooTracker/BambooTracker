#include "decrease_note_octave_in_pattern_qt_command.hpp"

DecreaseNoteOctaveInPatternQtCommand::DecreaseNoteOctaveInPatternQtCommand(PatternEditorPanel* panel, QUndoCommand* parent)
	: QUndoCommand(parent),
	  panel_(panel)
{
}

void DecreaseNoteOctaveInPatternQtCommand::redo()
{
	panel_->update();
}

void DecreaseNoteOctaveInPatternQtCommand::undo()
{
	panel_->update();
}

int DecreaseNoteOctaveInPatternQtCommand::id() const
{
	return 0x33;
}
