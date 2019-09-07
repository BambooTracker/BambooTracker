#include "insert_step_qt_command.hpp"
#include "command_id.hpp"

InsertStepQtCommand::InsertStepQtCommand(PatternEditorPanel* panel, QUndoCommand* parent)
	: QUndoCommand(parent),
	  panel_(panel)
{
}

void InsertStepQtCommand::redo()
{
	panel_->update();
}

void InsertStepQtCommand::undo()
{
	panel_->update();
}

int InsertStepQtCommand::id() const
{
	return CommandId::InsertStep;
}
