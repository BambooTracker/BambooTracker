#include "erase_cells_in_pattern_qt_command.hpp"
#include "command_id.hpp"

EraseCellsInPatternQtCommand::EraseCellsInPatternQtCommand(PatternEditorPanel* panel, QUndoCommand* parent)
	: QUndoCommand(parent),
	  panel_(panel)
{
}

void EraseCellsInPatternQtCommand::redo()
{
	panel_->redrawByPatternChanged();
}

void EraseCellsInPatternQtCommand::undo()
{
	panel_->redrawByPatternChanged();
}

int EraseCellsInPatternQtCommand::id() const
{
	return CommandId::EraseCellsInPattern;
}
