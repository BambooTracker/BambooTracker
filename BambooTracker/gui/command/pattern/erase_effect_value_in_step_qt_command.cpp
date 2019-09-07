#include "erase_effect_value_in_step_qt_command.hpp"
#include "command_id.hpp"

EraseEffectValueInStepQtCommand::EraseEffectValueInStepQtCommand(PatternEditorPanel* panel, QUndoCommand* parent)
	: QUndoCommand(parent),
	  panel_(panel)
{
}

void EraseEffectValueInStepQtCommand::redo()
{
	panel_->update();
}

void EraseEffectValueInStepQtCommand::undo()
{
	panel_->update();
}

int EraseEffectValueInStepQtCommand::id() const
{
	return CommandId::EraseEffectValueInStep;
}
