/*
 * Copyright (C) 2020 Rerrah
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

#ifndef ORDER_LIST_COMMON_QT_COMMAND_HPP
#define ORDER_LIST_COMMON_QT_COMMAND_HPP

#include <QUndoCommand>
#include "command/command_id.hpp"
#include "gui/order_list_editor/order_position.hpp"

class OrderListPanel;

namespace gui_command_impl
{
class OrderListCommonQtCommand : public QUndoCommand
{
public:
	virtual void redo() override;
	virtual void undo() override;
	int id() const override final;

protected:
	OrderListPanel* panel_;
	OrderListCommonQtCommand(CommandId id, OrderListPanel* panel,
							 bool redrawAll, QUndoCommand* parent);

private:
	CommandId id_;
	bool orderLenChanged_;
};

template<CommandId comId, bool redrawAll>
class OrderListCommonQtCommandRedraw final : public OrderListCommonQtCommand
{
public:
	OrderListCommonQtCommandRedraw(OrderListPanel* panel, QUndoCommand* parent = nullptr)
		: OrderListCommonQtCommand(comId, panel, redrawAll, parent) {}
};

template<CommandId id>
using OrderListCommonQtCommandRedrawAll = OrderListCommonQtCommandRedraw<id, true>;

template<CommandId id>
using OrderListCommonQtCommandRedrawText = OrderListCommonQtCommandRedraw<id, false>;

class OrderListEntryQtCommand : public OrderListCommonQtCommand
{
public:
	void undo() override;
	bool mergeWith(const QUndoCommand* other) override;

protected:
	OrderListEntryQtCommand(CommandId id, OrderListPanel* panel, bool redrawAll,
							const OrderPosition& pos, bool secondEntry, QUndoCommand* parent);

private:
	const OrderPosition pos_;
	bool isSecondEntry_;
};

template<CommandId comId, bool redrawAll>
class OrderListEntryQtCommandRedraw final : public OrderListEntryQtCommand
{
public:
	OrderListEntryQtCommandRedraw(OrderListPanel* panel, const OrderPosition& pos,
									  bool secondEntry, QUndoCommand* parent = nullptr)
		: OrderListEntryQtCommand(comId, panel, redrawAll, pos, secondEntry, parent) {}
};

template<CommandId id>
using OrderListEntryQtCommandRedrawAll = OrderListEntryQtCommandRedraw<id, true>;

template<CommandId id>
using OrderListEntryQtCommandRedrawText = OrderListEntryQtCommandRedraw<id, false>;
}

#endif // ORDER_LIST_COMMON_QT_COMMAND_HPP
