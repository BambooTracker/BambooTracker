#include "erase_volume_in_step_qt_command.hpp"
#include "command_id.hpp"

EraseVolumeInStepQtCommand::EraseVolumeInStepQtCommand(PatternEditorPanel* panel, QUndoCommand* parent)
	: QUndoCommand(parent),
	  panel_(panel)
{
}

void EraseVolumeInStepQtCommand::redo()
{
	panel_->redrawByPatternChanged();
}

void EraseVolumeInStepQtCommand::undo()
{
	panel_->redrawByPatternChanged();
}

int EraseVolumeInStepQtCommand::id() const
{
	return CommandId::EraseVolumeInStep;
}
