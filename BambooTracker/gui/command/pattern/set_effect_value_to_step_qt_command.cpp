#include "set_effect_value_to_step_qt_command.hpp"
#include "command_id.hpp"

SetEffectValueToStepQtCommand::SetEffectValueToStepQtCommand(PatternEditorPanel* panel, PatternPosition pos, bool secondEntry, QUndoCommand* parent)
	: QUndoCommand(parent),
	  panel_(panel),
	  pos_(pos),
	  isSecond_(secondEntry)
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
	return CommandId::SetEffectValueToStep;
}

bool SetEffectValueToStepQtCommand::mergeWith(const QUndoCommand* other)
{
	if (other->id() == id() && !isSecond_) {
		auto com = dynamic_cast<const SetEffectValueToStepQtCommand*>(other);
		if (com->getPos() == pos_ && com->isSecondEntry()) {
			isSecond_ = true;
			redo();
			return true;
		}
	}

	isSecond_ = true;
	return false;
}

PatternPosition SetEffectValueToStepQtCommand::getPos() const
{
	return pos_;
}

bool SetEffectValueToStepQtCommand::isSecondEntry() const
{
	return isSecond_;
}
