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

#ifndef ORDER_COMMANDS_QT_HPP
#define ORDER_COMMANDS_QT_HPP

#include <QUndoCommand>
#include "command/command_id.hpp"
#include "gui/order_list_editor/order_position.hpp"

class OrderListPanel;

class OrderListCommonQtCommand : public QUndoCommand
{
public:
	virtual void redo() override;
	virtual void undo() override;
	int id() const override final;

protected:
	OrderListPanel* panel_;
	OrderListCommonQtCommand(CommandId id, OrderListPanel* panel,
							 bool orderLengthChanged, QUndoCommand* parent);

private:
	CommandId id_;
	bool orderLenChanged_;
};

class CloneOrderQtCommand final : public OrderListCommonQtCommand
{
public:
	CloneOrderQtCommand(OrderListPanel* panel, QUndoCommand* parent = nullptr)
		: OrderListCommonQtCommand(CommandId::CloneOrder, panel, true, parent) {}
};

class ClonePatternsQtCommand final : public OrderListCommonQtCommand
{
public:
	ClonePatternsQtCommand(OrderListPanel* panel, QUndoCommand* parent = nullptr)
		: OrderListCommonQtCommand(CommandId::ClonePatterns, panel, false, parent) {}
};

class DeleteOrderQtCommand final : public OrderListCommonQtCommand
{
public:
	DeleteOrderQtCommand(OrderListPanel* panel, QUndoCommand* parent = nullptr)
		: OrderListCommonQtCommand(CommandId::DeleteOrder, panel, true, parent) {}
};

class DuplicateOrderQtCommand final : public OrderListCommonQtCommand
{
public:
	DuplicateOrderQtCommand(OrderListPanel* panel, QUndoCommand* parent = nullptr)
		: OrderListCommonQtCommand(CommandId::DuplicateOrder, panel, true, parent) {}
};

class InsertOrderBelowQtCommand final : public OrderListCommonQtCommand
{
public:
	InsertOrderBelowQtCommand(OrderListPanel* panel, QUndoCommand* parent = nullptr)
		: OrderListCommonQtCommand(CommandId::InsertOrderBelow, panel, true, parent) {}
};

class MoveOrderQtCommand final : public OrderListCommonQtCommand
{
public:
	MoveOrderQtCommand(OrderListPanel* panel, QUndoCommand* parent = nullptr)
		: OrderListCommonQtCommand(CommandId::MoveOrder, panel, false, parent) {}
};

class PasteCopiedDataToOrderQtCommand final : public OrderListCommonQtCommand
{
public:
	PasteCopiedDataToOrderQtCommand(OrderListPanel* panel, QUndoCommand* parent = nullptr)
		: OrderListCommonQtCommand(CommandId::PasteCopiedDataToOrder, panel, false, parent) {}
};

class SetPatternToOrderQtCommand final : public OrderListCommonQtCommand
{
public:
	SetPatternToOrderQtCommand(OrderListPanel* panel, const OrderPosition& pos,
							   bool secondEntry, QUndoCommand* parent = nullptr);
	void undo() override;
	bool mergeWith(const QUndoCommand* other) override;

private:
	const OrderPosition pos_;
	bool isSecondEntry_;
};

#endif // ORDER_COMMANDS_QT_HPP
