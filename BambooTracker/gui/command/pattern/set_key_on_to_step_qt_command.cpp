#include "set_key_on_to_step_qt_command.hpp"

SetKeyOnToStepQtCommand::SetKeyOnToStepQtCommand(PatternEditorPanel* panel, QUndoCommand* parent)
	: QUndoCommand(parent),
	  panel_(panel)
{
}

void SetKeyOnToStepQtCommand::redo()
{
	panel_->update();
}

void SetKeyOnToStepQtCommand::undo()
{
	panel_->update();
}

int SetKeyOnToStepQtCommand::id() const
{
	return 0x22;
}
