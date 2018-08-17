#include "set_volume_to_step_qt_command.hpp"

SetVolumeToStepQtCommand::SetVolumeToStepQtCommand(PatternEditorPanel* panel, PatternPosition pos, SoundSource src, QUndoCommand* parent)
	: QUndoCommand(parent),
	  panel_(panel),
	  pos_(pos),
	  src_(src)
{
	switch (src) {
	case SoundSource::FM:	isComplete_ = false;
	case SoundSource::SSG:	isComplete_ = true;
	}
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
	return 0x25;
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
