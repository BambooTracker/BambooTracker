#include "set_echo_buffer_access_qt_command.hpp"

SetEchoBufferAccessQtCommand::SetEchoBufferAccessQtCommand(PatternEditorPanel* panel, QUndoCommand* parent)
	: QUndoCommand(parent),
	  panel_(panel)
{
}

void SetEchoBufferAccessQtCommand::redo()
{
	panel_->update();
}

void SetEchoBufferAccessQtCommand::undo()
{
	panel_->update();
}

int SetEchoBufferAccessQtCommand::id() const
{
	return 0x36;
}
