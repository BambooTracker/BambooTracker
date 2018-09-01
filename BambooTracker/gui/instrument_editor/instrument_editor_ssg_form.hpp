#ifndef INSTRUMENT_EDITOR_SSG_FORM_HPP
#define INSTRUMENT_EDITOR_SSG_FORM_HPP

#include <QWidget>
#include "instrument.hpp"

namespace Ui {
	class InstrumentEditorSSGForm;
}

class InstrumentEditorSSGForm : public QWidget
{
	Q_OBJECT

public:
	InstrumentEditorSSGForm(int num, QWidget *parent = nullptr);
	~InstrumentEditorSSGForm();

private:
	Ui::InstrumentEditorSSGForm *ui;
	int instNum_;

	//--- Else
private slots:
	void on_gateCountSpinBox_valueChanged(int arg1);
};

#endif // INSTRUMENT_EDITOR_SSG_FORM_HPP
