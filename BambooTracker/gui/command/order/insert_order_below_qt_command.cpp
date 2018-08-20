#include "insert_order_below_qt_command.hpp"

InsertOrderBelowQtCommand::InsertOrderBelowQtCommand(OrderListPanel* panel, QUndoCommand* parent)
	: QUndoCommand(parent),
	  panel_(panel)
{
}

void InsertOrderBelowQtCommand::redo()
{
	panel_->update();
	panel_->onOrderEdited();
}

void InsertOrderBelowQtCommand::undo()
{
	panel_->update();
	panel_->onOrderEdited();
}

int InsertOrderBelowQtCommand::id() const
{
	return 0x41;
}
