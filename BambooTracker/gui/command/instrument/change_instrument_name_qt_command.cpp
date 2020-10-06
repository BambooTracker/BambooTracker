/*
 * Copyright (C) 2018-2020 Rerrah
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#include "change_instrument_name_qt_command.hpp"
#include "command_id.hpp"
#include "gui/instrument_list_misc.hpp"

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
{}

void ChangeInstrumentNameQtCommand::redo()
{
	auto item = list_->item(row_);
	auto title = makeInstrumentListText(num_, newName_);
	item->setText(title);

	if (auto form = formMan_.lock()->getForm(num_).get()) {
		form->setWindowTitle(title);
	}
}

void ChangeInstrumentNameQtCommand::undo()
{
	auto item = list_->item(row_);
	auto title = makeInstrumentListText(num_, oldName_);
	item->setText(title);

	if (auto form = formMan_.lock()->getForm(num_).get()) {
		form->setWindowTitle(title);
	}
}

int ChangeInstrumentNameQtCommand::id() const
{
	return CommandId::ChangeInstrumentName;
}
