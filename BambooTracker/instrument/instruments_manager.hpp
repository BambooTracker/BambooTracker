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

#pragma once

#include <string>
#include <memory>
#include <array>
#include <vector>
#include <set>
#include <unordered_map>
#include "instrument.hpp"
#include "envelope_fm.hpp"
#include "lfo_fm.hpp"
#include "waveform_adpcm.hpp"
#include "command_sequence.hpp"
#include "instrument_property_defs.hpp"
#include "enum_hash.hpp"
#include "misc.hpp"

enum class InstrumentType;
class AbstractInstrument;
class InstrumentFM;
enum class FMEnvelopeParameter;
class EnvelopeFM;
class InstrumentSSG;
class InstrumentADPCM;
class InstrumentDrumkit;

class InstrumentsManager
{
public:
	explicit InstrumentsManager(bool unedited);

	void addInstrument(int instNum, InstrumentType type, std::string name);
	void addInstrument(std::unique_ptr<AbstractInstrument> inst);
	std::unique_ptr<AbstractInstrument> removeInstrument(int instNum);
	void cloneInstrument(int cloneInstNum, int resInstNum);
	void deepCloneInstrument(int cloneInstNum, int resInstNum);
	void swapInstruments(int inst1Num, int inst2Num);
	std::shared_ptr<AbstractInstrument> getInstrumentSharedPtr(int instNum);
	void clearAll();
	std::vector<int> getInstrumentIndices() const;

	void setInstrumentName(int instNum, std::string name);
	std::string getInstrumentName(int instNum) const;
	std::vector<std::string> getInstrumentNameList() const;

	std::vector<int> getEntriedInstrumentIndices() const;

	void clearUnusedInstrumentProperties();

	int findFirstFreeInstrument() const;

	std::vector<std::vector<int>> checkDuplicateInstruments() const;

	void setPropertyFindMode(bool unedited);

private:
	std::array<std::shared_ptr<AbstractInstrument>, 128> insts_;
	bool regardingUnedited_;

	//----- FM methods -----
public:
	void setInstrumentFMEnvelope(int instNum, int envNum);
	int getInstrumentFMEnvelope(int instNum) const;
	void setEnvelopeFMParameter(int envNum, FMEnvelopeParameter param, int value);
	int getEnvelopeFMParameter(int envNum, FMEnvelopeParameter param) const;
	void setEnvelopeFMOperatorEnabled(int envNum, int opNum, bool enabled);
	bool getEnvelopeFMOperatorEnabled(int envNum, int opNum) const;
	std::multiset<int> getEnvelopeFMUsers(int envNum) const;
	std::vector<int> getEnvelopeFMEntriedIndices() const;
	int findFirstAssignableEnvelopeFM() const;

	void setInstrumentFMLFOEnabled(int instNum, bool enabled);
	bool getInstrumentFMLFOEnabled(int instNum) const;
	void setInstrumentFMLFO(int instNum, int lfoNum);
	int getInstrumentFMLFO(int instNum) const;
	void setLFOFMParameter(int lfoNum, FMLFOParameter param, int value);
	int getLFOFMparameter(int lfoNum, FMLFOParameter param) const;
	std::multiset<int> getLFOFMUsers(int lfoNum) const;
	std::vector<int> getLFOFMEntriedIndices() const;
	int findFirstAssignableLFOFM() const;

	void setInstrumentFMOperatorSequenceEnabled(int instNum, FMEnvelopeParameter param, bool enabled);
	bool getInstrumentFMOperatorSequenceEnabled(int instNum, FMEnvelopeParameter param) const;
	void setInstrumentFMOperatorSequence(int instNum, FMEnvelopeParameter param, int opSeqNum);
	int getInstrumentFMOperatorSequence(int instNum, FMEnvelopeParameter param);
	void addOperatorSequenceFMSequenceCommand(FMEnvelopeParameter param, int opSeqNum, int data);
	void removeOperatorSequenceFMSequenceCommand(FMEnvelopeParameter param, int opSeqNum);
	void setOperatorSequenceFMSequenceCommand(FMEnvelopeParameter param, int opSeqNum, int cnt, int data);
	std::vector<FMOperatorSequenceUnit> getOperatorSequenceFMSequence(FMEnvelopeParameter param, int opSeqNum);
	void addOperatorSequenceFMLoop(FMEnvelopeParameter param, int opSeqNum, const InstrumentSequenceLoop& loop);
	void removeOperatorSequenceFMLoop(FMEnvelopeParameter param, int opSeqNum, int begin, int end);
	void changeOperatorSequenceFMLoop(FMEnvelopeParameter param, int opSeqNum, int prevBegin, int prevEnd, const InstrumentSequenceLoop& loop);
	void clearOperatorSequenceFMLoops(FMEnvelopeParameter param, int opSeqNum);
	InstrumentSequenceLoopRoot getOperatorSequenceFMLoopRoot(FMEnvelopeParameter param, int opSeqNum) const;
	void setOperatorSequenceFMRelease(FMEnvelopeParameter param, int opSeqNum, const InstrumentSequenceRelease& release);
	InstrumentSequenceRelease getOperatorSequenceFMRelease(FMEnvelopeParameter param, int opSeqNum) const;
	FMOperatorSequenceIter getOperatorSequenceFMIterator(FMEnvelopeParameter param, int opSeqNum) const;
	std::multiset<int> getOperatorSequenceFMUsers(FMEnvelopeParameter param, int opSeqNum) const;
	std::vector<int> getOperatorSequenceFMEntriedIndices(FMEnvelopeParameter param) const;
	int findFirstAssignableOperatorSequenceFM(FMEnvelopeParameter param) const;

	void setInstrumentFMArpeggioEnabled(int instNum, FMOperatorType op, bool enabled);
	bool getInstrumentFMArpeggioEnabled(int instNum, FMOperatorType op) const;
	void setInstrumentFMArpeggio(int instNum, FMOperatorType op, int arpNum);
	int getInstrumentFMArpeggio(int instNum, FMOperatorType op);
	void setArpeggioFMType(int arpNum, SequenceType type);
	SequenceType getArpeggioFMType(int arpNum) const;
	void addArpeggioFMSequenceCommand(int arpNum, int type, int data);
	void removeArpeggioFMSequenceCommand(int arpNum);
	void setArpeggioFMSequenceCommand(int arpNum, int cnt, int type, int data);
	std::vector<CommandSequenceUnit> getArpeggioFMSequence(int arpNum);
	void setArpeggioFMLoops(int arpNum, std::vector<int> begins, std::vector<int> ends, std::vector<int> times);
	std::vector<Loop> getArpeggioFMLoops(int arpNum) const;
	void setArpeggioFMRelease(int arpNum, ReleaseType type, int begin);
	Release getArpeggioFMRelease(int arpNum) const;
	std::unique_ptr<CommandSequence::Iterator> getArpeggioFMIterator(int arpNum) const;
	std::multiset<int> getArpeggioFMUsers(int arpNum) const;
	std::vector<int> getArpeggioFMEntriedIndices() const;
	int findFirstAssignableArpeggioFM() const;

	void setInstrumentFMPitchEnabled(int instNum, FMOperatorType op, bool enabled);
	bool getInstrumentFMPitchEnabled(int instNum, FMOperatorType op) const;
	void setInstrumentFMPitch(int instNum, FMOperatorType op, int ptNum);
	int getInstrumentFMPitch(int instNum, FMOperatorType op);
	void setPitchFMType(int ptNum, SequenceType type);
	SequenceType getPitchFMType(int ptNum) const;
	void addPitchFMSequenceCommand(int ptNum, int type, int data);
	void removePitchFMSequenceCommand(int ptNum);
	void setPitchFMSequenceCommand(int ptNum, int cnt, int type, int data);
	std::vector<CommandSequenceUnit> getPitchFMSequence(int ptNum);
	void setPitchFMLoops(int ptNum, std::vector<int> begins, std::vector<int> ends, std::vector<int> times);
	std::vector<Loop> getPitchFMLoops(int ptNum) const;
	void setPitchFMRelease(int ptNum, ReleaseType type, int begin);
	Release getPitchFMRelease(int ptNum) const;
	std::unique_ptr<CommandSequence::Iterator> getPitchFMIterator(int ptNum) const;
	std::multiset<int> getPitchFMUsers(int ptNum) const;
	std::vector<int> getPitchFMEntriedIndices() const;
	int findFirstAssignablePitchFM() const;

	void setInstrumentFMEnvelopeResetEnabled(int instNum, FMOperatorType op, bool enabled);

private:
	std::array<std::shared_ptr<EnvelopeFM>, 128> envFM_;
	std::array<std::shared_ptr<LFOFM>, 128> lfoFM_;
	std::unordered_map<FMEnvelopeParameter, std::array<std::shared_ptr<InstrumentSequenceProperty<InstrumentSequenceBaseUnit>>, 128>> opSeqFM_;
	std::array<std::shared_ptr<CommandSequence>, 128> arpFM_;
	std::array<std::shared_ptr<CommandSequence>, 128> ptFM_;

	static const FMEnvelopeParameter ENV_FM_PARAMS_[38];
	static const FMOperatorType FM_OP_TYPES_[5];

	int cloneFMEnvelope(int srcNum);
	int cloneFMLFO(int srcNum);
	int cloneFMOperatorSequence(FMEnvelopeParameter param, int srcNum);
	int cloneFMArpeggio(int srcNum);
	int cloneFMPitch(int srcNum);

	bool equalPropertiesFM(std::shared_ptr<AbstractInstrument> a, std::shared_ptr<AbstractInstrument> b) const;

	//----- SSG methods -----
public:
	void setInstrumentSSGWaveformEnabled(int instNum, bool enabled);
	bool getInstrumentSSGWaveformEnabled(int instNum) const;
	void setInstrumentSSGWaveform(int instNum, int wfNum);
	int getInstrumentSSGWaveform(int instNum);
	void addWaveformSSGSequenceCommand(int wfNum, int type, int data);
	void removeWaveformSSGSequenceCommand(int wfNum);
	void setWaveformSSGSequenceCommand(int wfNum, int cnt, int type, int data);
	std::vector<CommandSequenceUnit> getWaveformSSGSequence(int wfNum);
	void setWaveformSSGLoops(int wfNum, std::vector<int> begins, std::vector<int> ends, std::vector<int> times);
	std::vector<Loop> getWaveformSSGLoops(int wfNum) const;
	void setWaveformSSGRelease(int wfNum, ReleaseType type, int begin);
	Release getWaveformSSGRelease(int wfNum) const;
	std::unique_ptr<CommandSequence::Iterator> getWaveformSSGIterator(int wfNum) const;
	std::multiset<int> getWaveformSSGUsers(int wfNum) const;
	std::vector<int> getWaveformSSGEntriedIndices() const;
	int findFirstAssignableWaveformSSG() const;

	void setInstrumentSSGToneNoiseEnabled(int instNum, bool enabled);
	bool getInstrumentSSGToneNoiseEnabled(int instNum) const;
	void setInstrumentSSGToneNoise(int instNum, int tnNum);
	int getInstrumentSSGToneNoise(int instNum);
	void addToneNoiseSSGSequenceCommand(int tnNum, int type, int data);
	void removeToneNoiseSSGSequenceCommand(int tnNum);
	void setToneNoiseSSGSequenceCommand(int tnNum, int cnt, int type, int data);
	std::vector<CommandSequenceUnit> getToneNoiseSSGSequence(int tnNum);
	void setToneNoiseSSGLoops(int tnNum, std::vector<int> begins, std::vector<int> ends, std::vector<int> times);
	std::vector<Loop> getToneNoiseSSGLoops(int tnNum) const;
	void setToneNoiseSSGRelease(int tnNum, ReleaseType type, int begin);
	Release getToneNoiseSSGRelease(int tnNum) const;
	std::unique_ptr<CommandSequence::Iterator> getToneNoiseSSGIterator(int tnNum) const;
	std::multiset<int> getToneNoiseSSGUsers(int tnNum) const;
	std::vector<int> getToneNoiseSSGEntriedIndices() const;
	int findFirstAssignableToneNoiseSSG() const;

	void setInstrumentSSGEnvelopeEnabled(int instNum, bool enabled);
	bool getInstrumentSSGEnvelopeEnabled(int instNum) const;
	void setInstrumentSSGEnvelope(int instNum, int envNum);
	int getInstrumentSSGEnvelope(int instNum);
	void addEnvelopeSSGSequenceCommand(int envNum, int type, int data);
	void removeEnvelopeSSGSequenceCommand(int envNum);
	void setEnvelopeSSGSequenceCommand(int envNum, int cnt, int type, int data);
	std::vector<CommandSequenceUnit> getEnvelopeSSGSequence(int envNum);
	void setEnvelopeSSGLoops(int envNum, std::vector<int> begins, std::vector<int> ends, std::vector<int> times);
	std::vector<Loop> getEnvelopeSSGLoops(int envNum) const;
	void setEnvelopeSSGRelease(int envNum, ReleaseType type, int begin);
	Release getEnvelopeSSGRelease(int envNum) const;
	std::unique_ptr<CommandSequence::Iterator> getEnvelopeSSGIterator(int envNum) const;
	std::multiset<int> getEnvelopeSSGUsers(int envNum) const;
	std::vector<int> getEnvelopeSSGEntriedIndices() const;
	int findFirstAssignableEnvelopeSSG() const;

	void setInstrumentSSGArpeggioEnabled(int instNum, bool enabled);
	bool getInstrumentSSGArpeggioEnabled(int instNum) const;
	void setInstrumentSSGArpeggio(int instNum, int arpNum);
	int getInstrumentSSGArpeggio(int instNum);
	void setArpeggioSSGType(int arpNum, SequenceType type);
	SequenceType getArpeggioSSGType(int arpNum) const;
	void addArpeggioSSGSequenceCommand(int arpNum, int type, int data);
	void removeArpeggioSSGSequenceCommand(int arpNum);
	void setArpeggioSSGSequenceCommand(int arpNum, int cnt, int type, int data);
	std::vector<CommandSequenceUnit> getArpeggioSSGSequence(int arpNum);
	void setArpeggioSSGLoops(int arpNum, std::vector<int> begins, std::vector<int> ends, std::vector<int> times);
	std::vector<Loop> getArpeggioSSGLoops(int arpNum) const;
	void setArpeggioSSGRelease(int arpNum, ReleaseType type, int begin);
	Release getArpeggioSSGRelease(int arpNum) const;
	std::unique_ptr<CommandSequence::Iterator> getArpeggioSSGIterator(int arpNum) const;
	std::multiset<int> getArpeggioSSGUsers(int arpNum) const;
	std::vector<int> getArpeggioSSGEntriedIndices() const;
	int findFirstAssignableArpeggioSSG() const;

	void setInstrumentSSGPitchEnabled(int instNum, bool enabled);
	bool getInstrumentSSGPitchEnabled(int instNum) const;
	void setInstrumentSSGPitch(int instNum, int ptNum);
	int getInstrumentSSGPitch(int instNum);
	void setPitchSSGType(int ptNum, SequenceType type);
	SequenceType getPitchSSGType(int ptNum) const;
	void addPitchSSGSequenceCommand(int ptNum, int type, int data);
	void removePitchSSGSequenceCommand(int ptNum);
	void setPitchSSGSequenceCommand(int ptNum, int cnt, int type, int data);
	std::vector<CommandSequenceUnit> getPitchSSGSequence(int ptNum);
	void setPitchSSGLoops(int ptNum, std::vector<int> begins, std::vector<int> ends, std::vector<int> times);
	std::vector<Loop> getPitchSSGLoops(int ptNum) const;
	void setPitchSSGRelease(int ptNum, ReleaseType type, int begin);
	Release getPitchSSGRelease(int ptNum) const;
	std::unique_ptr<CommandSequence::Iterator> getPitchSSGIterator(int ptNum) const;
	std::multiset<int> getPitchSSGUsers(int ptNum) const;
	std::vector<int> getPitchSSGEntriedIndices() const;
	int findFirstAssignablePitchSSG() const;

private:
	std::array<std::shared_ptr<CommandSequence>, 128> wfSSG_;
	std::array<std::shared_ptr<CommandSequence>, 128> envSSG_;
	std::array<std::shared_ptr<CommandSequence>, 128> tnSSG_;
	std::array<std::shared_ptr<CommandSequence>, 128> arpSSG_;
	std::array<std::shared_ptr<CommandSequence>, 128> ptSSG_;

	int cloneSSGWaveform(int srcNum);
	int cloneSSGToneNoise(int srcNum);
	int cloneSSGEnvelope(int srcNum);
	int cloneSSGArpeggio(int srcNum);
	int cloneSSGPitch(int srcNum);

	bool equalPropertiesSSG(std::shared_ptr<AbstractInstrument> a, std::shared_ptr<AbstractInstrument> b) const;

	//----- ADPCM methods -----
public:
	void setInstrumentADPCMSample(int instNum, int sampNum);
	int getInstrumentADPCMSample(int instNum);
	void setSampleADPCMRootKeyNumber(int sampNum, int n);
	int getSampleADPCMRootKeyNumber(int sampNum) const;
	void setSampleADPCMRootDeltaN(int sampNum, int dn);
	int getSampleADPCMRootDeltaN(int sampNum) const;
	void setSampleADPCMRepeatEnabled(int sampNum, bool enabled);
	bool isSampleADPCMRepeatable(int sampNum) const;
	void storeSampleADPCMRawSample(int sampNum, std::vector<uint8_t> sample);
	void clearSampleADPCMRawSample(int sampNum);
	std::vector<uint8_t> getSampleADPCMRawSample(int sampNum) const;
	void setSampleADPCMStartAddress(int sampNum, size_t addr);
	size_t getSampleADPCMStartAddress(int sampNum) const;
	void setSampleADPCMStopAddress(int sampNum, size_t addr);
	size_t getSampleADPCMStopAddress(int sampNum) const;
	std::multiset<int> getSampleADPCMUsers(int sampNum) const;
	std::vector<int> getSampleADPCMEntriedIndices() const;
	std::vector<int> getSampleADPCMValidIndices() const;
	void clearUnusedSamplesADPCM();
	int findFirstAssignableSampleADPCM(int startIndex = 0) const;

	void setInstrumentADPCMEnvelopeEnabled(int instNum, bool enabled);
	bool getInstrumentADPCMEnvelopeEnabled(int instNum) const;
	void setInstrumentADPCMEnvelope(int instNum, int envNum);
	int getInstrumentADPCMEnvelope(int instNum);
	void addEnvelopeADPCMSequenceCommand(int envNum, int type, int data);
	void removeEnvelopeADPCMSequenceCommand(int envNum);
	void setEnvelopeADPCMSequenceCommand(int envNum, int cnt, int type, int data);
	std::vector<CommandSequenceUnit> getEnvelopeADPCMSequence(int envNum);
	void setEnvelopeADPCMLoops(int envNum, std::vector<int> begins, std::vector<int> ends, std::vector<int> times);
	std::vector<Loop> getEnvelopeADPCMLoops(int envNum) const;
	void setEnvelopeADPCMRelease(int envNum, ReleaseType type, int begin);
	Release getEnvelopeADPCMRelease(int envNum) const;
	std::unique_ptr<CommandSequence::Iterator> getEnvelopeADPCMIterator(int envNum) const;
	std::multiset<int> getEnvelopeADPCMUsers(int envNum) const;
	std::vector<int> getEnvelopeADPCMEntriedIndices() const;
	int findFirstAssignableEnvelopeADPCM() const;

	void setInstrumentADPCMArpeggioEnabled(int instNum, bool enabled);
	bool getInstrumentADPCMArpeggioEnabled(int instNum) const;
	void setInstrumentADPCMArpeggio(int instNum, int arpNum);
	int getInstrumentADPCMArpeggio(int instNum);
	void setArpeggioADPCMType(int arpNum, SequenceType type);
	SequenceType getArpeggioADPCMType(int arpNum) const;
	void addArpeggioADPCMSequenceCommand(int arpNum, int type, int data);
	void removeArpeggioADPCMSequenceCommand(int arpNum);
	void setArpeggioADPCMSequenceCommand(int arpNum, int cnt, int type, int data);
	std::vector<CommandSequenceUnit> getArpeggioADPCMSequence(int arpNum);
	void setArpeggioADPCMLoops(int arpNum, std::vector<int> begins, std::vector<int> ends, std::vector<int> times);
	std::vector<Loop> getArpeggioADPCMLoops(int arpNum) const;
	void setArpeggioADPCMRelease(int arpNum, ReleaseType type, int begin);
	Release getArpeggioADPCMRelease(int arpNum) const;
	std::unique_ptr<CommandSequence::Iterator> getArpeggioADPCMIterator(int arpNum) const;
	std::multiset<int> getArpeggioADPCMUsers(int arpNum) const;
	std::vector<int> getArpeggioADPCMEntriedIndices() const;
	int findFirstAssignableArpeggioADPCM() const;

	void setInstrumentADPCMPitchEnabled(int instNum, bool enabled);
	bool getInstrumentADPCMPitchEnabled(int instNum) const;
	void setInstrumentADPCMPitch(int instNum, int ptNum);
	int getInstrumentADPCMPitch(int instNum);
	void setPitchADPCMType(int ptNum, SequenceType type);
	SequenceType getPitchADPCMType(int ptNum) const;
	void addPitchADPCMSequenceCommand(int ptNum, int type, int data);
	void removePitchADPCMSequenceCommand(int ptNum);
	void setPitchADPCMSequenceCommand(int ptNum, int cnt, int type, int data);
	std::vector<CommandSequenceUnit> getPitchADPCMSequence(int ptNum);
	void setPitchADPCMLoops(int ptNum, std::vector<int> begins, std::vector<int> ends, std::vector<int> times);
	std::vector<Loop> getPitchADPCMLoops(int ptNum) const;
	void setPitchADPCMRelease(int ptNum, ReleaseType type, int begin);
	Release getPitchADPCMRelease(int ptNum) const;
	std::unique_ptr<CommandSequence::Iterator> getPitchADPCMIterator(int ptNum) const;
	std::multiset<int> getPitchADPCMUsers(int ptNum) const;
	std::vector<int> getPitchADPCMEntriedIndices() const;
	int findFirstAssignablePitchADPCM() const;

private:
	std::array<std::shared_ptr<SampleADPCM>, 128> sampADPCM_;
	std::array<std::shared_ptr<CommandSequence>, 128> envADPCM_;
	std::array<std::shared_ptr<CommandSequence>, 128> arpADPCM_;
	std::array<std::shared_ptr<CommandSequence>, 128> ptADPCM_;

	int cloneADPCMSample(int srcNum);
	int cloneADPCMEnvelope(int srcNum);
	int cloneADPCMArpeggio(int srcNum);
	int cloneADPCMPitch(int srcNum);

	bool equalPropertiesADPCM(std::shared_ptr<AbstractInstrument> a, std::shared_ptr<AbstractInstrument> b) const;

	//----- Drumkit methods -----
public:
	void setInstrumentDrumkitSamplesEnabled(int instNum, int key, bool enabled);
	bool getInstrumentDrumkitSamplesEnabled(int instNum, int key) const;
	void setInstrumentDrumkitSamples(int instNum, int key, int sampNum);
	int getInstrumentDrumkitSamples(int instNum, int key);

	void setInstrumentDrumkitPitch(int instNum, int key, int pitch);

private:
	bool equalPropertiesDrumkit(std::shared_ptr<AbstractInstrument> a, std::shared_ptr<AbstractInstrument> b) const;
};
