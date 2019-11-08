#ifndef ARPEGGIOMACROEDITOR_HPP
#define ARPEGGIOMACROEDITOR_HPP

#include "visualized_instrument_macro_editor.hpp"
#include <QString>
#include "instrument.hpp"

class ArpeggioMacroEditor final : public VisualizedInstrumentMacroEditor
{
	Q_OBJECT

public:
	ArpeggioMacroEditor(QWidget *parent = nullptr);
	~ArpeggioMacroEditor() override;

protected:
	QString convertSequenceDataUnitToMML(Column col) override;
	bool interpretDataInMML(QString &text, int &cnt, std::vector<Column> &column) override;

private:
	static const QString TONE_LABS_[96];

	void updateLabels() override;
};

#endif // ARPEGGIOMACROEDITOR_HPP
