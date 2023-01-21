/*
 * Copyright (C) 2018-2023 Rerrah
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
#include "envelope_fm.hpp"
#include "lfo_fm.hpp"
#include "sequence_property.hpp"
#include "sample_repeat.hpp"
#include "instrument_property_defs.hpp"
#include "enum_hash.hpp"
#include "bamboo_tracker_defs.hpp"

class InstrumentsManager;

enum class InstrumentType : int { FM, SSG, ADPCM, Drumkit };

class AbstractInstrument
{
public:
	virtual ~AbstractInstrument() = default;

	inline int getNumber() const noexcept { return number_; }
	inline void setNumber(int n) noexcept { number_ = n; }
	inline SoundSource getSoundSource() const noexcept { return sndSrc_; }
	inline InstrumentType getType() const noexcept { return instType_; }
	inline std::string getName() const noexcept { return name_; }
	inline void setName(const std::string& name) { name_ = name; }
	bool isRegisteredWithManager() const;
	virtual AbstractInstrument* clone() = 0;

protected:
	InstrumentsManager* owner_;
	int number_;
	std::string name_;	// UTF-8
	AbstractInstrument(int number, SoundSource src, InstrumentType type, const std::string& name, InstrumentsManager* owner);

private:
	const SoundSource sndSrc_;
	const InstrumentType instType_;
};


class InstrumentFM final : public AbstractInstrument
{
public:
	InstrumentFM(int number, const std::string& name, InstrumentsManager* owner);
	AbstractInstrument* clone() override;

	inline void setEnvelopeNumber(int n) noexcept { envNum_ = n; }
	inline int getEnvelopeNumber() const noexcept { return envNum_; }
	int getEnvelopeParameter(FMEnvelopeParameter param) const;
	bool getOperatorEnabled(int n) const;

	inline void setLFOEnabled(bool enabled) noexcept { lfoEnabled_ = enabled; }
	inline bool getLFOEnabled() const noexcept { return lfoEnabled_; }
	inline void setLFONumber(int n) noexcept { lfoNum_ = n; }
	inline int getLFONumber() const noexcept { return lfoNum_; }
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

	void setPanEnabled(bool enabled);
	bool getPanEnabled() const;
	void setPanNumber(int n);
	int getPanNumber() const;
	std::vector<PanUnit> getPanSequence() const;
	InstrumentSequenceLoopRoot getPanLoopRoot() const;
	InstrumentSequenceRelease getPanRelease() const;
	PanIter getPanSequenceIterator() const;

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
	bool panEnabled_;
	int panNum_;

	std::unordered_map<FMOperatorType, bool> envResetEnabled_;
};


class InstrumentSSG final : public AbstractInstrument
{
public:
	InstrumentSSG(int number, const std::string& name, InstrumentsManager* owner);
	AbstractInstrument* clone() override;

	inline void setWaveformEnabled(bool enabled) noexcept { wfEnabled_ = enabled; }
	inline bool getWaveformEnabled() const noexcept { return wfEnabled_; }
	inline void setWaveformNumber(int n) noexcept { wfNum_ = n; }
	inline int getWaveformNumber() const noexcept { return wfNum_; }
	std::vector<SSGWaveformUnit> getWaveformSequence() const;
	InstrumentSequenceLoopRoot getWaveformLoopRoot() const;
	InstrumentSequenceRelease getWaveformRelease() const;
	SSGWaveformIter getWaveformSequenceIterator() const;

	inline void setToneNoiseEnabled(bool enabled) noexcept { tnEnabled_ = enabled; }
	inline bool getToneNoiseEnabled() const noexcept { return tnEnabled_; }
	inline void setToneNoiseNumber(int n) noexcept { tnNum_ = n; }
	inline int getToneNoiseNumber() const noexcept { return tnNum_; }
	std::vector<SSGToneNoiseUnit> getToneNoiseSequence() const;
	InstrumentSequenceLoopRoot getToneNoiseLoopRoot() const;
	InstrumentSequenceRelease getToneNoiseRelease() const;
	SSGToneNoiseIter getToneNoiseSequenceIterator() const;

	inline void setEnvelopeEnabled(bool enabled) noexcept { envEnabled_ = enabled; }
	inline bool getEnvelopeEnabled() const noexcept { return envEnabled_; }
	inline void setEnvelopeNumber(int n) noexcept { envNum_ = n; }
	inline int getEnvelopeNumber() const noexcept { return envNum_; }
	std::vector<SSGEnvelopeUnit> getEnvelopeSequence() const;
	InstrumentSequenceLoopRoot getEnvelopeLoopRoot() const;
	InstrumentSequenceRelease getEnvelopeRelease() const;
	SSGEnvelopeIter getEnvelopeSequenceIterator() const;

	inline void setArpeggioEnabled(bool enabled) noexcept { arpEnabled_ = enabled; }
	inline bool getArpeggioEnabled() const noexcept { return arpEnabled_; }
	inline void setArpeggioNumber(int n) noexcept { arpNum_ = n; }
	inline int getArpeggioNumber() const noexcept { return arpNum_; }
	SequenceType getArpeggioType() const;
	std::vector<ArpeggioUnit> getArpeggioSequence() const;
	InstrumentSequenceLoopRoot getArpeggioLoopRoot() const;
	InstrumentSequenceRelease getArpeggioRelease() const;
	ArpeggioIter getArpeggioSequenceIterator() const;

	inline void setPitchEnabled(bool enabled) noexcept { ptEnabled_ = enabled; }
	inline bool getPitchEnabled() const noexcept { return ptEnabled_; }
	inline void setPitchNumber(int n) noexcept { ptNum_ = n; }
	inline int getPitchNumber() const noexcept { return ptNum_; }
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


class InstrumentADPCM final : public AbstractInstrument
{
public:
	InstrumentADPCM(int number, const std::string& name, InstrumentsManager* owner);
	AbstractInstrument* clone() override;

	inline void setSampleNumber(int n) noexcept { sampNum_ = n; }
	inline int getSampleNumber() const noexcept { return sampNum_; }
	int getSampleRootKeyNumber() const;
	int getSampleRootDeltaN() const;
	bool isSampleRepeatable() const;
	SampleRepeatFlag getSampleRepeatFlag() const;
	SampleRepeatRange getSampleRepeatRange() const;
	std::vector<uint8_t> getRawSample() const;
	size_t getSampleStartAddress() const;
	size_t getSampleStopAddress() const;

	inline void setEnvelopeEnabled(bool enabled) noexcept { envEnabled_ = enabled; }
	inline bool getEnvelopeEnabled() const noexcept { return envEnabled_; }
	inline void setEnvelopeNumber(int n) noexcept { envNum_ = n; }
	inline int getEnvelopeNumber() const noexcept { return envNum_; }
	std::vector<ADPCMEnvelopeUnit> getEnvelopeSequence() const;
	InstrumentSequenceLoopRoot getEnvelopeLoopRoot() const;
	InstrumentSequenceRelease getEnvelopeRelease() const;
	ADPCMEnvelopeIter getEnvelopeSequenceIterator() const;

	inline void setArpeggioEnabled(bool enabled) noexcept { arpEnabled_ = enabled; }
	inline bool getArpeggioEnabled() const noexcept { return arpEnabled_; }
	inline void setArpeggioNumber(int n) noexcept { arpNum_ = n; }
	inline int getArpeggioNumber() const noexcept { return arpNum_; }
	SequenceType getArpeggioType() const;
	std::vector<ArpeggioUnit> getArpeggioSequence() const;
	InstrumentSequenceLoopRoot getArpeggioLoopRoot() const;
	InstrumentSequenceRelease getArpeggioRelease() const;
	ArpeggioIter getArpeggioSequenceIterator() const;

	inline void setPitchEnabled(bool enabled) noexcept { ptEnabled_ = enabled; }
	inline bool getPitchEnabled() const noexcept { return ptEnabled_; }
	inline void setPitchNumber(int n) noexcept { ptNum_ = n; }
	inline int getPitchNumber() const noexcept { return ptNum_; }
	SequenceType getPitchType() const;
	std::vector<PitchUnit> getPitchSequence() const;
	InstrumentSequenceLoopRoot getPitchLoopRoot() const;
	InstrumentSequenceRelease getPitchRelease() const;
	PitchIter getPitchSequenceIterator() const;

	inline void setPanEnabled(bool enabled) noexcept { panEnabled_ = enabled; }
	inline bool getPanEnabled() const noexcept { return panEnabled_; }
	inline void setPanNumber(int n) noexcept { panNum_ = n; }
	inline int getPanNumber() const noexcept { return panNum_; }
	std::vector<PanUnit> getPanSequence() const;
	InstrumentSequenceLoopRoot getPanLoopRoot() const;
	InstrumentSequenceRelease getPanRelease() const;
	PanIter getPanSequenceIterator() const;

private:
	int sampNum_;
	bool envEnabled_;
	int envNum_;
	bool arpEnabled_;
	int arpNum_;
	bool ptEnabled_;
	int ptNum_;
	bool panEnabled_;
	int panNum_;
};


class InstrumentDrumkit final : public AbstractInstrument
{
public:
	InstrumentDrumkit(int number, const std::string& name, InstrumentsManager* owner);
	AbstractInstrument* clone() override;

	std::vector<int> getAssignedKeys() const;

	void setSampleEnabled(int key, bool enabled);
	bool getSampleEnabled(int key) const;
	void setSampleNumber(int key, int n);
	int getSampleNumber(int key) const;
	int getSampleRootKeyNumber(int key) const;
	int getSampleRootDeltaN(int key) const;
	bool isSampleRepeatable(int key) const;
	SampleRepeatFlag getSampleRepeatFlag(int key) const;
	SampleRepeatRange getSampleRepeatRange(int key) const;
	std::vector<uint8_t> getRawSample(int key) const;
	size_t getSampleStartAddress(int key) const;
	size_t getSampleStopAddress(int key) const;

	void setPitch(int key, int pitch);
	int getPitch(int key) const;

	void setPan(int key, int pan);
	int getPan(int key) const;

private:
	struct KitProperty { int sampNum, pitch, pan; };
	std::unordered_map<int, KitProperty> kit_;
};
