#include "set_pattern_to_order_qt_command.hpp"
#include "command_id.hpp"

SetPatternToOrderQtCommand::SetPatternToOrderQtCommand(OrderListPanel* panel, OrderPosition pos, QUndoCommand* parent)
	: QUndoCommand(parent),
	  panel_(panel),
	  pos_(pos),
	  isComplete_(false)
{
}

void SetPatternToOrderQtCommand::redo()
{
	panel_->onOrderEdited();
	panel_->update();
}

void SetPatternToOrderQtCommand::undo()
{
	panel_->onOrderEdited();
	panel_->update();
}

int SetPatternToOrderQtCommand::id() const
{
	return CommandId::SetPatternToOrder;
}

bool SetPatternToOrderQtCommand::mergeWith(const QUndoCommand* other)
{
	if (other->id() == id() && !isComplete_) {
		auto com = dynamic_cast<const SetPatternToOrderQtCommand*>(other);
		if (com->getPos() == pos_) {
			isComplete_ = true;
			redo();
			return true;
		}
	}

	isComplete_ = true;
	return false;
}

OrderPosition SetPatternToOrderQtCommand::getPos() const
{
	return pos_;
}
