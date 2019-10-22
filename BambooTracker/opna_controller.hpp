#pragma once

#include <cstdint>
#include <memory>
#include <map>
#include <deque>
#include "opna.hpp"
#include "instrument.hpp"
#include "effect_iterator.hpp"
#include "chips/chip_misc.h"
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

struct WaveForm
{
	SSGWaveFormType type;
	int data;	// Same format with CommandSequenceUnit::data
};

class OPNAController
{
public:
	OPNAController(chip::Emu emu, int clock, int rate, int duration);

	// Reset and initialize
	void reset();

	// Forward instrument sequence
	void tickEvent(SoundSource src, int ch);

	// Stream type
	void useSCCI(SoundInterfaceManager* manager);
	bool isUsedSCCI() const;

	// Stream samples
	void getStreamSamples(int16_t* container, size_t nSamples);
	void getOutputHistory(int16_t* history);

	enum { OutputHistorySize = 1024 };

	// Chip mode
	void setMode(SongType mode);
	SongType getMode() const;

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
	SongType mode_;

	void initChip();

	void fillOutputHistory(const int16_t* outputs, size_t nSamples);
	void transferReadyHistory();

	/*----- FM -----*/
public:
	// Key on-off
	void keyOnFM(int ch, Note note, int octave, int pitch, bool isJam = false);
	void keyOnFM(int ch, int echoBuf);
	void keyOffFM(int ch, bool isJam = false);
	void updateEchoBufferFM(int ch, int octave, Note note, int pitch);

	// Set Instrument
	void setInstrumentFM(int ch, std::shared_ptr<InstrumentFM> inst);
	void updateInstrumentFM(int instNum);
	void updateInstrumentFMEnvelopeParameter(int envNum, FMEnvelopeParameter param);
	void setInstrumentFMOperatorEnabled(int envNum, int opNum);
	void updateInstrumentFMLFOParameter(int lfoNum, FMLFOParameter param);
	void resetFMChannelEnvelope(int ch);
	void restoreFMEnvelopeFromReset(int ch);

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

	// For state retrieve
	void haltSequencesFM(int ch);

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
	bool isKeyOnFM_[9];
	uint8_t fmOpEnables_[6];
	std::deque<ToneDetail> baseToneFM_[9];
	ToneDetail keyToneFM_[9];
	int sumPitchFM_[9];
	int baseVolFM_[9], tmpVolFM_[9];
	/// bit0: right on/off
	/// bit1: left on/off
	uint8_t panFM_[6];
	bool isMuteFM_[9];
	bool enableEnvResetFM_[9], hasResetEnvFM_[9];
	int lfoFreq_;
	int lfoStartCntFM_[6];
	bool hasPreSetTickEventFM_[9];
	bool needToneSetFM_[9];
	std::map<FMEnvelopeParameter, std::unique_ptr<CommandSequence::Iterator>> opSeqItFM_[6];
	std::unique_ptr<SequenceIteratorInterface> arpItFM_[9];
	std::unique_ptr<CommandSequence::Iterator> ptItFM_[9];
	bool isArpEffFM_[9];
	int prtmFM_[9];
	bool isTonePrtmFM_[9];
	std::unique_ptr<WavingEffectIterator> vibItFM_[9];
	std::unique_ptr<WavingEffectIterator> treItFM_[9];
	int volSldFM_[9], sumVolSldFM_[9];
	int detuneFM_[9];
	std::unique_ptr<NoteSlideEffectIterator> nsItFM_[9];
	int sumNoteSldFM_[9];
	bool noteSldFMSetFlag_[9];
	int transposeFM_[9];

	void initFM();

	int toInternalFMChannel(int ch) const;
	uint8_t getFMKeyOnOffChannelMask(int ch) const;
	uint32_t getFMChannelOffset(int ch, bool forPitch = false) const;
	FMOperatorType toChannelOperatorType(int ch) const;
	std::vector<FMEnvelopeParameter> getFMEnvelopeParametersForOperator(FMOperatorType op) const;

	void updateFMVolume(int ch);

	void writeFMEnvelopeToRegistersFromInstrument(int inch);
	void writeFMEnveropeParameterToRegister(int inch, FMEnvelopeParameter param, int value);

	void writeFMLFOAllRegisters(int inch);
	void writeFMLFORegister(int inch, FMLFOParameter param);
	void checkLFOUsed();

	void setFrontFMSequences(int ch);
	void releaseStartFMSequences(int ch);
	void tickEventFM(int ch);

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
		return (data > 127 - v) ? 127 : static_cast<uint8_t>(data + v);
	}

	/*----- SSG -----*/
public:
	// Key on-off
	void keyOnSSG(int ch, Note note, int octave, int pitch, bool isJam = false);
	void keyOnSSG(int ch, int echoBuf);
	void keyOffSSG(int ch, bool isJam = false);
	void updateEchoBufferSSG(int ch, int octave, Note note, int pitch);

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
	void setToneNoiseMixSSG(int ch, int value);
	void setNoisePitchSSG(int ch, int pitch);
	void setHardEnvelopePeriod(int ch, bool high, int period);
	void setAutoEnvelopeSSG(int ch, int shift, int shape);

	// For state retrieve
	void haltSequencesSSG(int ch);

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
	bool setHardEnvIfNecessary_[3];
	bool needMixSetSSG_[3];
	bool needToneSetSSG_[3];
	bool needSqMaskFreqSetSSG_[3];
	std::unique_ptr<CommandSequence::Iterator> wfItSSG_[3];
	WaveForm wfSSG_[3];
	std::unique_ptr<CommandSequence::Iterator> envItSSG_[3];
	CommandSequenceUnit envSSG_[3];
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
	int toneNoiseMixSSG_[3];
	int noisePitchSSG_;
	int hardEnvPeriodHighSSG_, hardEnvPeriodLowSSG_;
	std::unique_ptr<int16_t[]> outputHistory_;
	size_t outputHistoryIndex_;
	std::unique_ptr<int16_t[]> outputHistoryReady_;
	std::mutex outputHistoryReadyMutex_;

	void initSSG();

	void setFrontSSGSequences(int ch);
	void releaseStartSSGSequences(int ch);
	void tickEventSSG(int ch);

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

	inline uint8_t judgeSSEGRegisterValue(int v)
	{
		return (v == -1) ? 0 : (0x08 + static_cast<uint8_t>(v));
	}

	/*----- Drum -----*/
public:
	// Key on-off
	void setKeyOnFlagDrum(int ch);
	void setKeyOffFlagDrum(int ch);
	void updateKeyOnOffStatusDrum();

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
	uint8_t keyOnFlagDrum_, keyOffFlagDrum_;
	int volDrum_[6], mVolDrum_, tmpVolDrum_[6];
	/// bit0: right on/off
	/// bit1: left on/off
	uint8_t panDrum_[6];
	bool isMuteDrum_[6];

	void initDrum();
};
