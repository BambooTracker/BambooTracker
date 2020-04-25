#include "deep_clone_instrument_qt_command.hpp"
#include "instrument.hpp"
#include "command_id.hpp"
#include "gui/instrument_list_misc.hpp"

DeepCloneInstrumentQtCommand::DeepCloneInstrumentQtCommand(QListWidget *list, int num, InstrumentType type, QString name,
														   std::weak_ptr<InstrumentFormManager> formMan,
														   MainWindow* mainwin, bool onlyUsed, QUndoCommand *parent)
	: QUndoCommand(parent),
	  list_(list),
	  cloneNum_(num),
	  formMan_(formMan),
	  type_(type),
	  name_(name),
	  mainwin_(mainwin),
	  onlyUsed_(onlyUsed)
{}

void DeepCloneInstrumentQtCommand::redo()
{
	list_->insertItem(cloneNum_, createInstrumentListItem(cloneNum_, type_, name_));

	if (type_ == InstrumentType::ADPCM || type_ == InstrumentType::Drumkit)
		mainwin_->assignADPCMSamples();
}

void DeepCloneInstrumentQtCommand::undo()
{
	auto&& item = list_->takeItem(cloneNum_);
	delete item;

	formMan_.lock()->remove(cloneNum_);

	if ((type_ == InstrumentType::ADPCM || type_ == InstrumentType::Drumkit) && onlyUsed_) {
		mainwin_->assignADPCMSamples();
	}
}

int DeepCloneInstrumentQtCommand::id() const
{
	return CommandId::DeepCloneInstrument;
}
