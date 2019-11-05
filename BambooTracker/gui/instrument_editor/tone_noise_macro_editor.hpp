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
};

#endif // TONENOISEMACROEDITOR_HPP
