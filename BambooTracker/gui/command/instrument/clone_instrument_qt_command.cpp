#include "clone_instrument_qt_command.hpp"
#include "command_id.hpp"
#include "gui/instrument_list_misc.hpp"

CloneInstrumentQtCommand::CloneInstrumentQtCommand(QListWidget *list, int num, SoundSource src, QString name,
												   std::weak_ptr<InstrumentFormManager> formMan,
												   QUndoCommand *parent)
	: QUndoCommand(parent),
	  list_(list),
	  cloneNum_(num),
	  formMan_(formMan),
	  source_(src),
	  name_(name)
{}

void CloneInstrumentQtCommand::redo()
{
	list_->insertItem(cloneNum_, createInstrumentListItem(cloneNum_, source_, name_));
}

void CloneInstrumentQtCommand::undo()
{
	auto&& item = list_->takeItem(cloneNum_);
	delete item;

	formMan_.lock()->remove(cloneNum_);
}

int CloneInstrumentQtCommand::id() const
{
	return CommandId::CloneInstrument;
}
