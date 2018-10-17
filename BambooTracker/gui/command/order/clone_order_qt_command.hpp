#ifndef CLONE_ORDER_QT_COMMAND_HPP
#define CLONE_ORDER_QT_COMMAND_HPP

#include <QUndoCommand>
#include "gui/order_list_editor/order_list_panel.hpp"

class CloneOrderQtCommand : public QUndoCommand
{
public:
	CloneOrderQtCommand(OrderListPanel* panel, QUndoCommand* parent = nullptr);
	void redo() Q_DECL_OVERRIDE;
	void undo() Q_DECL_OVERRIDE;
	int id() const Q_DECL_OVERRIDE;

private:
	OrderListPanel* panel_;
};

#endif // CLONE_ORDER_QT_COMMAND_HPP
