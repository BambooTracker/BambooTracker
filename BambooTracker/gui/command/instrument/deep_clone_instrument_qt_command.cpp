#include "deep_clone_instrument_qt_command.hpp"
#include "command_id.hpp"
#include "gui/instrument_list_misc.hpp"

DeepCloneInstrumentQtCommand::DeepCloneInstrumentQtCommand(QListWidget *list, int num, SoundSource src, QString name,
														   std::weak_ptr<InstrumentFormManager> formMan,
														   MainWindow* mainwin, bool onlyUsed, QUndoCommand *parent)
	: QUndoCommand(parent),
	  list_(list),
	  cloneNum_(num),
	  formMan_(formMan),
	  source_(src),
	  name_(name),
	  mainwin_(mainwin),
	  onlyUsed_(onlyUsed)
{}

void DeepCloneInstrumentQtCommand::redo()
{
	list_->insertItem(cloneNum_, createInstrumentListItem(cloneNum_, source_, name_));

	if (source_ == SoundSource::ADPCM) mainwin_->assignADPCMSamples();
}

void DeepCloneInstrumentQtCommand::undo()
{
	auto&& item = list_->takeItem(cloneNum_);
	delete item;

	formMan_.lock()->remove(cloneNum_);

	if (source_ == SoundSource::ADPCM && onlyUsed_) {
		mainwin_->assignADPCMSamples();
	}
}

int DeepCloneInstrumentQtCommand::id() const
{
	return CommandId::DeepCloneInstrument;
}
