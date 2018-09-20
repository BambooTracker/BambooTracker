#pragma once

#include <string>
#include <memory>
#include <array>
#include <vector>
#include "instrument.hpp"
#include "envelope_fm.hpp"
#include "lfo_fm.hpp"
#include "command_sequence.hpp"
#include "misc.hpp"

class AbstructInstrument;
enum class FMEnvelopeParameter;
class EnvelopeFM;

class InstrumentsManager
{
public:
	InstrumentsManager();

	void addInstrument(int instNum, SoundSource source, std::string name);
	void addInstrument(std::unique_ptr<AbstructInstrument> inst);
	std::unique_ptr<AbstructInstrument> removeInstrument(int instNum);
	void cloneInstrument(int cloneInstNum, int resInstNum);
	void deepCloneInstrument(int cloneInstNum, int resInstNum);
	std::shared_ptr<AbstructInstrument> getInstrumentSharedPtr(int instNum);

	void setInstrumentName(int instNum, std::string name);
	std::string getInstrumentName(int instNum) const;

	int findFirstFreeInstrument() const;

	void setInstrumentGateCount(int instNum, int count);

private:
	std::array<std::shared_ptr<AbstructInstrument>, 128> insts_;

	//----- FM methods -----
public:
	void setInstrumentFMEnvelope(int instNum, int envNum);
    int getInstrumentFMEnvelope(int instNum) const;
	void setEnvelopeFMParameter(int envNum, FMEnvelopeParameter param, int value);
	int getEnvelopeFMParameter(int envNum, FMEnvelopeParameter param) const;
	void setEnvelopeFMOperatorEnabled(int envNum, int opNum, bool enabled);
	bool getEnvelopeFMOperatorEnabled(int envNum, int opNum) const;
	std::vector<int> getEnvelopeFMUsers(int envNum) const;

	void setInstrumentFMLFO(int instNum, int lfoNum);
	int getInstrumentFMLFO(int instNum) const;
	void setLFOFMParameter(int lfoNum, FMLFOParamter param, int value);
	int getLFOFMparameter(int lfoNum, FMLFOParamter param) const;
	std::vector<int> getLFOFMUsers(int lfoNum) const;

	void setInstrumentFMEnvelopeResetEnabled(int instNum, bool enabled);

private:
	std::array<std::shared_ptr<EnvelopeFM>, 128> envFM_;
	std::array<std::shared_ptr<LFOFM>, 128> lfoFM_;

	int cloneFMEnvelope(int srcNum);
	int cloneFMLFO(int srcNum);

	//----- SSG methods -----
public:
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

	void setInstrumentSSGArpeggio(int instNum, int arpNum);
	int getInstrumentSSGArpeggio(int instNum);
	void setArpeggioType(int arpNum, int type);
	int getArpeggioType(int arpNum) const;
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

	void setInstrumentSSGPitch(int instNum, int ptNum);
	int getInstrumentSSGPitch(int instNum);
	void setPitchType(int ptNum, int type);
	int getPitchType(int ptNum) const;
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
