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
#include <array>
#include <vector>
#include <set>
#include "instrument.hpp"
#include "envelope_fm.hpp"
#include "lfo_fm.hpp"
#include "sample_adpcm.hpp"
#include "sequence_property.hpp"
#include "instrument_property_defs.hpp"
#include "enum_hash.hpp"

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

	void addInstrument(int instNum, InstrumentType type, const std::string& name);
	void addInstrument(AbstractInstrument* newInstPtr);
	std::unique_ptr<AbstractInstrument> removeInstrument(int instNum);
	void cloneInstrument(int cloneInstNum, int resInstNum);
	void deepCloneInstrument(int cloneInstNum, int resInstNum);
	void swapInstruments(int inst1Num, int inst2Num);
	std::shared_ptr<AbstractInstrument> getInstrumentSharedPtr(int instNum);
	void clearAll();
	std::vector<int> getInstrumentIndices() const;

	void setInstrumentName(int instNum, const std::string& name);
	std::string getInstrumentName(int instNum) const;
	std::vector<std::string> getInstrumentNameList() const;

	void clearUnusedInstrumentProperties();

	int findFirstFreeInstrument() const;

	std::unordered_map<int, int> getDuplicateInstrumentMap() const;

	inline void setPropertyFindMode(bool unedited) noexcept { regardingUnedited_ = unedited; }

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
	void addOperatorSequenceFMSequenceData(FMEnvelopeParameter param, int opSeqNum, int data);
	void removeOperatorSequenceFMSequenceData(FMEnvelopeParameter param, int opSeqNum);
	void setOperatorSequenceFMSequenceData(FMEnvelopeParameter param, int opSeqNum, int cnt, int data);
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
	void addArpeggioFMSequenceData(int arpNum, int data);
	void removeArpeggioFMSequenceData(int arpNum);
	void setArpeggioFMSequenceData(int arpNum, int cnt, int data);
	std::vector<ArpeggioUnit> getArpeggioFMSequence(int arpNum);
	void addArpeggioFMLoop(int arpNum, const InstrumentSequenceLoop& loop);
	void removeArpeggioFMLoop(int arpNum, int begin, int end);
	void changeArpeggioFMLoop(int arpNum, int prevBegin, int prevEnd, const InstrumentSequenceLoop& loop);
	void clearArpeggioFMLoops(int arpNum);
	InstrumentSequenceLoopRoot getArpeggioFMLoopRoot(int arpNum) const;
	void setArpeggioFMRelease(int arpNum, const InstrumentSequenceRelease& release);
	InstrumentSequenceRelease getArpeggioFMRelease(int arpNum) const;
	ArpeggioIter getArpeggioFMIterator(int arpNum) const;
	std::multiset<int> getArpeggioFMUsers(int arpNum) const;
	std::vector<int> getArpeggioFMEntriedIndices() const;
	int findFirstAssignableArpeggioFM() const;

	void setInstrumentFMPitchEnabled(int instNum, FMOperatorType op, bool enabled);
	bool getInstrumentFMPitchEnabled(int instNum, FMOperatorType op) const;
	void setInstrumentFMPitch(int instNum, FMOperatorType op, int ptNum);
	int getInstrumentFMPitch(int instNum, FMOperatorType op);
	void setPitchFMType(int ptNum, SequenceType type);
	SequenceType getPitchFMType(int ptNum) const;
	void addPitchFMSequenceData(int ptNum, int data);
	void removePitchFMSequenceData(int ptNum);
	void setPitchFMSequenceData(int ptNum, int cnt, int data);
	std::vector<PitchUnit> getPitchFMSequence(int ptNum);
	void addPitchFMLoop(int ptNum, const InstrumentSequenceLoop& loop);
	void removePitchFMLoop(int ptNum, int begin, int end);
	void changePitchFMLoop(int ptNum, int prevBegin, int prevEnd, const InstrumentSequenceLoop& loop);
	void clearPitchFMLoops(int ptNum);
	InstrumentSequenceLoopRoot getPitchFMLoopRoot(int ptNum) const;
	void setPitchFMRelease(int ptNum, const InstrumentSequenceRelease& release);
	InstrumentSequenceRelease getPitchFMRelease(int ptNum) const;
	PitchIter getPitchFMIterator(int ptNum) const;
	std::multiset<int> getPitchFMUsers(int ptNum) const;
	std::vector<int> getPitchFMEntriedIndices() const;
	int findFirstAssignablePitchFM() const;

	void setInstrumentFMPanEnabled(int instNum, bool enabled);
	bool getInstrumentFMPanEnabled(int instNum) const;
	void setInstrumentFMPan(int instNum, int panNum);
	int getInstrumentFMPan(int instNum);
	void addPanFMSequenceData(int panNum, int data);
	void removePanFMSequenceData(int panNum);
	void setPanFMSequenceData(int panNum, int cnt, int data);
	std::vector<PanUnit> getPanFMSequence(int panNum);
	void addPanFMLoop(int panNum, const InstrumentSequenceLoop& loop);
	void removePanFMLoop(int panNum, int begin, int end);
	void changePanFMLoop(int panNum, int prevBegin, int prevEnd, const InstrumentSequenceLoop& loop);
	void clearPanFMLoops(int panNum);
	InstrumentSequenceLoopRoot getPanFMLoopRoot(int panNum) const;
	void setPanFMRelease(int panNum, const InstrumentSequenceRelease& release);
	InstrumentSequenceRelease getPanFMRelease(int panNum) const;
	PanIter getPanFMIterator(int panNum) const;
	std::multiset<int> getPanFMUsers(int panNum) const;
	std::vector<int> getPanFMEntriedIndices() const;
	int findFirstAssignablePanFM() const;

	void setInstrumentFMEnvelopeResetEnabled(int instNum, FMOperatorType op, bool enabled);

private:
	std::array<std::shared_ptr<EnvelopeFM>, 128> envFM_;
	std::array<std::shared_ptr<LFOFM>, 128> lfoFM_;
	std::unordered_map<FMEnvelopeParameter, std::array<std::shared_ptr<InstrumentSequenceProperty<FMOperatorSequenceUnit>>, 128>> opSeqFM_;
	std::array<std::shared_ptr<InstrumentSequenceProperty<ArpeggioUnit>>, 128> arpFM_;
	std::array<std::shared_ptr<InstrumentSequenceProperty<PitchUnit>>, 128> ptFM_;
	std::array<std::shared_ptr<InstrumentSequenceProperty<PanUnit>>, 128> panFM_;

	bool equalPropertiesFM(std::shared_ptr<AbstractInstrument> a, std::shared_ptr<AbstractInstrument> b) const;

	//----- SSG methods -----
public:
	void setInstrumentSSGWaveformEnabled(int instNum, bool enabled);
	bool getInstrumentSSGWaveformEnabled(int instNum) const;
	void setInstrumentSSGWaveform(int instNum, int wfNum);
	int getInstrumentSSGWaveform(int instNum);
	void addWaveformSSGSequenceData(int wfNum, const SSGWaveformUnit& data);
	void removeWaveformSSGSequenceData(int wfNum);
	void setWaveformSSGSequenceData(int wfNum, int cnt, const SSGWaveformUnit& data);
	std::vector<SSGWaveformUnit> getWaveformSSGSequence(int wfNum);
	void addWaveformSSGLoop(int wfNum, const InstrumentSequenceLoop& loop);
	void removeWaveformSSGLoop(int wfNum, int begin, int end);
	void changeWaveformSSGLoop(int wfNum, int prevBegin, int prevEnd, const InstrumentSequenceLoop& loop);
	void clearWaveformSSGLoops(int wfNum);
	InstrumentSequenceLoopRoot getWaveformSSGLoopRoot(int wfNum) const;
	void setWaveformSSGRelease(int wfNum, const InstrumentSequenceRelease& release);
	InstrumentSequenceRelease getWaveformSSGRelease(int wfNum) const;
	SSGWaveformIter getWaveformSSGIterator(int wfNum) const;
	std::multiset<int> getWaveformSSGUsers(int wfNum) const;
	std::vector<int> getWaveformSSGEntriedIndices() const;
	int findFirstAssignableWaveformSSG() const;

	void setInstrumentSSGToneNoiseEnabled(int instNum, bool enabled);
	bool getInstrumentSSGToneNoiseEnabled(int instNum) const;
	void setInstrumentSSGToneNoise(int instNum, int tnNum);
	int getInstrumentSSGToneNoise(int instNum);
	void addToneNoiseSSGSequenceData(int tnNum, int data);
	void removeToneNoiseSSGSequenceData(int tnNum);
	void setToneNoiseSSGSequenceData(int tnNum, int cnt, int data);
	std::vector<SSGToneNoiseUnit> getToneNoiseSSGSequence(int tnNum);
	void addToneNoiseSSGLoop(int tnNum, const InstrumentSequenceLoop& loop);
	void removeToneNoiseSSGLoop(int tnNum, int begin, int end);
	void changeToneNoiseSSGLoop(int tnNum, int prevBegin, int prevEnd, const InstrumentSequenceLoop& loop);
	void clearToneNoiseSSGLoops(int tnNum);
	InstrumentSequenceLoopRoot getToneNoiseSSGLoopRoot(int tnNum) const;
	void setToneNoiseSSGRelease(int tnNum, const InstrumentSequenceRelease& release);
	InstrumentSequenceRelease getToneNoiseSSGRelease(int tnNum) const;
	SSGToneNoiseIter getToneNoiseSSGIterator(int tnNum) const;
	std::multiset<int> getToneNoiseSSGUsers(int tnNum) const;
	std::vector<int> getToneNoiseSSGEntriedIndices() const;
	int findFirstAssignableToneNoiseSSG() const;

	void setInstrumentSSGEnvelopeEnabled(int instNum, bool enabled);
	bool getInstrumentSSGEnvelopeEnabled(int instNum) const;
	void setInstrumentSSGEnvelope(int instNum, int envNum);
	int getInstrumentSSGEnvelope(int instNum);
	void addEnvelopeSSGSequenceData(int envNum, const SSGEnvelopeUnit& data);
	void removeEnvelopeSSGSequenceData(int envNum);
	void setEnvelopeSSGSequenceData(int envNum, int cnt, const SSGEnvelopeUnit& data);
	std::vector<SSGEnvelopeUnit> getEnvelopeSSGSequence(int envNum);
	void addEnvelopeSSGLoop(int envNum, const InstrumentSequenceLoop& loop);
	void removeEnvelopeSSGLoop(int envNum, int begin, int end);
	void changeEnvelopeSSGLoop(int envNum, int prevBegin, int prevEnd, const InstrumentSequenceLoop& loop);
	void clearEnvelopeSSGLoops(int envNum);
	InstrumentSequenceLoopRoot getEnvelopeSSGLoopRoot(int envNum) const;
	void setEnvelopeSSGRelease(int envNum, const InstrumentSequenceRelease& release);
	InstrumentSequenceRelease getEnvelopeSSGRelease(int envNum) const;
	SSGEnvelopeIter getEnvelopeSSGIterator(int envNum) const;
	std::multiset<int> getEnvelopeSSGUsers(int envNum) const;
	std::vector<int> getEnvelopeSSGEntriedIndices() const;
	int findFirstAssignableEnvelopeSSG() const;

	void setInstrumentSSGArpeggioEnabled(int instNum, bool enabled);
	bool getInstrumentSSGArpeggioEnabled(int instNum) const;
	void setInstrumentSSGArpeggio(int instNum, int arpNum);
	int getInstrumentSSGArpeggio(int instNum);
	void setArpeggioSSGType(int arpNum, SequenceType type);
	SequenceType getArpeggioSSGType(int arpNum) const;
	void addArpeggioSSGSequenceData(int arpNum, int data);
	void removeArpeggioSSGSequenceData(int arpNum);
	void setArpeggioSSGSequenceData(int arpNum, int cnt, int data);
	std::vector<ArpeggioUnit> getArpeggioSSGSequence(int arpNum);
	void addArpeggioSSGLoop(int arpNum, const InstrumentSequenceLoop& loop);
	void removeArpeggioSSGLoop(int arpNum, int begin, int end);
	void changeArpeggioSSGLoop(int arpNum, int prevBegin, int prevEnd, const InstrumentSequenceLoop& loop);
	void clearArpeggioSSGLoops(int arpNum);
	InstrumentSequenceLoopRoot getArpeggioSSGLoopRoot(int arpNum) const;
	void setArpeggioSSGRelease(int arpNum, const InstrumentSequenceRelease& release);
	InstrumentSequenceRelease getArpeggioSSGRelease(int arpNum) const;
	ArpeggioIter getArpeggioSSGIterator(int arpNum) const;
	std::multiset<int> getArpeggioSSGUsers(int arpNum) const;
	std::vector<int> getArpeggioSSGEntriedIndices() const;
	int findFirstAssignableArpeggioSSG() const;

	void setInstrumentSSGPitchEnabled(int instNum, bool enabled);
	bool getInstrumentSSGPitchEnabled(int instNum) const;
	void setInstrumentSSGPitch(int instNum, int ptNum);
	int getInstrumentSSGPitch(int instNum);
	void setPitchSSGType(int ptNum, SequenceType type);
	SequenceType getPitchSSGType(int ptNum) const;
	void addPitchSSGSequenceData(int ptNum, int data);
	void removePitchSSGSequenceData(int ptNum);
	void setPitchSSGSequenceData(int ptNum, int cnt, int data);
	std::vector<PitchUnit> getPitchSSGSequence(int ptNum);
	void addPitchSSGLoop(int ptNum, const InstrumentSequenceLoop& loop);
	void removePitchSSGLoop(int ptNum, int begin, int end);
	void changePitchSSGLoop(int ptNum, int prevBegin, int prevEnd, const InstrumentSequenceLoop& loop);
	void clearPitchSSGLoops(int ptNum);
	InstrumentSequenceLoopRoot getPitchSSGLoopRoot(int ptNum) const;
	void setPitchSSGRelease(int ptNum, const InstrumentSequenceRelease& release);
	InstrumentSequenceRelease getPitchSSGRelease(int ptNum) const;
	PitchIter getPitchSSGIterator(int ptNum) const;
	std::multiset<int> getPitchSSGUsers(int ptNum) const;
	std::vector<int> getPitchSSGEntriedIndices() const;
	int findFirstAssignablePitchSSG() const;

private:
	std::array<std::shared_ptr<InstrumentSequenceProperty<SSGWaveformUnit>>, 128> wfSSG_;
	std::array<std::shared_ptr<InstrumentSequenceProperty<SSGEnvelopeUnit>>, 128> envSSG_;
	std::array<std::shared_ptr<InstrumentSequenceProperty<SSGToneNoiseUnit>>, 128> tnSSG_;
	std::array<std::shared_ptr<InstrumentSequenceProperty<ArpeggioUnit>>, 128> arpSSG_;
	std::array<std::shared_ptr<InstrumentSequenceProperty<PitchUnit>>, 128> ptSSG_;

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
	bool setSampleADPCMRepeatrange(int sampNum, const SampleRepeatRange& range);
	SampleRepeatFlag getSampleADPCMRepeatFlag(int sampNum) const;
	SampleRepeatRange getSampleADPCMRepeatRange(int sampNum) const;
	void storeSampleADPCMRawSample(int sampNum, const std::vector<uint8_t>& sample);
	void storeSampleADPCMRawSample(int sampNum, std::vector<uint8_t>&& sample);
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
	void addEnvelopeADPCMSequenceData(int envNum, int data);
	void removeEnvelopeADPCMSequenceData(int envNum);
	void setEnvelopeADPCMSequenceData(int envNum, int cnt, int data);
	std::vector<ADPCMEnvelopeUnit> getEnvelopeADPCMSequence(int envNum);
	void addEnvelopeADPCMLoop(int envNum, const InstrumentSequenceLoop& loop);
	void removeEnvelopeADPCMLoop(int envNum, int begin, int end);
	void changeEnvelopeADPCMLoop(int envNum, int prevBegin, int prevEnd, const InstrumentSequenceLoop& loop);
	void clearEnvelopeADPCMLoops(int envNum);
	InstrumentSequenceLoopRoot getEnvelopeADPCMLoopRoot(int envNum) const;
	void setEnvelopeADPCMRelease(int envNum, const InstrumentSequenceRelease& release);
	InstrumentSequenceRelease getEnvelopeADPCMRelease(int envNum) const;
	ADPCMEnvelopeIter getEnvelopeADPCMIterator(int envNum) const;
	std::multiset<int> getEnvelopeADPCMUsers(int envNum) const;
	std::vector<int> getEnvelopeADPCMEntriedIndices() const;
	int findFirstAssignableEnvelopeADPCM() const;

	void setInstrumentADPCMArpeggioEnabled(int instNum, bool enabled);
	bool getInstrumentADPCMArpeggioEnabled(int instNum) const;
	void setInstrumentADPCMArpeggio(int instNum, int arpNum);
	int getInstrumentADPCMArpeggio(int instNum);
	void setArpeggioADPCMType(int arpNum, SequenceType type);
	SequenceType getArpeggioADPCMType(int arpNum) const;
	void addArpeggioADPCMSequenceData(int arpNum, int data);
	void removeArpeggioADPCMSequenceData(int arpNum);
	void setArpeggioADPCMSequenceData(int arpNum, int cnt, int data);
	std::vector<ArpeggioUnit> getArpeggioADPCMSequence(int arpNum);
	void addArpeggioADPCMLoop(int arpNum, const InstrumentSequenceLoop& loop);
	void removeArpeggioADPCMLoop(int arpNum, int begin, int end);
	void changeArpeggioADPCMLoop(int arpNum, int prevBegin, int prevEnd, const InstrumentSequenceLoop& loop);
	void clearArpeggioADPCMLoops(int arpNum);
	InstrumentSequenceLoopRoot getArpeggioADPCMLoopRoot(int arpNum) const;
	void setArpeggioADPCMRelease(int arpNum, const InstrumentSequenceRelease& release);
	InstrumentSequenceRelease getArpeggioADPCMRelease(int arpNum) const;
	ArpeggioIter getArpeggioADPCMIterator(int arpNum) const;
	std::multiset<int> getArpeggioADPCMUsers(int arpNum) const;
	std::vector<int> getArpeggioADPCMEntriedIndices() const;
	int findFirstAssignableArpeggioADPCM() const;

	void setInstrumentADPCMPitchEnabled(int instNum, bool enabled);
	bool getInstrumentADPCMPitchEnabled(int instNum) const;
	void setInstrumentADPCMPitch(int instNum, int ptNum);
	int getInstrumentADPCMPitch(int instNum);
	void setPitchADPCMType(int ptNum, SequenceType type);
	SequenceType getPitchADPCMType(int ptNum) const;
	void addPitchADPCMSequenceData(int ptNum, int data);
	void removePitchADPCMSequenceData(int ptNum);
	void setPitchADPCMSequenceData(int ptNum, int cnt, int data);
	std::vector<PitchUnit> getPitchADPCMSequence(int ptNum);
	void addPitchADPCMLoop(int ptNum, const InstrumentSequenceLoop& loop);
	void removePitchADPCMLoop(int ptNum, int begin, int end);
	void changePitchADPCMLoop(int ptNum, int prevBegin, int prevEnd, const InstrumentSequenceLoop& loop);
	void clearPitchADPCMLoops(int ptNum);
	InstrumentSequenceLoopRoot getPitchADPCMLoopRoot(int ptNum) const;
	void setPitchADPCMRelease(int ptNum, const InstrumentSequenceRelease& release);
	InstrumentSequenceRelease getPitchADPCMRelease(int ptNum) const;
	PitchIter getPitchADPCMIterator(int ptNum) const;
	std::multiset<int> getPitchADPCMUsers(int ptNum) const;
	std::vector<int> getPitchADPCMEntriedIndices() const;
	int findFirstAssignablePitchADPCM() const;

	void setInstrumentADPCMPanEnabled(int instNum, bool enabled);
	bool getInstrumentADPCMPanEnabled(int instNum) const;
	void setInstrumentADPCMPan(int instNum, int panNum);
	int getInstrumentADPCMPan(int instNum);
	void addPanADPCMSequenceData(int panNum, int data);
	void removePanADPCMSequenceData(int panNum);
	void setPanADPCMSequenceData(int panNum, int cnt, int data);
	std::vector<PanUnit> getPanADPCMSequence(int panNum);
	void addPanADPCMLoop(int panNum, const InstrumentSequenceLoop& loop);
	void removePanADPCMLoop(int panNum, int begin, int end);
	void changePanADPCMLoop(int panNum, int prevBegin, int prevEnd, const InstrumentSequenceLoop& loop);
	void clearPanADPCMLoops(int panNum);
	InstrumentSequenceLoopRoot getPanADPCMLoopRoot(int panNum) const;
	void setPanADPCMRelease(int panNum, const InstrumentSequenceRelease& release);
	InstrumentSequenceRelease getPanADPCMRelease(int panNum) const;
	PanIter getPanADPCMIterator(int panNum) const;
	std::multiset<int> getPanADPCMUsers(int panNum) const;
	std::vector<int> getPanADPCMEntriedIndices() const;
	int findFirstAssignablePanADPCM() const;

private:
	std::array<std::shared_ptr<SampleADPCM>, 128> sampADPCM_;
	std::array<std::shared_ptr<InstrumentSequenceProperty<ADPCMEnvelopeUnit>>, 128> envADPCM_;
	std::array<std::shared_ptr<InstrumentSequenceProperty<ArpeggioUnit>>, 128> arpADPCM_;
	std::array<std::shared_ptr<InstrumentSequenceProperty<PitchUnit>>, 128> ptADPCM_;
	std::array<std::shared_ptr<InstrumentSequenceProperty<PanUnit>>, 128> panADPCM_;

	bool equalPropertiesADPCM(std::shared_ptr<AbstractInstrument> a, std::shared_ptr<AbstractInstrument> b) const;

	//----- Drumkit methods -----
public:
	void setInstrumentDrumkitSamplesEnabled(int instNum, int key, bool enabled);
	bool getInstrumentDrumkitSamplesEnabled(int instNum, int key) const;
	void setInstrumentDrumkitSamples(int instNum, int key, int sampNum);
	int getInstrumentDrumkitSamples(int instNum, int key);

	void setInstrumentDrumkitPitch(int instNum, int key, int pitch);
	void setInstrumentDrumkitPan(int instNum, int key, int pan);

private:
	bool equalPropertiesDrumkit(std::shared_ptr<AbstractInstrument> a, std::shared_ptr<AbstractInstrument> b) const;
};
