#include "set_volume_to_step_qt_command.hpp"
#include "command_id.hpp"

SetVolumeToStepQtCommand::SetVolumeToStepQtCommand(PatternEditorPanel* panel, PatternPosition pos, bool secondEntry, QUndoCommand* parent)
	: QUndoCommand(parent),
	  panel_(panel),
	  pos_(pos),
	  isSecond_(secondEntry)
{
}

void SetVolumeToStepQtCommand::redo()
{
	panel_->redrawByPatternChanged();
}

void SetVolumeToStepQtCommand::undo()
{
	panel_->redrawByPatternChanged();
}

int SetVolumeToStepQtCommand::id() const
{
	return CommandId::SetVolumeToStep;
}

bool SetVolumeToStepQtCommand::mergeWith(const QUndoCommand* other)
{
	if (other->id() == id() && !isSecond_) {
		auto com = dynamic_cast<const SetVolumeToStepQtCommand*>(other);
		if (com->getPos() == pos_ && com->isSecondEntry()) {
			isSecond_ = true;
			redo();
			return true;
		}
	}

	isSecond_ = true;
	return false;
}

PatternPosition SetVolumeToStepQtCommand::getPos() const
{
	return pos_;
}

bool SetVolumeToStepQtCommand::isSecondEntry() const
{
	return isSecond_;
}
