/*
 * Copyright (C) 2020-2022 Rerrah
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

#ifndef SWAP_INSTRUMENTS_QT_COMMAND_HPP
#define SWAP_INSTRUMENTS_QT_COMMAND_HPP

#include <QUndoCommand>
#include <memory>
#include <QListWidget>
#include <QString>
#include "gui/instrument_editor/instrument_editor_manager.hpp"

class PatternEditor;

class SwapInstrumentsQtCommand final : public QUndoCommand
{
public:
	SwapInstrumentsQtCommand(QListWidget *list, int inst1Row, int inst2Row,
							 const QString& inst1Name, const QString& inst2Name,
							 std::weak_ptr<InstrumentEditorManager> dialogMan,
							 PatternEditor* pattern, QUndoCommand* parent = nullptr);
	void undo() override;
	void redo() override;
	int id() const override;

private:
	QListWidget* list_;
	PatternEditor* ptn_;
	int inst1Row_, inst2Row_;
	std::weak_ptr<InstrumentEditorManager> dialogMan_;
	QString inst1Name_, inst2Name_;

	void swap(int above, int below, QString aboveName, QString belowName);
};

#endif // SWAP_INSTRUMENTS_QT_COMMAND_HPP
