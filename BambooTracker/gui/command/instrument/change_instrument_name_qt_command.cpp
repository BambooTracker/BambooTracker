#include "change_instrument_name_qt_command.hpp"
#include "gui/instrument_editor/instrument_editor_fm_form.hpp"
#include "gui/instrument_editor/instrument_editor_psg_form.hpp"
#include "misc.hpp"

ChangeInstrumentNameQtCommand::ChangeInstrumentNameQtCommand(QListWidget *list, int num, int row,
															 std::map<int, std::unique_ptr<QWidget> > &map,
															 QString oldName, QString newName,
															 QUndoCommand *parent) :
	QUndoCommand(parent), list_(list), num_(num), row_(row), map_(map), oldName_(oldName), newName_(newName)
{
    source_ = static_cast<SoundSource>(map.at(num)->property("SoundSource").toInt());
}

void ChangeInstrumentNameQtCommand::redo()
{
	auto item = list_->item(row_);
    auto title = QString("%1: %2").arg(num_, 2, 16, QChar('0')).toUpper().arg(newName_);
    item->setText(title);
	auto form = map_.at(num_).get();
    form->setProperty("Name", newName_);
    switch (source_) {
	case SoundSource::FM:
	{
		auto fmForm = qobject_cast<InstrumentEditorFMForm*>(form);
        fmForm->setWindowTitle(title);
	}
		break;
	case SoundSource::PSG:
	{
		// UNDONE: PSG form name change
	}
		break;
	}
}

void ChangeInstrumentNameQtCommand::undo()
{
	auto item = list_->item(row_);
    auto title = QString("%1: %2").arg(num_, 2, 16, QChar('0')).toUpper().arg(oldName_);
    item->setText(title);
	auto form = map_.at(num_).get();
    form->setProperty("Name", oldName_);
	switch (source_) {
	case SoundSource::FM:
	{
		auto fmForm = qobject_cast<InstrumentEditorFMForm*>(form);
        fmForm->setWindowTitle(title);
	}
		break;
	case SoundSource::PSG:
	{
		// UNDONE: PSG form name change
	}
		break;
	}
}

int ChangeInstrumentNameQtCommand::id() const
{
	return 2;
}
