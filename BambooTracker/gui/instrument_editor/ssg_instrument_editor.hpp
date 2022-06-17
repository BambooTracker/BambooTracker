/*
 * Copyright (C) 2018-2022 Rerrah
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#pragma once

#include <QKeyEvent>
#include <memory>
#include "instrument.hpp"
#include "instrument/instrument_property_defs.hpp"
#include "gui/instrument_editor/instrument_editor.hpp"
#include "gui/instrument_editor/visualized_instrument_macro_editor.hpp"

namespace Ui {
class SsgInstrumentEditor;
}

class SsgInstrumentEditor final : public InstrumentEditor
{
	Q_OBJECT

public:
	SsgInstrumentEditor(int num, QWidget* parent = nullptr);
	~SsgInstrumentEditor() override;

	/**
	 * @brief Return sound source of instrument related to this dialog.
	 * @return SSG.
	 */
	SoundSource getSoundSource() const override;

	/**
	 * @brief Return instrument type of instrument related to this dialog.
	 * @return SSG.
	 */
	InstrumentType getInstrumentType() const override;

	void updateByConfigurationChange() override;

protected:
	void keyPressEvent(QKeyEvent* event) override;
	void keyReleaseEvent(QKeyEvent* event) override;

private:
	Ui::SsgInstrumentEditor *ui;
	bool isIgnoreEvent_;

	// SSG specific process called in settiing core / cofiguration / color palette.
	void updateBySettingCore() override;
	void updateBySettingConfiguration() override {}
	void updateBySettingColorPalette() override;

	void updateInstrumentParameters();

	//========== Waveform ==========//
signals:
	void waveformNumberChanged();
	void waveformParameterChanged(int wfNum, int fromInstNum);

public slots:
	void onWaveformNumberChanged();
	void onWaveformParameterChanged(int wfNum);

private:
	void setInstrumentWaveformParameters();
	SSGWaveformUnit setWaveformSequenceColumn(int col, int wfRow);

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
	SSGEnvelopeUnit setEnvelopeSequenceColumn(int col, int envRow);

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
	void onArpeggioTypeChanged(int);
	void on_arpEditGroupBox_toggled(bool arg1);
	void on_arpNumSpinBox_valueChanged(int arg1);

	//========== Pitch ==========//
signals:
	void pitchNumberChanged();
	void pitchParameterChanged(int ptNum, int fromInstNum);

public slots:
	void onPitchNumberChanged();
	void onPitchParameterChanged(int ptNum);

private:
	void setInstrumentPitchParameters();

private slots:
	void onPitchTypeChanged(int);
	void on_ptEditGroupBox_toggled(bool arg1);
	void on_ptNumSpinBox_valueChanged(int arg1);
};
