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

#include "remove_instrument_qt_command.hpp"
#include <utility>
#include "instrument.hpp"
#include "command_id.hpp"
#include "gui/instrument_list_misc.hpp"

RemoveInstrumentQtCommand::RemoveInstrumentQtCommand(QListWidget *list, int num, int row, QString name, InstrumentType type,
													 std::weak_ptr<InstrumentFormManager> formMan,
													 MainWindow* mainwin, bool updateRequested, QUndoCommand *parent)
	: QUndoCommand(parent),
	  list_(list),
	  num_(num),
	  name_(name),
	  row_(row),
	  type_(type),
	  formMan_(formMan),
	  mainwin_(mainwin),
	  updateRequested_(updateRequested)
{}

void RemoveInstrumentQtCommand::undo()
{
	list_->insertItem(row_, createInstrumentListItem(num_, type_, name_));

	if (updateRequested_ && (type_ == InstrumentType::ADPCM || type_ == InstrumentType::Drumkit)) {
		mainwin_->assignADPCMSamples();
	}
}

void RemoveInstrumentQtCommand::redo()
{
	auto&& item = list_->takeItem(row_);
	delete item;

	formMan_.lock()->remove(num_);

	if (updateRequested_ && (type_ == InstrumentType::ADPCM || type_ == InstrumentType::Drumkit)) {
		mainwin_->assignADPCMSamples();
	}
}

int RemoveInstrumentQtCommand::id() const
{
	return CommandId::RemoveInstrument;
}
