/*
 * Copyright (C) 2018-2022 Rerrah
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

#include "add_instrument_qt_command.hpp"
#include <utility>
#include "command/command_id.hpp"
#include "instrument_command_qt_utils.hpp"

AddInstrumentQtCommand::AddInstrumentQtCommand(QListWidget *list, int num, const QString& name, InstrumentType type,
											   std::weak_ptr<InstrumentEditorManager> dialogMan, MainWindow* mainWin,
											   bool onlyUsed, bool preventFirstStore, QUndoCommand *parent)
	: QUndoCommand(parent),
	  list_(list),
	  num_(num),
	  name_(name),
	  type_(type),
	  dialogMan_(dialogMan),
	  mainWin_(mainWin),
	  onlyUsed_(onlyUsed),
	  hasDone_(!preventFirstStore)
{
}

void AddInstrumentQtCommand::undo()
{
	auto&& item = list_->takeItem(num_);
	delete item;

	dialogMan_.lock()->remove(num_);

	if ((type_ == InstrumentType::ADPCM || type_ == InstrumentType::Drumkit) && onlyUsed_) {
		mainWin_->assignADPCMSamples();
	}
}

void AddInstrumentQtCommand::redo()
{
	list_->insertItem(num_, gui_command_utils::createInstrumentListItem(num_, type_, name_));

	if (hasDone_ && (type_ == InstrumentType::ADPCM || type_ == InstrumentType::Drumkit)) {
		mainWin_->assignADPCMSamples();
	}
	hasDone_ = true;
}

int AddInstrumentQtCommand::id() const
{
	return CommandId::AddInstrument;
}
