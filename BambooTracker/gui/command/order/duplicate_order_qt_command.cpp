#include "duplicate_order_qt_command.hpp"

DuplicateOrderQtCommand::DuplicateOrderQtCommand(OrderListPanel* panel, QUndoCommand* parent)
	: QUndoCommand(parent),
	  panel_(panel)
{
}

void DuplicateOrderQtCommand::redo()
{
	panel_->onOrderEdited();
	panel_->update();
}

void DuplicateOrderQtCommand::undo()
{
	panel_->onOrderEdited();
	panel_->update();
}

int DuplicateOrderQtCommand::id() const
{
	return 0x44;
}
