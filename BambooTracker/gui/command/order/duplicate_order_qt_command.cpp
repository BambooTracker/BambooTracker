#include "duplicate_order_qt_command.hpp"
#include "command_id.hpp"

DuplicateOrderQtCommand::DuplicateOrderQtCommand(OrderListPanel* panel, QUndoCommand* parent)
	: QUndoCommand(parent),
	  panel_(panel)
{
}

void DuplicateOrderQtCommand::redo()
{
	panel_->onOrderEdited();
	panel_->redrawByPatternChanged();
}

void DuplicateOrderQtCommand::undo()
{
	panel_->onOrderEdited();
	panel_->redrawByPatternChanged();
}

int DuplicateOrderQtCommand::id() const
{
	return CommandId::DuplicateOrder;
}
