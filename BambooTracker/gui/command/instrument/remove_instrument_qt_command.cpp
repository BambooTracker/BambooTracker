#include "remove_instrument_qt_command.hpp"

RemoveInstrumentQtCommand::RemoveInstrumentQtCommand(QListWidget *list, int num, int row, QUndoCommand *parent) :
	QUndoCommand(parent),
	list_(list),
	num_(num),
	row_(row)
{}

void RemoveInstrumentQtCommand::undo()
{
	auto item = new QListWidgetItem(name_);
	item->setData(Qt::UserRole, num_);
	list_->insertItem(row_, item);
}

void RemoveInstrumentQtCommand::redo()
{
	auto&& item = list_->takeItem(row_);
	name_ = item->text();
	delete item;
}
