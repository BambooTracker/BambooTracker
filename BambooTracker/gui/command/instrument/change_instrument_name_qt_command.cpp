#include "change_instrument_name_qt_command.hpp"
#include "command_id.hpp"
#include "gui/instrument_list_misc.hpp"

ChangeInstrumentNameQtCommand::ChangeInstrumentNameQtCommand(QListWidget *list, int num, int row,
															 std::weak_ptr<InstrumentFormManager> formMan,
															 QString oldName, QString newName,
															 QUndoCommand *parent)
	: QUndoCommand(parent),
	  list_(list),
	  num_(num),
	  row_(row),
	  formMan_(formMan),
	  oldName_(oldName),
	  newName_(newName)
{}

void ChangeInstrumentNameQtCommand::redo()
{
	auto item = list_->item(row_);
	auto title = makeInstrumentListText(num_, newName_);
	item->setText(title);

	if (auto form = formMan_.lock()->getForm(num_).get()) {
		form->setWindowTitle(title);
	}
}

void ChangeInstrumentNameQtCommand::undo()
{
	auto item = list_->item(row_);
	auto title = makeInstrumentListText(num_, oldName_);
	item->setText(title);

	if (auto form = formMan_.lock()->getForm(num_).get()) {
		form->setWindowTitle(title);
	}
}

int ChangeInstrumentNameQtCommand::id() const
{
	return CommandId::ChangeInstrumentName;
}
