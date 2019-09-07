#include "paste_copied_data_to_pattern_qt_command.hpp"
#include "command_id.hpp"

PasteCopiedDataToPatternQtCommand::PasteCopiedDataToPatternQtCommand(PatternEditorPanel* panel, QUndoCommand* parent)
	: QUndoCommand(parent),
	  panel_(panel)
{
}

void PasteCopiedDataToPatternQtCommand::redo()
{
	panel_->update();
}

void PasteCopiedDataToPatternQtCommand::undo()
{
	panel_->update();
}

int PasteCopiedDataToPatternQtCommand::id() const
{
	return CommandId::PasteCopiedDataToPattern;
}
