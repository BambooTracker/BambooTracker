#include "erase_step_qt_command.hpp"
#include "command_id.hpp"

EraseStepQtCommand::EraseStepQtCommand(PatternEditorPanel* panel, QUndoCommand* parent)
	: QUndoCommand(parent),
	  panel_(panel)
{
}

void EraseStepQtCommand::redo()
{
	panel_->redrawByPatternChanged(true);
}

void EraseStepQtCommand::undo()
{
	panel_->redrawByPatternChanged(true);
}

int EraseStepQtCommand::id() const
{
	return CommandId::EraseStep;
}
