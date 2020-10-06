/*
 * Copyright (C) 2018-2019 Rerrah
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

#ifndef SET_PATTERN_TO_ORDER_QT_COMMAND_HPP
#define SET_PATTERN_TO_ORDER_QT_COMMAND_HPP

#include <QUndoCommand>
#include "gui/order_list_editor/order_list_panel.hpp"
#include "gui/order_list_editor/order_position.hpp"

class SetPatternToOrderQtCommand : public QUndoCommand
{
public:
	SetPatternToOrderQtCommand(OrderListPanel* panel, OrderPosition pos, bool secondEntry, QUndoCommand* parent = nullptr);
	void redo() Q_DECL_OVERRIDE;
	void undo() Q_DECL_OVERRIDE;
	int id() const Q_DECL_OVERRIDE;
	bool mergeWith(const QUndoCommand* other) Q_DECL_OVERRIDE;

	OrderPosition getPos() const;
	bool isSecondEntry() const;

private:
	OrderListPanel* panel_;
	OrderPosition pos_;
	bool isSecond_;
};

#endif // SET_PATTERN_TO_ORDER_QT_COMMAND_HPP
