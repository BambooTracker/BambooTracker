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

class AbstractInstrument
{
public:
	virtual ~AbstractInstrument() = default;

	int getNumber() const;
	void setNumber(int n);
	SoundSource getSoundSource() const;
	std::string getName() const;
	void setName(std::string name);
	bool isRegisteredWithManager() const;
	virtual std::unique_ptr<AbstractInstrument> clone() = 0;

protected:
	InstrumentsManager* owner_;
    std::string name_;	// UTF-8
	AbstractInstrument(int number, SoundSource source, std::string name, InstrumentsManager* owner);

private:
	int number_;
	SoundSource source_;
};


class InstrumentFM : public AbstractInstrument
{
public:
	InstrumentFM(int number, std::string name, InstrumentsManager* owner);
	std::unique_ptr<AbstractInstrument> clone() override;

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
	std::vector<CommandInSequence> getOperatorSequenceSequence(FMEnvelopeParameter param) const;
	std::vector<Loop> getOperatorSequenceLoops(FMEnvelopeParameter param) const;
	Release getOperatorSequenceRelease(FMEnvelopeParameter param) const;
	std::unique_ptr<CommandSequence::Iterator> getOperatorSequenceSequenceIterator(FMEnvelopeParameter param) const;

	void setArpeggioEnabled(bool enabled);
	bool getArpeggioEnabled() const;
	void setArpeggioNumber(int n);
	int getArpeggioNumber() const;
	int getArpeggioType() const;
	std::vector<CommandInSequence> getArpeggioSequence() const;
	std::vector<Loop> getArpeggioLoops() const;
	Release getArpeggioRelease() const;
	std::unique_ptr<CommandSequence::Iterator> getArpeggioSequenceIterator() const;

	void setPitchEnabled(bool enabled);
	bool getPitchEnabled() const;
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
	bool lfoEnabled_;
	int lfoNum_;
	std::map<FMEnvelopeParameter, bool> opSeqEnabled_;
	std::map<FMEnvelopeParameter, int> opSeqNum_;
	bool arpEnabled_;
	int arpNum_;
	bool ptEnabled_;
	int ptNum_;

	bool envResetEnabled_;
};


class InstrumentSSG : public AbstractInstrument
{
public:
	InstrumentSSG(int number, std::string name, InstrumentsManager* owner);
	std::unique_ptr<AbstractInstrument> clone() override;

	void setWaveFormEnabled(bool enabled);
	bool getWaveFormEnabled() const;
	void setWaveFormNumber(int n);
	int getWaveFormNumber() const;
	std::vector<CommandInSequence> getWaveFormSequence() const;
	std::vector<Loop> getWaveFormLoops() const;
	Release getWaveFormRelease() const;
	std::unique_ptr<CommandSequence::Iterator> getWaveFormSequenceIterator() const;

	void setToneNoiseEnabled(bool enabled);
	bool getToneNoiseEnabled() const;
	void setToneNoiseNumber(int n);
	int getToneNoiseNumber() const;
	std::vector<CommandInSequence> getToneNoiseSequence() const;
	std::vector<Loop> getToneNoiseLoops() const;
	Release getToneNoiseRelease() const;
	std::unique_ptr<CommandSequence::Iterator> getToneNoiseSequenceIterator() const;

	void setEnvelopeEnabled(bool enabled);
	bool getEnvelopeEnabled() const;
	void setEnvelopeNumber(int n);
	int getEnvelopeNumber() const;
	std::vector<CommandInSequence> getEnvelopeSequence() const;
	std::vector<Loop> getEnvelopeLoops() const;
	Release getEnvelopeRelease() const;
	std::unique_ptr<CommandSequence::Iterator> getEnvelopeSequenceIterator() const;

	void setArpeggioEnabled(bool enabled);
	bool getArpeggioEnabled() const;
	void setArpeggioNumber(int n);
	int getArpeggioNumber() const;
	int getArpeggioType() const;
	std::vector<CommandInSequence> getArpeggioSequence() const;
	std::vector<Loop> getArpeggioLoops() const;
	Release getArpeggioRelease() const;
	std::unique_ptr<CommandSequence::Iterator> getArpeggioSequenceIterator() const;

	void setPitchEnabled(bool enabled);
	bool getPitchEnabled() const;
	void setPitchNumber(int n);
	int getPitchNumber() const;
	int getPitchType() const;
	std::vector<CommandInSequence> getPitchSequence() const;
	std::vector<Loop> getPitchLoops() const;
	Release getPitchRelease() const;
	std::unique_ptr<CommandSequence::Iterator> getPitchSequenceIterator() const;

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
