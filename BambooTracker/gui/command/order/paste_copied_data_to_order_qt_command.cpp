#include "paste_copied_data_to_order_qt_command.hpp"
#include "command_id.hpp"

PasteCopiedDataToOrderQtCommand::PasteCopiedDataToOrderQtCommand(OrderListPanel* panel, QUndoCommand* parent)
	: QUndoCommand(parent),
	  panel_(panel)
{
}

void PasteCopiedDataToOrderQtCommand::redo()
{
	panel_->onOrderEdited();
	panel_->update();
}

void PasteCopiedDataToOrderQtCommand::undo()
{
	panel_->update();
	panel_->onOrderEdited();
}

int PasteCopiedDataToOrderQtCommand::id() const
{
   return CommandId::PasteCopiedDataToOrder;
}
