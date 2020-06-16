#include "tone_noise_macro_editor.hpp"
#include <QPainter>
#include <QRegularExpression>

ToneNoiseMacroEditor::ToneNoiseMacroEditor(QWidget *parent)
	: VisualizedInstrumentMacroEditor(parent)
{
	setMaximumDisplayedRowCount(16);
	setDefaultRow(0);
	AddRow(tr("Tone"), false);
	AddRow(tr("Noise"), false);
	for (int i = 0; i < 32; ++i) {
		AddRow(QString::number(i), false);
	}
	autoFitLabelWidth();
}

ToneNoiseMacroEditor::~ToneNoiseMacroEditor() {}

void ToneNoiseMacroEditor::drawField()
{
	QPainter painter(pixmap_.get());
	painter.setFont(font_);

	int dispCnt = getDisplayedRowCount();
	int textOffset = fontAscend_ - fontHeight_ + fontLeading_ / 2;

	// Backgtound
	// Tone
	int thi = dispCnt - 1;
	int ty = std::accumulate(rowHeights_.begin(), rowHeights_.begin() + thi, 0);
	painter.fillRect(0, ty, panelWidth(), rowHeights_[static_cast<size_t>(thi)], palette_->tnToneBackColor);
	// Noise
	int nhi = dispCnt - 2;
	int ny = std::accumulate(rowHeights_.begin(), rowHeights_.begin() + nhi, 0);
	painter.fillRect(0, ny, panelWidth(), rowHeights_[static_cast<size_t>(nhi)], palette_->tnNoiseBackColor);

	// Row label
	painter.setPen(palette_->instSeqTagColor);
	if (isLabelOmitted_ && !labels_.empty()) {
		painter.drawText(1,
						 rowHeights_.front() + textOffset,
						 labels_[static_cast<size_t>(upperRow_)]);
		int c = dispCnt / 2;
		painter.drawText(1,
						 std::accumulate(rowHeights_.begin(), rowHeights_.begin() + c + 1, 0) + textOffset,
						 labels_[static_cast<size_t>(upperRow_ - c)]);
		int l = dispCnt - 3;
		painter.drawText(1,
						 std::accumulate(rowHeights_.begin(), rowHeights_.begin() + l + 1, 0) + textOffset,
						 labels_[static_cast<size_t>(upperRow_ - l)]);
	}
	else {
		int l = dispCnt - 2;
		for (int i = 0; i < l; ++i) {
			painter.drawText(1,
							 std::accumulate(rowHeights_.begin(), rowHeights_.begin() + i + 1, 0) + textOffset,
							 labels_[static_cast<size_t>(upperRow_ - i)]);
		}
	}
	// Noise
	painter.setPen(palette_->tnNoiseTextColor);
	int n = dispCnt - 2;
	painter.drawText(1,
					 std::accumulate(rowHeights_.begin(), rowHeights_.begin() + n + 1, 0) + textOffset,
					 labels_[static_cast<size_t>(1)]);
	// Tone
	painter.setPen(palette_->tnToneTextColor);
	int t = dispCnt - 1;
	painter.drawText(1,
					 std::accumulate(rowHeights_.begin(), rowHeights_.begin() + t + 1, 0) + textOffset,
					 labels_[static_cast<size_t>(0)]);

	for (size_t i = 1; i < cols_.size(); i += 2) {
		painter.fillRect(labWidth_
						 + std::accumulate(colWidths_.begin(), colWidths_.begin() + static_cast<int>(i), 0),
						 0,
						 colWidths_[i],
						 std::accumulate(rowHeights_.begin(), rowHeights_.begin() + dispCnt - 2, 0),
						 palette_->instSeqOddColColor);
	}

	// Sequence
	painter.setPen(palette_->instSeqCellTextColor);
	for (size_t i = 0; i < cols_.size(); ++i) {
		int v = cols_[i].row;
		int x = labWidth_ + std::accumulate(colWidths_.begin(), colWidths_.begin() + static_cast<int>(i), 0);
		if (!v || (32 < v && v < 65)) {	// Tone
			painter.fillRect(x, ty, colWidths_[i], rowHeights_[static_cast<size_t>(thi)], palette_->tnToneCellColor);
			painter.drawText(x + 2,
							 ty + rowHeights_[static_cast<size_t>(thi)] + textOffset,
					cols_[i].text);
		}
		if (0 < v && v < 65) {
			// Noise
			painter.fillRect(x, ny, colWidths_[i], rowHeights_[static_cast<size_t>(nhi)], palette_->tnNoiseCellColor);
			painter.drawText(x + 2,
							 ny + rowHeights_[static_cast<size_t>(nhi)] + textOffset,
					cols_[i].text);

			// Noise period
			int r = (v - 1) % 32 + 2;
			if (upperRow_ >= r && r > upperRow_ - dispCnt + 2) {
				int hi = upperRow_ - r;
				int y = std::accumulate(rowHeights_.begin(), rowHeights_.begin() + hi, 0);
				painter.fillRect(x, y, colWidths_[i], rowHeights_[static_cast<size_t>(hi)], palette_->instSeqCellColor);
				painter.drawText(x + 2,
								 y + rowHeights_[static_cast<size_t>(hi)] + textOffset,
						cols_[i].text);
			}
		}
	}

	if (hovCol_ >= 0 && hovRow_ >= 0) {
		painter.fillRect(labWidth_ + std::accumulate(colWidths_.begin(), colWidths_.begin() + hovCol_, 0),
						 std::accumulate(rowHeights_.begin(), rowHeights_.begin() + hovRow_, 0),
						 colWidths_[static_cast<size_t>(hovCol_)], rowHeights_[static_cast<size_t>(hovRow_)],
				palette_->instSeqHovColor);
	}
}

int ToneNoiseMacroEditor::detectRowNumberForMouseEvent(int col, int internalRow) const
{
	int r = upperRow_ - internalRow;
	int b = upperRow_ - getDisplayedRowCount() + 1;
	int v = cols_.at(static_cast<size_t>(col)).row;

	if (r == b) {	// Tone
		if (!v) return 65;				// Tone to None
		else if (v == 65) return 0;		// None to Tone
		else if (v < 33) return v + 32;	// Noise to Tone+Noise
		else return v - 32;				// Tone+Noise to Noise
	}
	else if (r == b + 1) {	// Noise
		if (!v) return 33;			// Tone to Tone+Noise(0)
		else if (v == 65) return 1;	// None to Noise(0)
		else if (v < 33) return 65;	// Noise to None
		else return 0;				// Tone+Noise to Tone
	}
	else {	// Noise period
		r -= 2;			// Set noise period
		if (0 < v && v < 33) return 1 + r;	// Noise
		else return 33 + r;					// Tone+Noise
	}
}

int ToneNoiseMacroEditor::maxInMML() const
{
	return 66;
}

QString ToneNoiseMacroEditor::convertSequenceDataUnitToMML(Column col)
{
	if (col.row == 0) return "t";
	else {
		int p = (col.row - 1) % 32;
		if (col.row == 65) return "u";
		else if (col.row < 33) return QString("%1n").arg(p);
		else return QString("%1tn").arg(p);
	}
}

bool ToneNoiseMacroEditor::interpretDataInMML(QString &text, int &cnt, std::vector<Column> &column)
{
	// Tone+Noise
	QRegularExpressionMatch m = QRegularExpression("^(\\d+)(nt|tn)").match(text);
	if (m.hasMatch()) {
		int p = m.captured(1).toInt();
		if (p < 0 || 31 < p) return false;
		column.push_back({ 33 + p, -1, "" });
		++cnt;
		text.remove(QRegularExpression("^\\d+(nt|tn)"));
		return true;
	}

	// Noise
	m = QRegularExpression("^(\\d+)n").match(text);
	if (m.hasMatch()) {
		int p = m.captured(1).toInt();
		if (p < 0 || 31 < p) return false;
		column.push_back({ 1 + p, -1, "" });
		++cnt;
		text.remove(QRegularExpression("^\\d+n"));
		return true;
	}

	// Noise
	m = QRegularExpression(R"(^(\d+)?t)").match(text);
	if (m.hasMatch()) {
		column.push_back({ 0, -1, "" });
		++cnt;
		text.remove(QRegularExpression("^(\\d+)?t"));
		return true;
	}

	// None
	if (text.front() == "u") {
		column.push_back({ 65, -1, "" });
		++cnt;
		text.remove(0, 1);
		return true;
	}

	return false;
}
