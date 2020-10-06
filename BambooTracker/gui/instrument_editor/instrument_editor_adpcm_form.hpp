/*
 * Copyright (C) 2020 Rerrah
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

#ifndef INSTRUMENT_EDITOR_ADPCM_FORM_HPP
#define INSTRUMENT_EDITOR_ADPCM_FORM_HPP

#include <memory>
#include <QWidget>
#include <QKeyEvent>
#include "bamboo_tracker.hpp"
#include "configuration.hpp"
#include "jam_manager.hpp"
#include "gui/color_palette.hpp"
#include "gui/instrument_editor/visualized_instrument_macro_editor.hpp"

namespace Ui {
	class InstrumentEditorADPCMForm;
}

class InstrumentEditorADPCMForm : public QWidget
{
	Q_OBJECT

public:
	explicit InstrumentEditorADPCMForm(int num, QWidget *parent = nullptr);
	~InstrumentEditorADPCMForm() override;
	void setInstrumentNumber(int num);
	int getInstrumentNumber() const;
	void setCore(std::weak_ptr<BambooTracker> core);
	void setConfiguration(std::weak_ptr<Configuration> config);
	void setColorPalette(std::shared_ptr<ColorPalette> palette);

signals:
	void jamKeyOnEvent(JamKey key);
	void jamKeyOffEvent(JamKey key);
	void modified();

protected:
	void keyPressEvent(QKeyEvent* event) override;
	void keyReleaseEvent(QKeyEvent* event) override;

private:
	Ui::InstrumentEditorADPCMForm *ui;
	int instNum_;
	bool isIgnoreEvent_;

	std::weak_ptr<BambooTracker> bt_;
	std::weak_ptr<Configuration> config_;
	std::shared_ptr<ColorPalette> palette_;

	void updateInstrumentParameters();

	//========== Sample ==========//
signals:
	void sampleNumberChanged();
	void sampleParameterChanged(int sampNum, int fromInstNum);
	void sampleAssignRequested();
	void sampleMemoryChanged();

public slots:
	void onSampleNumberChanged();
	void onSampleParameterChanged(int sampNum);
	void onSampleMemoryUpdated();

private:
	void setInstrumentSampleParameters();

	//========== Envelope ==========//
signals:
	void envelopeNumberChanged();
	void envelopeParameterChanged(int envNum, int fromInstNum);

public slots:
	void onEnvelopeNumberChanged();
	void onEnvelopeParameterChanged(int envNum);

private:
	void setInstrumentEnvelopeParameters();

private slots:
	void on_envEditGroupBox_toggled(bool arg1);
	void on_envNumSpinBox_valueChanged(int arg1);

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

#endif // INSTRUMENT_EDITOR_ADPCM_FORM_HPP
