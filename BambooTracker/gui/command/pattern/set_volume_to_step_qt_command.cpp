#include "set_volume_to_step_qt_command.hpp"
#include "command_id.hpp"

SetVolumeToStepQtCommand::SetVolumeToStepQtCommand(PatternEditorPanel* panel, PatternPosition pos, QUndoCommand* parent)
	: QUndoCommand(parent),
	  panel_(panel),
	  pos_(pos),
	  isComplete_(false)
{
}

void SetVolumeToStepQtCommand::redo()
{
	panel_->update();
}

void SetVolumeToStepQtCommand::undo()
{
	panel_->update();
}

int SetVolumeToStepQtCommand::id() const
{
	return CommandId::SetVolumeToStep;
}

bool SetVolumeToStepQtCommand::mergeWith(const QUndoCommand* other)
{
	if (other->id() == id() && !isComplete_) {
		auto com = dynamic_cast<const SetVolumeToStepQtCommand*>(other);
		if (com->getPos() == pos_) {
			isComplete_ = true;
			redo();
			return true;
		}
	}

	isComplete_ = true;
	return false;
}

PatternPosition SetVolumeToStepQtCommand::getPos() const
{
	return pos_;
}
