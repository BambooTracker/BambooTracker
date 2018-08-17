#include "set_effect_value_to_step_qt_command.hpp"

SetEffectValueToStepQtCommand::SetEffectValueToStepQtCommand(PatternEditorPanel* panel, PatternPosition pos, QUndoCommand* parent)
	: QUndoCommand(parent),
	  panel_(panel),
	  pos_(pos),
	  isComplete_(false)
{
}

void SetEffectValueToStepQtCommand::redo()
{
	panel_->update();
}

void SetEffectValueToStepQtCommand::undo()
{
	panel_->update();
}

int SetEffectValueToStepQtCommand::id() const
{
	return 0x2a;
}

bool SetEffectValueToStepQtCommand::mergeWith(const QUndoCommand* other)
{
	if (other->id() == id() && !isComplete_) {
		auto com = dynamic_cast<const SetEffectValueToStepQtCommand*>(other);
		if (com->getPos() == pos_) {
			isComplete_ = true;
			redo();
			return true;
		}
	}

	isComplete_ = true;
	return false;
}

PatternPosition SetEffectValueToStepQtCommand::getPos() const
{
	return pos_;
}
