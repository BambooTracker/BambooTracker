#include "remove_instrument_qt_command.hpp"
#include <utility>
#include <QListWidgetItem>
#include <QApplication>
#include <QClipboard>
#include <QRegularExpression>
#include "command_id.hpp"
#include "gui/instrument_editor/instrument_editor_fm_form.hpp"
#include "gui/instrument_editor/instrument_editor_ssg_form.hpp"
#include "gui/instrument_editor/instrument_editor_adpcm_form.hpp"

RemoveInstrumentQtCommand::RemoveInstrumentQtCommand(QListWidget *list, int num, int row,
													 std::weak_ptr<InstrumentFormManager> formMan,
													 MainWindow* mainwin, bool updateRequested, QUndoCommand *parent)
	: QUndoCommand(parent),
	  list_(list),
	  num_(num),
	  row_(row),
	  formMan_(formMan),
	  mainwin_(mainwin),
	  updateRequested_(updateRequested)
{
	source_ = formMan.lock()->getFormInstrumentSoundSource(num);
}

void RemoveInstrumentQtCommand::undo()
{
	QListWidgetItem *item;
	std::shared_ptr<QWidget> form;
	auto title = QString("%1: %2").arg(num_, 2, 16, QChar('0')).toUpper().arg(name_);
	switch (source_) {
	case SoundSource::FM:
		item = new QListWidgetItem(QIcon(":/icon/inst_fm"), title);
		form = std::make_shared<InstrumentEditorFMForm>(num_);
		break;
	case SoundSource::SSG:
		item = new QListWidgetItem(QIcon(":/icon/inst_ssg"), title);
		form = std::make_shared<InstrumentEditorSSGForm>(num_);
		break;
	case SoundSource::ADPCM:
		item = new QListWidgetItem(QIcon(":/icon/inst_adpcm"), title);
		form = std::make_shared<InstrumentEditorADPCMForm>(num_);
		break;
	default:
		return;
	}

	// KEEP CODE ORDER //
	formMan_.lock()->add(num_, std::move(form), name_, source_);

	item->setSizeHint(QSize(130, 17));
	item->setData(Qt::UserRole, num_);
	list_->insertItem(row_, item);
	//----------//

	if (updateRequested_ && source_ == SoundSource::ADPCM) {
		mainwin_->assignADPCMSamples();
	}
}

void RemoveInstrumentQtCommand::redo()
{
	auto&& item = list_->takeItem(row_);
	delete item;

	name_ = formMan_.lock()->getFormInstrumentName(num_);
	formMan_.lock()->remove(num_);

	if (QApplication::clipboard()->text().contains(
				QRegularExpression("^.+_INSTRUMENT:"+QString::number(num_),
								   QRegularExpression::DotMatchesEverythingOption))) {
		QApplication::clipboard()->clear();
	}

	if (updateRequested_ && source_ == SoundSource::ADPCM) {
		mainwin_->assignADPCMSamples();
	}
}

int RemoveInstrumentQtCommand::id() const
{
	return CommandId::RemoveInstrument;
}
