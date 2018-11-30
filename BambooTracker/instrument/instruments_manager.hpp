#pragma once

#include <string>
#include <memory>
#include <array>
#include <vector>
#include <map>
#include "instrument.hpp"
#include "envelope_fm.hpp"
#include "lfo_fm.hpp"
#include "command_sequence.hpp"
#include "misc.hpp"

class AbstractInstrument;
enum class FMEnvelopeParameter;
class EnvelopeFM;

class InstrumentsManager
{
public:
	InstrumentsManager();

	void addInstrument(int instNum, SoundSource source, std::string name);
	void addInstrument(std::unique_ptr<AbstractInstrument> inst);
	std::unique_ptr<AbstractInstrument> removeInstrument(int instNum);
	void cloneInstrument(int cloneInstNum, int resInstNum);
	void deepCloneInstrument(int cloneInstNum, int resInstNum);
	std::shared_ptr<AbstractInstrument> getInstrumentSharedPtr(int instNum);
	void clearAll();
	std::vector<int> getInstrumentIndices() const;

	void setInstrumentName(int instNum, std::string name);
	std::string getInstrumentName(int instNum) const;

	std::vector<int> getEntriedInstrumentIndices() const;

	void clearUnusedInstrumentProperties();

	int findFirstFreeInstrument() const;

private:
	std::array<std::shared_ptr<AbstractInstrument>, 128> insts_;

	//----- FM methods -----
public:
	void setInstrumentFMEnvelope(int instNum, int envNum);
    int getInstrumentFMEnvelope(int instNum) const;
	void setEnvelopeFMParameter(int envNum, FMEnvelopeParameter param, int value);
	int getEnvelopeFMParameter(int envNum, FMEnvelopeParameter param) const;
	void setEnvelopeFMOperatorEnabled(int envNum, int opNum, bool enabled);
	bool getEnvelopeFMOperatorEnabled(int envNum, int opNum) const;
	std::vector<int> getEnvelopeFMUsers(int envNum) const;
	std::vector<int> getEnvelopeFMEntriedIndices() const;
	int findFirstFreeEnvelopeFM() const;

	void setInstrumentFMLFOEnabled(int instNum, bool enabled);
	bool getInstrumentFMLFOEnabled(int instNum) const;
	void setInstrumentFMLFO(int instNum, int lfoNum);
	int getInstrumentFMLFO(int instNum) const;
	void setLFOFMParameter(int lfoNum, FMLFOParameter param, int value);
	int getLFOFMparameter(int lfoNum, FMLFOParameter param) const;
	std::vector<int> getLFOFMUsers(int lfoNum) const;
	std::vector<int> getLFOFMEntriedIndices() const;
	int findFirstFreeLFOFM() const;

	void setInstrumentFMOperatorSequenceEnabled(int instNum, FMEnvelopeParameter param, bool enabled);
	bool getInstrumentFMOperatorSequenceEnabled(int instNum, FMEnvelopeParameter param) const;
	void setInstrumentFMOperatorSequence(int instNum, FMEnvelopeParameter param, int opSeqNum);
	int getInstrumentFMOperatorSequence(int instNum, FMEnvelopeParameter param);
	void addOperatorSequenceFMSequenceCommand(FMEnvelopeParameter param, int opSeqNum, int type, int data);
	void removeOperatorSequenceFMSequenceCommand(FMEnvelopeParameter param, int opSeqNum);
	void setOperatorSequenceFMSequenceCommand(FMEnvelopeParameter param, int opSeqNum, int cnt, int type, int data);
	std::vector<CommandInSequence> getOperatorSequenceFMSequence(FMEnvelopeParameter param, int opSeqNum);
	void setOperatorSequenceFMLoops(FMEnvelopeParameter param, int opSeqNum, std::vector<int> begins, std::vector<int> ends, std::vector<int> times);
	std::vector<Loop> getOperatorSequenceFMLoops(FMEnvelopeParameter param, int opSeqNum) const;
	void setOperatorSequenceFMRelease(FMEnvelopeParameter param, int opSeqNum, ReleaseType type, int begin);
	Release getOperatorSequenceFMRelease(FMEnvelopeParameter param, int opSeqNum) const;
	std::unique_ptr<CommandSequence::Iterator> getOperatorSequenceFMIterator(FMEnvelopeParameter param, int opSeqNum) const;
	std::vector<int> getOperatorSequenceFMUsers(FMEnvelopeParameter param, int opSeqNum) const;
	std::vector<int> getOperatorSequenceFMEntriedIndices(FMEnvelopeParameter param) const;
	int findFirstFreeOperatorSequenceFM(FMEnvelopeParameter param) const;

	void setInstrumentFMArpeggioEnabled(int instNum, bool enabled);
	bool getInstrumentFMArpeggioEnabled(int instNum) const;
	void setInstrumentFMArpeggio(int instNum, int arpNum);
	int getInstrumentFMArpeggio(int instNum);
	void setArpeggioFMType(int arpNum, int type);
	int getArpeggioFMType(int arpNum) const;
	void addArpeggioFMSequenceCommand(int arpNum, int type, int data);
	void removeArpeggioFMSequenceCommand(int arpNum);
	void setArpeggioFMSequenceCommand(int arpNum, int cnt, int type, int data);
	std::vector<CommandInSequence> getArpeggioFMSequence(int arpNum);
	void setArpeggioFMLoops(int arpNum, std::vector<int> begins, std::vector<int> ends, std::vector<int> times);
	std::vector<Loop> getArpeggioFMLoops(int arpNum) const;
	void setArpeggioFMRelease(int arpNum, ReleaseType type, int begin);
	Release getArpeggioFMRelease(int arpNum) const;
	std::unique_ptr<CommandSequence::Iterator> getArpeggioFMIterator(int arpNum) const;
	std::vector<int> getArpeggioFMUsers(int arpNum) const;
	std::vector<int> getArpeggioFMEntriedIndices() const;
	int findFirstFreeArpeggioFM() const;

	void setInstrumentFMPitchEnabled(int instNum, bool enabled);
	bool getInstrumentFMPitchEnabled(int instNum) const;
	void setInstrumentFMPitch(int instNum, int ptNum);
	int getInstrumentFMPitch(int instNum);
	void setPitchFMType(int ptNum, int type);
	int getPitchFMType(int ptNum) const;
	void addPitchFMSequenceCommand(int ptNum, int type, int data);
	void removePitchFMSequenceCommand(int ptNum);
	void setPitchFMSequenceCommand(int ptNum, int cnt, int type, int data);
	std::vector<CommandInSequence> getPitchFMSequence(int ptNum);
	void setPitchFMLoops(int ptNum, std::vector<int> begins, std::vector<int> ends, std::vector<int> times);
	std::vector<Loop> getPitchFMLoops(int ptNum) const;
	void setPitchFMRelease(int ptNum, ReleaseType type, int begin);
	Release getPitchFMRelease(int ptNum) const;
	std::unique_ptr<CommandSequence::Iterator> getPitchFMIterator(int ptNum) const;
	std::vector<int> getPitchFMUsers(int ptNum) const;
	std::vector<int> getPitchFMEntriedIndices() const;
	int findFirstFreePitchFM() const;

	void setInstrumentFMEnvelopeResetEnabled(int instNum, bool enabled);

private:
	std::array<std::shared_ptr<EnvelopeFM>, 128> envFM_;
	std::array<std::shared_ptr<LFOFM>, 128> lfoFM_;
	std::map<FMEnvelopeParameter, std::array<std::shared_ptr<CommandSequence>, 128>> opSeqFM_;
	std::array<std::shared_ptr<CommandSequence>, 128> arpFM_;
	std::array<std::shared_ptr<CommandSequence>, 128> ptFM_;

	std::vector<FMEnvelopeParameter> envFMParams_;

	int cloneFMEnvelope(int srcNum);
	int cloneFMLFO(int srcNum);
	int cloneFMOperatorSequence(FMEnvelopeParameter param, int srcNum);
	int cloneFMArpeggio(int srcNum);
	int cloneFMPitch(int srcNum);

	//----- SSG methods -----
public:
	void setInstrumentSSGWaveFormEnabled(int instNum, bool enabled);
	bool getInstrumentSSGWaveFormEnabled(int instNum) const;
	void setInstrumentSSGWaveForm(int instNum, int wfNum);
	int getInstrumentSSGWaveForm(int instNum);
	void addWaveFormSSGSequenceCommand(int wfNum, int type, int data);
	void removeWaveFormSSGSequenceCommand(int wfNum);
	void setWaveFormSSGSequenceCommand(int wfNum, int cnt, int type, int data);
	std::vector<CommandInSequence> getWaveFormSSGSequence(int wfNum);
	void setWaveFormSSGLoops(int wfNum, std::vector<int> begins, std::vector<int> ends, std::vector<int> times);
	std::vector<Loop> getWaveFormSSGLoops(int wfNum) const;
	void setWaveFormSSGRelease(int wfNum, ReleaseType type, int begin);
	Release getWaveFormSSGRelease(int wfNum) const;
	std::unique_ptr<CommandSequence::Iterator> getWaveFormSSGIterator(int wfNum) const;
	std::vector<int> getWaveFormSSGUsers(int wfNum) const;
	std::vector<int> getWaveFormSSGEntriedIndices() const;
	int findFirstFreeWaveFormSSG() const;

	void setInstrumentSSGToneNoiseEnabled(int instNum, bool enabled);
	bool getInstrumentSSGToneNoiseEnabled(int instNum) const;
	void setInstrumentSSGToneNoise(int instNum, int tnNum);
	int getInstrumentSSGToneNoise(int instNum);
	void addToneNoiseSSGSequenceCommand(int tnNum, int type, int data);
	void removeToneNoiseSSGSequenceCommand(int tnNum);
	void setToneNoiseSSGSequenceCommand(int tnNum, int cnt, int type, int data);
	std::vector<CommandInSequence> getToneNoiseSSGSequence(int tnNum);
	void setToneNoiseSSGLoops(int tnNum, std::vector<int> begins, std::vector<int> ends, std::vector<int> times);
	std::vector<Loop> getToneNoiseSSGLoops(int tnNum) const;
	void setToneNoiseSSGRelease(int tnNum, ReleaseType type, int begin);
	Release getToneNoiseSSGRelease(int tnNum) const;
	std::unique_ptr<CommandSequence::Iterator> getToneNoiseSSGIterator(int tnNum) const;
	std::vector<int> getToneNoiseSSGUsers(int tnNum) const;
	std::vector<int> getToneNoiseSSGEntriedIndices() const;
	int findFirstFreeToneNoiseSSG() const;

	void setInstrumentSSGEnvelopeEnabled(int instNum, bool enabled);
	bool getInstrumentSSGEnvelopeEnabled(int instNum) const;
	void setInstrumentSSGEnvelope(int instNum, int envNum);
	int getInstrumentSSGEnvelope(int instNum);
	void addEnvelopeSSGSequenceCommand(int envNum, int type, int data);
	void removeEnvelopeSSGSequenceCommand(int envNum);
	void setEnvelopeSSGSequenceCommand(int envNum, int cnt, int type, int data);
	std::vector<CommandInSequence> getEnvelopeSSGSequence(int envNum);
	void setEnvelopeSSGLoops(int envNum, std::vector<int> begins, std::vector<int> ends, std::vector<int> times);
	std::vector<Loop> getEnvelopeSSGLoops(int envNum) const;
	void setEnvelopeSSGRelease(int envNum, ReleaseType type, int begin);
	Release getEnvelopeSSGRelease(int envNum) const;
	std::unique_ptr<CommandSequence::Iterator> getEnvelopeSSGIterator(int envNum) const;
	std::vector<int> getEnvelopeSSGUsers(int envNum) const;
	std::vector<int> getEnvelopeSSGEntriedIndices() const;
	int findFirstFreeEnvelopeSSG() const;

	void setInstrumentSSGArpeggioEnabled(int instNum, bool enabled);
	bool getInstrumentSSGArpeggioEnabled(int instNum) const;
	void setInstrumentSSGArpeggio(int instNum, int arpNum);
	int getInstrumentSSGArpeggio(int instNum);
	void setArpeggioSSGType(int arpNum, int type);
	int getArpeggioSSGType(int arpNum) const;
	void addArpeggioSSGSequenceCommand(int arpNum, int type, int data);
	void removeArpeggioSSGSequenceCommand(int arpNum);
	void setArpeggioSSGSequenceCommand(int arpNum, int cnt, int type, int data);
	std::vector<CommandInSequence> getArpeggioSSGSequence(int arpNum);
	void setArpeggioSSGLoops(int arpNum, std::vector<int> begins, std::vector<int> ends, std::vector<int> times);
	std::vector<Loop> getArpeggioSSGLoops(int arpNum) const;
	void setArpeggioSSGRelease(int arpNum, ReleaseType type, int begin);
	Release getArpeggioSSGRelease(int arpNum) const;
	std::unique_ptr<CommandSequence::Iterator> getArpeggioSSGIterator(int arpNum) const;
	std::vector<int> getArpeggioSSGUsers(int arpNum) const;
	std::vector<int> getArpeggioSSGEntriedIndices() const;
	int findFirstFreeArpeggioSSG() const;

	void setInstrumentSSGPitchEnabled(int instNum, bool enabled);
	bool getInstrumentSSGPitchEnabled(int instNum) const;
	void setInstrumentSSGPitch(int instNum, int ptNum);
	int getInstrumentSSGPitch(int instNum);
	void setPitchSSGType(int ptNum, int type);
	int getPitchSSGType(int ptNum) const;
	void addPitchSSGSequenceCommand(int ptNum, int type, int data);
	void removePitchSSGSequenceCommand(int ptNum);
	void setPitchSSGSequenceCommand(int ptNum, int cnt, int type, int data);
	std::vector<CommandInSequence> getPitchSSGSequence(int ptNum);
	void setPitchSSGLoops(int ptNum, std::vector<int> begins, std::vector<int> ends, std::vector<int> times);
	std::vector<Loop> getPitchSSGLoops(int ptNum) const;
	void setPitchSSGRelease(int ptNum, ReleaseType type, int begin);
	Release getPitchSSGRelease(int ptNum) const;
	std::unique_ptr<CommandSequence::Iterator> getPitchSSGIterator(int ptNum) const;
	std::vector<int> getPitchSSGUsers(int ptNum) const;
	std::vector<int> getPitchSSGEntriedIndices() const;
	int findFirstFreePitchSSG() const;

private:
	std::array<std::shared_ptr<CommandSequence>, 128> wfSSG_;
	std::array<std::shared_ptr<CommandSequence>, 128> envSSG_;
	std::array<std::shared_ptr<CommandSequence>, 128> tnSSG_;
	std::array<std::shared_ptr<CommandSequence>, 128> arpSSG_;
	std::array<std::shared_ptr<CommandSequence>, 128> ptSSG_;

	int cloneSSGWaveForm(int srcNum);
	int cloneSSGToneNoise(int srcNum);
	int cloneSSGEnvelope(int srcNum);
	int cloneSSGArpeggio(int srcNum);
	int cloneSSGPitch(int srcNum);
};
