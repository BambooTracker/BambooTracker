#ifndef INSTRUMENTEDITORFMFORM_HPP
#define INSTRUMENTEDITORFMFORM_HPP

#include <QWidget>
#include <QKeyEvent>
#include <QClipboard>
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
	void setCore(std::weak_ptr<BambooTracker> core);
	void checkEnvelopeChange(int envNum);
	void updateInstrumentParameters();

signals:
	void instrumentFMEnvelopeParameterChanged(int envNum, int fromInstNum);
	void jamKeyOnEvent(QKeyEvent* event);
	void jamKeyOffEvent(QKeyEvent* event);

protected:
	void keyPressEvent(QKeyEvent* event) override;
	void keyReleaseEvent(QKeyEvent* event) override;

private slots:
	void on_envNumSpinBox_valueChanged(int arg1);
	void on_envelopeTab_customContextMenuRequested(const QPoint &pos);

private:
	Ui::InstrumentEditorFMForm *ui;
	int instNum_;
	bool isIgnoreEvent_;

	std::weak_ptr<BambooTracker> bt_;

	void setInstrumentEnvelopeParameters();
	void setInstrumentEnvelopeParameters(QString data);

	QString toEnvelopeString() const;
};

#endif // INSTRUMENTEDITORFMFORM_HPP
