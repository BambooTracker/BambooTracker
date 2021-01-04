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

#pragma once

#include <string>
#include <memory>
#include <unordered_map>
#include <vector>
#include "instruments_manager.hpp"
#include "envelope_fm.hpp"
#include "lfo_fm.hpp"
#include "command_sequence.hpp"
#include "instrument_property_defs.hpp"
#include "enum_hash.hpp"
#include "misc.hpp"

class InstrumentsManager;

enum class InstrumentType : int { FM, SSG, ADPCM, Drumkit };

class AbstractInstrument
{
public:
	virtual ~AbstractInstrument() = default;

	int getNumber() const;
	void setNumber(int n);
	virtual SoundSource getSoundSource() const = 0;
	virtual InstrumentType getType() const = 0;
	std::string getName() const;
	void setName(std::string name);
	bool isRegisteredWithManager() const;
	virtual AbstractInstrument* clone() = 0;

protected:
	InstrumentsManager* owner_;
	int number_;
	std::string name_;	// UTF-8
	AbstractInstrument(int number, std::string name, InstrumentsManager* owner);
};


class InstrumentFM : public AbstractInstrument
{
public:
	InstrumentFM(int number, std::string name, InstrumentsManager* owner);
	SoundSource getSoundSource() const override;
	InstrumentType getType() const override;
	AbstractInstrument* clone() override;

	void setEnvelopeNumber(int n);
	int getEnvelopeNumber() const;
	int getEnvelopeParameter(FMEnvelopeParameter param) const;
	bool getOperatorEnabled(int n) const;

	void setLFOEnabled(bool enabled);
	bool getLFOEnabled() const;
	void setLFONumber(int n);
	int getLFONumber() const;
	int getLFOParameter(FMLFOParameter param) const;

	void setOperatorSequenceEnabled(FMEnvelopeParameter param, bool enabled);
	bool getOperatorSequenceEnabled(FMEnvelopeParameter param) const;
	void setOperatorSequenceNumber(FMEnvelopeParameter param, int n);
	int getOperatorSequenceNumber(FMEnvelopeParameter param) const;
	std::vector<FMOperatorSequenceUnit> getOperatorSequenceSequence(FMEnvelopeParameter param) const;
	InstrumentSequenceLoopRoot getOperatorSequenceLoopRoot(FMEnvelopeParameter param) const;
	InstrumentSequenceRelease getOperatorSequenceRelease(FMEnvelopeParameter param) const;
	FMOperatorSequenceIter getOperatorSequenceSequenceIterator(FMEnvelopeParameter param) const;

	void setArpeggioEnabled(FMOperatorType op, bool enabled);
	bool getArpeggioEnabled(FMOperatorType op) const;
	void setArpeggioNumber(FMOperatorType op, int n);
	int getArpeggioNumber(FMOperatorType op) const;
	SequenceType getArpeggioType(FMOperatorType op) const;
	std::vector<ArpeggioUnit> getArpeggioSequence(FMOperatorType op) const;
	InstrumentSequenceLoopRoot getArpeggioLoopRoot(FMOperatorType op) const;
	InstrumentSequenceRelease getArpeggioRelease(FMOperatorType op) const;
	ArpeggioIter getArpeggioSequenceIterator(FMOperatorType op) const;

	void setPitchEnabled(FMOperatorType op, bool enabled);
	bool getPitchEnabled(FMOperatorType op) const;
	void setPitchNumber(FMOperatorType op, int n);
	int getPitchNumber(FMOperatorType op) const;
	SequenceType getPitchType(FMOperatorType op) const;
	std::vector<PitchUnit> getPitchSequence(FMOperatorType op) const;
	InstrumentSequenceLoopRoot getPitchLoopRoot(FMOperatorType op) const;
	InstrumentSequenceRelease getPitchRelease(FMOperatorType op) const;
	PitchIter getPitchSequenceIterator(FMOperatorType op) const;

	void setEnvelopeResetEnabled(FMOperatorType op, bool enabled);
	bool getEnvelopeResetEnabled(FMOperatorType op) const;

private:
	int envNum_;
	bool lfoEnabled_;
	int lfoNum_;
	std::unordered_map<FMEnvelopeParameter, bool> opSeqEnabled_;
	std::unordered_map<FMEnvelopeParameter, int> opSeqNum_;
	std::unordered_map<FMOperatorType, bool> arpEnabled_;
	std::unordered_map<FMOperatorType, int> arpNum_;
	std::unordered_map<FMOperatorType, bool> ptEnabled_;
	std::unordered_map<FMOperatorType, int> ptNum_;

	std::unordered_map<FMOperatorType, bool> envResetEnabled_;
};


class InstrumentSSG : public AbstractInstrument
{
public:
	InstrumentSSG(int number, std::string name, InstrumentsManager* owner);
	SoundSource getSoundSource() const override;
	InstrumentType getType() const override;
	AbstractInstrument* clone() override;

	void setWaveformEnabled(bool enabled);
	bool getWaveformEnabled() const;
	void setWaveformNumber(int n);
	int getWaveformNumber() const;
	std::vector<SSGWaveformUnit> getWaveformSequence() const;
	InstrumentSequenceLoopRoot getWaveformLoopRoot() const;
	InstrumentSequenceRelease getWaveformRelease() const;
	SSGWaveformIter getWaveformSequenceIterator() const;

	void setToneNoiseEnabled(bool enabled);
	bool getToneNoiseEnabled() const;
	void setToneNoiseNumber(int n);
	int getToneNoiseNumber() const;
	std::vector<SSGToneNoiseUnit> getToneNoiseSequence() const;
	InstrumentSequenceLoopRoot getToneNoiseLoopRoot() const;
	InstrumentSequenceRelease getToneNoiseRelease() const;
	SSGToneNoiseIter getToneNoiseSequenceIterator() const;

	void setEnvelopeEnabled(bool enabled);
	bool getEnvelopeEnabled() const;
	void setEnvelopeNumber(int n);
	int getEnvelopeNumber() const;
	std::vector<CommandSequenceUnit> getEnvelopeSequence() const;
	std::vector<Loop> getEnvelopeLoops() const;
	Release getEnvelopeRelease() const;
	std::unique_ptr<CommandSequence::Iterator> getEnvelopeSequenceIterator() const;

	void setArpeggioEnabled(bool enabled);
	bool getArpeggioEnabled() const;
	void setArpeggioNumber(int n);
	int getArpeggioNumber() const;
	SequenceType getArpeggioType() const;
	std::vector<ArpeggioUnit> getArpeggioSequence() const;
	InstrumentSequenceLoopRoot getArpeggioLoopRoot() const;
	InstrumentSequenceRelease getArpeggioRelease() const;
	ArpeggioIter getArpeggioSequenceIterator() const;

	void setPitchEnabled(bool enabled);
	bool getPitchEnabled() const;
	void setPitchNumber(int n);
	int getPitchNumber() const;
	SequenceType getPitchType() const;
	std::vector<PitchUnit> getPitchSequence() const;
	InstrumentSequenceLoopRoot getPitchLoopRoot() const;
	InstrumentSequenceRelease getPitchRelease() const;
	PitchIter getPitchSequenceIterator() const;

private:
	bool wfEnabled_;
	int wfNum_;
	bool tnEnabled_;
	int tnNum_;
	bool envEnabled_;
	int envNum_;
	bool arpEnabled_;
	int arpNum_;
	bool ptEnabled_;
	int ptNum_;
};


class InstrumentADPCM : public AbstractInstrument
{
public:
	InstrumentADPCM(int number, std::string name, InstrumentsManager* owner);
	SoundSource getSoundSource() const override;
	InstrumentType getType() const override;
	AbstractInstrument* clone() override;

	void setSampleNumber(int n);
	int getSampleNumber() const;
	int getSampleRootKeyNumber() const;
	int getSampleRootDeltaN() const;
	bool isSampleRepeatable() const;
	std::vector<uint8_t> getRawSample() const;
	size_t getSampleStartAddress() const;
	size_t getSampleStopAddress() const;

	void setEnvelopeEnabled(bool enabled);
	bool getEnvelopeEnabled() const;
	void setEnvelopeNumber(int n);
	int getEnvelopeNumber() const;
	std::vector<ADPCMEnvelopeUnit> getEnvelopeSequence() const;
	InstrumentSequenceLoopRoot getEnvelopeLoopRoot() const;
	InstrumentSequenceRelease getEnvelopeRelease() const;
	ADPCMEnvelopeIter getEnvelopeSequenceIterator() const;

	void setArpeggioEnabled(bool enabled);
	bool getArpeggioEnabled() const;
	void setArpeggioNumber(int n);
	int getArpeggioNumber() const;
	SequenceType getArpeggioType() const;
	std::vector<ArpeggioUnit> getArpeggioSequence() const;
	InstrumentSequenceLoopRoot getArpeggioLoopRoot() const;
	InstrumentSequenceRelease getArpeggioRelease() const;
	ArpeggioIter getArpeggioSequenceIterator() const;

	void setPitchEnabled(bool enabled);
	bool getPitchEnabled() const;
	void setPitchNumber(int n);
	int getPitchNumber() const;
	SequenceType getPitchType() const;
	std::vector<PitchUnit> getPitchSequence() const;
	InstrumentSequenceLoopRoot getPitchLoopRoot() const;
	InstrumentSequenceRelease getPitchRelease() const;
	PitchIter getPitchSequenceIterator() const;

private:
	int sampNum_;
	bool envEnabled_;
	int envNum_;
	bool arpEnabled_;
	int arpNum_;
	bool ptEnabled_;
	int ptNum_;
};


class InstrumentDrumkit : public AbstractInstrument
{
public:
	InstrumentDrumkit(int number, std::string name, InstrumentsManager* owner);
	SoundSource getSoundSource() const override;
	InstrumentType getType() const override;
	AbstractInstrument* clone() override;

	std::vector<int> getAssignedKeys() const;

	void setSampleEnabled(int key, bool enabled);
	bool getSampleEnabled(int key) const;
	void setSampleNumber(int key, int n);
	int getSampleNumber(int key) const;
	int getSampleRootKeyNumber(int key) const;
	int getSampleRootDeltaN(int key) const;
	bool isSampleRepeatable(int key) const;
	std::vector<uint8_t> getRawSample(int key) const;
	size_t getSampleStartAddress(int key) const;
	size_t getSampleStopAddress(int key) const;

	void setPitch(int key, int pitch);
	int getPitch(int key) const;

private:
	struct KitProperty { int sampNum, pitch; };
	std::unordered_map<int, KitProperty> kit_;
};
