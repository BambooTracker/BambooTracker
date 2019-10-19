#include "set_echo_buffer_access_qt_command.hpp"
#include "command_id.hpp"

SetEchoBufferAccessQtCommand::SetEchoBufferAccessQtCommand(PatternEditorPanel* panel, QUndoCommand* parent)
	: QUndoCommand(parent),
	  panel_(panel)
{
}

void SetEchoBufferAccessQtCommand::redo()
{
	panel_->redrawByPatternChanged();
}

void SetEchoBufferAccessQtCommand::undo()
{
	panel_->redrawByPatternChanged();
}

int SetEchoBufferAccessQtCommand::id() const
{
	return CommandId::SetEchoBufferAccess;
}
