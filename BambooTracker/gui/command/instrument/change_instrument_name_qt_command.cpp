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
}

void ChangeInstrumentNameQtCommand::redo()
{
	auto item = list_->item(row_);
	item->setText(newName_);
	auto form = map_.at(num_).get();
	switch (source_) {
	case SoundSource::FM:
	{
		auto fmForm = qobject_cast<InstrumentEditorFMForm*>(form);
		fmForm->setWindowTitle(newName_);
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
	item->setText(oldName_);
	auto form = map_.at(num_).get();
	switch (source_) {
	case SoundSource::FM:
	{
		auto fmForm = qobject_cast<InstrumentEditorFMForm*>(form);
		fmForm->setWindowTitle(oldName_);
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
