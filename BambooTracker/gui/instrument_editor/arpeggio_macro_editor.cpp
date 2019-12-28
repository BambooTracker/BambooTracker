#include "arpeggio_macro_editor.hpp"
#include <QRegularExpression>

const QString ArpeggioMacroEditor::TONE_LABS_[96] = {
	"C-0", "C#0", "D-0", "D#0", "E-0", "F-0", "F#0", "G-0", "G#0", "A-0", "A#0", "B-0",
	"C-1", "C#1", "D-1", "D#1", "E-1", "F-1", "F#1", "G-1", "G#1", "A-1", "A#1", "B-1",
	"C-2", "C#2", "D-2", "D#2", "E-2", "F-2", "F#2", "G-2", "G#2", "A-2", "A#2", "B-2",
	"C-3", "C#3", "D-3", "D#3", "E-3", "F-3", "F#3", "G-3", "G#3", "A-3", "A#3", "B-3",
	"C-4", "C#4", "D-4", "D#4", "E-4", "F-4", "F#4", "G-4", "G#4", "A-4", "A#4", "B-4",
	"C-5", "C#5", "D-5", "D#5", "E-5", "F-5", "F#5", "G-5", "G#5", "A-5", "A#5", "B-5",
	"C-6", "C#6", "D-6", "D#6", "E-6", "F-6", "F#6", "G-6", "G#6", "A-6", "A#6", "B-6",
	"C-7", "C#7", "D-7", "D#7", "E-7", "F-7", "F#7", "G-7", "G#7", "A-7", "A#7", "B-7"
};

ArpeggioMacroEditor::ArpeggioMacroEditor(QWidget* parent)
	: VisualizedInstrumentMacroEditor(parent)
{
	setMaximumDisplayedRowCount(15);
	setDefaultRow(48);
	setLabelDiaplayMode(true);
	for (int i = 0; i < 96; ++i) {
		AddRow(QString::asprintf("%+d", i - 48), false);
	}
	autoFitLabelWidth();
	setUpperRow(55);
	setMMLDisplay0As(-48);
	setSequenceType(SequenceType::Absolute);
}

ArpeggioMacroEditor::~ArpeggioMacroEditor() {}

QString ArpeggioMacroEditor::convertSequenceDataUnitToMML(Column col)
{
	if (type_ == SequenceType::Fixed)	return TONE_LABS_[col.row];
	else return VisualizedInstrumentMacroEditor::convertSequenceDataUnitToMML(col);
}

bool ArpeggioMacroEditor::interpretDataInMML(QString &text, int &cnt, std::vector<Column> &column)
{
	if (type_ == SequenceType::Fixed) {
		QRegularExpressionMatch m = QRegularExpression("^([A-G][-#])([0-7])").match(text);
		if (m.hasMatch()) {
			QString tone = m.captured(1);
			int oct = m.captured(2).toInt();
			int d = 12 * oct;
			if (tone.left(1) == "C") {
				if (tone.right(1) == "-") ;
				else d += 1;
			}
			else if (tone.left(1) == "D") {
				if (tone.right(1) == "-") d += 2;
				else d += 3;
			}
			else if (tone.left(1) == "E") d +=4;
			else if (tone.left(1) == "F") {
				if (tone.right(1) == "-") d += 5;
				else d += 6;
			}
			else if (tone.left(1) == "G") {
				if (tone.right(1) == "-") d += 7;
				else d += 8;
			}
			else if (tone.left(1) == "A") {
				if (tone.right(1) == "-") d += 9;
				else d += 10;
			}
			else d += 11;	// 'B'

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
	if (type_ == SequenceType::Fixed) {
		for (int i = 0; i < 96; ++i)
			setLabel(i, TONE_LABS_[i]);
	}
	else {
		for (int i = 0; i < 96; ++i)
			setLabel(i, QString::asprintf("%+d", i - 48));
	}
}
