#include "deep_clone_instrument_qt_command.hpp"
#include "gui/instrument_editor/instrument_editor_fm_form.hpp"
#include "gui/instrument_editor/instrument_editor_ssg_form.hpp"

DeepCloneInstrumentQtCommand::DeepCloneInstrumentQtCommand(QListWidget *list, int num, int refNum,
												   std::weak_ptr<InstrumentFormManager> formMan, QUndoCommand *parent)
	: QUndoCommand(parent),
	  list_(list),
	  cloneNum_(num),
	  refNum_(refNum),
	  formMan_(formMan)
{
	source_ = formMan.lock()->getFormInstrumentSoundSource(refNum);
}

void DeepCloneInstrumentQtCommand::redo()
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
		item = nullptr;
		break;
	}

	// KEEP CODE ORDER //
	formMan_.lock()->add(cloneNum_, std::move(form), refName, source_);

	item->setData(Qt::UserRole, cloneNum_);
	list_->insertItem(cloneNum_, item);
	//----------//
}

void DeepCloneInstrumentQtCommand::undo()
{
	auto&& item = list_->takeItem(cloneNum_);
	delete item;

	formMan_.lock()->remove(cloneNum_);
}

int DeepCloneInstrumentQtCommand::id() const
{
	return 0x14;
}
