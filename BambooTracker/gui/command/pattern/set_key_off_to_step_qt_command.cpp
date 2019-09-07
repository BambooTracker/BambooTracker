#include "set_key_off_to_step_qt_command.hpp"
#include "command_id.hpp"

SetKeyOffToStepQtCommand::SetKeyOffToStepQtCommand(PatternEditorPanel* panel, QUndoCommand* parent)
	: QUndoCommand(parent),
	  panel_(panel)
{
}

void SetKeyOffToStepQtCommand::redo()
{
	panel_->update();
}

void SetKeyOffToStepQtCommand::undo()
{
	panel_->update();
}

int SetKeyOffToStepQtCommand::id() const
{
	return CommandId::SetKeyOffToStep;
}
