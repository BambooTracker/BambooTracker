#include "move_order_qt_command.hpp"

MoveOrderQtCommand::MoveOrderQtCommand(OrderListPanel* panel, QUndoCommand* parent)
	: QUndoCommand(parent),
	  panel_(panel)
{
}

void MoveOrderQtCommand::redo()
{
	panel_->onOrderEdited();
	panel_->update();
}

void MoveOrderQtCommand::undo()
{
	panel_->onOrderEdited();
	panel_->update();
}

int MoveOrderQtCommand::id() const
{
	return 0x45;
}
