#include "change_instrument_name_qt_command.hpp"
#include "gui/instrument_editor/instrument_editor_fm_form.hpp"
#include "gui/instrument_editor/instrument_editor_ssg_form.hpp"
#include "misc.hpp"

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
{
	source_ = static_cast<SoundSource>(formMan.lock()->getForm(num)->property("SoundSource").toInt());
}

void ChangeInstrumentNameQtCommand::redo()
{
	auto item = list_->item(row_);
    auto title = QString("%1: %2").arg(num_, 2, 16, QChar('0')).toUpper().arg(newName_);
    item->setText(title);
	formMan_.lock()->setFormInstrumentName(num_, newName_);
	auto form = formMan_.lock()->getForm(num_).get();
    switch (source_) {
	case SoundSource::FM:
	{
		auto fmForm = qobject_cast<InstrumentEditorFMForm*>(form);
        fmForm->setWindowTitle(title);
	}
		break;
	case SoundSource::SSG:
	{
		// UNDONE: SSG form name change
	}
		break;
	}
}

void ChangeInstrumentNameQtCommand::undo()
{
	auto item = list_->item(row_);
    auto title = QString("%1: %2").arg(num_, 2, 16, QChar('0')).toUpper().arg(oldName_);
    item->setText(title);
	formMan_.lock()->setFormInstrumentName(num_, oldName_);
	auto form = formMan_.lock()->getForm(num_).get();
	switch (source_) {
	case SoundSource::FM:
	{
		auto fmForm = qobject_cast<InstrumentEditorFMForm*>(form);
        fmForm->setWindowTitle(title);
	}
		break;
	case SoundSource::SSG:
	{
		// UNDONE: SSG form name change
	}
		break;
	}
}

int ChangeInstrumentNameQtCommand::id() const
{
	return 0x12;
}
