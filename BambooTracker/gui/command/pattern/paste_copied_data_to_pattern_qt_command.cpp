#include "paste_copied_data_to_pattern_qt_command.hpp"
#include "command_id.hpp"

PasteCopiedDataToPatternQtCommand::PasteCopiedDataToPatternQtCommand(PatternEditorPanel* panel, QUndoCommand* parent)
	: QUndoCommand(parent),
	  panel_(panel)
{
}

void PasteCopiedDataToPatternQtCommand::redo()
{
	panel_->redrawByPatternChanged();
}

void PasteCopiedDataToPatternQtCommand::undo()
{
	panel_->redrawByPatternChanged();
}

int PasteCopiedDataToPatternQtCommand::id() const
{
	return CommandId::PasteCopiedDataToPattern;
}
