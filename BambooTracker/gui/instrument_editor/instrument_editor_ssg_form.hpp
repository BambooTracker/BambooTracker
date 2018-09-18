#ifndef INSTRUMENT_EDITOR_SSG_FORM_HPP
#define INSTRUMENT_EDITOR_SSG_FORM_HPP

#include <QWidget>
#include <QKeyEvent>
#include <memory>
#include "bamboo_tracker.hpp"
#include "instrument.hpp"
#include "gui/instrument_editor/visualized_instrument_macro_editor.hpp"

namespace Ui {
	class InstrumentEditorSSGForm;
}

class InstrumentEditorSSGForm : public QWidget
{
	Q_OBJECT

public:
	InstrumentEditorSSGForm(int num, QWidget *parent = nullptr);
	~InstrumentEditorSSGForm() override;
	int getInstrumentNumber() const;
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
	Ui::InstrumentEditorSSGForm *ui;
	int instNum_;
	bool isIgnoreEvent_;

	std::weak_ptr<BambooTracker> bt_;

	ReleaseType convertReleaseTypeForData(VisualizedInstrumentMacroEditor::ReleaseType type);
	VisualizedInstrumentMacroEditor::ReleaseType convertReleaseTypeForUI(ReleaseType type);

	void updateInstrumentParameters();

	//========== Wave form ==========//
public:
	int getWaveFormNumber() const;

signals:
	void waveFormNumberChanged();
	void waveFormParameterChanged(int wfNum, int fromInstNum);

public slots:
	void onWaveFormNumberChanged();
	void onWaveFormParameterChanged(int wfNum);

private:
	void setInstrumentWaveFormParameters();

private slots:
	void on_waveEditGroupBox_toggled(bool arg1);
	void on_waveNumSpinBox_valueChanged(int arg1);

	//========== Tone/Noise ==========//
public:
	int getToneNoiseNumber() const;

signals:
	void toneNoiseNumberChanged();
	void toneNoiseParameterChanged(int tnNum, int fromInstNum);

public slots:
	void onToneNoiseNumberChanged();
	void onToneNoiseParameterChanged(int tnNum);

private:
	void setInstrumentToneNoiseParameters();

private slots:
	void on_tnEditGroupBox_toggled(bool arg1);
	void on_tnNumSpinBox_valueChanged(int arg1);

	//========== Envelope ==========//
public:
	int getEnvelopeNumber() const;

signals:
	void envelopeNumberChanged();
	void envelopeParameterChanged(int wfNum, int fromInstNum);

public slots:
	void onEnvelopeNumberChanged();
	void onEnvelopeParameterChanged(int envNum);

private:
	void setInstrumentEnvelopeParameters();

private slots:
	void on_envEditGroupBox_toggled(bool arg1);
	void on_envNumSpinBox_valueChanged(int arg1);
	void on_hardFreqSpinBox_valueChanged(int arg1);

	//========== Else ==========//
private slots:
	void on_gateCountSpinBox_valueChanged(int arg1);
};

#endif // INSTRUMENT_EDITOR_SSG_FORM_HPP
