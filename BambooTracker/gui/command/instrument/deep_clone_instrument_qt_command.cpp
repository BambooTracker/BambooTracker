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

#include "deep_clone_instrument_qt_command.hpp"
#include "instrument.hpp"
#include "command/command_id.hpp"
#include "gui/instrument_list_misc.hpp"

DeepCloneInstrumentQtCommand::DeepCloneInstrumentQtCommand(QListWidget *list, int num, InstrumentType type, QString name,
														   std::weak_ptr<InstrumentFormManager> formMan,
														   MainWindow* mainwin, bool onlyUsed, QUndoCommand *parent)
	: QUndoCommand(parent),
	  list_(list),
	  cloneNum_(num),
	  formMan_(formMan),
	  type_(type),
	  name_(name),
	  mainwin_(mainwin),
	  onlyUsed_(onlyUsed)
{
}

void DeepCloneInstrumentQtCommand::redo()
{
	list_->insertItem(cloneNum_, createInstrumentListItem(cloneNum_, type_, name_));

	if (type_ == InstrumentType::ADPCM || type_ == InstrumentType::Drumkit)
		mainwin_->assignADPCMSamples();
}

void DeepCloneInstrumentQtCommand::undo()
{
	auto&& item = list_->takeItem(cloneNum_);
	delete item;

	formMan_.lock()->remove(cloneNum_);

	if ((type_ == InstrumentType::ADPCM || type_ == InstrumentType::Drumkit) && onlyUsed_) {
		mainwin_->assignADPCMSamples();
	}
}

int DeepCloneInstrumentQtCommand::id() const
{
	return CommandId::DeepCloneInstrument;
}
