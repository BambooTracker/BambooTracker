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

#include <cstdint>
#include <memory>
#include <unordered_map>
#include "song.hpp"
#include "instrument.hpp"
#include "effect_iterator.hpp"
#include "note.hpp"
#include "echo_buffer.hpp"
#include "chip/opna.hpp"
#include "chip/scci/scci.hpp"
#include "chip/c86ctl/c86ctl_wrapper.hpp"
#include "enum_hash.hpp"
#include "opna_defs.hpp"

struct SSGToneNoise
{
	bool isTone, isNoise;
	int noisePeriod_;
};

class OPNAController
{
public:
	OPNAController(chip::OpnaEmulator emu, int clock, int rate, int duration);

	// Reset and initialize
	void reset();

	// Forward instrument sequence
	void tickEvent(SoundSource src, int ch);

	// Direct register set
	void sendRegister(int address, int value);

	// DRAM
	size_t getDRAMSize() const;

	// Update register states after tick process
	void updateRegisterStates();

	// Real chip interface
	void useSCCI(scci::SoundInterfaceManager* manager);
	bool isUsedSCCI() const;
	void useC86CTL(C86ctlBase* base);
	bool isUsedC86CTL() const;

	// Stream samples
	void getStreamSamples(int16_t* container, size_t nSamples);
	void getOutputHistory(int16_t* history);

	static constexpr int OUTPUT_HISTORY_SIZE = 1024;

	// Chip mode
	void setMode(SongType mode);
	SongType getMode() const;

	// Mute
	void setMuteState(SoundSource src, int chInSrc, bool isMute);
	bool isMute(SoundSource src, int chInSrc);

	// Stream details
	int getRate() const;
	void setRate(int rate);
	int getDuration() const;
	void setDuration(int duration);
	void setMasterVolume(int percentage);

	// Export
	void setExportContainer(std::shared_ptr<chip::AbstractRegisterWriteLogger> cntr = nullptr);

private:
	std::unique_ptr<chip::OPNA> opna_;
	SongType mode_;

	std::vector<std::pair<int, int>> registerSetBuf_;

	std::unique_ptr<int16_t[]> outputHistory_;
	size_t outputHistoryIndex_;
	std::unique_ptr<int16_t[]> outputHistoryReady_;
	std::mutex outputHistoryReadyMutex_;

	void initChip();

	void fillOutputHistory(const int16_t* outputs, size_t nSamples);
	void transferReadyHistory();

	using ArpeggioIterInterface = std::unique_ptr<SequenceIteratorInterface<ArpeggioUnit>>;
	void checkRealToneByArpeggio(const ArpeggioIterInterface& arpIt,
								 const EchoBuffer& echoBuf, Note& baseNote, bool& needToneSet);
	void checkPortamento(const ArpeggioIterInterface& arpIt, int prtm, bool hasKeyOnBefore,
						 bool isTonePrtm, EchoBuffer& echoBuf, Note& baseNote,
						 bool& needToneSet);
	void checkRealToneByPitch(const std::unique_ptr<InstrumentSequenceProperty<InstrumentSequenceBaseUnit>::Iterator>& ptIt,
							  int& sumPitch, bool& needToneSet);

	/*----- FM -----*/
public:
	// Key on-off
	void keyOnFM(int ch, const Note& note, bool isJam = false);
	void keyOnFM(int ch, int echoBuf);
	void keyOffFM(int ch, bool isJam = false);
	void updateEchoBufferFM(int ch, const Note& note);

	// Set instrument
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

	// Set effect
	void setPanFM(int ch, int value);
	void setArpeggioEffectFM(int ch, int second, int third);
	void setPortamentoEffectFM(int ch, int depth, bool isTonePortamento = false);
	void setVibratoEffectFM(int ch, int period, int depth);
	void setTremoloEffectFM(int ch, int period, int depth);
	void setVolumeSlideFM(int ch, int depth, bool isUp);
	void setDetuneFM(int ch, int pitch);
	void setFineDetuneFM(int ch, int pitch);
	void setNoteSlideFM(int ch, int speed, int seminote);
	void setTransposeEffectFM(int ch, int seminote);
	void setFBControlFM(int ch, int value);
	void setTLControlFM(int ch, int op, int value);
	void setMLControlFM(int ch, int op, int value);
	void setARControlFM(int ch, int op, int value);
	void setDRControlFM(int ch, int op, int value);
	void setRRControlFM(int ch, int op, int value);
	void setBrightnessFM(int ch, int value);

	// For state retrieve
	void haltSequencesFM(int ch);

	// Chip details
	bool isKeyOnFM(int ch) const;
	bool isTonePortamentoFM(int ch) const;
	bool enableFMEnvelopeReset(int ch) const;
	Note getFMTone(int ch) const;	// TODO: fix

private:
	std::shared_ptr<InstrumentFM> refInstFM_[6];
	std::unique_ptr<EnvelopeFM> envFM_[6];
	bool isKeyOnFM_[9], hasKeyOnBeforeFM_[9];
	uint8_t fmOpEnables_[6];
	EchoBuffer echoBufFM_[9];
	bool neverSetBaseNoteFM_[9];
	Note baseNoteFM_[9];
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
	std::unordered_map<FMEnvelopeParameter, FMOperatorSequenceIter> opSeqItFM_[6];
	ArpeggioIterInterface arpItFM_[9];
	PitchIter ptItFM_[9];
	bool isArpEffFM_[9];
	int prtmFM_[9];
	bool isTonePrtmFM_[9];
	std::unique_ptr<WavingEffectIterator> vibItFM_[9];
	std::unique_ptr<WavingEffectIterator> treItFM_[9];
	int volSldFM_[9], sumVolSldFM_[9];
	int detuneFM_[9], fdetuneFM_[9];
	std::unique_ptr<NoteSlideEffectIterator> nsItFM_[9];
	int sumNoteSldFM_[9];
	bool noteSldFMSetFlag_[9];
	int transposeFM_[9];
	bool isFBCtrlFM_[6], isTLCtrlFM_[6][4], isMLCtrlFM_[6][4], isARCtrlFM_[6][4];
	bool isDRCtrlFM_[6][4], isRRCtrlFM_[6][4];
	bool isBrightFM_[6][4];

	void initFM();

	void setMuteFMState(int ch, bool isMuteFM);
	bool isMuteFM(int ch);

	uint32_t getFMChannelOffset(int ch, bool forPitch = false) const;
	FMOperatorType toChannelOperatorType(int ch) const;
	const std::unordered_map<FMOperatorType, std::vector<FMEnvelopeParameter>> FM_ENV_PARAMS_OP_;

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

	inline void checkRealToneFMByArpeggio(int ch)
	{
		checkRealToneByArpeggio(arpItFM_[ch], echoBufFM_[ch], baseNoteFM_[ch], needToneSetFM_[ch]);
	}

	inline void checkPortamentoFM(int ch)
	{
		checkPortamento(arpItFM_[ch], prtmFM_[ch], hasKeyOnBeforeFM_[ch], isTonePrtmFM_[ch], echoBufFM_[ch],
						baseNoteFM_[ch], needToneSetFM_[ch]);
	}

	inline void checkRealToneFMByPitch(int ch)
	{
		checkRealToneByPitch(ptItFM_[ch], sumPitchFM_[ch], needToneSetFM_[ch]);
	}

	void writePitchFM(int ch);

	void setInstrumentFMProperties(int ch);

	static std::vector<int> getOperatorsInLevel(int level, int al);

	const bool CARRIER_JUDGE_[4][8] /* [operator][algorithm] */ = {
		{ false, false, false, false, false, false, false, true },
		{ false, false, false, false, true, true, true, true },
		{ false, false, false, false, false, true, true, true },
		{ true, true, true, true, true, true, true, true },
	};
	const uint8_t FM_KEYOFF_MASK_[6] = { 0, 1, 2, 4, 5, 6 };
	const std::unordered_map<int, uint8_t> FM3_KEY_OFF_MASK_ = { { 2, 0xe }, { 6, 0xd }, { 7, 0xb }, { 8, 0x7 }	};
	const FMEnvelopeParameter PARAM_TL_[4] = {
		FMEnvelopeParameter::TL1, FMEnvelopeParameter::TL2, FMEnvelopeParameter::TL3, FMEnvelopeParameter::TL4
	};
	const FMEnvelopeParameter PARAM_ML_[4] = {
		FMEnvelopeParameter::ML1, FMEnvelopeParameter::ML2, FMEnvelopeParameter::ML3, FMEnvelopeParameter::ML4
	};
	const FMEnvelopeParameter PARAM_AR_[4] = {
		FMEnvelopeParameter::AR1, FMEnvelopeParameter::AR2, FMEnvelopeParameter::AR3, FMEnvelopeParameter::AR4
	};
	const FMEnvelopeParameter PARAM_DR_[4] = {
		FMEnvelopeParameter::DR1, FMEnvelopeParameter::DR2, FMEnvelopeParameter::DR3, FMEnvelopeParameter::DR4
	};
	const FMEnvelopeParameter PARAM_RR_[4] = {
		FMEnvelopeParameter::RR1, FMEnvelopeParameter::RR2, FMEnvelopeParameter::RR3, FMEnvelopeParameter::RR4
	};

	inline int toInternalFMChannel(int ch) const
	{
		if (0 <= ch && ch < 6) return ch;
		else if (mode_ == SongType::FM3chExpanded && 6 <= ch && ch < 9) return 2;
		else throw std::out_of_range("Out of channel range.");
	}

	inline uint8_t getFMKeyOnOffChannelMask(int ch) const
	{
		return FM_KEYOFF_MASK_[toInternalFMChannel(ch)];
	}

	inline uint8_t getFM3SlotValidStatus() const
	{
		return fmOpEnables_[2] & (static_cast<uint8_t>(isKeyOnFM_[2]) | (static_cast<uint8_t>(isKeyOnFM_[6]) << 1)
				| (static_cast<uint8_t>(isKeyOnFM_[7]) << 2) | (static_cast<uint8_t>(isKeyOnFM_[8]) << 3));
	}

	inline bool isCarrier(int op, int al) { return CARRIER_JUDGE_[op][al]; }

	inline uint8_t calculateTL(int ch, uint8_t data) const
	{
		int v = (tmpVolFM_[ch] == -1) ? baseVolFM_[ch] : tmpVolFM_[ch];
		return (data > 127 - v) ? 127 : static_cast<uint8_t>(data + v);
	}

	/*----- SSG -----*/
public:
	// Key on-off
	void keyOnSSG(int ch, const Note& note, bool isJam = false);
	void keyOnSSG(int ch, int echoBuf);
	void keyOffSSG(int ch, bool isJam = false);
	void updateEchoBufferSSG(int ch, const Note& note);

	// Set instrument
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
	void setFineDetuneSSG(int ch, int pitch);
	void setNoteSlideSSG(int ch, int speed, int seminote);
	void setTransposeEffectSSG(int ch, int seminote);
	void setToneNoiseMixSSG(int ch, int value);
	void setNoisePitchSSG(int ch, int pitch);
	void setHardEnvelopePeriod(int ch, bool high, int period);
	void setAutoEnvelopeSSG(int ch, int shift, int shape);

	void haltSequencesSSG(int ch);

	// Chip details
	bool isKeyOnSSG(int ch) const;
	bool isTonePortamentoSSG(int ch) const;
	Note getSSGTone(int ch) const;	// TODO: fix

private:
	std::shared_ptr<InstrumentSSG> refInstSSG_[3];
	bool isKeyOnSSG_[3], hasKeyOnBeforeSSG_[3];
	uint8_t mixerSSG_;
	EchoBuffer echoBufSSG_[3];
	bool neverSetBaseNoteSSG_[3];
	Note baseNoteSSG_[3];
	int sumPitchSSG_[3];
	SSGToneNoise tnSSG_[3];
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
	SSGWaveformIter wfItSSG_[3];
	SSGWaveformUnit wfSSG_[3];
	SSGEnvelopeIter envItSSG_[3];
	SSGEnvelopeUnit envSSG_[3];
	SSGToneNoiseIter tnItSSG_[3];
	ArpeggioIterInterface arpItSSG_[3];
	PitchIter ptItSSG_[3];
	bool isArpEffSSG_[3];
	int prtmSSG_[3];
	bool isTonePrtmSSG_[3];
	std::unique_ptr<WavingEffectIterator> vibItSSG_[3];
	std::unique_ptr<WavingEffectIterator> treItSSG_[3];
	int volSldSSG_[3], sumVolSldSSG_[3];
	int detuneSSG_[3], fdetuneSSG_[3];
	std::unique_ptr<NoteSlideEffectIterator> nsItSSG_[3];
	int sumNoteSldSSG_[3];
	bool noteSldSSGSetFlag_;
	int transposeSSG_[3];
	int toneNoiseMixSSG_[3];
	int noisePitchSSG_;
	int hardEnvPeriodHighSSG_, hardEnvPeriodLowSSG_;
	const int AUTO_ENV_SHAPE_TYPE_[15] = { 17, 17, 17, 21, 21, 21, 21, 16, 17, 18, 19, 20, 21, 22, 23 };

	void initSSG();

	void setMuteSSGState(int ch, bool isMuteFM);
	bool isMuteSSG(int ch);

	void setFrontSSGSequences(int ch);
	void releaseStartSSGSequences(int ch);
	void tickEventSSG(int ch);

	void writeWaveformSSGToRegister(int ch);
	void writeSquareWaveform(int ch);

	void writeToneNoiseSSGToRegister(int ch);
	void writeToneNoiseSSGToRegisterNoReference(int ch);

	void writeEnvelopeSSGToRegister(int ch);

	inline uint8_t SSGToneFlag(int ch) { return (1 << ch); }
	inline uint8_t SSGNoiseFlag(int ch) { return (8 << ch); }

	inline void checkRealToneSSGByArpeggio(int ch)
	{
		checkRealToneByArpeggio(arpItSSG_[ch], echoBufSSG_[ch], baseNoteSSG_[ch], needToneSetSSG_[ch]);
	}

	inline void checkPortamentoSSG(int ch)
	{
		checkPortamento(arpItSSG_[ch], prtmSSG_[ch], hasKeyOnBeforeSSG_[ch], isTonePrtmSSG_[ch], echoBufSSG_[ch],
						baseNoteSSG_[ch], needToneSetSSG_[ch]);
	}

	inline void checkRealToneSSGByPitch(int ch)
	{
		checkRealToneByPitch(ptItSSG_[ch], sumPitchSSG_[ch], needToneSetSSG_[ch]);
	}

	void writePitchSSG(int ch);
	void writeAutoEnvelopePitchSSG(int ch, double tonePitch);
	void writeSquareMaskPitchSSG(int ch, double tonePitch, bool isTriangle);

	void setRealVolumeSSG(int ch);

	inline uint8_t judgeSSEGRegisterValue(int v)
	{
		return (v == -1) ? 0 : (0x08 + static_cast<uint8_t>(v));
	}

	/*----- Rhythm -----*/
public:
	// Key on/off
	void setKeyOnFlagRhythm(int ch);
	void setKeyOffFlagRhythm(int ch);

	// Set volume
	void setVolumeRhythm(int ch, int volume);
	void setMasterVolumeRhythm(int volume);
	void setTemporaryVolumeRhythm(int ch, int volume);

	// Set effect
	void setPanRhythm(int ch, int value);

private:
	uint8_t keyOnFlagRhythm_, keyOffFlagRhythm_;
	int volRhythm_[6], mVolRhythm_, tmpVolRhythm_[6];
	/// bit0: right on/off
	/// bit1: left on/off
	uint8_t panRhythm_[6];
	bool isMuteRhythm_[6];

	void initRhythm();

	void setMuteRhythmState(int ch, bool isMute);
	bool isMuteRhythm(int ch);

	void updateKeyOnOffStatusRhythm();

	/*----- ADPCM/Drumkit -----*/
public:
	// Key on-off
	void keyOnADPCM(const Note& note, bool isJam = false);
	void keyOnADPCM(int echoBuf);
	void keyOffADPCM(bool isJam = false);
	void updateEchoBufferADPCM(const Note& note);

	// Set instrument
	void setInstrumentADPCM(std::shared_ptr<InstrumentADPCM> inst);
	void updateInstrumentADPCM(int instNum);
	void setInstrumentDrumkit(std::shared_ptr<InstrumentDrumkit> inst);
	void updateInstrumentDrumkit(int instNum, int key);
	void clearSamplesADPCM();
	/// return: [0]: start address, [1]: stop address
	std::vector<size_t> storeSampleADPCM(std::vector<uint8_t> sample);

	// Set volume
	void setVolumeADPCM(int volume);
	void setTemporaryVolumeADPCM(int volume);

	// Set effect
	void setPanADPCM(int value);
	void setArpeggioEffectADPCM(int second, int third);
	void setPortamentoEffectADPCM(int depth, bool isTonePortamento = false);
	void setVibratoEffectADPCM(int period, int depth);
	void setTremoloEffectADPCM(int period, int depth);
	void setVolumeSlideADPCM(int depth, bool isUp);
	void setDetuneADPCM(int pitch);
	void setFineDetuneADPCM(int pitch);
	void setNoteSlideADPCM(int speed, int seminote);
	void setTransposeEffectADPCM(int seminote);

	// For state retrieve
	void haltSequencesADPCM();

	// Chip details
	bool isKeyOnADPCM() const;
	bool isTonePortamentoADPCM() const;
	Note getADPCMTone() const;	// TODO: fix
	size_t getADPCMStoredSize() const;

private:
	std::shared_ptr<InstrumentADPCM> refInstADPCM_;
	std::shared_ptr<InstrumentDrumkit> refInstKit_;
	bool isKeyOnADPCM_, hasKeyOnBeforeADPCM_;
	EchoBuffer echoBufADPCM_;
	bool neverSetBaseNoteADPCM_;
	Note baseNoteADPCM_;
	int sumPitchADPCM_;
	int baseVolADPCM_, tmpVolADPCM_;
	uint8_t panADPCM_;
	bool isMuteADPCM_;
	bool hasPreSetTickEventADPCM_;
	bool needEnvSetADPCM_;
	bool needToneSetADPCM_;
	size_t startAddrADPCM_, stopAddrADPCM_;	// By 32 bytes
	size_t storePointADPCM_;	// Move by 32 bytes
	ADPCMEnvelopeIter envItADPCM_;
	ArpeggioIterInterface arpItADPCM_;
	PitchIter ptItADPCM_;
	bool isArpEffADPCM_;
	int prtmADPCM_;
	bool isTonePrtmADPCM_;
	std::unique_ptr<WavingEffectIterator> vibItADPCM_;
	std::unique_ptr<WavingEffectIterator> treItADPCM_;
	int volSldADPCM_, sumVolSldADPCM_;
	int detuneADPCM_, fdetuneADPCM_;
	std::unique_ptr<NoteSlideEffectIterator> nsItADPCM_;
	int sumNoteSldADPCM_;
	bool noteSldADPCMSetFlag_;
	int transposeADPCM_;
	bool hasStartRequestedKit_;

	void initADPCM();

	void setMuteADPCMState(bool isMuteFM);
	bool isMuteADPCM();

	void setFrontADPCMSequences();
	void releaseStartADPCMSequences();
	void tickEventADPCM();

	void writeEnvelopeADPCMToRegister();

	inline void checkRealToneADPCMByArpeggio()
	{
		checkRealToneByArpeggio(arpItADPCM_, echoBufADPCM_, baseNoteADPCM_, needToneSetADPCM_);
	}

	inline void checkPortamentoADPCM()
	{
		checkPortamento(arpItADPCM_, prtmADPCM_, hasKeyOnBeforeADPCM_, isTonePrtmADPCM_, echoBufADPCM_,
						baseNoteADPCM_, needToneSetADPCM_);
	}

	inline void checkRealToneADPCMByPitch()
	{
		checkRealToneByPitch(ptItADPCM_, sumPitchADPCM_, needToneSetADPCM_);
	}

	void writePitchADPCM();
	void writePitchADPCMToRegister(int pitchDiff, int rtDeltaN);

	void setRealVolumeADPCM();

	void triggerSamplePlayADPCM(size_t startAddress, size_t stopAddress, bool repeatable);
};
