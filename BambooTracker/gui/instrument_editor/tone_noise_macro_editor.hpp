/*
 * Copyright (C) 2019 Rerrah
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

#ifndef TONENOISEMACROEDITOR_HPP
#define TONENOISEMACROEDITOR_HPP

#include "visualized_instrument_macro_editor.hpp"

class ToneNoiseMacroEditor final : public VisualizedInstrumentMacroEditor
{
	Q_OBJECT

	// Only change drawing process.
	// The values of ``cols_`` is left as raw values.
	// For example, the value of "Tone+Noise 2" is 35.

public:
	explicit ToneNoiseMacroEditor(QWidget *parent = nullptr);
	~ToneNoiseMacroEditor() override;

protected:
	void drawField() override;

	int detectRowNumberForMouseEvent(int col, int internalRow) const override;
	int maxInMML() const override;
	QString convertSequenceDataUnitToMML(Column col) override;
	bool interpretDataInMML(QString &text, int &cnt, std::vector<Column> &column) override;
};

#endif // TONENOISEMACROEDITOR_HPP
