#ifndef INSTRUMENTEDITORFMFORM_HPP
#define INSTRUMENTEDITORFMFORM_HPP

#include <QWidget>
#include "instrument_fm.hpp"

namespace Ui {
	class InstrumentEditorFMForm;
}

class InstrumentEditorFMForm : public QWidget
{
	Q_OBJECT

public:
	InstrumentEditorFMForm(int num, QWidget *parent = 0);
	~InstrumentEditorFMForm();

	void setInstrumentParameters(const InstrumentFM inst);

signals:
	void parameterChanged(int instNum, FMParameter param, int value);
	void operatorEnableChanged(int instNum, int opNum, bool enable);

private:
	Ui::InstrumentEditorFMForm *ui;
	int instNum_;
	bool isValidEmit;
};

#endif // INSTRUMENTEDITORFMFORM_HPP
