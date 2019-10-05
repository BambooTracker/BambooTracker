#include "erase_effect_value_in_step_qt_command.hpp"
#include "command_id.hpp"

EraseEffectValueInStepQtCommand::EraseEffectValueInStepQtCommand(PatternEditorPanel* panel, QUndoCommand* parent)
	: QUndoCommand(parent),
	  panel_(panel)
{
}

void EraseEffectValueInStepQtCommand::redo()
{
	panel_->redrawByPatternChanged();
}

void EraseEffectValueInStepQtCommand::undo()
{
	panel_->redrawByPatternChanged();
}

int EraseEffectValueInStepQtCommand::id() const
{
	return CommandId::EraseEffectValueInStep;
}
