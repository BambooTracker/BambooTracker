#include "remove_instrument_qt_command.hpp"
#include <utility>
#include <QApplication>
#include <QClipboard>
#include <QRegularExpression>
#include "instrument.hpp"
#include "command_id.hpp"
#include "gui/instrument_list_misc.hpp"

RemoveInstrumentQtCommand::RemoveInstrumentQtCommand(QListWidget *list, int num, int row, QString name, InstrumentType type,
													 std::weak_ptr<InstrumentFormManager> formMan,
													 MainWindow* mainwin, bool updateRequested, QUndoCommand *parent)
	: QUndoCommand(parent),
	  list_(list),
	  num_(num),
	  name_(name),
	  row_(row),
	  type_(type),
	  formMan_(formMan),
	  mainwin_(mainwin),
	  updateRequested_(updateRequested)
{}

void RemoveInstrumentQtCommand::undo()
{
	list_->insertItem(row_, createInstrumentListItem(num_, type_, name_));

	if (updateRequested_ && (type_ == InstrumentType::ADPCM || type_ == InstrumentType::Drumkit)) {
		mainwin_->assignADPCMSamples();
	}
}

void RemoveInstrumentQtCommand::redo()
{
	auto&& item = list_->takeItem(row_);
	delete item;

	formMan_.lock()->remove(num_);

	if (QApplication::clipboard()->text().contains(
				QRegularExpression("^.+_INSTRUMENT:"+QString::number(num_),
								   QRegularExpression::DotMatchesEverythingOption))) {
		QApplication::clipboard()->clear();
	}

	if (updateRequested_ && (type_ == InstrumentType::ADPCM || type_ == InstrumentType::Drumkit)) {
		mainwin_->assignADPCMSamples();
	}
}

int RemoveInstrumentQtCommand::id() const
{
	return CommandId::RemoveInstrument;
}
