#ifndef INSTRUMENT_EDITOR_PSG_FORM_HPP
#define INSTRUMENT_EDITOR_PSG_FORM_HPP

#include <QWidget>
#include "instrument.hpp"

namespace Ui {
	class InstrumentEditorPSGForm;
}

class InstrumentEditorPSGForm : public QWidget
{
	Q_OBJECT

public:
	InstrumentEditorPSGForm(int num, QWidget *parent = 0);
	~InstrumentEditorPSGForm();

	void setInstrumentParameters(const InstrumentPSG inst);

private:
	Ui::InstrumentEditorPSGForm *ui;
	int instNum_;
};

#endif // INSTRUMENT_EDITOR_PSG_FORM_HPP
