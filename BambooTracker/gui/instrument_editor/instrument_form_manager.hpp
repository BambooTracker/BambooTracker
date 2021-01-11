/*
 * Copyright (C) 2018-2021 Rerrah
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

#ifndef INSTRUMENT_FORM_MANAGER_HPP
#define INSTRUMENT_FORM_MANAGER_HPP

#include <QObject>
#include <QWidget>
#include <memory>
#include <unordered_map>
#include "instrument/envelope_fm.hpp"
#include "opna_defs.hpp"

enum class InstrumentType;

class InstrumentFormManager : public QObject
{
	Q_OBJECT

public:
	void updateByConfiguration();

	const std::shared_ptr<QWidget> getForm(int n) const;
	void remove(int n);
	void add(int n, std::shared_ptr<QWidget> form, SoundSource src, InstrumentType type);
	void swap(int a, int b);

	void showForm(int n);
	void closeAll();
	void clearAll();

	SoundSource getFormInstrumentSoundSource(int n) const;
	InstrumentType getFormInstrumentType(int n) const;

	int checkActivatedFormNumber() const;

public slots:
	void onInstrumentFMEnvelopeParameterChanged(int envNum, int fromInstNum);
	void onInstrumentFMEnvelopeNumberChanged();
	void onInstrumentFMLFOParameterChanged(int lfoNum, int fromInstNum);
	void onInstrumentFMLFONumberChanged();
	void onInstrumentFMOperatorSequenceParameterChanged(FMEnvelopeParameter param, int opSeqNum, int fromInstNum);
	void onInstrumentFMOperatorSequenceNumberChanged();
	void onInstrumentFMArpeggioParameterChanged(int arpNum, int fromInstNum);
	void onInstrumentFMArpeggioNumberChanged();
	void onInstrumentFMPitchParameterChanged(int ptNum, int fromInstNum);
	void onInstrumentFMPitchNumberChanged();

	void onInstrumentSSGWaveformParameterChanged(int wfNum, int fromInstNum);
	void onInstrumentSSGWaveformNumberChanged();
	void onInstrumentSSGToneNoiseParameterChanged(int tnNum, int fromInstNum);
	void onInstrumentSSGToneNoiseNumberChanged();
	void onInstrumentSSGEnvelopeParameterChanged(int envNum, int fromInstNum);
	void onInstrumentSSGEnvelopeNumberChanged();
	void onInstrumentSSGArpeggioParameterChanged(int arpNum, int fromInstNum);
	void onInstrumentSSGArpeggioNumberChanged();
	void onInstrumentSSGPitchParameterChanged(int ptNum, int fromInstNum);
	void onInstrumentSSGPitchNumberChanged();

	void onInstrumentADPCMSampleParameterChanged(int sampNum, int fromInstNum);
	void onInstrumentADPCMSampleNumberChanged();
	void onInstrumentADPCMSampleMemoryUpdated();
	void onInstrumentADPCMEnvelopeParameterChanged(int envNum, int fromInstNum);
	void onInstrumentADPCMEnvelopeNumberChanged();
	void onInstrumentADPCMArpeggioParameterChanged(int arpNum, int fromInstNum);
	void onInstrumentADPCMArpeggioNumberChanged();
	void onInstrumentADPCMPitchParameterChanged(int ptNum, int fromInstNum);
	void onInstrumentADPCMPitchNumberChanged();

private:
	std::unordered_map<int, std::shared_ptr<QWidget>> map_;
};


#endif // INSTRUMENT_FORM_MANAGER_HPP
