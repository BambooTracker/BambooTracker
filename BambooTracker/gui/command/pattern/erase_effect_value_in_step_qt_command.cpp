#include "erase_effect_value_in_step_qt_command.hpp"
#include "command_id.hpp"

EraseEffectValueInStepQtCommand::EraseEffectValueInStepQtCommand(PatternEditorPanel* panel, QUndoCommand* parent)
	: QUndoCommand(parent),
	  panel_(panel)
{
}

void EraseEffectValueInStepQtCommand::redo()
{
	panel_->redrawByPatternChanged(true);
}

void EraseEffectValueInStepQtCommand::undo()
{
	panel_->redrawByPatternChanged(true);
}

int EraseEffectValueInStepQtCommand::id() const
{
	return CommandId::EraseEffectValueInStep;
}
