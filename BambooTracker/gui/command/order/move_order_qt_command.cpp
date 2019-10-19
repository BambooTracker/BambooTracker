#include "move_order_qt_command.hpp"
#include "command_id.hpp"

MoveOrderQtCommand::MoveOrderQtCommand(OrderListPanel* panel, QUndoCommand* parent)
	: QUndoCommand(parent),
	  panel_(panel)
{
}

void MoveOrderQtCommand::redo()
{
	panel_->onOrderEdited();
	panel_->redrawByPatternChanged();
}

void MoveOrderQtCommand::undo()
{
	panel_->onOrderEdited();
	panel_->redrawByPatternChanged();
}

int MoveOrderQtCommand::id() const
{
	return CommandId::MoveOrder;
}
