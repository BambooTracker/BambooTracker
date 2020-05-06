#include "paste_insert_copied_data_to_pattern_qt_command.hpp"
#include "command_id.hpp"

PasteInsertCopiedDataToPatternQtCommand::PasteInsertCopiedDataToPatternQtCommand(PatternEditorPanel* panel, QUndoCommand* parent)
	: QUndoCommand(parent),
	  panel_(panel)
{
}

void PasteInsertCopiedDataToPatternQtCommand::redo()
{
	panel_->redrawByPatternChanged(true);
}

void PasteInsertCopiedDataToPatternQtCommand::undo()
{
	panel_->redrawByPatternChanged(true);
}

int PasteInsertCopiedDataToPatternQtCommand::id() const
{
	return CommandId::PasteInsertCopiedDataToPattern;
}
