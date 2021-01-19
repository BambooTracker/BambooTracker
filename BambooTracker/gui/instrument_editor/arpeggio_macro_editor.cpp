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
#include <QHash>
#include "note.hpp"

namespace
{
const QString TONE_LABS[96] = {
	"C-0", "C#0", "D-0", "D#0", "E-0", "F-0", "F#0", "G-0", "G#0", "A-0", "A#0", "B-0",
	"C-1", "C#1", "D-1", "D#1", "E-1", "F-1", "F#1", "G-1", "G#1", "A-1", "A#1", "B-1",
	"C-2", "C#2", "D-2", "D#2", "E-2", "F-2", "F#2", "G-2", "G#2", "A-2", "A#2", "B-2",
	"C-3", "C#3", "D-3", "D#3", "E-3", "F-3", "F#3", "G-3", "G#3", "A-3", "A#3", "B-3",
	"C-4", "C#4", "D-4", "D#4", "E-4", "F-4", "F#4", "G-4", "G#4", "A-4", "A#4", "B-4",
	"C-5", "C#5", "D-5", "D#5", "E-5", "F-5", "F#5", "G-5", "G#5", "A-5", "A#5", "B-5",
	"C-6", "C#6", "D-6", "D#6", "E-6", "F-6", "F#6", "G-6", "G#6", "A-6", "A#6", "B-6",
	"C-7", "C#7", "D-7", "D#7", "E-7", "F-7", "F#7", "G-7", "G#7", "A-7", "A#7", "B-7"
};
static_assert (sizeof(TONE_LABS) == sizeof(QString) * Note::NOTE_NUMBER_RANGE, "Invalid note name table size");

const QHash<QString, int> NOTE_NAME_NUMS = {
	{ "C-", 0 }, { "C#", 1 }, { "D-", 2 }, { "D#", 3 }, { "E", 4 }, { "F", 5 },
	{ "F#", 6 }, { "G-", 7 }, { "G#", 8 }, { "A-", 9 }, { "A#", 10 }, { "B", 11 }
};
}

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
	if (type_ == SequenceType::FixedSequence) return TONE_LABS[col.row];
	else return VisualizedInstrumentMacroEditor::convertSequenceDataUnitToMML(col);
}

bool ArpeggioMacroEditor::interpretDataInMML(QString &text, int &cnt, std::vector<Column> &column)
{
	if (type_ == SequenceType::FixedSequence) {
		QRegularExpressionMatch m = QRegularExpression("^([A-G][-#])([0-7])").match(text);
		if (m.hasMatch()) {
			int oct = m.captured(2).toInt();
			int d = 12 * oct + NOTE_NAME_NUMS[m.captured(1)];
			column.push_back({ d, -1, "" });
			++cnt;
			text.remove(QRegularExpression("^[A-G][-#][0-7]"));
			return true;
		}
		return false;
	}
	else {
		return VisualizedInstrumentMacroEditor::interpretDataInMML(text, cnt, column);
	}
}

void ArpeggioMacroEditor::updateLabels()
{
	if (type_ == SequenceType::FixedSequence) {
		for (int i = 0; i < Note::NOTE_NUMBER_RANGE; ++i)
			setLabel(i, TONE_LABS[i]);
	}
	else {
		for (int i = 0; i < Note::NOTE_NUMBER_RANGE; ++i)
			setLabel(i, QString::asprintf("%+d", i - Note::DEFAULT_NOTE_NUM));
	}
}
