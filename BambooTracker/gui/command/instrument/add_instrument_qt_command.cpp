#include "add_instrument_qt_command.hpp"
#include <utility>
#include <QListWidgetItem>
#include <QApplication>
#include <QRegularExpression>
#include "gui/instrument_editor/instrument_editor_fm_form.hpp"
#include "gui/instrument_editor/instrument_editor_ssg_form.hpp"

AddInstrumentQtCommand::AddInstrumentQtCommand(QListWidget *list, int num, QString name, SoundSource source,
											   std::weak_ptr<InstrumentFormManager> formMan, QUndoCommand *parent)
	: QUndoCommand(parent),
	  list_(list),
	  num_(num),
	  name_(name),
	  source_(source),
	  formMan_(formMan)
{}

void AddInstrumentQtCommand::undo()
{
	auto&& item = list_->takeItem(num_);
	delete item;

	formMan_.lock()->remove(num_);

	if (QApplication::clipboard()->text().contains(
				QRegularExpression("^.+_INSTRUMENT:"+QString::number(num_),
								   QRegularExpression::DotMatchesEverythingOption))) {
		QApplication::clipboard()->clear();
	}
}

void AddInstrumentQtCommand::redo()
{
	QListWidgetItem *item;
	std::unique_ptr<QWidget> form;
	auto title = QString("%1: %2").arg(num_, 2, 16, QChar('0')).toUpper().arg(name_);
	switch (source_) {
	case SoundSource::FM:
		item = new QListWidgetItem(QIcon(), title);
		form = std::make_unique<InstrumentEditorFMForm>(num_);
		break;
	case SoundSource::SSG:
		item = new QListWidgetItem(QIcon(), title);
		form = std::make_unique<InstrumentEditorSSGForm>(num_);
		break;
	}

	// KEEP CODE ORDER //
	formMan_.lock()->add(num_, std::move(form), name_, source_);

	item->setData(Qt::UserRole, num_);
	list_->insertItem(num_, item);
	//----------//
}

int AddInstrumentQtCommand::id() const
{
	return 0x10;
}
