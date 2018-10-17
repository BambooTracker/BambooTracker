#ifndef MOVE_ORDER_QT_COMMAND_HPP
#define MOVE_ORDER_QT_COMMAND_HPP

#include <QUndoCommand>
#include "gui/order_list_editor/order_list_panel.hpp"

class MoveOrderQtCommand : public QUndoCommand
{
public:
	MoveOrderQtCommand(OrderListPanel* panel, QUndoCommand* parent = nullptr);
	void redo() Q_DECL_OVERRIDE;
	void undo() Q_DECL_OVERRIDE;
	int id() const Q_DECL_OVERRIDE;

private:
	OrderListPanel* panel_;

};

#endif // MOVE_ORDER_QT_COMMAND_HPP
