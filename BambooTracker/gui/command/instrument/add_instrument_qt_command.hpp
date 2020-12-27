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

#ifndef ADD_INSTRUMENT_QT_COMMAND_HPP
#define ADD_INSTRUMENT_QT_COMMAND_HPP

#include <memory>
#include <QUndoCommand>
#include <QWidget>
#include <QListWidget>
#include <QString>
#include "gui/mainwindow.hpp"
#include "gui/instrument_editor/instrument_form_manager.hpp"

enum class InstrumentType;

class AddInstrumentQtCommand : public QUndoCommand
{
public:
	AddInstrumentQtCommand(QListWidget *list, int num, QString name, InstrumentType type,
						   std::weak_ptr<InstrumentFormManager> formMan,
						   MainWindow* mainwin, bool onlyUsed, bool preventFirstStore = false,
						   QUndoCommand *parent = nullptr);
	void undo() Q_DECL_OVERRIDE;
	void redo() Q_DECL_OVERRIDE;
	int id() const Q_DECL_OVERRIDE;

private:
	QListWidget *list_;
	const int num_;
	const QString name_;
	const InstrumentType type_;
	std::weak_ptr<InstrumentFormManager> formMan_;
	MainWindow* mainwin_;
	const bool onlyUsed_;
	bool hasDone_;
};

#endif // ADD_INSTRUMENT_QT_COMMAND_HPP
