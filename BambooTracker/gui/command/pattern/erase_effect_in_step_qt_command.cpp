#include "erase_effect_in_step_qt_command.hpp"
#include "command_id.hpp"

EraseEffectInStepQtCommand::EraseEffectInStepQtCommand(PatternEditorPanel* panel, QUndoCommand* parent)
	: QUndoCommand(parent),
	  panel_(panel)
{
}

void EraseEffectInStepQtCommand::redo()
{
	panel_->update();
}

void EraseEffectInStepQtCommand::undo()
{
	panel_->update();
}

int EraseEffectInStepQtCommand::id() const
{
	return CommandId::EraseEffectInStep;
}
