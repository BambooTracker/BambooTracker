#include "set_note_to_step_qt_command.hpp"

SetNoteToStepQtCommand::SetNoteToStepQtCommand(PatternEditorPanel* panel, QUndoCommand* parent)
	: QUndoCommand(parent),
	  panel_(panel)
{
}

void SetNoteToStepQtCommand::redo()
{
	panel_->update();
}

void SetNoteToStepQtCommand::undo()
{
	panel_->update();
}

int SetNoteToStepQtCommand::id() const
{
	return 0x21;
}
