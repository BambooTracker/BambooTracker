#include "delete_previous_step_qt_command.hpp"

DeletePreviousStepQtCommand::DeletePreviousStepQtCommand(PatternEditorPanel* panel, QUndoCommand* parent)
	: QUndoCommand(parent),
	  panel_(panel)
{
}

void DeletePreviousStepQtCommand::redo()
{
	panel_->update();
}

void DeletePreviousStepQtCommand::undo()
{
	panel_->update();
}

int DeletePreviousStepQtCommand::id() const
{
	return 0x2c;
}
