#include "remove_instrument_qt_command.hpp"
#include <utility>
#include <QListWidgetItem>
#include "gui/instrument_editor/instrument_editor_fm_form.hpp"
#include "gui/instrument_editor/instrument_editor_psg_form.hpp"

RemoveInstrumentQtCommand::RemoveInstrumentQtCommand(QListWidget *list, int num, int row,
													 std::map<int, std::unique_ptr<QWidget>> &map, QUndoCommand *parent) :
	QUndoCommand(parent),
	list_(list),
	num_(num),
	row_(row),
	map_(map)
{
	source_ = static_cast<SoundSource>(map.at(num)->property("SoundSource").toInt());
}

void RemoveInstrumentQtCommand::undo()
{
	QListWidgetItem *item;
	std::unique_ptr<QWidget> form;
	switch (source_) {
	case SoundSource::FM:
		item = new QListWidgetItem(QIcon(), name_);
		form = std::make_unique<InstrumentEditorFMForm>(num_);
		break;
	case SoundSource::PSG:
		item = new QListWidgetItem(QIcon(), name_);
		form = std::make_unique<InstrumentEditorPSGForm>(num_);
		break;
	}
	item->setData(Qt::UserRole, num_);
	list_->insertItem(row_, item);
	form->setProperty("SoundSource", static_cast<int>(source_));
	map_.emplace(num_, std::move(form));
}

void RemoveInstrumentQtCommand::redo()
{
	auto&& item = list_->takeItem(row_);
	name_ = item->text();
	delete item;

	map_.at(num_)->close();
	map_.erase(num_);
}

int RemoveInstrumentQtCommand::id() const
{
	return 1;
}
