#include "paste_instrument_qt_command.hpp"
#include <QRegularExpression>
#include <utility>
#include "gui/instrument_editor/instrument_editor_fm_form.hpp"

PasteInstrumentQtCommand::PasteInstrumentQtCommand(QListWidget *list, int oldRow, int refRow, int oldNum, int refNum,
												   std::map<int, std::unique_ptr<QWidget>>& map, SoundSource source, QUndoCommand *parent)
	: QUndoCommand(parent),
	  list_(list),
	  oldRow_(oldRow),
	  refRow_(refRow),
	  oldNum_(oldNum),
	  refNum_(refNum),
	  map_(map),
	  source_(source)
{
	oldName_ = map.at(oldNum)->property("Name").toString();
	refName_ = map.at(refNum)->property("Name").toString();
}

void PasteInstrumentQtCommand::redo()
{
	list_->item(oldRow_)->setText(QString("%1: %2").arg(oldNum_, 2, 16, QChar('0')).toUpper().arg(refName_));
	auto& form = map_.at(oldNum_);
	form->setProperty("Name", refName_);
	switch (source_) {
	case SoundSource::FM:
	{
		auto fmForm = qobject_cast<InstrumentEditorFMForm*>(map_.at(oldNum_).get());
		fmForm->updateInstrumentParameters();
		break;
	}
	case SoundSource::SSG:
		// UNDONE
		break;
	}
}

void PasteInstrumentQtCommand::undo()
{
	list_->item(oldRow_)->setText(QString("%1: %2").arg(oldNum_, 2, 16, QChar('0')).toUpper().arg(oldName_));
	auto& form = map_.at(oldNum_);
	form->setProperty("Name", oldName_);
	switch (source_) {
	case SoundSource::FM:
	{
		auto fmForm = dynamic_cast<InstrumentEditorFMForm*>(form.get());
		fmForm->updateInstrumentParameters();
		break;
	}
	case SoundSource::SSG:
		// UNDONE
		break;
	}
}

int PasteInstrumentQtCommand::id() const
{
	return 0x14;
}
