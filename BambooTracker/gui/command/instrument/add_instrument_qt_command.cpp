#include "add_instrument_qt_command.hpp"
#include <utility>
#include <QListWidgetItem>
#include "gui/instrument_editor/instrument_editor_fm_form.hpp"
#include "gui/instrument_editor/instrument_editor_psg_form.hpp"

AddInstrumentQtCommand::AddInstrumentQtCommand(QListWidget *list, int num, QString name, SoundSource source,
											   std::map<int, std::unique_ptr<QWidget>> &map, QUndoCommand *parent) :
	QUndoCommand(parent),
	list_(list),
	num_(num),
	name_(name),
	source_(source),
	map_(map)
{}

void AddInstrumentQtCommand::undo()
{
	auto&& item = list_->takeItem(num_);
	delete item;

	map_.at(num_)->close();
	map_.erase(num_);
}

void AddInstrumentQtCommand::redo()
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
	list_->insertItem(num_, item);
	form->setProperty("SoundSource", static_cast<int>(source_));
	map_.emplace(num_, std::move(form));
}

int AddInstrumentQtCommand::id() const
{
	return 0;
}
