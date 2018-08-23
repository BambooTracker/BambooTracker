#ifndef PASTE_COPIED_DATA_TO_ORDER_QT_COMMAND_HPP
#define PASTE_COPIED_DATA_TO_ORDER_QT_COMMAND_HPP

#include <QUndoCommand>
#include "gui/order_list_editor/order_list_panel.hpp"

class PasteCopiedDataToOrderQtCommand : public QUndoCommand
{
public:
	PasteCopiedDataToOrderQtCommand(OrderListPanel* panel, QUndoCommand* parent = nullptr);
	void redo() Q_DECL_OVERRIDE;
	void undo() Q_DECL_OVERRIDE;
	int id() const Q_DECL_OVERRIDE;

private:
	OrderListPanel* panel_;
};

#endif // PASTE_COPIED_DATA_TO_ORDER_QT_COMMAND_HPP
