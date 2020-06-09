#include "delete_order_qt_command.hpp"
#include "command_id.hpp"

DeleteOrderQtCommand::DeleteOrderQtCommand(OrderListPanel* panel, QUndoCommand* parent)
	: QUndoCommand(parent),
	  panel_(panel)
{
}

void DeleteOrderQtCommand::redo()
{
	panel_->onOrderEdited();
	panel_->redrawByPatternChanged(true);
}

void DeleteOrderQtCommand::undo()
{
	panel_->onOrderEdited();
	panel_->redrawByPatternChanged(true);
}

int DeleteOrderQtCommand::id() const
{
	return CommandId::DeleteOrder;
}
