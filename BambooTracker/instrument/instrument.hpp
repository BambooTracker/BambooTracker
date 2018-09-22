#pragma once

#include <string>
#include <memory>
#include <map>
#include <vector>
#include "instruments_manager.hpp"
#include "envelope_fm.hpp"
#include "lfo_fm.hpp"
#include "command_sequence.hpp"
#include "misc.hpp"

class InstrumentsManager;

class AbstructInstrument
{
public:
	virtual ~AbstructInstrument() = default;

	int getNumber() const;
	void setNumber(int n);
	SoundSource getSoundSource() const;
	std::string getName() const;
	void setName(std::string name);
	virtual std::unique_ptr<AbstructInstrument> clone() = 0;

	void setGateCount(int count);
	int getGateCount() const;

protected:
	InstrumentsManager* owner_;
    std::string name_;	// UTF-8
	AbstructInstrument(int number, SoundSource source, std::string name, InstrumentsManager* owner);

private:
	int number_;
    SoundSource source_;

	int gateCount_;
};


class InstrumentFM : public AbstructInstrument
{
public:
	InstrumentFM(int number, std::string name, InstrumentsManager* owner);
	std::unique_ptr<AbstructInstrument> clone() override;

	void setEnvelopeNumber(int n);
	int getEnvelopeNumber() const;
	int getEnvelopeParameter(FMEnvelopeParameter param) const;
	bool getOperatorEnabled(int n) const;

	void setLFONumber(int n);
	int getLFONumber() const;
	int getLFOParameter(FMLFOParameter param) const;

	void setOperatorSequenceNumber(FMEnvelopeParameter param, int n);
	int getOperatorSequenceNumber(FMEnvelopeParameter param) const;
	std::vector<CommandInSequence> getOperatorSequenceSequence(FMEnvelopeParameter param) const;
	std::vector<Loop> getOperatorSequenceLoops(FMEnvelopeParameter param) const;
	Release getOperatorSequenceRelease(FMEnvelopeParameter param) const;
	std::unique_ptr<CommandSequence::Iterator> getOperatorSequenceSequenceIterator(FMEnvelopeParameter param) const;

	void setArpeggioNumber(int n);
	int getArpeggioNumber() const;
	int getArpeggioType() const;
	std::vector<CommandInSequence> getArpeggioSequence() const;
	std::vector<Loop> getArpeggioLoops() const;
	Release getArpeggioRelease() const;
	std::unique_ptr<CommandSequence::Iterator> getArpeggioSequenceIterator() const;

	void setPitchNumber(int n);
	int getPitchNumber() const;
	int getPitchType() const;
	std::vector<CommandInSequence> getPitchSequence() const;
	std::vector<Loop> getPitchLoops() const;
	Release getPitchRelease() const;
	std::unique_ptr<CommandSequence::Iterator> getPitchSequenceIterator() const;

	void setEnvelopeResetEnabled(bool enabled);
	bool getEnvelopeResetEnabled() const;

private:
	int envNum_;
	int lfoNum_;
	std::map<FMEnvelopeParameter, int> opSeqNum_;
	int arpNum_;
	int ptNum_;

	bool envResetEnabled_;
};


class InstrumentSSG : public AbstructInstrument
{
public:
	InstrumentSSG(int number, std::string name, InstrumentsManager* owner);
	std::unique_ptr<AbstructInstrument> clone() override;

	void setWaveFormNumber(int n);
	int getWaveFormNumber() const;
	std::vector<CommandInSequence> getWaveFormSequence() const;
	std::vector<Loop> getWaveFormLoops() const;
	Release getWaveFormRelease() const;
	std::unique_ptr<CommandSequence::Iterator> getWaveFormSequenceIterator() const;

	void setToneNoiseNumber(int n);
	int getToneNoiseNumber() const;
	std::vector<CommandInSequence> getToneNoiseSequence() const;
	std::vector<Loop> getToneNoiseLoops() const;
	Release getToneNoiseRelease() const;
	std::unique_ptr<CommandSequence::Iterator> getToneNoiseSequenceIterator() const;

	void setEnvelopeNumber(int n);
	int getEnvelopeNumber() const;
	std::vector<CommandInSequence> getEnvelopeSequence() const;
	std::vector<Loop> getEnvelopeLoops() const;
	Release getEnvelopeRelease() const;
	std::unique_ptr<CommandSequence::Iterator> getEnvelopeSequenceIterator() const;

	void setArpeggioNumber(int n);
	int getArpeggioNumber() const;
	int getArpeggioType() const;
	std::vector<CommandInSequence> getArpeggioSequence() const;
	std::vector<Loop> getArpeggioLoops() const;
	Release getArpeggioRelease() const;
	std::unique_ptr<CommandSequence::Iterator> getArpeggioSequenceIterator() const;

	void setPitchNumber(int n);
	int getPitchNumber() const;
	int getPitchType() const;
	std::vector<CommandInSequence> getPitchSequence() const;
	std::vector<Loop> getPitchLoops() const;
	Release getPitchRelease() const;
	std::unique_ptr<CommandSequence::Iterator> getPitchSequenceIterator() const;

private:
	int wfNum_;
	int tnNum_;
	int envNum_;
	int arpNum_;
	int ptNum_;
};
