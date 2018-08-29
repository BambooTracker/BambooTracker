#include "remove_instrument_qt_command.hpp"
#include <utility>
#include <QListWidgetItem>
#include <QApplication>
#include <QRegularExpression>
#include "gui/instrument_editor/instrument_editor_fm_form.hpp"
#include "gui/instrument_editor/instrument_editor_ssg_form.hpp"

RemoveInstrumentQtCommand::RemoveInstrumentQtCommand(QListWidget *list, int num, int row,
													 std::weak_ptr<InstrumentFormManager> formMan, QUndoCommand *parent)
	: QUndoCommand(parent),
	  list_(list),
	  num_(num),
	  row_(row),
	  formMan_(formMan)
{
	source_ = formMan.lock()->getFormInstrumentSoundSource(num);
}

void RemoveInstrumentQtCommand::undo()
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
	list_->insertItem(row_, item);
	//----------//
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
}

int RemoveInstrumentQtCommand::id() const
{
	return 0x11;
}
