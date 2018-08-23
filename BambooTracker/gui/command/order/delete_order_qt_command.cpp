#include "delete_order_qt_command.hpp"

DeleteOrderQtCommand::DeleteOrderQtCommand(OrderListPanel* panel, QUndoCommand* parent)
	: QUndoCommand(parent),
	  panel_(panel)
{
}

void DeleteOrderQtCommand::redo()
{
	panel_->onOrderEdited();
	panel_->update();
}

void DeleteOrderQtCommand::undo()
{
	panel_->onOrderEdited();
	panel_->update();
}

int DeleteOrderQtCommand::id() const
{
   return 0x42;
}
