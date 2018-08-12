#include "clone_instrument_qt_command.hpp"
#include <QApplication>
#include <QRegularExpression>
#include "gui/instrument_editor/instrument_editor_fm_form.hpp"
#include "gui/instrument_editor/instrument_editor_ssg_form.hpp"

CloneInstrumentQtCommand::CloneInstrumentQtCommand(QListWidget *list, int num, int refNum,
												   std::map<int, std::unique_ptr<QWidget>>& map, QUndoCommand *parent) :
	QUndoCommand(parent), list_(list), cloneNum_(num), refNum_(refNum), map_(map)
{
	source_ = static_cast<SoundSource>(map_.at(refNum)->property("SoundSource").toInt());
}

void CloneInstrumentQtCommand::redo()
{
	QListWidgetItem *item;
	std::unique_ptr<QWidget> form;
	QString refName = map_.at(refNum_)->property("Name").toString();
	auto title = QString("%1: %2")
				 .arg(cloneNum_, 2, 16, QChar('0')).toUpper()
				 .arg(refName);
	switch (source_) {
	case SoundSource::FM:
		item = new QListWidgetItem(QIcon(), title);
		form = std::make_unique<InstrumentEditorFMForm>(cloneNum_);
		break;
	case SoundSource::SSG:
		item = new QListWidgetItem(QIcon(), title);
		form = std::make_unique<InstrumentEditorSSGForm>(cloneNum_);
		break;
	}

	// KEEP CODE ORDER //
	form->setProperty("Name", refName);
	form->setProperty("Shown", false);
	form->setProperty("SoundSource", static_cast<int>(source_));
	map_.emplace(cloneNum_, std::move(form));

	item->setData(Qt::UserRole, cloneNum_);
	list_->insertItem(cloneNum_, item);
	//----------//
}

void CloneInstrumentQtCommand::undo()
{
	auto&& item = list_->takeItem(cloneNum_);
	delete item;

	map_.at(cloneNum_)->close();
	map_.erase(cloneNum_);

	if (QApplication::clipboard()->text().contains(
				QRegularExpression("^.+_INSTRUMENT:"+QString::number(cloneNum_),
								   QRegularExpression::DotMatchesEverythingOption))) {
		QApplication::clipboard()->clear();
	}
}

int CloneInstrumentQtCommand::id() const
{
	return 4;
}
