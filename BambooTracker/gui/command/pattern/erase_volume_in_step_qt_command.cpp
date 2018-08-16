#include "erase_volume_in_step_qt_command.hpp"

EraseVolumeInStepQtCommand::EraseVolumeInStepQtCommand(PatternEditorPanel* panel, QUndoCommand* parent)
	: QUndoCommand(parent),
	  panel_(panel)
{
}

void EraseVolumeInStepQtCommand::redo()
{
	panel_->update();
}

void EraseVolumeInStepQtCommand::undo()
{
	panel_->update();
}

int EraseVolumeInStepQtCommand::id() const
{
	return 0x27;
}
