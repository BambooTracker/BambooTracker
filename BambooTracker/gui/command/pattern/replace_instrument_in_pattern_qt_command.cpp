#include "replace_instrument_in_pattern_qt_command.hpp"
#include "command_id.hpp"

ReplaceInstrumentInPatternQtCommand::ReplaceInstrumentInPatternQtCommand(PatternEditorPanel* panel, QUndoCommand* parent)
	: QUndoCommand(parent),
	  panel_(panel)
{
}

void ReplaceInstrumentInPatternQtCommand::redo()
{
	panel_->redrawByPatternChanged();
}

void ReplaceInstrumentInPatternQtCommand::undo()
{
	panel_->redrawByPatternChanged();
}

int ReplaceInstrumentInPatternQtCommand::id() const
{
	return CommandId::ReplaceInstrumentInPattern;
}
