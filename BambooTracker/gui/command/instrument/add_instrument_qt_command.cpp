#include "add_instrument_qt_command.hpp"
#include <utility>
#include <QApplication>
#include <QClipboard>
#include <QRegularExpression>
#include "command_id.hpp"
#include "gui/instrument_list_misc.hpp"

AddInstrumentQtCommand::AddInstrumentQtCommand(QListWidget *list, int num, QString name, SoundSource source,
											   std::weak_ptr<InstrumentFormManager> formMan, MainWindow* mainwin,
											   bool onlyUsed, bool preventFirstStore, QUndoCommand *parent)
	: QUndoCommand(parent),
	  list_(list),
	  num_(num),
	  name_(name),
	  source_(source),
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

	if (QApplication::clipboard()->text().contains(
				QRegularExpression("^.+_INSTRUMENT:"+QString::number(num_),
								   QRegularExpression::DotMatchesEverythingOption))) {
		QApplication::clipboard()->clear();
	}

	if (source_ == SoundSource::ADPCM && onlyUsed_) {
		mainwin_->assignADPCMSamples();
	}
}

void AddInstrumentQtCommand::redo()
{
	list_->insertItem(num_, createInstrumentListItem(num_, source_, name_));

	if (hasDone_ && source_ == SoundSource::ADPCM) {
		mainwin_->assignADPCMSamples();
	}
	hasDone_ = true;
}

int AddInstrumentQtCommand::id() const
{
	return CommandId::AddInstrument;
}
