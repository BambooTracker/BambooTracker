#include "set_key_release_to_step_qt_command.hpp"

SetKeyReleaseToStepQtCommand::SetKeyReleaseToStepQtCommand(PatternEditorPanel* panel, QUndoCommand* parent)
	: QUndoCommand(parent),
	  panel_(panel)
{
}

void SetKeyReleaseToStepQtCommand::redo()
{
	panel_->update();
}

void SetKeyReleaseToStepQtCommand::undo()
{
	panel_->update();
}

int SetKeyReleaseToStepQtCommand::id() const
{
	return 0x24;
}
