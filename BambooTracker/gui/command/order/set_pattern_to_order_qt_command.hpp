#ifndef SET_PATTERN_TO_ORDER_QT_COMMAND_HPP
#define SET_PATTERN_TO_ORDER_QT_COMMAND_HPP

#include <QUndoCommand>
#include "gui/order_list_editor/order_list_panel.hpp"
#include "gui/order_list_editor/order_position.hpp"

class SetPatternToOrderQtCommand : public QUndoCommand
{
public:
	SetPatternToOrderQtCommand(OrderListPanel* panel, OrderPosition pos, QUndoCommand* parent = nullptr);
	void redo() Q_DECL_OVERRIDE;
	void undo() Q_DECL_OVERRIDE;
	int id() const Q_DECL_OVERRIDE;
	bool mergeWith(const QUndoCommand* other) Q_DECL_OVERRIDE;

	OrderPosition getPos() const;

private:
	OrderListPanel* panel_;
	OrderPosition pos_;
	bool isComplete_;
};

#endif // SET_PATTERN_TO_ORDER_QT_COMMAND_HPP
