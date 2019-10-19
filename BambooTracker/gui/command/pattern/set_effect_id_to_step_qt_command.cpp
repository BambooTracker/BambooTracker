#include "set_effect_id_to_step_qt_command.hpp"
#include "command_id.hpp"

SetEffectIDToStepQtCommand::SetEffectIDToStepQtCommand(PatternEditorPanel* panel, PatternPosition pos, bool secondEntry, QUndoCommand* parent)
	: QUndoCommand(parent),
	  panel_(panel),
	  pos_(pos),
	  isSecond_(secondEntry)
{
}

void SetEffectIDToStepQtCommand::redo()
{
	panel_->redrawByPatternChanged();
}

void SetEffectIDToStepQtCommand::undo()
{
	panel_->redrawByPatternChanged();
	panel_->resetEntryCount();
}

int SetEffectIDToStepQtCommand::id() const
{
	return CommandId::SetEffectIDToStep;
}

bool SetEffectIDToStepQtCommand::mergeWith(const QUndoCommand* other)
{
	if (other->id() == id() && !isSecond_) {
		auto com = dynamic_cast<const SetEffectIDToStepQtCommand*>(other);
		if (com->getPos() == pos_ && com->isSecondEntry()) {
			isSecond_ = true;
			redo();
			return true;
		}
	}

	isSecond_ = true;
	return false;
}

PatternPosition SetEffectIDToStepQtCommand::getPos() const
{
	return pos_;
}

bool SetEffectIDToStepQtCommand::isSecondEntry() const
{
	return isSecond_;
}
