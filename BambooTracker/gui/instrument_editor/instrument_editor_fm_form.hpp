#ifndef INSTRUMENTEDITORFMFORM_HPP
#define INSTRUMENTEDITORFMFORM_HPP

#include <QWidget>
#include <QKeyEvent>
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
	int getInstrumentNumber() const;
	void setCore(std::shared_ptr<BambooTracker> core);
	void checkEnvelopeChange(int envNum);

signals:
	void instrumentFMEnvelopeParameterChanged(int envNum, int fromInstNum);
	void jamKeyOnEvent(QKeyEvent* event);
	void jamKeyOffEvent(QKeyEvent* event);

protected:
	void keyPressEvent(QKeyEvent* event) override;
	void keyReleaseEvent(QKeyEvent* event) override;

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
