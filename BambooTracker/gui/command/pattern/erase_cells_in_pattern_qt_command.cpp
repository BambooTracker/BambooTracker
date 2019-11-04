#include "erase_cells_in_pattern_qt_command.hpp"
#include "command_id.hpp"

EraseCellsInPatternQtCommand::EraseCellsInPatternQtCommand(PatternEditorPanel* panel, QUndoCommand* parent)
	: QUndoCommand(parent),
	  panel_(panel)
{
}

void EraseCellsInPatternQtCommand::redo()
{
	panel_->redrawByPatternChanged(true);
}

void EraseCellsInPatternQtCommand::undo()
{
	panel_->redrawByPatternChanged(true);
}

int EraseCellsInPatternQtCommand::id() const
{
	return CommandId::EraseCellsInPattern;
}
