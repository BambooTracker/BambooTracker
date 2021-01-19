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

#ifndef ARPEGGIO_MACRO_EDITOR_HPP
#define ARPEGGIO_MACRO_EDITOR_HPP

#include "visualized_instrument_macro_editor.hpp"
#include <QString>
#include "instrument.hpp"

class ArpeggioMacroEditor final : public VisualizedInstrumentMacroEditor
{
	Q_OBJECT

public:
	ArpeggioMacroEditor(QWidget *parent = nullptr);

protected:
	QString convertSequenceDataUnitToMML(Column col) override;
	bool interpretDataInMML(QString &text, int &cnt, std::vector<Column> &column) override;

private:
	void updateLabels() override;
};

#endif // ARPEGGIO_MACRO_EDITOR_HPP
