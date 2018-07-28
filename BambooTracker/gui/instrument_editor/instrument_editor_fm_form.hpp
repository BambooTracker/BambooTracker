#ifndef INSTRUMENTEDITORFMFORM_HPP
#define INSTRUMENTEDITORFMFORM_HPP

#include <QWidget>
#include <memory>
#include "bamboo_tracker.hpp"
#include "instrument.hpp"

namespace Ui {
	class InstrumentEditorFMForm;
}

class InstrumentEditorFMForm : public QWidget
{
	Q_OBJECT

public:
	InstrumentEditorFMForm(int num, QWidget *parent = 0);
	~InstrumentEditorFMForm();
	void setCore(std::shared_ptr<BambooTracker> core);
	void checkEnvelopeChange(int envNum);

signals:
	void instrumentFMEnvelopeParameterChanged(int envNum, int fromInstNum);

private slots:
	void on_envNumSpinBox_valueChanged(int arg1);

private:
	Ui::InstrumentEditorFMForm *ui;
	int instNum_;
	bool isIgnoreEvent_;

	std::shared_ptr<BambooTracker> bt_;

	void setInstrumentParameters();
	void setInstrumentEnvelopeParameters();
};

#endif // INSTRUMENTEDITORFMFORM_HPP
