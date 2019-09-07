#include "set_effect_id_to_step_qt_command.hpp"
#include "command_id.hpp"

SetEffectIDToStepQtCommand::SetEffectIDToStepQtCommand(PatternEditorPanel* panel, PatternPosition pos, QUndoCommand* parent)
	: QUndoCommand(parent),
	  panel_(panel),
	  pos_(pos),
	  isComplete_(false)
{
}

void SetEffectIDToStepQtCommand::redo()
{
	panel_->update();
}

void SetEffectIDToStepQtCommand::undo()
{
	panel_->update();
}

int SetEffectIDToStepQtCommand::id() const
{
	return CommandId::SetEffectIDToStep;
}

bool SetEffectIDToStepQtCommand::mergeWith(const QUndoCommand* other)
{
	if (other->id() == id() && !isComplete_) {
		auto com = dynamic_cast<const SetEffectIDToStepQtCommand*>(other);
		if (com->getPos() == pos_) {
			isComplete_ = true;
			redo();
			return true;
		}
	}

	isComplete_ = true;
	return false;
}

PatternPosition SetEffectIDToStepQtCommand::getPos() const
{
	return pos_;
}
