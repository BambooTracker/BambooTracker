#include "erase_cells_in_pattern_qt_command.hpp"

EraseCellsInPatternQtCommand::EraseCellsInPatternQtCommand(PatternEditorPanel* panel, QUndoCommand* parent)
	: QUndoCommand(parent),
	  panel_(panel)
{
}

void EraseCellsInPatternQtCommand::redo()
{
	panel_->update();
}

void EraseCellsInPatternQtCommand::undo()
{
	panel_->update();
}

int EraseCellsInPatternQtCommand::id() const
{
	return 0x2e;
}
