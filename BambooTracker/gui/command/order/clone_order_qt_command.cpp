#include "clone_order_qt_command.hpp"
#include "command_id.hpp"

CloneOrderQtCommand::CloneOrderQtCommand(OrderListPanel* panel, QUndoCommand* parent)
	: QUndoCommand(parent),
	  panel_(panel)
{
}

void CloneOrderQtCommand::redo()
{
	panel_->onOrderEdited();
	panel_->update();
}

void CloneOrderQtCommand::undo()
{
	panel_->onOrderEdited();
	panel_->update();
}

int CloneOrderQtCommand::id() const
{
	return CommandId::CloneOrder;
}
