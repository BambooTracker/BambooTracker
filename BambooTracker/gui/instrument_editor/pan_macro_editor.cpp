/*
 * Copyright (C) 2021 Rerrah
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

#include "pan_macro_editor.hpp"
#include <QPainter>

namespace
{
const QString MML_DISP[4] = { "n", "r", "l", "c" };
}

PanMacroEditor::PanMacroEditor(QWidget* parent)
	: VisualizedInstrumentMacroEditor(parent)
{
	setMaximumDisplayedRowCount(2);
	setDefaultRow(3);
	AddRow(tr("Right"), false);
	AddRow(tr("Left"), false);
	autoFitLabelWidth();
}

PanMacroEditor::~PanMacroEditor() = default;

void PanMacroEditor::drawField()
{
	QPainter painter(&pixmap_);
	painter.setFont(font_);

	int textOffset = fontAscend_ - fontHeight_ + fontLeading_ / 2;

	// Row label
	painter.setPen(palette_->instSeqTagColor);
	painter.drawText(1, rowHeights_[0] + textOffset, labels_[1]);
	painter.drawText(1, rowHeights_[0] + rowHeights_[1] + textOffset, labels_[0]);

	for (size_t i = 1; i < cols_.size(); i += 2) {
		painter.fillRect(labWidth_
						 + std::accumulate(colWidths_.begin(), colWidths_.begin() + static_cast<int>(i), 0),
						 0,
						 colWidths_[i], fieldHeight_,
						 palette_->instSeqOddColColor);
	}

	// Sequence
	painter.setPen(palette_->instSeqCellTextColor);
	for (size_t i = 0; i < cols_.size(); ++i) {
		int v = cols_[i].row;
		int x = labWidth_ + std::accumulate(colWidths_.begin(), colWidths_.begin() + static_cast<int>(i), 0);
		if (v & 0b10) {	// Left
			painter.fillRect(x, 0, colWidths_[i], rowHeights_[0], palette_->instSeqCellColor);
		}
		if (v & 0b01) {	// Right
			painter.fillRect(x, rowHeights_[0], colWidths_[i], rowHeights_[1], palette_->instSeqCellColor);
		}
	}

	if (hovCol_ >= 0 && hovRow_ >= 0) {
		painter.fillRect(labWidth_ + std::accumulate(colWidths_.begin(), colWidths_.begin() + hovCol_, 0),
						 std::accumulate(rowHeights_.begin(), rowHeights_.begin() + hovRow_, 0),
						 colWidths_[static_cast<size_t>(hovCol_)], rowHeights_[static_cast<size_t>(hovRow_)],
				palette_->instSeqHovColor);
	}
}

int PanMacroEditor::detectRowNumberForMouseEvent(int col, int internalRow) const
{
	int lrFlag = internalRow ? 0b01 /* Right */ : 0b10; /* Left */
	return cols_.at(static_cast<size_t>(col)).row ^ lrFlag;
}

int PanMacroEditor::maxInMML() const
{
	return 3;
}

QString PanMacroEditor::convertSequenceDataUnitToMML(Column col)
{
	return MML_DISP[col.row];
}

bool PanMacroEditor::interpretDataInMML(QString& text, int& cnt, std::vector<Column>& column)
{
	auto c = text.left(1);
	for (int i = 0; i < 4; ++i) {
		if (c == MML_DISP[i]) {
			column.push_back({ i, -1, "" });
			++cnt;
			text.remove(0, 1);
			return true;
		}
	}
	return false;
}
