#include "swap_instruments_qt_command.hpp"
#include <QListWidgetItem>
#include "instrument.hpp"
#include "command_id.hpp"
#include "gui/pattern_editor/pattern_editor.hpp"
#include "gui/instrument_list_misc.hpp"

SwapInstrumentsQtCommand::SwapInstrumentsQtCommand(QListWidget* list, int inst1Row, int inst2Row,
												   QString inst1Name, QString inst2Name,
												   std::weak_ptr<InstrumentFormManager> formMan,
												   PatternEditor* pattern,
												   QUndoCommand* parent)
	: QUndoCommand(parent),
	  list_(list),
	  ptn_(pattern),
	  formMan_(formMan)
{
	if (inst1Row < inst2Row) {
		inst1Row_ = inst1Row;
		inst2Row_ = inst2Row;
		inst1Name_ = inst1Name;
		inst2Name_ = inst2Name;
	}
	else {
		inst1Row_ = inst2Row;
		inst2Row_ = inst1Row;
		inst1Name_ = inst2Name;
		inst2Name_ = inst1Name;
	}
}

void SwapInstrumentsQtCommand::undo()
{
	swap(inst1Row_, inst2Row_, inst2Name_, inst1Name_);
}

void SwapInstrumentsQtCommand::redo()
{
	swap(inst1Row_, inst2Row_, inst1Name_, inst2Name_);
}

int SwapInstrumentsQtCommand::id() const
{
	return CommandId::SwapInstruments;
}

void SwapInstrumentsQtCommand::swap(int above, int below, QString aboveName, QString belowName)
{
	QListWidgetItem* belowItem = list_->takeItem(below);
	int belowId = belowItem->data(Qt::UserRole).toInt();
	QListWidgetItem* aboveItem = list_->takeItem(above);
	int aboveId = aboveItem->data(Qt::UserRole).toInt();

	QString newBelowName = makeInstrumentListText(belowId, aboveName);
	aboveItem->setText(newBelowName);
	aboveItem->setData(Qt::UserRole, belowId);
	QString newAboveName = makeInstrumentListText(aboveId, belowName);
	belowItem->setText(newAboveName);
	belowItem->setData(Qt::UserRole, aboveId);

	list_->insertItem(above, belowItem);
	list_->insertItem(below, aboveItem);

	formMan_.lock()->swap(aboveId, belowId);
	if (auto aboveForm = formMan_.lock()->getForm(aboveId).get())
		aboveForm->setWindowTitle(newAboveName);
	if (auto belowForm = formMan_.lock()->getForm(belowId).get())
		belowForm->setWindowTitle(newBelowName);

	ptn_->onPatternDataGlobalChanged();
}
