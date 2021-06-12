/*
 * Copyright (C) 2019-2021 Rerrah
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

#include "arpeggio_macro_editor.hpp"
#include <QRegularExpression>
#include "note.hpp"
#include "gui/note_name_manager.hpp"

ArpeggioMacroEditor::ArpeggioMacroEditor(QWidget* parent)
	: VisualizedInstrumentMacroEditor(parent)
{
	constexpr int MAX_DISP_CNT = 15;
	setMaximumDisplayedRowCount(MAX_DISP_CNT);
	setDefaultRow(Note::DEFAULT_NOTE_NUM);
	setLabelDiaplayMode(true);
	for (int i = 0; i < Note::NOTE_NUMBER_RANGE; ++i) {
		AddRow(QString::asprintf("%+d", i - Note::DEFAULT_NOTE_NUM), false);
	}
	autoFitLabelWidth();
	setUpperRow(Note::DEFAULT_NOTE_NUM + MAX_DISP_CNT / 2);
	setMMLDisplay0As(-Note::DEFAULT_NOTE_NUM);
	setSequenceType(SequenceType::AbsoluteSequence);
}

QString ArpeggioMacroEditor::convertSequenceDataUnitToMML(Column col)
{
	if (type_ == SequenceType::FixedSequence) return NoteNameManager::getManager().getNoteString(col.row);
	else return VisualizedInstrumentMacroEditor::convertSequenceDataUnitToMML(col);
}

bool ArpeggioMacroEditor::interpretDataInMML(QString &text, int &cnt, std::vector<Column> &column)
{
	if (type_ == SequenceType::FixedSequence) {
		NoteNameManager& nnm = NoteNameManager::getManager();
		for (int i = 0; i < 12; ++i) {
			QString name = QString("%1").arg(nnm.getNoteName(i), -2, QChar('-'));
			QRegularExpressionMatch m = QRegularExpression("^" + name + "([0-7])").match(text);
			if (m.hasMatch()) {
				int oct = m.captured(1).toInt();
				int d = 12 * oct + i;
				column.push_back({ d, -1, "" });
				++cnt;
				text.remove(QRegularExpression("^" + name + "[0-7]"));
				return true;
			}
		}
		return false;
	}
	else {
		return VisualizedInstrumentMacroEditor::interpretDataInMML(text, cnt, column);
	}
}

void ArpeggioMacroEditor::onNoteNamesUpdated()
{
	updateLabels();
	printMML();
}

void ArpeggioMacroEditor::updateLabels()
{
	if (type_ == SequenceType::FixedSequence) {
		for (int i = 0; i < Note::NOTE_NUMBER_RANGE; ++i)
			setLabel(i, NoteNameManager::getManager().getNoteString(i));
	}
	else {
		for (int i = 0; i < Note::NOTE_NUMBER_RANGE; ++i)
			setLabel(i, QString::asprintf("%+d", i - Note::DEFAULT_NOTE_NUM));
	}
}
