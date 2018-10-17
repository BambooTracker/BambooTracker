#ifndef DUPLICATE_ORDER_QT_COMMAND_HPP
#define DUPLICATE_ORDER_QT_COMMAND_HPP

#include <QUndoCommand>
#include "gui/order_list_editor/order_list_panel.hpp"

class DuplicateOrderQtCommand : public QUndoCommand
{
public:
	DuplicateOrderQtCommand(OrderListPanel* panel, QUndoCommand* parent = nullptr);
	void redo() Q_DECL_OVERRIDE;
	void undo() Q_DECL_OVERRIDE;
	int id() const Q_DECL_OVERRIDE;

private:
	OrderListPanel* panel_;
};

#endif // DUPLICATE_ORDER_QT_COMMAND_HPP
