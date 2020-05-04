#include "add_instrument_qt_command.hpp"
#include <utility>
#include "command_id.hpp"
#include "gui/instrument_list_misc.hpp"

AddInstrumentQtCommand::AddInstrumentQtCommand(QListWidget *list, int num, QString name, InstrumentType type,
											   std::weak_ptr<InstrumentFormManager> formMan, MainWindow* mainwin,
											   bool onlyUsed, bool preventFirstStore, QUndoCommand *parent)
	: QUndoCommand(parent),
	  list_(list),
	  num_(num),
	  name_(name),
	  type_(type),
	  formMan_(formMan),
	  mainwin_(mainwin),
	  onlyUsed_(onlyUsed),
	  hasDone_(!preventFirstStore)
{}

void AddInstrumentQtCommand::undo()
{
	auto&& item = list_->takeItem(num_);
	delete item;

	formMan_.lock()->remove(num_);

	if ((type_ == InstrumentType::ADPCM || type_ == InstrumentType::Drumkit) && onlyUsed_) {
		mainwin_->assignADPCMSamples();
	}
}

void AddInstrumentQtCommand::redo()
{
	list_->insertItem(num_, createInstrumentListItem(num_, type_, name_));

	if (hasDone_ && (type_ == InstrumentType::ADPCM || type_ == InstrumentType::Drumkit)) {
		mainwin_->assignADPCMSamples();
	}
	hasDone_ = true;
}

int AddInstrumentQtCommand::id() const
{
	return CommandId::AddInstrument;
}
