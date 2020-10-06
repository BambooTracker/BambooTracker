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

#ifndef CLONE_INSTRUMENT_QT_COMMAND_H
#define CLONE_INSTRUMENT_QT_COMMAND_H

#include <QUndoCommand>
#include <QListWidget>
#include <memory>
#include "gui/instrument_editor/instrument_form_manager.hpp"

enum class InstrumentType;

class CloneInstrumentQtCommand : public QUndoCommand
{
public:
	CloneInstrumentQtCommand(QListWidget *list, int num, InstrumentType type, QString name,
							 std::weak_ptr<InstrumentFormManager> formMan,
							 QUndoCommand* parent = nullptr);
	void undo() Q_DECL_OVERRIDE;
	void redo() Q_DECL_OVERRIDE;
	int id() const Q_DECL_OVERRIDE;

private:
	QListWidget* list_;
	int cloneNum_;
	std::weak_ptr<InstrumentFormManager> formMan_;
	InstrumentType type_;
	QString name_;
};

#endif // CLONE_INSTRUMENT_QT_COMMAND_H
