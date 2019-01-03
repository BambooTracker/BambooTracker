#pragma once

#include <cstdint>
#include <memory>
#include <map>
#include <deque>
#include "opna.hpp"
#include "instrument.hpp"
#include "effect_iterator.hpp"
#include "chips/scci/scci.h"
#include "misc.hpp"

struct ToneDetail
{
	int octave;
	Note note;
	int pitch;
};

struct ToneNoise
{
	bool isTone_, isNoise_;
	int noisePeriod_;
};

class OPNAController
{
public:
	OPNAController(int clock, int rate, int duration);

	// Reset and initialize
	void reset();

	// Forward instrument sequence
	void tickEvent(SoundSource src, int ch, bool isStep = false);

	// Stream type
	void useSCCI(SoundInterfaceManager* manager);
	bool isUsedSCCI() const;

	// Stream samples
	void getStreamSamples(int16_t* container, size_t nSamples);

	// Stream details
	int getRate() const;
	void setRate(int rate);
	int getDuration() const;
	void setDuration(int duration);
	void setMasterVolume(int percentage);

	// Export
	void setExportContainer(std::shared_ptr<chip::ExportContainerInterface> cntr = nullptr);


private:
	std::unique_ptr<chip::OPNA> opna_;

	void initChip();

	/*----- FM -----*/
public:
	// Key on-off
	void keyOnFM(int ch, Note note, int octave, int pitch, bool isJam = false);
	void keyOnFM(int ch, int echoBuf);
	void keyOffFM(int ch, bool isJam = false);
	void resetFMChannelEnvelope(int ch);

	// Set Instrument
	void setInstrumentFM(int ch, std::shared_ptr<InstrumentFM> inst);
	void updateInstrumentFM(int instNum);
	void updateInstrumentFMEnvelopeParameter(int envNum, FMEnvelopeParameter param);
	void setInstrumentFMOperatorEnabled(int envNum, int opNum);
	void updateInstrumentFMLFOParameter(int lfoNum, FMLFOParameter param);

	// Set volume
	void setVolumeFM(int ch, int volume);
	void setTemporaryVolumeFM(int ch, int volume);
	void setMasterVolumeFM(double dB);

	// Set pan
	void setPanFM(int ch, int value);

	// Set effect
	void setArpeggioEffectFM(int ch, int second, int third);
	void setPortamentoEffectFM(int ch, int depth, bool isTonePortamento = false);
	void setVibratoEffectFM(int ch, int period, int depth);
	void setTremoloEffectFM(int ch, int period, int depth);
	void setVolumeSlideFM(int ch, int depth, bool isUp);
	void setDetuneFM(int ch, int pitch);
	void setNoteSlideFM(int ch, int speed, int seminote);
	void setTransposeEffectFM(int ch, int seminote);

	// Mute
	void setMuteFMState(int ch, bool isMuteFM);
	bool isMuteFM(int ch);

	// Chip details
	bool isKeyOnFM(int ch) const;
	bool isTonePortamentoFM(int ch) const;
	bool enableFMEnvelopeReset(int ch) const;
	ToneDetail getFMTone(int ch) const;

private:
	std::shared_ptr<InstrumentFM> refInstFM_[6];
	std::unique_ptr<EnvelopeFM> envFM_[6];
	bool isKeyOnFM_[6];
	uint8_t fmOpEnables_[6];
	std::deque<ToneDetail> baseToneFM_[6];
	ToneDetail keyToneFM_[6];
	int sumPitchFM_[6];
	int baseVolFM_[6], tmpVolFM_[6];
	/// bit0: right on/off
	/// bit1: left on/off
	uint8_t panFM_[6];
	bool isMuteFM_[6];
	bool enableEnvResetFM_[6];
	int lfoFreq_;
	int lfoStartCntFM_[6];
	bool hasPreSetTickEventFM_[6];
	bool needToneSetFM_[6];
	std::map<FMEnvelopeParameter, std::unique_ptr<CommandSequence::Iterator>> opSeqItFM_[6];
	std::unique_ptr<SequenceIteratorInterface> arpItFM_[6];
	std::unique_ptr<CommandSequence::Iterator> ptItFM_[6];
	bool isArpEffFM_[6];
	int prtmFM_[6];
	bool isTonePrtmFM_[6];
	std::unique_ptr<WavingEffectIterator> vibItFM_[6];
	std::unique_ptr<WavingEffectIterator> treItFM_[6];
	int volSldFM_[6], sumVolSldFM_[6];
	int detuneFM_[6];
	std::unique_ptr<NoteSlideEffectIterator> nsItFM_[6];
	int sumNoteSldFM_[6];
	bool noteSldFMSetFlag_;
	int transposeFM_[6];

	void initFM();

	uint32_t getFmChannelMask(int ch);
	uint32_t getFMChannelOffset(int ch);

	void updateEchoBufferFM(int ch, int octave, Note note, int pitch);

	void writeFMEnvelopeToRegistersFromInstrument(int ch);
	void writeFMEnveropeParameterToRegister(int ch, FMEnvelopeParameter param, int value);

	void writeFMLFOAllRegisters(int ch);
	void writeFMLFORegister(int ch, FMLFOParameter param);
	void checkLFOUsed();

	void setFrontFMSequences(int ch);
	void releaseStartFMSequences(int ch);
	void tickEventFM(int ch, bool isStep);

	void checkOperatorSequenceFM(int ch, int type);
	void checkVolumeEffectFM(int ch);
	void checkRealToneFMByArpeggio(int ch, int seqPos);
	void checkPortamentoFM(int ch);
	void checkRealToneFMByPitch(int ch, int seqPos);
	void writePitchFM(int ch);

	void setInstrumentFMProperties(int ch);

	bool isCareer(int op, int al);

	inline uint8_t calculateTL(int ch, uint8_t data) const
	{
		int v = (tmpVolFM_[ch] == -1) ? baseVolFM_[ch] : tmpVolFM_[ch];
		return (data > 127 - v) ? 127 : (data + v);
	}

	/*----- SSG -----*/
public:
	// Key on-off
	void keyOnSSG(int ch, Note note, int octave, int pitch, bool isJam = false);
	void keyOnSSG(int ch, int echoBuf);
	void keyOffSSG(int ch, bool isJam = false);

	// Set Instrument
	void setInstrumentSSG(int ch, std::shared_ptr<InstrumentSSG> inst);
	void updateInstrumentSSG(int instNum);

	// Set volume
	void setVolumeSSG(int ch, int volume);
	void setTemporaryVolumeSSG(int ch, int volume);
	void setMasterVolumeSSG(double dB);

	// Set effect
	void setArpeggioEffectSSG(int ch, int second, int third);
	void setPortamentoEffectSSG(int ch, int depth, bool isTonePortamento = false);
	void setVibratoEffectSSG(int ch, int period, int depth);
	void setTremoloEffectSSG(int ch, int period, int depth);
	void setVolumeSlideSSG(int ch, int depth, bool isUp);
	void setDetuneSSG(int ch, int pitch);
	void setNoteSlideSSG(int ch, int speed, int seminote);
	void setTransposeEffectSSG(int ch, int seminote);

	// Mute
	void setMuteSSGState(int ch, bool isMuteFM);
	bool isMuteSSG(int ch);

	// Chip details
	bool isKeyOnSSG(int ch) const;
	bool isTonePortamentoSSG(int ch) const;
	ToneDetail getSSGTone(int ch) const;

private:
	std::shared_ptr<InstrumentSSG> refInstSSG_[3];
	bool isKeyOnSSG_[3];
	uint8_t mixerSSG_;
	std::deque<ToneDetail> baseToneSSG_[3];
	ToneDetail keyToneSSG_[3];
	int sumPitchSSG_[3];
	ToneNoise tnSSG_[3];
	int baseVolSSG_[3], tmpVolSSG_[3];
	bool isBuzzEffSSG_[3];
	bool isHardEnvSSG_[3];
	bool isMuteSSG_[3];
	bool hasPreSetTickEventSSG_[3];
	bool needEnvSetSSG_[3];
	bool needMixSetSSG_[3];
	bool needToneSetSSG_[3];
	std::unique_ptr<CommandSequence::Iterator> wfItSSG_[3];
	CommandInSequence wfSSG_[3];
	std::unique_ptr<CommandSequence::Iterator> envItSSG_[3];
	CommandInSequence envSSG_[3];
	std::unique_ptr<CommandSequence::Iterator> tnItSSG_[3];
	std::unique_ptr<SequenceIteratorInterface> arpItSSG_[3];
	std::unique_ptr<CommandSequence::Iterator> ptItSSG_[3];
	bool isArpEffSSG_[3];
	int prtmSSG_[3];
	bool isTonePrtmSSG_[3];
	std::unique_ptr<WavingEffectIterator> vibItSSG_[3];
	std::unique_ptr<WavingEffectIterator> treItSSG_[3];
	int volSldSSG_[3], sumVolSldSSG_[3];
	int detuneSSG_[3];
	std::unique_ptr<NoteSlideEffectIterator> nsItSSG_[3];
	int sumNoteSldSSG_[3];
	bool noteSldSSGSetFlag_;
	int transposeSSG_[3];

	void initSSG();

	void updateEchoBufferSSG(int ch, int octave, Note note, int pitch);

	void setFrontSSGSequences(int ch);
	void releaseStartSSGSequences(int ch);
	void tickEventSSG(int ch, bool isStep);

	void writeWaveFormSSGToRegister(int ch, int seqPos);
	void writeSquareWaveForm(int ch);

	void writeToneNoiseSSGToRegister(int ch, int seqPos);
	void writeToneNoiseSSGToRegisterNoReference(int ch);

	void writeEnvelopeSSGToRegister(int ch, int seqPos);
	void checkRealToneSSGByArpeggio(int ch, int seqPos);
	void checkPortamentoSSG(int ch);
	void checkRealToneSSGByPitch(int ch, int seqPos);
	void writePitchSSG(int ch);

	void setRealVolumeSSG(int ch);

	inline uint8_t judgeSSEGRegisterValue(int v) {
		return (v == -1) ? 0 : (0x08 + v);
	}

	/*----- Drum -----*/
public:
	// Key on-off
	void keyOnDrum(int ch);
	void keyOffDrum(int ch);

	// Set volume
	void setVolumeDrum(int ch, int volume);
	void setMasterVolumeDrum(int volume);
	void setTemporaryVolumeDrum(int ch, int volume);

	// Set pan
	void setPanDrum(int ch, int value);

	// Mute
	void setMuteDrumState(int ch, bool isMute);
	bool isMuteDrum(int ch);

private:
	int volDrum_[6], mVolDrum_, tmpVolDrum_[6];
	/// bit0: right on/off
	/// bit1: left on/off
	uint8_t panDrum_[6];
	bool isMuteDrum_[6];

	void initDrum();
};
