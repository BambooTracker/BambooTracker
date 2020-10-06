/*
 * Copyright (C) 2018 Rerrah
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

#ifndef PASTE_INSTRUMENT_QT_COMMAND_H
#define PASTE_INSTRUMENT_QT_COMMAND_H

#include <QUndoCommand>
#include <QListWidget>
#include <QString>
#include <memory>
#include <map>
#include "misc.hpp"

class PasteInstrumentQtCommand : public QUndoCommand
{
public:
	PasteInstrumentQtCommand(QListWidget *list, int oldRow, int refRow, int oldNum, int refNum,
							 std::map<int, std::unique_ptr<QWidget>>& map, SoundSource source,
							 QUndoCommand* parent = nullptr);
    void undo() Q_DECL_OVERRIDE;
    void redo() Q_DECL_OVERRIDE;
    int id() const Q_DECL_OVERRIDE;

private:
    QListWidget* list_;
	int oldRow_, refRow_;
	int oldNum_, refNum_;
	QString oldName_, refName_;
    std::map<int, std::unique_ptr<QWidget>>& map_;
	SoundSource source_;
};

#endif // PASTE_INSTRUMENT_QT_COMMAND_H
