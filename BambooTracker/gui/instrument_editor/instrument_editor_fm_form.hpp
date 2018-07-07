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
	explicit InstrumentEditorFMForm(QWidget *parent = 0);
	~InstrumentEditorFMForm();

	void setInstrumentParameters(const InstrumentFM inst);

private:
	Ui::InstrumentEditorFMForm *ui;
};

#endif // INSTRUMENTEDITORFMFORM_HPP
