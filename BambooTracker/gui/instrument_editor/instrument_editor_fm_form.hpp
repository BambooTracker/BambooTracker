#ifndef INSTRUMENT_EDITOR_FM_FORM_HPP
#define INSTRUMENT_EDITOR_FM_FORM_HPP

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
	InstrumentEditorFMForm(int num, QWidget *parent = nullptr);
	~InstrumentEditorFMForm() override;
	int getInstrumentNumber() const;
	int getEnvelopeNumber() const;
	void setCore(std::weak_ptr<BambooTracker> core);

signals:
	void jamKeyOnEvent(QKeyEvent* event);
	void jamKeyOffEvent(QKeyEvent* event);
	void octaveChanged(bool upFlag);
	void modified();

protected:
	void keyPressEvent(QKeyEvent* event) override;
	void keyReleaseEvent(QKeyEvent* event) override;

private:
	Ui::InstrumentEditorFMForm *ui;
	int instNum_;
	bool isIgnoreEvent_;

	std::weak_ptr<BambooTracker> bt_;

	//========== Envelope ==========//
public:
	void updateInstrumentParameters();

signals:
	void envelopeNumberChanged(int envNum);
	void envelopeParameterChanged(int envNum, int fromInstNum);

public slots:
	void onEnvelopeParameterChanged(int envNum);
	void onEnvelopeNumberChanged(int n);

private:
	void setInstrumentEnvelopeParameters();
	void setInstrumentEnvelopeParameters(QString data);

	QString toEnvelopeString() const;

private slots:
	void on_envNumSpinBox_valueChanged(int arg1);
	void on_envelopeTab_customContextMenuRequested(const QPoint &pos);

	//========== Else ==========//
private slots:
	void on_envResetCheckBox_stateChanged(int arg1);
	void on_gateCountSpinBox_valueChanged(int arg1);
};

#endif // INSTRUMENT_EDITOR_FM_FORM_HPP
