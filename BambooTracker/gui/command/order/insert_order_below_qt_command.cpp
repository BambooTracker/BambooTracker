#include "insert_order_below_qt_command.hpp"
#include "command_id.hpp"

InsertOrderBelowQtCommand::InsertOrderBelowQtCommand(OrderListPanel* panel, QUndoCommand* parent)
	: QUndoCommand(parent),
	  panel_(panel)
{
}

void InsertOrderBelowQtCommand::redo()
{
	panel_->onOrderEdited();
	panel_->update();
}

void InsertOrderBelowQtCommand::undo()
{
	panel_->onOrderEdited();
	panel_->update();
}

int InsertOrderBelowQtCommand::id() const
{
	return CommandId::InsertOrderBelow;
}
