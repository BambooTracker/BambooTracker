#ifndef INSTRUMENT_EDITOR_SSG_FORM_HPP
#define INSTRUMENT_EDITOR_SSG_FORM_HPP

#include <QWidget>
#include <QKeyEvent>
#include <memory>
#include "bamboo_tracker.hpp"
#include "instrument.hpp"
#include "configuration.hpp"
#include "jam_manager.hpp"
#include "gui/instrument_editor/visualized_instrument_macro_editor.hpp"
#include "gui/color_palette.hpp"

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
	void setConfiguration(std::weak_ptr<Configuration> config);
	void setColorPalette(std::shared_ptr<ColorPalette> palette);

signals:
	void jamKeyOnEvent(JamKey key);
	void jamKeyOffEvent(JamKey key);
	void octaveChanged(bool upFlag);
	void modified();
	/// 0: play song
	/// 1: play from start
	/// 2: play pattern
	/// 3: play from cursor
	/// -1: stop
	void playStatusChanged(int stat);

protected:
	void keyPressEvent(QKeyEvent* event) override;
	void keyReleaseEvent(QKeyEvent* event) override;

private:
	Ui::InstrumentEditorSSGForm *ui;
	int instNum_;
	bool isIgnoreEvent_;

	std::weak_ptr<BambooTracker> bt_;
	std::weak_ptr<Configuration> config_;

	SequenceType convertSequenceTypeForData(VisualizedInstrumentMacroEditor::SequenceType type);
	VisualizedInstrumentMacroEditor::SequenceType convertSequenceTypeForUI(SequenceType type);

	ReleaseType convertReleaseTypeForData(VisualizedInstrumentMacroEditor::ReleaseType type);
	VisualizedInstrumentMacroEditor::ReleaseType convertReleaseTypeForUI(ReleaseType type);

	void updateInstrumentParameters();

	//========== Wave form ==========//
signals:
	void waveFormNumberChanged();
	void waveFormParameterChanged(int wfNum, int fromInstNum);

public slots:
	void onWaveFormNumberChanged();
	void onWaveFormParameterChanged(int wfNum);

private:
	void setInstrumentWaveFormParameters();
	void setWaveFormSequenceColumn(int col);

private slots:
	void on_waveEditGroupBox_toggled(bool arg1);
	void on_waveNumSpinBox_valueChanged(int arg1);
	void on_squareMaskRawSpinBox_valueChanged(int arg1);

	//========== Tone/Noise ==========//
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
signals:
	void envelopeNumberChanged();
	void envelopeParameterChanged(int wfNum, int fromInstNum);

public slots:
	void onEnvelopeNumberChanged();
	void onEnvelopeParameterChanged(int envNum);

private:
	void setInstrumentEnvelopeParameters();
	void setEnvelopeSequenceColumn(int col);

private slots:
	void on_envEditGroupBox_toggled(bool arg1);
	void on_envNumSpinBox_valueChanged(int arg1);
	void on_hardFreqRawSpinBox_valueChanged(int arg1);

	//========== Arpeggio ==========//
signals:
	void arpeggioNumberChanged();
	void arpeggioParameterChanged(int arpNum, int fromInstNum);

public slots:
	void onArpeggioNumberChanged();
	void onArpeggioParameterChanged(int arpNum);

private:
	void setInstrumentArpeggioParameters();

private slots:
	void onArpeggioTypeChanged(int index);
	void on_arpEditGroupBox_toggled(bool arg1);
	void on_arpNumSpinBox_valueChanged(int arg1);

	//========== Pitch ==========//
signals:
	void pitchNumberChanged();
	void pitchParameterChanged(int ptNum, int fromInstNum);

public slots:
	void onPitchNumberChanged();
	void onPitchParameterChanged(int arpNum);

private:
	void setInstrumentPitchParameters();

private slots:
	void onPitchTypeChanged(int index);
	void on_ptEditGroupBox_toggled(bool arg1);
	void on_ptNumSpinBox_valueChanged(int arg1);
};

#endif // INSTRUMENT_EDITOR_SSG_FORM_HPP
