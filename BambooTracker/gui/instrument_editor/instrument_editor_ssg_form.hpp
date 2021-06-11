/*
 * Copyright (C) 2018-2020 Rerrah
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

#ifndef INSTRUMENT_EDITOR_SSG_FORM_HPP
#define INSTRUMENT_EDITOR_SSG_FORM_HPP

#include <QWidget>
#include <QKeyEvent>
#include <memory>
#include "bamboo_tracker.hpp"
#include "instrument.hpp"
#include "configuration.hpp"
#include "jamming.hpp"
#include "instrument/instrument_property_defs.hpp"
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
	void setInstrumentNumber(int num);
	int getInstrumentNumber() const;
	void setCore(std::weak_ptr<BambooTracker> core);
	void setConfiguration(std::weak_ptr<Configuration> config);
	void updateConfigurationForDisplay();
	void setColorPalette(std::shared_ptr<ColorPalette> palette);

signals:
	void jamKeyOnEvent(JamKey key);
	void jamKeyOffEvent(JamKey key);
	void modified();

protected:
	void keyPressEvent(QKeyEvent* event) override;
	void keyReleaseEvent(QKeyEvent* event) override;

private:
	Ui::InstrumentEditorSSGForm *ui;
	int instNum_;
	bool isIgnoreEvent_;

	std::weak_ptr<BambooTracker> bt_;
	std::weak_ptr<Configuration> config_;

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
