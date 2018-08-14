#include "erase_note_in_step_qt_command.hpp"

EraseNoteInStepQtCommand::EraseNoteInStepQtCommand(PatternEditorPanel* panel, QUndoCommand* parent)
	: QUndoCommand(parent),
	  panel_(panel)
{
}

void EraseNoteInStepQtCommand::redo()
{
	panel_->update();
}

void EraseNoteInStepQtCommand::undo()
{
	panel_->update();
}

int EraseNoteInStepQtCommand::id() const
{
	return 0x25;
}
