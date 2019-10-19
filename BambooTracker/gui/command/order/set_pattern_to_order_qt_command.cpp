#include "set_pattern_to_order_qt_command.hpp"
#include "command_id.hpp"

SetPatternToOrderQtCommand::SetPatternToOrderQtCommand(OrderListPanel* panel, OrderPosition pos, bool secondEntry, QUndoCommand* parent)
	: QUndoCommand(parent),
	  panel_(panel),
	  pos_(pos),
	  isSecond_(secondEntry)
{
}

void SetPatternToOrderQtCommand::redo()
{
	panel_->onOrderEdited();
	panel_->redrawByPatternChanged();
}

void SetPatternToOrderQtCommand::undo()
{
	panel_->onOrderEdited();
	panel_->redrawByPatternChanged();
	panel_->resetEntryCount();
}

int SetPatternToOrderQtCommand::id() const
{
	return CommandId::SetPatternToOrder;
}

bool SetPatternToOrderQtCommand::mergeWith(const QUndoCommand* other)
{
	if (other->id() == id() && !isSecond_) {
		auto com = dynamic_cast<const SetPatternToOrderQtCommand*>(other);
		if (com->getPos() == pos_ && com->isSecondEntry()) {
			isSecond_ = true;
			redo();
			return true;
		}
	}

	isSecond_ = true;
	return false;
}

OrderPosition SetPatternToOrderQtCommand::getPos() const
{
	return pos_;
}

bool SetPatternToOrderQtCommand::isSecondEntry() const
{
	return isSecond_;
}
