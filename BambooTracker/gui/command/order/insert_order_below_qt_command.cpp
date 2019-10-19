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
	panel_->redrawByPatternChanged();
}

void InsertOrderBelowQtCommand::undo()
{
	panel_->onOrderEdited();
	panel_->redrawByPatternChanged();
}

int InsertOrderBelowQtCommand::id() const
{
	return CommandId::InsertOrderBelow;
}
