#include "erase_instrument_in_step_qt_command.hpp"
#include "command_id.hpp"

EraseInstrumentInStepQtCommand::EraseInstrumentInStepQtCommand(PatternEditorPanel* panel, QUndoCommand* parent)
	: QUndoCommand(parent),
	  panel_(panel)
{
}

void EraseInstrumentInStepQtCommand::redo()
{
	panel_->redrawByPatternChanged();
}

void EraseInstrumentInStepQtCommand::undo()
{
	panel_->redrawByPatternChanged();
}

int EraseInstrumentInStepQtCommand::id() const
{
	return CommandId::EraseInstrumentInStep;
}
