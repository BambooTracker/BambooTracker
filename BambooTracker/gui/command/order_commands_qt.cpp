/*
 * Copyright (C) 2018-2020 Rerrah
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#include "order_commands_qt.hpp"
#include "gui/order_list_editor/order_list_panel.hpp"

OrderListCommonQtCommand::OrderListCommonQtCommand(CommandId id, OrderListPanel* panel, bool orderLengthChanged, QUndoCommand* parent)
	: QUndoCommand(parent), panel_(panel), id_(id), orderLenChanged_(orderLengthChanged)
{
}

void OrderListCommonQtCommand::redo()
{
	panel_->onOrderEdited();
	panel_->redrawByPatternChanged(orderLenChanged_);
}

void OrderListCommonQtCommand::undo()
{
	panel_->onOrderEdited();
	panel_->redrawByPatternChanged(orderLenChanged_);
}

int OrderListCommonQtCommand::id() const
{
	return id_;
}

SetPatternToOrderQtCommand::SetPatternToOrderQtCommand(
		OrderListPanel* panel, const OrderPosition& pos, bool secondEntry, QUndoCommand* parent)
	: OrderListCommonQtCommand(CommandId::SetPatternToOrder, panel, false, parent),
	  pos_(pos),
	  isSecondEntry_(secondEntry)
{
}

void SetPatternToOrderQtCommand::undo()
{
	OrderListCommonQtCommand::undo();
	panel_->resetEntryCount();
}

bool SetPatternToOrderQtCommand::mergeWith(const QUndoCommand* other)
{
	if (other->id() == id() && !isSecondEntry_) {
		auto com = dynamic_cast<const SetPatternToOrderQtCommand*>(other);
		if (com->pos_ == pos_ && com->isSecondEntry_) {
			isSecondEntry_ = true;
			redo();
			return true;
		}
	}

	isSecondEntry_ = true;
	return false;
}
