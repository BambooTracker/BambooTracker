/*
 * Copyright (C) 2018-2022 Rerrah
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
#include "enum_hash.hpp"
#include "bamboo_tracker_defs.hpp"

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
	void connectToRealChip(RealChipInterfaceType type, RealChipInterfaceGeneratorFunc* f);
	RealChipInterfaceType getRealChipInterfaceType() const;
	bool hasConnectedToRealChip() const;

	// Stream samples
	void getStreamSamples(int16_t* container, size_t nSamples);
	void getOutputHistory(int16_t* history);

	// Chip mode
	void setMode(SongType mode);
	SongType getMode() const noexcept;

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

	void resetState();

	std::unique_ptr<int16_t[]> outputHistory_;
	size_t outputHistoryIndex_;
	std::unique_ptr<int16_t[]> outputHistoryReady_;
	std::mutex outputHistoryReadyMutex_;
	void fillOutputHistory(const int16_t* outputs, size_t nSamples);

	using ArpeggioIterInterface = std::unique_ptr<SequenceIteratorInterface<ArpeggioUnit>>;
	void checkRealToneByArpeggio(const ArpeggioIterInterface& arpItr,
								 const EchoBuffer& echoBuf, Note& baseNote, bool& shouldSetTone);
	void checkPortamento(const ArpeggioIterInterface& arpItr, int prtm, bool hasKeyOnBefore,
						 bool isTonePrtm, EchoBuffer& echoBuf, Note& baseNote,
						 bool& shouldSetTone);
	void checkRealToneByPitch(const std::unique_ptr<InstrumentSequenceProperty<InstrumentSequenceBaseUnit>::Iterator>& ptItr,
							  int& sumPitch, bool& shouldSetTone);

	/*----- FM -----*/
public:
	// Key on-off
	void keyOnFM(int ch, const Note& note, bool isJam = false);
	void keyOnFM(int ch, int echoBuf);
	void keyOffFM(int ch, bool isJam = false);
	void retriggerKeyOnFM(int ch, int volDiff);

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
	void setOneshotVolumeFM(int ch, int volume);
	void setMasterVolumeFM(double dB);

	// Set effect
	void setPanFM(int ch, int value);
	void setArpeggioEffectFM(int ch, int second, int third);
	void setPortamentoEffectFM(int ch, int depth, bool isTonePortamento = false);
	void setVibratoEffectFM(int ch, int period, int depth);
	void setTremoloEffectFM(int ch, int period, int depth);
	void setVolumeSlideFM(int ch, int depth, bool isUp);
	void setXVolumeSlideFM(int ch, int factor);
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
	Note getFMLatestNote(int ch) const;

private:
	// Seperate FM 3ch in FM3chEx mode
	struct FMChannel
	{
		size_t ch, inCh;
		bool isKeyOn, hasKeyOnBefore;
		EchoBuffer echoBuf;
		bool neverSetBaseNote;
		Note baseNote;
		int baseVol, oneshotVol;
		bool isMute;
		bool isEnabledEnvReset, hasResetEnv;
		bool hasPreSetTickEvent;
		bool shouldSetTone;
		ArpeggioIterInterface arpItr;
		PitchIter ptItr;
		int ptSum;
		bool isArpEff;
		int prtmDepth;
		bool isTonePrtm;
		std::unique_ptr<WavingEffectIterator> vibItr;
		std::unique_ptr<WavingEffectIterator> treItr;
		int volSld, volSldSum;
		std::unique_ptr<XVolumeSlideEffectIterator> xVolSldItr;
		int xvolSldSum;
		int detune, fdetune;
		std::unique_ptr<NoteSlideEffectIterator> nsItr;
		int nsSum;
		bool hasSetNs;
		int transpose;
		FMOperatorType opType;
	};
	FMChannel fm_[9];

	// Share FM 3ch in FM3chEx mode
	std::shared_ptr<InstrumentFM> refInstFM_[6];
	std::unique_ptr<EnvelopeFM> envFM_[6];
	uint8_t fmOpEnables_[6];
	uint8_t panStateFM_[6];
	PanIter panItrFM_[6];
	int lfoFreq_;
	int lfoStartCntFM_[6];
	std::unordered_map<FMEnvelopeParameter, FMOperatorSequenceIter> opSeqIrtFM_[6];
	bool isFBCtrlFM_[6], isTLCtrlFM_[6][4], isMLCtrlFM_[6][4], isARCtrlFM_[6][4];
	bool isDRCtrlFM_[6][4], isRRCtrlFM_[6][4];
	bool isBrightFM_[6][4];

	void initFM();

	void setMuteFMState(int ch, bool isMuteFM);
	bool isMuteFM(int ch);

	uint32_t getFMChannelOffset(int ch, bool forPitch = false) const;

	void updateFMVolume(FMChannel& fm);

	void writeFMEnvelopeToRegistersFromInstrument(size_t inch);
	void writeFMEnveropeParameterToRegister(size_t inch, FMEnvelopeParameter param, int value);

	void writeFMLFOAllRegisters(size_t inch);
	void writeFMLFORegister(size_t inch, FMLFOParameter param);
	void checkLFOUsedByInstrument();

	void setFrontFMSequences(FMChannel& fm);
	void releaseStartFMSequences(FMChannel& fm);
	void tickEventFM(FMChannel& fm);

	void checkOperatorSequenceFM(FMChannel& fm, int type);
	void checkVolumeEffectFM(FMChannel& fm);

	void checkPanFM(FMChannel& fm, int type);
	void writePanFM(FMChannel& fm, int pos);

	void checkRealToneFMByArpeggio(FMChannel& fm);
	void checkPortamentoFM(FMChannel& fm);
	void checkRealToneFMByPitch(FMChannel& fm);

	void writePitchFM(FMChannel& fm);

	void setInstrumentFMProperties(FMChannel& fm);

	uint8_t getFM3SlotValidStatus() const;
	uint8_t calculateTL(int ch, uint8_t data) const;

	/*----- SSG -----*/
public:
	// Key on-off
	void keyOnSSG(int ch, const Note& note, bool isJam = false);
	void keyOnSSG(int ch, int echoBuf);
	void keyOffSSG(int ch, bool isJam = false);
	void retriggerKeyOnSSG(int ch, int volDiff);

	// Set instrument
	void setInstrumentSSG(int ch, std::shared_ptr<InstrumentSSG> inst);
	void updateInstrumentSSG(int instNum);

	// Set volume
	void setVolumeSSG(int ch, int volume);
	void setOneshotVolumeSSG(int ch, int volume);
	void setMasterVolumeSSG(double dB);

	// Set effect
	void setArpeggioEffectSSG(int ch, int second, int third);
	void setPortamentoEffectSSG(int ch, int depth, bool isTonePortamento = false);
	void setVibratoEffectSSG(int ch, int period, int depth);
	void setTremoloEffectSSG(int ch, int period, int depth);
	void setVolumeSlideSSG(int ch, int depth, bool isUp);
	void setXVolumeSlideSSG(int ch, int factor);
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
	Note getSSGLatestNote(int ch) const;

private:
	struct SSGChannel
	{
		size_t ch;
		std::shared_ptr<InstrumentSSG> refInst;
		bool isKeyOn, hasKeyOnBefore, isInKeyOnProcess_;
		EchoBuffer echoBuf;
		bool neverSetBaseNote;
		Note baseNote;
		int baseVol, oneshotVol;
		bool isMute;
		bool shouldSkip1stTickExec;
		bool shouldSetEnv;
		bool shouldSetSqMaskFreq;
		bool shouldSetHardEnvFreq;
		bool shouldUpdateMixState;
		bool shouldSetTone;
		SSGWaveformIter wfItr;
		SSGWaveformUnit wfChState;
		SSGEnvelopeIter envItr;
		SSGEnvelopeUnit envState;
		bool isHardEnv;
		SSGToneNoiseIter tnItr;
		bool hasRequestedTnEffSet;
		ArpeggioIterInterface arpItr;
		PitchIter ptItr;
		int ptSum;
		bool isArpEff;
		int prtmDepth;
		bool isTonePrtm;
		std::unique_ptr<WavingEffectIterator> vibItr;
		std::unique_ptr<WavingEffectIterator> treItr;
		int volSld, volSldSum;
		std::unique_ptr<XVolumeSlideEffectIterator> xVolSldItr;
		int detune, fdetune;
		std::unique_ptr<NoteSlideEffectIterator> nsItr;
		int nsSum;
		bool hasSetNs;
		int transpose;
	} ssg_[3];
	/// Flag "on" is key "off"
	uint8_t mixerSSG_;
	uint8_t noisePeriodSSG_;
	int hardEnvPeriodHighSSG_, hardEnvPeriodLowSSG_;

	void initSSG();

	void setMuteSSGState(int ch, bool isMuteFM);
	bool isMuteSSG(int ch);

	void setFrontSSGSequences(SSGChannel& ssg);
	void releaseStartSSGSequences(SSGChannel& ssg);
	void tickEventSSG(SSGChannel& ssg);

	void writeWaveformSSGToRegister(SSGChannel& ssg);
	void writeSquareWaveform(SSGChannel& ssg);

	void xslideVolumeSsg(SSGChannel& ssg);
	void writeEnvelopeSSGToRegister(SSGChannel& ssg);
	void setRealVolumeSSG(SSGChannel& ssg);

	void writeMixerSSGToRegisterByEffect(SSGChannel& ssg);
	void writeMixerSSGToRegisterBySequence(SSGChannel& ssg);
	void writeMixerSSGToRegisterByNoReference(SSGChannel& ssg);

	void checkRealToneSSGByArpeggio(SSGChannel& ssg);
	void checkPortamentoSSG(SSGChannel& ssg);
	void checkRealToneSSGByPitch(SSGChannel& ssg);

	void writePitchSSG(SSGChannel& ssg);
	void writeAutoEnvelopePitchSSG(SSGChannel& ssg, double tonePitch);
	void writeSquareMaskPitchSSG(SSGChannel& ssg, double tonePitch, bool isTriangle);

	/*----- Rhythm -----*/
public:
	// Key on/off
	void setKeyOnFlagRhythm(int ch);
	void setKeyOffFlagRhythm(int ch);
	void retriggerKeyOnFlagRhythm(int ch, int volDiff);

	// Set volume
	void setVolumeRhythm(int ch, int volume);
	void setOneshotVolumeRhythm(int ch, int volume);
	void setMasterVolumeRhythm(int volume);

	// Set effect
	void setPanRhythm(int ch, int value);

private:
	struct RhythmChannel
	{
		int baseVol, oneshotVol;
		/// bit0: right on/off
		/// bit1: left on/off
		uint8_t panState;
		bool isMute;
	} rhythm_[6];
	uint8_t keyOnRequestFlagsRhythm_, keyOffRequestFlagsRhythm_;
	int masterVolRhythm_;

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
	void retriggerKeyOnADPCM(int volDiff);

	// Set instrument
	void setInstrumentADPCM(std::shared_ptr<InstrumentADPCM> inst);
	void updateInstrumentADPCM(int instNum);
	void setInstrumentDrumkit(std::shared_ptr<InstrumentDrumkit> inst);
	void updateInstrumentDrumkit(int instNum, int key);
	void clearSamplesADPCM();
	/// [Return] true if sample assignment is success
	bool storeSampleADPCM(const std::vector<uint8_t>& sample, size_t& startAddr, size_t& stopAddr);

	// Set volume
	void setVolumeADPCM(int volume);
	void setOneshotVolumeADPCM(int volume);

	// Set effect
	void setPanADPCM(int value);
	void setArpeggioEffectADPCM(int second, int third);
	void setPortamentoEffectADPCM(int depth, bool isTonePortamento = false);
	void setVibratoEffectADPCM(int period, int depth);
	void setTremoloEffectADPCM(int period, int depth);
	void setVolumeSlideADPCM(int depth, bool isUp);
	void setXVolumeSlideADPCM(int factor);
	void setDetuneADPCM(int pitch);
	void setFineDetuneADPCM(int pitch);
	void setNoteSlideADPCM(int speed, int seminote);
	void setTransposeEffectADPCM(int seminote);

	// For state retrieve
	void haltSequencesADPCM();

	// Chip details
	bool isKeyOnADPCM() const;
	bool isTonePortamentoADPCM() const;
	Note getADPCMLatestNote() const;
	size_t getADPCMStoredSize() const;

private:
	std::shared_ptr<InstrumentADPCM> refInstADPCM_;
	std::shared_ptr<InstrumentDrumkit> refInstKit_;
	bool isKeyOnADPCM_, hasKeyOnBeforeADPCM_;
	EchoBuffer echoBufADPCM_;
	bool neverSetBaseNoteADPCM_;
	Note baseNoteADPCM_;
	int baseVolADPCM_, oneshotVolADPCM_;
	uint8_t panStateADPCM_;
	PanIter panItrADPCM_;
	bool isMuteADPCM_;
	bool shouldSkip1stTickExecADPCM_;	// Use to execute key on/off process in jamming
	bool shouldWriteEnvADPCM_;
	bool shouldSetToneADPCM_;
	size_t startAddrADPCM_, stopAddrADPCM_;	// By 32 bytes
	size_t storePointADPCM_;	// Move by 32 bytes
	ADPCMEnvelopeIter envItrADPCM_;
	ArpeggioIterInterface arpItrADPCM_;
	PitchIter ptItrADPCM_;
	int ptSumADPCM_;
	bool hasArpEffADPCM_;
	int prtmDepthADPCM_;
	bool hasTonePrtmADPCM_;
	std::unique_ptr<WavingEffectIterator> vibItrADPCM_;
	std::unique_ptr<WavingEffectIterator> treItrADPCM_;
	int volSldADPCM_, volSldSumADPCM_;
	std::unique_ptr<XVolumeSlideEffectIterator> xVolSldItrAdpcm_;
	int detuneADPCM_, fdetuneADPCM_;
	std::unique_ptr<NoteSlideEffectIterator> nsItADPCM_;
	int nsSumADPCM_;
	bool hasSetNsADPCM_;
	int transposeADPCM_;
	bool hasStartRequestedKit_;

	void initADPCM();

	void setMuteADPCMState(bool isMuteFM);
	bool isMuteADPCM();

	void setFrontADPCMSequences();
	void releaseStartADPCMSequences();
	void tickEventADPCM();

	void xslideVolumeAdpcm();
	void writeEnvelopeADPCMToRegister();

	void checkPanADPCM(int type);
	void writePanADPCM(int pos);

	void checkRealToneADPCMByArpeggio();
	void checkPortamentoADPCM();
	void checkRealToneADPCMByPitch();

	void writePitchADPCM();
	void writePitchADPCMToRegister(int pitchDiff, int rtDeltaN);

	void setRealVolumeADPCM();

	void triggerSamplePlayADPCM(size_t startAddress, size_t stopAddress, bool repeatable);
};

//-----------------------------------------------------------------------------

inline SongType OPNAController::getMode() const noexcept
{
	return mode_;
}

/*----- FM -----*/
inline void OPNAController::checkRealToneFMByArpeggio(FMChannel& fm)
{
	checkRealToneByArpeggio(fm.arpItr, fm.echoBuf, fm.baseNote, fm.shouldSetTone);
}

inline void OPNAController::checkPortamentoFM(FMChannel& fm)
{
	checkPortamento(fm.arpItr, fm.prtmDepth, fm.hasKeyOnBefore, fm.isTonePrtm, fm.echoBuf,
					fm.baseNote, fm.shouldSetTone);
}

inline void OPNAController::checkRealToneFMByPitch(FMChannel& fm)
{
	checkRealToneByPitch(fm.ptItr, fm.ptSum, fm.shouldSetTone);
}

inline uint8_t OPNAController::getFM3SlotValidStatus() const
{
	return fmOpEnables_[2] & (static_cast<uint8_t>(fm_[2].isKeyOn) | (static_cast<uint8_t>(fm_[6].isKeyOn) << 1)
			| (static_cast<uint8_t>(fm_[7].isKeyOn) << 2) | (static_cast<uint8_t>(fm_[8].isKeyOn) << 3));
}

/*----- SSG -----*/
inline void OPNAController::checkRealToneSSGByArpeggio(OPNAController::SSGChannel& ssg)
{
	checkRealToneByArpeggio(ssg.arpItr, ssg.echoBuf, ssg.baseNote, ssg.shouldSetTone);
}

inline void OPNAController::checkPortamentoSSG(OPNAController::SSGChannel& ssg)
{
	checkPortamento(ssg.arpItr, ssg.prtmDepth, ssg.hasKeyOnBefore, ssg.isTonePrtm, ssg.echoBuf,
					ssg.baseNote, ssg.shouldSetTone);
}

inline void OPNAController::checkRealToneSSGByPitch(OPNAController::SSGChannel& ssg)
{
	checkRealToneByPitch(ssg.ptItr, ssg.ptSum, ssg.shouldSetTone);
}

/*----- ADPCM/Drumkit -----*/
inline void OPNAController::checkRealToneADPCMByArpeggio()
{
	checkRealToneByArpeggio(arpItrADPCM_, echoBufADPCM_, baseNoteADPCM_, shouldSetToneADPCM_);
}

inline void OPNAController::checkPortamentoADPCM()
{
	checkPortamento(arpItrADPCM_, prtmDepthADPCM_, hasKeyOnBeforeADPCM_, hasTonePrtmADPCM_, echoBufADPCM_,
					baseNoteADPCM_, shouldSetToneADPCM_);
}

inline void OPNAController::checkRealToneADPCMByPitch()
{
	checkRealToneByPitch(ptItrADPCM_, ptSumADPCM_, shouldSetToneADPCM_);
}
