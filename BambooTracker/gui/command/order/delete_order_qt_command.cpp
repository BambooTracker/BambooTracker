#include "delete_order_qt_command.hpp"

DeleteOrderQtCommand::DeleteOrderQtCommand(OrderListPanel* panel, QUndoCommand* parent)
	: QUndoCommand(parent),
	  panel_(panel)
{
}

void DeleteOrderQtCommand::redo()
{
	panel_->update();
	panel_->onOrderEdited();
}

void DeleteOrderQtCommand::undo()
{
	panel_->update();
	panel_->onOrderEdited();
}

int DeleteOrderQtCommand::id() const
{
   return 0x42;
}
