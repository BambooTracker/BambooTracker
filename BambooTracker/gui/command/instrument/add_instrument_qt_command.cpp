#include "add_instrument_qt_command.hpp"
#include <QListWidgetItem>

AddInstrumentQtCommand::AddInstrumentQtCommand(QListWidget *list, int num, QString name, QUndoCommand *parent) :
	QUndoCommand(parent),
	list_(list),
	num_(num),
	name_(name)
{
}

void AddInstrumentQtCommand::undo()
{
	auto&& item = list_->takeItem(num_);
	delete item;
}

void AddInstrumentQtCommand::redo()
{
	auto item = new QListWidgetItem(name_);
	item->setData(Qt::UserRole, num_);
	list_->insertItem(num_, item);
}
