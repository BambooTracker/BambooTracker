#include "clone_instrument_qt_command.hpp"
#include "command_id.hpp"
#include "gui/instrument_editor/instrument_editor_fm_form.hpp"
#include "gui/instrument_editor/instrument_editor_ssg_form.hpp"

CloneInstrumentQtCommand::CloneInstrumentQtCommand(QListWidget *list, int num, int refNum,
												   std::weak_ptr<InstrumentFormManager> formMan, QUndoCommand *parent)
	: QUndoCommand(parent),
	  list_(list),
	  cloneNum_(num),
	  refNum_(refNum),
	  formMan_(formMan)
{
	source_ = formMan.lock()->getFormInstrumentSoundSource(refNum);
}

void CloneInstrumentQtCommand::redo()
{
	QListWidgetItem *item;
	std::unique_ptr<QWidget> form;
	QString refName = formMan_.lock()->getFormInstrumentName(refNum_);
	auto title = QString("%1: %2")
				 .arg(cloneNum_, 2, 16, QChar('0')).toUpper()
				 .arg(refName);
	switch (source_) {
	case SoundSource::FM:
		item = new QListWidgetItem(QIcon(":/icon/inst_fm"), title);
		form = std::make_unique<InstrumentEditorFMForm>(cloneNum_);
		break;
	case SoundSource::SSG:
		item = new QListWidgetItem(QIcon(":/icon/inst_ssg"), title);
		form = std::make_unique<InstrumentEditorSSGForm>(cloneNum_);
		break;
	default:
		return;
	}

	// KEEP CODE ORDER //
	formMan_.lock()->add(cloneNum_, std::move(form), refName, source_);

	item->setSizeHint(QSize(-1, 17));
	item->setData(Qt::UserRole, cloneNum_);
	list_->insertItem(cloneNum_, item);
	//----------//
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
