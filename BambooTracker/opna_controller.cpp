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

#include "opna_controller.hpp"
#include <stdexcept>
#include <limits>
#include "calc_pitch.hpp"

OPNAController::OPNAController(chip::OpnaEmulator emu, int clock, int rate, int duration)
	: mode_(SongType::Standard),
	  FM_ENV_PARAMS_OP_({
{ FMOperatorType::All, {
						FMEnvelopeParameter::AL, FMEnvelopeParameter::FB,
						FMEnvelopeParameter::AR1, FMEnvelopeParameter::DR1, FMEnvelopeParameter::SR1, FMEnvelopeParameter::RR1,
						FMEnvelopeParameter::SL1, FMEnvelopeParameter::TL1, FMEnvelopeParameter::KS1, FMEnvelopeParameter::ML1,
						FMEnvelopeParameter::DT1,
						FMEnvelopeParameter::AR2, FMEnvelopeParameter::DR2, FMEnvelopeParameter::SR2, FMEnvelopeParameter::RR2,
						FMEnvelopeParameter::SL2, FMEnvelopeParameter::TL2, FMEnvelopeParameter::KS2, FMEnvelopeParameter::ML2,
						FMEnvelopeParameter::DT2,
						FMEnvelopeParameter::AR3, FMEnvelopeParameter::DR3, FMEnvelopeParameter::SR3, FMEnvelopeParameter::RR3,
						FMEnvelopeParameter::SL3, FMEnvelopeParameter::TL3, FMEnvelopeParameter::KS3, FMEnvelopeParameter::ML3,
						FMEnvelopeParameter::DT3,
						FMEnvelopeParameter::AR4, FMEnvelopeParameter::DR4, FMEnvelopeParameter::SR4, FMEnvelopeParameter::RR4,
						FMEnvelopeParameter::SL4, FMEnvelopeParameter::TL4, FMEnvelopeParameter::KS4, FMEnvelopeParameter::ML4,
						FMEnvelopeParameter::DT4
						}},
{ FMOperatorType::Op1, {
						FMEnvelopeParameter::AL, FMEnvelopeParameter::FB,
						FMEnvelopeParameter::AR1, FMEnvelopeParameter::DR1, FMEnvelopeParameter::SR1, FMEnvelopeParameter::RR1,
						FMEnvelopeParameter::SL1, FMEnvelopeParameter::TL1, FMEnvelopeParameter::KS1, FMEnvelopeParameter::ML1,
						FMEnvelopeParameter::DT1
						}},
{ FMOperatorType::Op2, {
						FMEnvelopeParameter::AR2, FMEnvelopeParameter::DR2, FMEnvelopeParameter::SR2, FMEnvelopeParameter::RR2,
						FMEnvelopeParameter::SL2, FMEnvelopeParameter::TL2, FMEnvelopeParameter::KS2, FMEnvelopeParameter::ML2,
						FMEnvelopeParameter::DT2
						}},
{ FMOperatorType::Op3, {
						FMEnvelopeParameter::AR3, FMEnvelopeParameter::DR3, FMEnvelopeParameter::SR3, FMEnvelopeParameter::RR3,
						FMEnvelopeParameter::SL3, FMEnvelopeParameter::TL3, FMEnvelopeParameter::KS3, FMEnvelopeParameter::ML3,
						FMEnvelopeParameter::DT3
						}},
{ FMOperatorType::Op4, {
						FMEnvelopeParameter::AR4, FMEnvelopeParameter::DR4, FMEnvelopeParameter::SR4, FMEnvelopeParameter::RR4,
						FMEnvelopeParameter::SL4, FMEnvelopeParameter::TL4, FMEnvelopeParameter::KS4, FMEnvelopeParameter::ML4,
						FMEnvelopeParameter::DT4
						}}
						}),
	  storePointADPCM_(0)
{
	constexpr size_t DRAM_SIZE = 262144;	// 256KiB
	opna_ = std::make_unique<chip::OPNA>(emu, clock, rate, duration, DRAM_SIZE,
										 std::make_unique<chip::LinearResampler>(),
										 std::make_unique<chip::LinearResampler>());

	for (int ch = 0; ch < 6; ++ch) {
		fmOpEnables_[ch] = 0xf;
		isMuteFM_[ch] = false;
		for (auto ep : FM_ENV_PARAMS_OP_.at(FMOperatorType::All))
			opSeqItFM_[ch].emplace(ep, nullptr);
	}

	for (int ch = 0; ch < 3; ++ch) {
		isMuteSSG_[ch] = false;
	}

	for (int ch = 0; ch < 6; ++ch) {
		isMuteRhythm_[ch] = false;
	}

	isMuteADPCM_ = false;

	initChip();

	outputHistory_.reset(new int16_t[2 * OUTPUT_HISTORY_SIZE]{});
	outputHistoryReady_.reset(new int16_t[2 * OUTPUT_HISTORY_SIZE]{});
	outputHistoryIndex_ = 0;
}

/********** Reset and initialize **********/
void OPNAController::reset()
{
	opna_->reset();
	initChip();
	std::fill(&outputHistory_[0], &outputHistory_[2 * OUTPUT_HISTORY_SIZE], 0);
	std::fill(&outputHistoryReady_[0], &outputHistoryReady_[2 * OUTPUT_HISTORY_SIZE], 0);
}

void OPNAController::initChip()
{
	opna_->setRegister(0x29, 0x80);		// Init interrupt / YM2608 mode

	registerSetBuf_.clear();

	initFM();
	initSSG();
	initRhythm();
	initADPCM();
}

/********** Forward instrument sequence **********/
void OPNAController::tickEvent(SoundSource src, int ch)
{
	switch (src) {
	case SoundSource::FM:		tickEventFM(ch);	break;
	case SoundSource::SSG:		tickEventSSG(ch);	break;
	case SoundSource::RHYTHM:	break;
	case SoundSource::ADPCM:	tickEventADPCM();	break;
	}
}

/********** Direct register set **********/
void OPNAController::sendRegister(int address, int value)
{
	registerSetBuf_.push_back(std::make_pair(address, value));
}

/********** DRAM **********/
size_t OPNAController::getDRAMSize() const
{
	return opna_->getDRAMSize();
}

/********** Update register states after tick process **********/
void OPNAController::updateRegisterStates()
{
	updateKeyOnOffStatusRhythm();

	// Check direct register set
	if (!registerSetBuf_.empty()) {
		for (auto& unit : registerSetBuf_) {
			opna_->setRegister(static_cast<uint32_t>(unit.first), static_cast<uint8_t>(unit.second));
		}
		registerSetBuf_.clear();
	}
}

/********** Real chip interface **********/
void OPNAController::useSCCI(scci::SoundInterfaceManager* manager)
{
	opna_->useSCCI(manager);
}

bool OPNAController::isUsedSCCI() const
{
	return opna_->isUsedSCCI();
}

void OPNAController::useC86CTL(C86ctlBase* base)
{
	opna_->useC86CTL(base);
}

bool OPNAController::isUsedC86CTL() const
{
	return opna_->isUsedC86CTL();
}

/********** Stream samples **********/
void OPNAController::getStreamSamples(int16_t* container, size_t nSamples)
{
	opna_->mix(container, nSamples);

	size_t nHistory = std::min<size_t>(nSamples, OUTPUT_HISTORY_SIZE);
	fillOutputHistory(&container[2 * (nSamples - nHistory)], nHistory);
}

void OPNAController::getOutputHistory(int16_t* container)
{
	std::lock_guard<std::mutex> lock(outputHistoryReadyMutex_);
	int16_t *history = outputHistoryReady_.get();
	std::copy(history, &history[2 * OUTPUT_HISTORY_SIZE], container);
}

void OPNAController::fillOutputHistory(const int16_t* outputs, size_t nSamples)
{
	int16_t *history = outputHistory_.get();
	size_t historyIndex = outputHistoryIndex_;

	// copy as many as possible to the back
	size_t backCapacity = OUTPUT_HISTORY_SIZE - historyIndex;
	size_t nBack = std::min(nSamples, backCapacity);
	std::copy(outputs, &outputs[2 * nBack], &history[2 * historyIndex]);

	// copy the rest to the front
	std::copy(&outputs[2 * nBack], &outputs[2 * nSamples], history);

	// update the write position
	historyIndex = (historyIndex + nSamples) % OUTPUT_HISTORY_SIZE;
	outputHistoryIndex_ = historyIndex;

	// if no one holds the ready buffer, update the contents
	std::unique_lock<std::mutex> lock(outputHistoryReadyMutex_, std::try_to_lock);
	if (lock.owns_lock())
		transferReadyHistory();
}

void OPNAController::transferReadyHistory()
{
	const int16_t* src = outputHistory_.get();
	int16_t* dst = outputHistoryReady_.get();
	size_t index = outputHistoryIndex_;

	// copy the back, and then the front
	std::copy(&src[2 * index], &src[2 * OUTPUT_HISTORY_SIZE], dst);
	std::copy(&src[0], &src[2 * index], &dst[2 * (OUTPUT_HISTORY_SIZE - index)]);
}

/********** Chip mode **********/
void OPNAController::setMode(SongType mode)
{
	mode_ = mode;
	reset();
}

SongType OPNAController::getMode() const
{
	return mode_;
}

/********** Mute **********/
void OPNAController::setMuteState(SoundSource src, int chInSrc, bool isMute)
{
	switch (src) {
	case SoundSource::FM:		setMuteFMState(chInSrc, isMute);	break;
	case SoundSource::SSG:		setMuteSSGState(chInSrc, isMute);	break;
	case SoundSource::RHYTHM:	setMuteRhythmState(chInSrc, isMute);	break;
	case SoundSource::ADPCM:	setMuteADPCMState(isMute);			break;
	}
}

bool OPNAController::isMute(SoundSource src, int chInSrc)
{
	switch (src) {
	case SoundSource::FM:		return isMuteFM(chInSrc);
	case SoundSource::SSG:		return isMuteSSG(chInSrc);
	case SoundSource::RHYTHM:	return isMuteRhythm(chInSrc);
	case SoundSource::ADPCM:	return isMuteADPCM();
	default:	throw std::invalid_argument("Invalid sound source");
	}
}

/********** Stream details **********/
int OPNAController::getRate() const
{
	return opna_->getRate();
}

void OPNAController::setRate(int rate)
{
	opna_->setRate(rate);
}

int OPNAController::getDuration() const
{
	return static_cast<int>(opna_->getMaxDuration());
}

void OPNAController::setDuration(int duration)
{
	opna_->setMaxDuration(static_cast<size_t>(duration));
}

void OPNAController::setMasterVolume(int percentage)
{
	opna_->setMasterVolume(percentage);
}

void OPNAController::setExportContainer(std::shared_ptr<chip::AbstractRegisterWriteLogger> cntr)
{
	opna_->setRegisterWriteLogger(cntr);
}

/********** Internal process **********/
void OPNAController::checkRealToneByArpeggio(const ArpeggioIterInterface& arpIt,
											 const std::deque<ToneDetail>& baseTone, ToneDetail& keyTone,
											 bool& needToneSet)
{
	if (arpIt->hasEnded()) return;

	switch (arpIt->type()) {
	case SequenceType::AbsoluteSequence:
	{
		std::pair<int, Note> pair = noteNumberToOctaveAndNote(
										octaveAndNoteToNoteNumber(baseTone.front().octave,
																  baseTone.front().note)
										+ arpIt->data().data - 48);
		keyTone.octave = pair.first;
		keyTone.note = pair.second;
		break;
	}
	case SequenceType::FixedSequence:
	{
		std::pair<int, Note> pair = noteNumberToOctaveAndNote(arpIt->data().data);
		keyTone.octave = pair.first;
		keyTone.note = pair.second;
		break;
	}
	case SequenceType::RelativeSequence:
	{
		std::pair<int, Note> pair = noteNumberToOctaveAndNote(
										octaveAndNoteToNoteNumber(keyTone.octave, keyTone.note)
										+ arpIt->data().data - 48);
		keyTone.octave = pair.first;
		keyTone.note = pair.second;
		break;
	}
	default:
		return ;
	}

	needToneSet = true;
}

void OPNAController::checkPortamento(const ArpeggioIterInterface& arpIt,
									 int prtm, bool hasKeyOnBefore, bool isTonePrtm,
									 const std::deque<ToneDetail>& baseTone,
									 ToneDetail& keyTone, bool& needToneSet)
{
	if ((!arpIt || arpIt->hasEnded()) && prtm && hasKeyOnBefore) {
		if (isTonePrtm) {
			int dif = ( octaveAndNoteToNoteNumber(baseTone.front().octave, baseTone.front().note)
						* calc_pitch::SEMINOTE_PITCH + baseTone.front().pitch )
					  - ( octaveAndNoteToNoteNumber(keyTone.octave, keyTone.note)
						  * calc_pitch::SEMINOTE_PITCH + keyTone.pitch );
			if (dif > 0) {
				if (dif - prtm < 0) {
					keyTone = baseTone.front();
				}
				else {
					keyTone.pitch += prtm;
				}
				needToneSet = true;
			}
			else if (dif < 0) {
				if (dif + prtm > 0) {
					keyTone = baseTone.front();
				}
				else {
					keyTone.pitch -= prtm;
				}
				needToneSet = true;
			}
		}
		else {
			keyTone.pitch += prtm;
			needToneSet = true;
		}
	}
}

void OPNAController::checkRealToneByPitch(const std::unique_ptr<InstrumentSequenceProperty<InstrumentSequenceBaseUnit>::Iterator>& ptIt,
										  int& sumPitch, bool& needToneSet)
{
	if (ptIt->hasEnded()) return;

	switch (ptIt->type()) {
	case SequenceType::AbsoluteSequence:
		sumPitch = ptIt->data().data - 127;
		break;
	case SequenceType::RelativeSequence:
		sumPitch += (ptIt->data().data - 127);
		break;
	default:
		return;
	}

	needToneSet = true;
}

//---------- FM ----------//
/********** Key on-off **********/
void OPNAController::keyOnFM(int ch, Note note, int octave, int pitch, bool isJam)
{
	if (isMuteFM_[ch]) return;

	updateEchoBufferFM(ch, octave, note, pitch);

	bool isTonePrtm = isTonePrtmFM_[ch] && hasKeyOnBeforeFM_[ch];
	if (isTonePrtm) {
		keyToneFM_[ch].pitch += (sumNoteSldFM_[ch] + transposeFM_[ch]);
	}
	else {
		keyToneFM_[ch] = baseToneFM_[ch].front();
		sumPitchFM_[ch] = 0;
		sumVolSldFM_[ch] = 0;
	}
	if (tmpVolFM_[ch] != -1) {
		setVolumeFM(ch, baseVolFM_[ch]);
	}
	if (!noteSldFMSetFlag_[ch]) {
		nsItFM_[ch].reset();
	}
	noteSldFMSetFlag_[ch] = false;
	needToneSetFM_[ch] = true;
	sumNoteSldFM_[ch] = 0;
	transposeFM_[ch] = 0;

	setFrontFMSequences(ch);
	hasPreSetTickEventFM_[ch] = isJam;

	if (!isTonePrtm) {
		uint8_t chdata = getFMKeyOnOffChannelMask(ch);
		switch (mode_) {
		case SongType::Standard:
		{
			if (isKeyOnFM_[ch]) opna_->setRegister(0x28, chdata);	// Key off
			else isKeyOnFM_[ch] = true;
			opna_->setRegister(0x28, static_cast<uint8_t>(fmOpEnables_[ch] << 4) | chdata);
			break;
		}
		case SongType::FM3chExpanded:
		{
			uint8_t slot = 0;
			switch (ch) {
			case 2:
			case 6:
			case 7:
			case 8:
			{
				bool prev = isKeyOnFM_[ch];
				isKeyOnFM_[ch] = true;
				slot = getFM3SlotValidStatus();
				if (prev) {	// Key off
					uint8_t flags = static_cast<uint8_t>(((slot & FM3_KEY_OFF_MASK_.at(ch)) << 4)) | chdata;
					opna_->setRegister(0x28, flags);
				}
				break;
			}
			default:
				slot = fmOpEnables_[ch];
				if (isKeyOnFM_[ch]) opna_->setRegister(0x28, chdata);	// Key off
				else isKeyOnFM_[ch] = true;
				break;
			}
			opna_->setRegister(0x28, static_cast<uint8_t>(slot << 4) | chdata);
			break;
		}
		}
	}

	hasKeyOnBeforeFM_[ch] = true;
}

void OPNAController::keyOnFM(int ch, int echoBuf)
{
	ToneDetail& td = baseToneFM_[ch].at(static_cast<size_t>(echoBuf));
	if (td.octave == -1) return;
	keyOnFM(ch, td.note, td.octave, td.pitch);
}

void OPNAController::keyOffFM(int ch, bool isJam)
{
	if (!isKeyOnFM_[ch]) {
		tickEventFM(ch);
		return;
	}
	releaseStartFMSequences(ch);
	hasPreSetTickEventFM_[ch] = isJam;

	isKeyOnFM_[ch] = false;

	uint8_t chdata = getFMKeyOnOffChannelMask(ch);
	switch (mode_) {
	case SongType::Standard:
	{
		opna_->setRegister(0x28, chdata);
		break;
	}
	case SongType::FM3chExpanded:
	{
		uint8_t slot = (toInternalFMChannel(ch) == 2) ? static_cast<uint8_t>(getFM3SlotValidStatus() << 4) : 0;
		opna_->setRegister(0x28, slot | chdata);
		break;
	}
	}
}

// Change register only
void OPNAController::resetFMChannelEnvelope(int ch)
{
	keyOffFM(ch);
	hasResetEnvFM_[ch] = true;

	if (mode_ == SongType::FM3chExpanded && toInternalFMChannel(ch) == 2) {
		FMEnvelopeParameter param;
		switch (ch) {
		case 2:	param = FMEnvelopeParameter::RR1;	break;
		case 6:	param = FMEnvelopeParameter::RR2;	break;
		case 7:	param = FMEnvelopeParameter::RR3;	break;
		case 8:	param = FMEnvelopeParameter::RR4;	break;
		default:	throw std::out_of_range("out of range.");
		}
		int prev = envFM_[2]->getParameterValue(param);
		writeFMEnveropeParameterToRegister(2, param, 127);
		envFM_[2]->setParameterValue(param, prev);
	}
	else {
		int prev = envFM_[ch]->getParameterValue(FMEnvelopeParameter::RR1);
		writeFMEnveropeParameterToRegister(ch, FMEnvelopeParameter::RR1, 127);
		envFM_[ch]->setParameterValue(FMEnvelopeParameter::RR1, prev);

		prev = envFM_[ch]->getParameterValue(FMEnvelopeParameter::RR2);
		writeFMEnveropeParameterToRegister(ch, FMEnvelopeParameter::RR2, 127);
		envFM_[ch]->setParameterValue(FMEnvelopeParameter::RR2, prev);

		prev = envFM_[ch]->getParameterValue(FMEnvelopeParameter::RR3);
		writeFMEnveropeParameterToRegister(ch, FMEnvelopeParameter::RR3, 127);
		envFM_[ch]->setParameterValue(FMEnvelopeParameter::RR3, prev);

		prev = envFM_[ch]->getParameterValue(FMEnvelopeParameter::RR4);
		writeFMEnveropeParameterToRegister(ch, FMEnvelopeParameter::RR4, 127);
		envFM_[ch]->setParameterValue(FMEnvelopeParameter::RR4, prev);
	}
}

void OPNAController::updateEchoBufferFM(int ch, int octave, Note note, int pitch)
{
	baseToneFM_[ch].pop_back();
	baseToneFM_[ch].push_front({ octave, note, pitch });
}

/********** Set instrument **********/
/// NOTE: inst != nullptr
void OPNAController::setInstrumentFM(int ch, std::shared_ptr<InstrumentFM> inst)
{
	int inch = toInternalFMChannel(ch);
	FMOperatorType opType = toChannelOperatorType(ch);

	if (!refInstFM_[inch] || !refInstFM_[inch]->isRegisteredWithManager()
			|| refInstFM_[inch]->getNumber() != inst->getNumber()) {
		refInstFM_[inch] = inst;
		writeFMEnvelopeToRegistersFromInstrument(inch);
		fmOpEnables_[inch] = static_cast<uint8_t>(inst->getOperatorEnabled(0))
							 | static_cast<uint8_t>(inst->getOperatorEnabled(1) << 1)
							 | static_cast<uint8_t>(inst->getOperatorEnabled(2) << 2)
							 | static_cast<uint8_t>(inst->getOperatorEnabled(3) << 3);
	}
	else {
		if (isFBCtrlFM_[inch]) {
			isFBCtrlFM_[inch] = false;
			writeFMEnveropeParameterToRegister(inch, FMEnvelopeParameter::FB,
											   inst->getEnvelopeParameter(FMEnvelopeParameter::FB));
		}
		for (int op = 0; op < 4; ++op) {
			if (isTLCtrlFM_[inch][op] || isBrightFM_[inch][op]) {
				isTLCtrlFM_[inch][op] = false;
				isBrightFM_[inch][op] = false;
				FMEnvelopeParameter tl = PARAM_TL_[op];
				writeFMEnveropeParameterToRegister(inch, tl, inst->getEnvelopeParameter(tl));
			}
			if (isMLCtrlFM_[inch][op]) {
				isMLCtrlFM_[inch][op] = false;
				FMEnvelopeParameter ml = PARAM_ML_[op];
				writeFMEnveropeParameterToRegister(inch, ml, inst->getEnvelopeParameter(ml));
			}
			if (isARCtrlFM_[inch][op]) {
				isARCtrlFM_[inch][op] = false;
				FMEnvelopeParameter ar = PARAM_AR_[op];
				writeFMEnveropeParameterToRegister(inch, ar, inst->getEnvelopeParameter(ar));
			}
			if (isDRCtrlFM_[inch][op]) {
				isDRCtrlFM_[inch][op] = false;
				FMEnvelopeParameter dr = PARAM_DR_[op];
				writeFMEnveropeParameterToRegister(inch, dr, inst->getEnvelopeParameter(dr));
			}
			if (isRRCtrlFM_[inch][op]) {
				isRRCtrlFM_[inch][op] = false;
				FMEnvelopeParameter rr = PARAM_RR_[op];
				writeFMEnveropeParameterToRegister(inch, rr, inst->getEnvelopeParameter(rr));
			}
		}
		restoreFMEnvelopeFromReset(ch);
	}

	if (isKeyOnFM_[ch] && lfoStartCntFM_[inch] == -1) writeFMLFOAllRegisters(inch);
	for (auto& p : FM_ENV_PARAMS_OP_.at(opType)) {
		if (refInstFM_[inch]->getOperatorSequenceEnabled(p)) {
			opSeqItFM_[inch].at(p) = refInstFM_[inch]->getOperatorSequenceSequenceIterator(p);
			switch (p) {
			case FMEnvelopeParameter::FB:	isFBCtrlFM_[inch] = false;		break;
			case FMEnvelopeParameter::TL1:
				isTLCtrlFM_[inch][0] = false;
				isBrightFM_[inch][0] = false;
				break;
			case FMEnvelopeParameter::TL2:
				isTLCtrlFM_[inch][1] = false;
				isBrightFM_[inch][1] = false;
				break;
			case FMEnvelopeParameter::TL3:
				isTLCtrlFM_[inch][2] = false;
				isBrightFM_[inch][2] = false;
				break;
			case FMEnvelopeParameter::TL4:
				isTLCtrlFM_[inch][3] = false;
				isBrightFM_[inch][3] = false;
				break;
			case FMEnvelopeParameter::ML1:	isMLCtrlFM_[inch][0] = false;	break;
			case FMEnvelopeParameter::ML2:	isMLCtrlFM_[inch][1] = false;	break;
			case FMEnvelopeParameter::ML3:	isMLCtrlFM_[inch][2] = false;	break;
			case FMEnvelopeParameter::ML4:	isMLCtrlFM_[inch][3] = false;	break;
			case FMEnvelopeParameter::AR1:	isARCtrlFM_[inch][0] = false;	break;
			case FMEnvelopeParameter::AR2:	isARCtrlFM_[inch][1] = false;	break;
			case FMEnvelopeParameter::AR3:	isARCtrlFM_[inch][2] = false;	break;
			case FMEnvelopeParameter::AR4:	isARCtrlFM_[inch][3] = false;	break;
			case FMEnvelopeParameter::DR1:	isDRCtrlFM_[inch][0] = false;	break;
			case FMEnvelopeParameter::DR2:	isDRCtrlFM_[inch][1] = false;	break;
			case FMEnvelopeParameter::DR3:	isDRCtrlFM_[inch][2] = false;	break;
			case FMEnvelopeParameter::DR4:	isDRCtrlFM_[inch][3] = false;	break;
			case FMEnvelopeParameter::RR1:	isRRCtrlFM_[inch][0] = false;	break;
			case FMEnvelopeParameter::RR2:	isRRCtrlFM_[inch][1] = false;	break;
			case FMEnvelopeParameter::RR3:	isRRCtrlFM_[inch][2] = false;	break;
			case FMEnvelopeParameter::RR4:	isRRCtrlFM_[inch][3] = false;	break;
			default:	break;
			}
		}
		else {
			opSeqItFM_[inch].at(p).reset();
		}
	}
	if (!isArpEffFM_[ch]) {
		if (refInstFM_[inch]->getArpeggioEnabled(opType))
			arpItFM_[ch] = refInstFM_[inch]->getArpeggioSequenceIterator(opType);
		else
			arpItFM_[ch].reset();
	}
	if (refInstFM_[inch]->getPitchEnabled(opType))
		ptItFM_[ch] = refInstFM_[inch]->getPitchSequenceIterator(opType);
	else
		ptItFM_[ch].reset();
	setInstrumentFMProperties(ch);

	checkLFOUsed();
}

void OPNAController::updateInstrumentFM(int instNum)
{
	int cnt = static_cast<int>(getFMChannelCount(mode_));
	for (int ch = 0; ch < cnt; ++ch) {
		int inch = toInternalFMChannel(ch);

		if (refInstFM_[inch] && refInstFM_[inch]->isRegisteredWithManager()
				&& refInstFM_[inch]->getNumber() == instNum) {
			writeFMEnvelopeToRegistersFromInstrument(inch);
			if (isKeyOnFM_[ch] && lfoStartCntFM_[inch] == -1) writeFMLFOAllRegisters(inch);
			FMOperatorType opType = toChannelOperatorType(ch);
			for (auto& p : FM_ENV_PARAMS_OP_.at(opType)) {
				if (!refInstFM_[inch]->getOperatorSequenceEnabled(p))
					opSeqItFM_[inch].at(p).reset();
			}
			if (!refInstFM_[inch]->getArpeggioEnabled(opType)) arpItFM_[ch].reset();
			if (!refInstFM_[inch]->getPitchEnabled(opType)) ptItFM_[ch].reset();
			setInstrumentFMProperties(ch);
		}
	}

	checkLFOUsed();
}

void OPNAController::updateInstrumentFMEnvelopeParameter(int envNum, FMEnvelopeParameter param)
{
	for (int ch = 0; ch < 6; ++ch) {
		if (refInstFM_[ch] && refInstFM_[ch]->getEnvelopeNumber() == envNum) {
			writeFMEnveropeParameterToRegister(ch, param, refInstFM_[ch]->getEnvelopeParameter(param));
		}
	}
}

void OPNAController::setInstrumentFMOperatorEnabled(int envNum, int opNum)
{
	int chsize = static_cast<int>(getFMChannelCount(mode_));
	for (int ch = 0; ch < chsize; ++ch) {
		int inch = toInternalFMChannel(ch);
		if (refInstFM_[inch] && refInstFM_[inch]->getEnvelopeNumber() == envNum) {
			bool enabled = refInstFM_[inch]->getOperatorEnabled(opNum);
			envFM_[inch]->setOperatorEnabled(opNum, enabled);
			if (enabled) {
				fmOpEnables_[inch] |= (1 << opNum);
			}
			else {
				fmOpEnables_[inch] &= ~(1 << opNum);
			}
			if (isKeyOnFM_[ch]) {
				uint8_t chdata = getFMKeyOnOffChannelMask(ch);
				switch (mode_) {
				case SongType::Standard:
				{
					opna_->setRegister(0x28, static_cast<uint8_t>(fmOpEnables_[inch] << 4) | chdata);
					break;
				}
				case SongType::FM3chExpanded:
				{
					uint8_t slot = (inch == 2) ? getFM3SlotValidStatus() : fmOpEnables_[inch];
					opna_->setRegister(0x28, static_cast<uint8_t>(slot << 4) | chdata);
					break;
				}
				}
			}
		}
	}
}

void OPNAController::updateInstrumentFMLFOParameter(int lfoNum, FMLFOParameter param)
{
	for (int ch = 0; ch < 6; ++ch) {
		if (refInstFM_[ch] && refInstFM_[ch]->getLFOEnabled() && refInstFM_[ch]->getLFONumber() == lfoNum) {
			writeFMLFORegister(ch, param);
		}
	}
}

/********** Set volume **********/
void OPNAController::setVolumeFM(int ch, int volume)
{
	baseVolFM_[ch] = volume;
	tmpVolFM_[ch] = -1;

	if (refInstFM_[toInternalFMChannel(ch)]) updateFMVolume(ch);	// Change TL
}

void OPNAController::setTemporaryVolumeFM(int ch, int volume)
{
	tmpVolFM_[ch] = volume;

	if (refInstFM_[toInternalFMChannel(ch)]) updateFMVolume(ch);	// Change TL
}

void OPNAController::updateFMVolume(int ch)
{
	int inch = toInternalFMChannel(ch);
	switch (toChannelOperatorType(ch)) {
	case FMOperatorType::All:
		writeFMEnveropeParameterToRegister(inch, FMEnvelopeParameter::TL1,
										   refInstFM_[inch]->getEnvelopeParameter(FMEnvelopeParameter::TL1));
		writeFMEnveropeParameterToRegister(inch, FMEnvelopeParameter::TL2,
										   refInstFM_[inch]->getEnvelopeParameter(FMEnvelopeParameter::TL2));
		writeFMEnveropeParameterToRegister(inch, FMEnvelopeParameter::TL3,
										   refInstFM_[inch]->getEnvelopeParameter(FMEnvelopeParameter::TL3));
		writeFMEnveropeParameterToRegister(inch, FMEnvelopeParameter::TL4,
										   refInstFM_[inch]->getEnvelopeParameter(FMEnvelopeParameter::TL4));
		break;
	case FMOperatorType::Op1:
		writeFMEnveropeParameterToRegister(inch, FMEnvelopeParameter::TL1,
										   refInstFM_[inch]->getEnvelopeParameter(FMEnvelopeParameter::TL1));
		break;
	case FMOperatorType::Op2:
		writeFMEnveropeParameterToRegister(inch, FMEnvelopeParameter::TL2,
										   refInstFM_[inch]->getEnvelopeParameter(FMEnvelopeParameter::TL2));
		break;
	case FMOperatorType::Op3:
		writeFMEnveropeParameterToRegister(inch, FMEnvelopeParameter::TL3,
										   refInstFM_[inch]->getEnvelopeParameter(FMEnvelopeParameter::TL3));
		break;
	case FMOperatorType::Op4:
		writeFMEnveropeParameterToRegister(inch, FMEnvelopeParameter::TL4,
										   refInstFM_[inch]->getEnvelopeParameter(FMEnvelopeParameter::TL4));
		break;
	}
}

void OPNAController::setMasterVolumeFM(double dB)
{
	opna_->setVolumeFM(dB);
}

/********** Set effect **********/
void OPNAController::setPanFM(int ch, int value)
{
	int inch = toInternalFMChannel(ch);

	panFM_[inch] = static_cast<uint8_t>(value);

	uint32_t bch = getFMChannelOffset(ch);	// Bank and channel offset
	uint8_t data = static_cast<uint8_t>(value << 6);
	if (refInstFM_[inch] && refInstFM_[inch]->getLFOEnabled()) {
		data |= (refInstFM_[inch]->getLFOParameter(FMLFOParameter::AMS) << 4);
		data |= refInstFM_[inch]->getLFOParameter(FMLFOParameter::PMS);
	}
	opna_->setRegister(0xb4 + bch, data);
}

void OPNAController::setArpeggioEffectFM(int ch, int second, int third)
{
	if (second || third) {
		arpItFM_[ch] = std::make_unique<ArpeggioEffectIterator>(second, third);
		isArpEffFM_[ch] = true;
	}
	else {
		int inch = toInternalFMChannel(ch);
		if (refInstFM_[inch]) {
			FMOperatorType op = toChannelOperatorType(ch);
			if (!refInstFM_[inch]->getArpeggioEnabled(op)) arpItFM_[ch].reset();
			else arpItFM_[ch] = refInstFM_[inch]->getArpeggioSequenceIterator(op);
		}
		isArpEffFM_[ch] = false;
	}
}

void OPNAController::setPortamentoEffectFM(int ch, int depth, bool isTonePortamento)
{
	prtmFM_[ch] = depth;
	isTonePrtmFM_[ch] = depth ? isTonePortamento : false;
}

void OPNAController::setVibratoEffectFM(int ch, int period, int depth)
{
	if (period && depth) vibItFM_[ch] = std::make_unique<WavingEffectIterator>(period, depth);
	else vibItFM_[ch].reset();
}

void OPNAController::setTremoloEffectFM(int ch, int period, int depth)
{
	if (period && depth) treItFM_[ch] = std::make_unique<WavingEffectIterator>(period, depth);
	else treItFM_[ch].reset();
}

void OPNAController::setVolumeSlideFM(int ch, int depth, bool isUp)
{
	volSldFM_[ch] = isUp ? -depth : depth;
}

void OPNAController::setDetuneFM(int ch, int pitch)
{
	detuneFM_[ch] = pitch;
	needToneSetFM_[ch] = true;
}

void OPNAController::setFineDetuneFM(int ch, int pitch)
{
	fdetuneFM_[ch] = pitch;
	needToneSetFM_[ch] = true;
}

void OPNAController::setNoteSlideFM(int ch, int speed, int seminote)
{
	if (seminote) {
		nsItFM_[ch] = std::make_unique<NoteSlideEffectIterator>(speed, seminote);
		noteSldFMSetFlag_[ch] = true;
	}
	else nsItFM_[ch].reset();
}


void OPNAController::setTransposeEffectFM(int ch, int seminote)
{
	transposeFM_[ch] += (seminote * calc_pitch::SEMINOTE_PITCH);
	needToneSetFM_[ch] = true;
}

void OPNAController::setFBControlFM(int ch, int value)
{
	int inch = toInternalFMChannel(ch);
	writeFMEnveropeParameterToRegister(inch, FMEnvelopeParameter::FB, value);
	isFBCtrlFM_[inch] = true;
	opSeqItFM_[inch].at(FMEnvelopeParameter::FB).reset();
}

void OPNAController::setTLControlFM(int ch, int op, int value)
{
	int inch = toInternalFMChannel(ch);
	FMEnvelopeParameter param = PARAM_TL_[op];
	writeFMEnveropeParameterToRegister(inch, param, value);
	isTLCtrlFM_[inch][op] = true;
	opSeqItFM_[inch].at(param).reset();
}

void OPNAController::setMLControlFM(int ch, int op, int value)
{
	int inch = toInternalFMChannel(ch);
	FMEnvelopeParameter param = PARAM_ML_[op];
	writeFMEnveropeParameterToRegister(inch, param, value);
	isMLCtrlFM_[inch][op] = true;
	opSeqItFM_[inch].at(param).reset();
}

void OPNAController::setARControlFM(int ch, int op, int value)
{
	int inch = toInternalFMChannel(ch);
	FMEnvelopeParameter param = PARAM_AR_[op];
	writeFMEnveropeParameterToRegister(inch, param, value);
	isARCtrlFM_[inch][op] = true;
	opSeqItFM_[inch].at(param).reset();
}

void OPNAController::setDRControlFM(int ch, int op, int value)
{
	int inch = toInternalFMChannel(ch);
	FMEnvelopeParameter param = PARAM_DR_[op];
	writeFMEnveropeParameterToRegister(inch, param, value);
	isDRCtrlFM_[inch][op] = true;
	opSeqItFM_[inch].at(param).reset();
}

void OPNAController::setRRControlFM(int ch, int op, int value)
{
	int inch = toInternalFMChannel(ch);
	FMEnvelopeParameter param = PARAM_RR_[op];
	writeFMEnveropeParameterToRegister(inch, param, value);
	isRRCtrlFM_[inch][op] = true;
	opSeqItFM_[inch].at(param).reset();
}

void OPNAController::setBrightnessFM(int ch, int value)
{
	int inch = toInternalFMChannel(ch);
	std::vector<int> ops = getOperatorsInLevel(1, envFM_[inch]->getParameterValue(FMEnvelopeParameter::AL));
	for (auto& op : ops) {
		FMEnvelopeParameter param = PARAM_TL_[op];
		int v = clamp(envFM_[inch]->getParameterValue(param) + value, 0, 127);
		writeFMEnveropeParameterToRegister(inch, param, v);
		isBrightFM_[inch][op] = true;
		opSeqItFM_[inch].at(param).reset();
	}
}

/********** For state retrieve **********/
void OPNAController::haltSequencesFM(int ch)
{
	int inch = toInternalFMChannel(ch);
	for (auto& p : FM_ENV_PARAMS_OP_.at(toChannelOperatorType(ch))) {
		if (auto& it = opSeqItFM_[inch].at(p)) it->end();
	}
	if (auto& treIt = treItFM_[ch]) treIt->end();
	if (auto& arpIt = arpItFM_[ch]) arpIt->end();
	if (auto& ptIt = ptItFM_[ch]) ptIt->end();
	if (auto& vibIt = vibItFM_[ch]) vibIt->end();
	if (auto& nsIt = nsItFM_[ch]) nsIt->end();
}

/********** Chip details **********/
bool OPNAController::isKeyOnFM(int ch) const
{
	return isKeyOnFM_[ch];
}

bool OPNAController::isTonePortamentoFM(int ch) const
{
	return isTonePrtmFM_[ch];
}

bool OPNAController::enableFMEnvelopeReset(int ch) const
{
	return envFM_[toInternalFMChannel(ch)] ? enableEnvResetFM_[ch] : true;
}

ToneDetail OPNAController::getFMTone(int ch) const
{
	return baseToneFM_[ch].front();
}

/***********************************/
void OPNAController::initFM()
{
	lfoFreq_ = -1;

	uint8_t mode = 0;
	switch (mode_) {
	case SongType::Standard:		mode = 0;		break;
	case SongType::FM3chExpanded:	mode = 0x40;	break;
	}
	opna_->setRegister(0x27, mode);

	for (int inch = 0; inch < 6; ++inch) {
		// Init envelope
		envFM_[inch] = std::make_unique<EnvelopeFM>(-1);
		refInstFM_[inch].reset();

		// Init pan
		uint32_t bch = getFMChannelOffset(inch);
		panFM_[inch] = 3;
		opna_->setRegister(0xb4 + bch, 0xc0);

		// Init sequence
		for (auto& p : opSeqItFM_[inch]) {
			p.second.reset();
		}

		lfoStartCntFM_[inch] = -1;

		isFBCtrlFM_[inch] = false;
		for (int op = 0; op < 4; ++op) {
			isTLCtrlFM_[inch][op] = false;
			isMLCtrlFM_[inch][op] = false;
			isARCtrlFM_[inch][op] = false;
			isDRCtrlFM_[inch][op] = false;
			isRRCtrlFM_[inch][op] = false;
			isBrightFM_[inch][op] = false;
		}
	}

	size_t fmch = getFMChannelCount(mode_);
	for (size_t ch = 0; ch < fmch; ++ch) {
		// Init operators key off
		isKeyOnFM_[ch] = false;
		hasKeyOnBeforeFM_[ch] = false;

		// Init echo buffer
		baseToneFM_[ch] = std::deque<ToneDetail>(4);
		for (auto& td : baseToneFM_[ch]) {
			td.octave = -1;
		}

		keyToneFM_[ch].note = Note::C;	// Dummy
		keyToneFM_[ch].octave = -1;
		keyToneFM_[ch].pitch = 0;	// Dummy
		sumPitchFM_[ch] = 0;
		baseVolFM_[ch] = 0;	// Init volume
		tmpVolFM_[ch] = -1;
		enableEnvResetFM_[ch] = false;
		hasResetEnvFM_[ch] = false;

		// Init sequence
		hasPreSetTickEventFM_[ch] = false;
		arpItFM_[ch].reset();
		ptItFM_[ch].reset();
		needToneSetFM_[ch] = false;

		// Effect
		isArpEffFM_[ch] = false;
		prtmFM_[ch] = 0;
		isTonePrtmFM_[ch] = false;
		vibItFM_[ch].reset();
		treItFM_[ch].reset();
		volSldFM_[ch] = 0;
		sumVolSldFM_[ch] = 0;
		detuneFM_[ch] = 0;
		fdetuneFM_[ch] = 0;
		nsItFM_[ch].reset();
		sumNoteSldFM_[ch] = 0;
		noteSldFMSetFlag_[ch] = false;
		transposeFM_[ch] = 0;
	}
}

void OPNAController::setMuteFMState(int ch, bool isMute)
{
	isMuteFM_[ch] = isMute;

	if (isMute) {
		resetFMChannelEnvelope(ch);
	}
	else {
		int inch = toInternalFMChannel(ch);
		switch (toChannelOperatorType(ch)) {
		case FMOperatorType::All:
			writeFMEnveropeParameterToRegister(inch, FMEnvelopeParameter::RR1,
											   envFM_[inch]->getParameterValue(FMEnvelopeParameter::RR1));
			writeFMEnveropeParameterToRegister(inch, FMEnvelopeParameter::RR2,
											   envFM_[inch]->getParameterValue(FMEnvelopeParameter::RR2));
			writeFMEnveropeParameterToRegister(inch, FMEnvelopeParameter::RR3,
											   envFM_[inch]->getParameterValue(FMEnvelopeParameter::RR3));
			writeFMEnveropeParameterToRegister(inch, FMEnvelopeParameter::RR4,
											   envFM_[inch]->getParameterValue(FMEnvelopeParameter::RR4));
			break;
		case FMOperatorType::Op1:
			writeFMEnveropeParameterToRegister(inch, FMEnvelopeParameter::RR1,
											   envFM_[inch]->getParameterValue(FMEnvelopeParameter::RR1));
			break;
		case FMOperatorType::Op2:
			writeFMEnveropeParameterToRegister(inch, FMEnvelopeParameter::RR2,
											   envFM_[inch]->getParameterValue(FMEnvelopeParameter::RR2));
			break;
		case FMOperatorType::Op3:
			writeFMEnveropeParameterToRegister(inch, FMEnvelopeParameter::RR3,
											   envFM_[inch]->getParameterValue(FMEnvelopeParameter::RR3));
			break;
		case FMOperatorType::Op4:
			writeFMEnveropeParameterToRegister(inch, FMEnvelopeParameter::RR4,
											   envFM_[inch]->getParameterValue(FMEnvelopeParameter::RR4));
			break;
		}
	}
}

bool OPNAController::isMuteFM(int ch)
{
	return isMuteFM_[ch];
}

uint32_t OPNAController::getFMChannelOffset(int ch, bool forPitch) const
{
	if (mode_ == SongType::FM3chExpanded && forPitch) {
		switch (ch) {
		case 0:
		case 1:
			return static_cast<uint32_t>(ch);
		case 3:
		case 4:
		case 5:
			return static_cast<uint32_t>(0x100 + ch % 3);
		case 2:	// FM3-OP1
			return 9;
		case 6:	// FM3-OP2
			return 10;
		case 7:	// FM3-OP3
			return 8;
		case 8:	// FM3-OP4
			return 2;
		default:
			return 0;
		}
	}
	else {
		switch (toInternalFMChannel(ch)) {
		case 0:
		case 1:
		case 2:
			return static_cast<uint32_t>(ch);
		case 3:
		case 4:
		case 5:
			return static_cast<uint32_t>(0x100 + ch % 3);
		default:
			return 0;
		}
	}
}

FMOperatorType OPNAController::toChannelOperatorType(int ch) const
{
	FMOperatorType opType;
	if (mode_ == SongType::FM3chExpanded && toInternalFMChannel(ch) == 2) {
		switch (ch) {
		case 2:	opType = FMOperatorType::Op1;	break;
		case 6:	opType = FMOperatorType::Op2;	break;
		case 7:	opType = FMOperatorType::Op3;	break;
		case 8:	opType = FMOperatorType::Op4;	break;
		default:	throw std::out_of_range("out of range.");
		}
	}
	else {
		opType = FMOperatorType::All;
	}
	return opType;
}

void OPNAController::writeFMEnvelopeToRegistersFromInstrument(int inch)
{
	uint32_t bch = getFMChannelOffset(inch);	// Bank and channel offset
	uint8_t data1, data2;
	int al;

	data1 = static_cast<uint8_t>(refInstFM_[inch]->getEnvelopeParameter(FMEnvelopeParameter::FB));
	envFM_[inch]->setParameterValue(FMEnvelopeParameter::FB, data1);
	data1 <<= 3;
	al = refInstFM_[inch]->getEnvelopeParameter(FMEnvelopeParameter::AL);
	envFM_[inch]->setParameterValue(FMEnvelopeParameter::AL, al);
	data1 += al;
	opna_->setRegister(0xb0 + bch, data1);

	uint32_t offset = bch;	// Operator 1

	data1 = static_cast<uint8_t>(refInstFM_[inch]->getEnvelopeParameter(FMEnvelopeParameter::DT1));
	envFM_[inch]->setParameterValue(FMEnvelopeParameter::DT1, data1);
	data1 <<= 4;
	data2 = static_cast<uint8_t>(refInstFM_[inch]->getEnvelopeParameter(FMEnvelopeParameter::ML1));
	envFM_[inch]->setParameterValue(FMEnvelopeParameter::ML1, data2);
	data1 |= data2;
	opna_->setRegister(0x30 + offset, data1);

	data1 = static_cast<uint8_t>(refInstFM_[inch]->getEnvelopeParameter(FMEnvelopeParameter::TL1));
	// Adjust volume
	if (mode_ == SongType::FM3chExpanded && inch == 2) data1 = calculateTL(2, data1);
	else if (isCarrier(0, al)) data1 = calculateTL(inch, data1);
	envFM_[inch]->setParameterValue(FMEnvelopeParameter::TL1, data1);
	opna_->setRegister(0x40 + offset, data1);

	data1 = static_cast<uint8_t>(refInstFM_[inch]->getEnvelopeParameter(FMEnvelopeParameter::KS1));
	envFM_[inch]->setParameterValue(FMEnvelopeParameter::KS1, data1);
	data1 <<= 6;
	data2 = static_cast<uint8_t>(refInstFM_[inch]->getEnvelopeParameter(FMEnvelopeParameter::AR1));
	envFM_[inch]->setParameterValue(FMEnvelopeParameter::AR1, data2);
	data1 |= data2;
	opna_->setRegister(0x50 + offset, data1);

	data1 = refInstFM_[inch]->getLFOEnabled() ? static_cast<uint8_t>(refInstFM_[inch]->getLFOParameter(FMLFOParameter::AM1)) : 0;
	data1 <<= 7;
	data2 = static_cast<uint8_t>(refInstFM_[inch]->getEnvelopeParameter(FMEnvelopeParameter::DR1));
	envFM_[inch]->setParameterValue(FMEnvelopeParameter::DR1, data2);
	data1 |= data2;
	opna_->setRegister(0x60 + offset, data1);

	data1 = static_cast<uint8_t>(refInstFM_[inch]->getEnvelopeParameter(FMEnvelopeParameter::SR1));
	envFM_[inch]->setParameterValue(FMEnvelopeParameter::SR1, data2);
	opna_->setRegister(0x70 + offset, data1);

	data1 = static_cast<uint8_t>(refInstFM_[inch]->getEnvelopeParameter(FMEnvelopeParameter::SL1));
	envFM_[inch]->setParameterValue(FMEnvelopeParameter::SL1, data1);
	data1 <<= 4;
	data2 = static_cast<uint8_t>(refInstFM_[inch]->getEnvelopeParameter(FMEnvelopeParameter::RR1));
	envFM_[inch]->setParameterValue(FMEnvelopeParameter::RR1, data2);
	data1 |= data2;
	opna_->setRegister(0x80 + offset, data1);

	int tmp = refInstFM_[inch]->getEnvelopeParameter(FMEnvelopeParameter::SSGEG1);
	envFM_[inch]->setParameterValue(FMEnvelopeParameter::SSGEG1, tmp);
	data1 = judgeSSEGRegisterValue(tmp);
	opna_->setRegister(0x90 + offset, data1);

	offset = bch + 8;	// Operator 2

	data1 = static_cast<uint8_t>(refInstFM_[inch]->getEnvelopeParameter(FMEnvelopeParameter::DT2));
	envFM_[inch]->setParameterValue(FMEnvelopeParameter::DT2, data1);
	data1 <<= 4;
	data2 = static_cast<uint8_t>(refInstFM_[inch]->getEnvelopeParameter(FMEnvelopeParameter::ML2));
	envFM_[inch]->setParameterValue(FMEnvelopeParameter::ML2, data2);
	data1 |= data2;
	opna_->setRegister(0x30 + offset, data1);

	data1 = static_cast<uint8_t>(refInstFM_[inch]->getEnvelopeParameter(FMEnvelopeParameter::TL2));
	// Adjust volume
	if (mode_ == SongType::FM3chExpanded && inch == 2) data1 = calculateTL(6, data1);
	else if (isCarrier(1, al)) data1 = calculateTL(inch, data1);
	envFM_[inch]->setParameterValue(FMEnvelopeParameter::TL2, data1);
	opna_->setRegister(0x40 + offset, data1);

	data1 = static_cast<uint8_t>(refInstFM_[inch]->getEnvelopeParameter(FMEnvelopeParameter::KS2));
	envFM_[inch]->setParameterValue(FMEnvelopeParameter::KS2, data1);
	data1 <<= 6;
	data2 = static_cast<uint8_t>(refInstFM_[inch]->getEnvelopeParameter(FMEnvelopeParameter::AR2));
	envFM_[inch]->setParameterValue(FMEnvelopeParameter::AR2, data2);
	data1 |= data2;
	opna_->setRegister(0x50 + offset, data1);

	data1 = refInstFM_[inch]->getLFOEnabled() ? static_cast<uint8_t>(refInstFM_[inch]->getLFOParameter(FMLFOParameter::AM2)) : 0;
	data1 <<= 7;
	data2 = static_cast<uint8_t>(refInstFM_[inch]->getEnvelopeParameter(FMEnvelopeParameter::DR2));
	envFM_[inch]->setParameterValue(FMEnvelopeParameter::DR2, data2);
	data1 |= data2;
	opna_->setRegister(0x60 + offset, data1);

	data1 = static_cast<uint8_t>(refInstFM_[inch]->getEnvelopeParameter(FMEnvelopeParameter::SR2));
	envFM_[inch]->setParameterValue(FMEnvelopeParameter::SR2, data2);
	opna_->setRegister(0x70 + offset, data1);

	data1 = static_cast<uint8_t>(refInstFM_[inch]->getEnvelopeParameter(FMEnvelopeParameter::SL2));
	envFM_[inch]->setParameterValue(FMEnvelopeParameter::SL2, data1);
	data1 <<= 4;
	data2 = static_cast<uint8_t>(refInstFM_[inch]->getEnvelopeParameter(FMEnvelopeParameter::RR2));
	envFM_[inch]->setParameterValue(FMEnvelopeParameter::RR2, data2);
	data1 |= data2;
	opna_->setRegister(0x80 + offset, data1);

	tmp = refInstFM_[inch]->getEnvelopeParameter(FMEnvelopeParameter::SSGEG2);
	envFM_[inch]->setParameterValue(FMEnvelopeParameter::SSGEG2, tmp);
	data1 = judgeSSEGRegisterValue(tmp);
	opna_->setRegister(0x90 + offset, data1);

	offset = bch + 4;	// Operator 3

	data1 = static_cast<uint8_t>(refInstFM_[inch]->getEnvelopeParameter(FMEnvelopeParameter::DT3));
	envFM_[inch]->setParameterValue(FMEnvelopeParameter::DT3, data1);
	data1 <<= 4;
	data2 = static_cast<uint8_t>(refInstFM_[inch]->getEnvelopeParameter(FMEnvelopeParameter::ML3));
	envFM_[inch]->setParameterValue(FMEnvelopeParameter::ML3, data2);
	data1 |= data2;
	opna_->setRegister(0x30 + offset, data1);

	data1 = static_cast<uint8_t>(refInstFM_[inch]->getEnvelopeParameter(FMEnvelopeParameter::TL3));
	// Adjust volume
	if (mode_ == SongType::FM3chExpanded && inch == 2) data1 = calculateTL(7, data1);
	else if (isCarrier(2, al)) data1 = calculateTL(inch, data1);
	envFM_[inch]->setParameterValue(FMEnvelopeParameter::TL3, data1);
	opna_->setRegister(0x40 + offset, data1);

	data1 = static_cast<uint8_t>(refInstFM_[inch]->getEnvelopeParameter(FMEnvelopeParameter::KS3));
	envFM_[inch]->setParameterValue(FMEnvelopeParameter::KS3, data1);
	data1 <<= 6;
	data2 = static_cast<uint8_t>(refInstFM_[inch]->getEnvelopeParameter(FMEnvelopeParameter::AR3));
	envFM_[inch]->setParameterValue(FMEnvelopeParameter::AR3, data2);
	data1 |= data2;
	opna_->setRegister(0x50 + offset, data1);

	data1 = refInstFM_[inch]->getLFOEnabled() ? static_cast<uint8_t>(refInstFM_[inch]->getLFOParameter(FMLFOParameter::AM3)) : 0;
	data1 <<= 7;
	data2 = static_cast<uint8_t>(refInstFM_[inch]->getEnvelopeParameter(FMEnvelopeParameter::DR3));
	envFM_[inch]->setParameterValue(FMEnvelopeParameter::DR3, data2);
	data1 |= data2;
	opna_->setRegister(0x60 + offset, data1);

	data1 = static_cast<uint8_t>(refInstFM_[inch]->getEnvelopeParameter(FMEnvelopeParameter::SR3));
	envFM_[inch]->setParameterValue(FMEnvelopeParameter::SR3, data2);
	opna_->setRegister(0x70 + offset, data1);

	data1 = static_cast<uint8_t>(refInstFM_[inch]->getEnvelopeParameter(FMEnvelopeParameter::SL3));
	envFM_[inch]->setParameterValue(FMEnvelopeParameter::SL3, data1);
	data1 <<= 4;
	data2 = static_cast<uint8_t>(refInstFM_[inch]->getEnvelopeParameter(FMEnvelopeParameter::RR3));
	envFM_[inch]->setParameterValue(FMEnvelopeParameter::RR3, data2);
	data1 |= data2;
	opna_->setRegister(0x80 + offset, data1);

	tmp = refInstFM_[inch]->getEnvelopeParameter(FMEnvelopeParameter::SSGEG3);
	envFM_[inch]->setParameterValue(FMEnvelopeParameter::SSGEG3, tmp);
	data1 = judgeSSEGRegisterValue(tmp);
	opna_->setRegister(0x90 + offset, data1);

	offset = bch + 12;	// Operator 4

	data1 = static_cast<uint8_t>(refInstFM_[inch]->getEnvelopeParameter(FMEnvelopeParameter::DT4));
	envFM_[inch]->setParameterValue(FMEnvelopeParameter::DT4, data1);
	data1 <<= 4;
	data2 = static_cast<uint8_t>(refInstFM_[inch]->getEnvelopeParameter(FMEnvelopeParameter::ML4));
	envFM_[inch]->setParameterValue(FMEnvelopeParameter::ML4, data2);
	data1 |= data2;
	opna_->setRegister(0x30 + offset, data1);

	data1 = static_cast<uint8_t>(refInstFM_[inch]->getEnvelopeParameter(FMEnvelopeParameter::TL4));
	// Adjust volume
	if (mode_ == SongType::FM3chExpanded && inch == 2) data1 = calculateTL(8, data1);
	else data1 = calculateTL(inch, data1);
	envFM_[inch]->setParameterValue(FMEnvelopeParameter::TL4, data1);
	opna_->setRegister(0x40 + offset, data1);

	data1 = static_cast<uint8_t>(refInstFM_[inch]->getEnvelopeParameter(FMEnvelopeParameter::KS4));
	envFM_[inch]->setParameterValue(FMEnvelopeParameter::KS4, data1);
	data1 <<= 6;
	data2 = static_cast<uint8_t>(refInstFM_[inch]->getEnvelopeParameter(FMEnvelopeParameter::AR4));
	envFM_[inch]->setParameterValue(FMEnvelopeParameter::AR4, data2);
	data1 |= data2;
	opna_->setRegister(0x50 + offset, data1);

	data1 = refInstFM_[inch]->getLFOEnabled() ? static_cast<uint8_t>(refInstFM_[inch]->getLFOParameter(FMLFOParameter::AM4)) : 0;
	data1 <<= 7;
	data2 = static_cast<uint8_t>(refInstFM_[inch]->getEnvelopeParameter(FMEnvelopeParameter::DR4));
	envFM_[inch]->setParameterValue(FMEnvelopeParameter::DR4, data2);
	data1 |= data2;
	opna_->setRegister(0x60 + offset, data1);

	data1 = static_cast<uint8_t>(refInstFM_[inch]->getEnvelopeParameter(FMEnvelopeParameter::SR4));
	envFM_[inch]->setParameterValue(FMEnvelopeParameter::SR4, data2);
	opna_->setRegister(0x70 + offset, data1);

	data1 = static_cast<uint8_t>(refInstFM_[inch]->getEnvelopeParameter(FMEnvelopeParameter::SL4));
	envFM_[inch]->setParameterValue(FMEnvelopeParameter::SL4, data1);
	data1 <<= 4;
	data2 = static_cast<uint8_t>(refInstFM_[inch]->getEnvelopeParameter(FMEnvelopeParameter::RR4));
	envFM_[inch]->setParameterValue(FMEnvelopeParameter::RR4, data2);
	data1 |= data2;
	opna_->setRegister(0x80 + offset, data1);

	tmp = refInstFM_[inch]->getEnvelopeParameter(FMEnvelopeParameter::SSGEG4);
	envFM_[inch]->setParameterValue(FMEnvelopeParameter::SSGEG4, tmp);
	data1 = judgeSSEGRegisterValue(tmp);
	opna_->setRegister(0x90 + offset, data1);
}

void OPNAController::writeFMEnveropeParameterToRegister(int inch, FMEnvelopeParameter param, int value)
{
	uint32_t bch = getFMChannelOffset(inch);	// Bank and channel offset
	uint8_t data;
	int tmp;

	envFM_[inch]->setParameterValue(param, value);

	switch (param) {
	case FMEnvelopeParameter::AL:
	case FMEnvelopeParameter::FB:
		data = static_cast<uint8_t>(envFM_[inch]->getParameterValue(FMEnvelopeParameter::FB) << 3);
		data += envFM_[inch]->getParameterValue(FMEnvelopeParameter::AL);
		opna_->setRegister(0xb0 + bch, data);
		break;
	case FMEnvelopeParameter::DT1:
	case FMEnvelopeParameter::ML1:
		data = static_cast<uint8_t>(envFM_[inch]->getParameterValue(FMEnvelopeParameter::DT1) << 4);
		data |= envFM_[inch]->getParameterValue(FMEnvelopeParameter::ML1);
		opna_->setRegister(0x30 + bch, data);
		break;
	case FMEnvelopeParameter::TL1:
		data = static_cast<uint8_t>(envFM_[inch]->getParameterValue(FMEnvelopeParameter::TL1));
		// Adjust volume
		if (mode_ == SongType::FM3chExpanded && inch == 2) {
			data = calculateTL(2, data);
			envFM_[inch]->setParameterValue(FMEnvelopeParameter::TL1, data);	// Update
		}
		else if (isCarrier(0, envFM_[inch]->getParameterValue(FMEnvelopeParameter::AL))) {
			data = calculateTL(inch, data);
			envFM_[inch]->setParameterValue(FMEnvelopeParameter::TL1, data);	// Update
		}
		opna_->setRegister(0x40 + bch, data);
		break;
	case FMEnvelopeParameter::KS1:
	case FMEnvelopeParameter::AR1:
		data = static_cast<uint8_t>(envFM_[inch]->getParameterValue(FMEnvelopeParameter::KS1) << 6);
		data |= envFM_[inch]->getParameterValue(FMEnvelopeParameter::AR1);
		opna_->setRegister(0x50 + bch, data);
		break;
	case FMEnvelopeParameter::DR1:
		if (refInstFM_[inch] && refInstFM_[inch]->getLFOEnabled()) {
			data = static_cast<uint8_t>(refInstFM_[inch]->getLFOParameter(FMLFOParameter::AM1) << 7);
		}
		else {
			data = 0;
		}
		data |= envFM_[inch]->getParameterValue(FMEnvelopeParameter::DR1);
		opna_->setRegister(0x60 + bch, data);
		break;
	case FMEnvelopeParameter::SR1:
		data = static_cast<uint8_t>(envFM_[inch]->getParameterValue(FMEnvelopeParameter::SR1));
		opna_->setRegister(0x70 + bch, data);
		break;
	case FMEnvelopeParameter::SL1:
	case FMEnvelopeParameter::RR1:
		data = static_cast<uint8_t>(envFM_[inch]->getParameterValue(FMEnvelopeParameter::SL1) << 4);
		data |= envFM_[inch]->getParameterValue(FMEnvelopeParameter::RR1);
		opna_->setRegister(0x80 + bch, data);
		break;
	case::FMEnvelopeParameter::SSGEG1:
		tmp = envFM_[inch]->getParameterValue(FMEnvelopeParameter::SSGEG1);
		data = (tmp == -1) ? 0 : static_cast<uint8_t>(0x08 + tmp);
		opna_->setRegister(0x90 + bch, data);
		break;
	case FMEnvelopeParameter::DT2:
	case FMEnvelopeParameter::ML2:
		data = static_cast<uint8_t>(envFM_[inch]->getParameterValue(FMEnvelopeParameter::DT2) << 4);
		data |= envFM_[inch]->getParameterValue(FMEnvelopeParameter::ML2);
		opna_->setRegister(0x30 + bch + 8, data);
		break;
	case FMEnvelopeParameter::TL2:
		data = static_cast<uint8_t>(envFM_[inch]->getParameterValue(FMEnvelopeParameter::TL2));
		// Adjust volume
		if (mode_ == SongType::FM3chExpanded && inch == 2) {
			data = calculateTL(6, data);
			envFM_[inch]->setParameterValue(FMEnvelopeParameter::TL2, data);	// Update
		}
		else if (isCarrier(1, envFM_[inch]->getParameterValue(FMEnvelopeParameter::AL))) {
			data = calculateTL(inch, data);
			envFM_[inch]->setParameterValue(FMEnvelopeParameter::TL2, data);	// Update
		}
		opna_->setRegister(0x40 + bch + 8, data);
		break;
	case FMEnvelopeParameter::KS2:
	case FMEnvelopeParameter::AR2:
		data = static_cast<uint8_t>(envFM_[inch]->getParameterValue(FMEnvelopeParameter::KS2) << 6);
		data |= envFM_[inch]->getParameterValue(FMEnvelopeParameter::AR2);
		opna_->setRegister(0x50 + bch + 8, data);
		break;
	case FMEnvelopeParameter::DR2:
		if (refInstFM_[inch] && refInstFM_[inch]->getLFOEnabled()) {
			data = static_cast<uint8_t>(refInstFM_[inch]->getLFOParameter(FMLFOParameter::AM2) << 7);
		}
		else {
			data = 0;
		}
		data |= envFM_[inch]->getParameterValue(FMEnvelopeParameter::DR2);
		opna_->setRegister(0x60 + bch + 8, data);
		break;
	case FMEnvelopeParameter::SR2:
		data = static_cast<uint8_t>(envFM_[inch]->getParameterValue(FMEnvelopeParameter::SR2));
		opna_->setRegister(0x70 + bch + 8, data);
		break;
	case FMEnvelopeParameter::SL2:
	case FMEnvelopeParameter::RR2:
		data = static_cast<uint8_t>(envFM_[inch]->getParameterValue(FMEnvelopeParameter::SL2) << 4);
		data |= envFM_[inch]->getParameterValue(FMEnvelopeParameter::RR2);
		opna_->setRegister(0x80 + bch + 8, data);
		break;
	case FMEnvelopeParameter::SSGEG2:
		tmp = envFM_[inch]->getParameterValue(FMEnvelopeParameter::SSGEG2);
		data = (tmp == -1) ? 0 : static_cast<uint8_t>(0x08 + tmp);
		opna_->setRegister(0x90 + bch + 8, data);
		break;
	case FMEnvelopeParameter::DT3:
	case FMEnvelopeParameter::ML3:
		data = static_cast<uint8_t>(envFM_[inch]->getParameterValue(FMEnvelopeParameter::DT3) << 4);
		data |= envFM_[inch]->getParameterValue(FMEnvelopeParameter::ML3);
		opna_->setRegister(0x30 + bch + 4, data);
		break;
	case FMEnvelopeParameter::TL3:
		data = static_cast<uint8_t>(envFM_[inch]->getParameterValue(FMEnvelopeParameter::TL3));
		// Adjust volume
		if (mode_ == SongType::FM3chExpanded && inch == 2) {
			data = calculateTL(7, data);
			envFM_[inch]->setParameterValue(FMEnvelopeParameter::TL3, data);	// Update
		}
		else if (isCarrier(2, envFM_[inch]->getParameterValue(FMEnvelopeParameter::AL))) {
			data = calculateTL(inch, data);
			envFM_[inch]->setParameterValue(FMEnvelopeParameter::TL3, data);	// Update
		}
		opna_->setRegister(0x40 + bch + 4, data);
		break;
	case FMEnvelopeParameter::KS3:
	case FMEnvelopeParameter::AR3:
		data = static_cast<uint8_t>(envFM_[inch]->getParameterValue(FMEnvelopeParameter::KS3) << 6);
		data |= envFM_[inch]->getParameterValue(FMEnvelopeParameter::AR3);
		opna_->setRegister(0x50 + bch + 4, data);
		break;
	case FMEnvelopeParameter::DR3:
		if (refInstFM_[inch] && refInstFM_[inch]->getLFOEnabled()) {
			data = static_cast<uint8_t>(refInstFM_[inch]->getLFOParameter(FMLFOParameter::AM3) << 7);
		}
		else {
			data = 0;
		}
		data |= envFM_[inch]->getParameterValue(FMEnvelopeParameter::DR3);
		opna_->setRegister(0x60 + bch + 4, data);
		break;
	case FMEnvelopeParameter::SR3:
		data = static_cast<uint8_t>(envFM_[inch]->getParameterValue(FMEnvelopeParameter::SR3));
		opna_->setRegister(0x70 + bch + 4, data);
		break;
	case FMEnvelopeParameter::SL3:
	case FMEnvelopeParameter::RR3:
		data = static_cast<uint8_t>(envFM_[inch]->getParameterValue(FMEnvelopeParameter::SL3) << 4);
		data |= envFM_[inch]->getParameterValue(FMEnvelopeParameter::RR3);
		opna_->setRegister(0x80 + bch + 4, data);
		break;
	case FMEnvelopeParameter::SSGEG3:
		tmp = envFM_[inch]->getParameterValue(FMEnvelopeParameter::SSGEG3);
		data = (tmp == -1) ? 0 : static_cast<uint8_t>(0x08 + tmp);
		opna_->setRegister(0x90 + bch + 4, data);
		break;
	case FMEnvelopeParameter::DT4:
	case FMEnvelopeParameter::ML4:
		data = static_cast<uint8_t>(envFM_[inch]->getParameterValue(FMEnvelopeParameter::DT4) << 4);
		data |= envFM_[inch]->getParameterValue(FMEnvelopeParameter::ML4);
		opna_->setRegister(0x30 + bch + 12, data);
		break;
	case FMEnvelopeParameter::TL4:
		data = static_cast<uint8_t>(envFM_[inch]->getParameterValue(FMEnvelopeParameter::TL4));
		// Adjust volume
		if (mode_ == SongType::FM3chExpanded && inch == 2) {
			data = calculateTL(8, data);
			envFM_[inch]->setParameterValue(FMEnvelopeParameter::TL4, data);	// Update
		}
		else {
			data = calculateTL(inch, data);
			envFM_[inch]->setParameterValue(FMEnvelopeParameter::TL4, data);	// Update
		}
		opna_->setRegister(0x40 + bch + 12, data);
		break;
	case FMEnvelopeParameter::KS4:
	case FMEnvelopeParameter::AR4:
		data = static_cast<uint8_t>(envFM_[inch]->getParameterValue(FMEnvelopeParameter::KS4) << 6);
		data |= envFM_[inch]->getParameterValue(FMEnvelopeParameter::AR4);
		opna_->setRegister(0x50 + bch + 12, data);
		break;
	case FMEnvelopeParameter::DR4:
		if (refInstFM_[inch] && refInstFM_[inch]->getLFOEnabled()) {
			data = static_cast<uint8_t>(refInstFM_[inch]->getLFOParameter(FMLFOParameter::AM4) << 7);
		}
		else {
			data = 0;
		}
		data |= envFM_[inch]->getParameterValue(FMEnvelopeParameter::DR4);
		opna_->setRegister(0x60 + bch + 12, data);
		break;
	case FMEnvelopeParameter::SR4:
		data = static_cast<uint8_t>(envFM_[inch]->getParameterValue(FMEnvelopeParameter::SR4));
		opna_->setRegister(0x70 + bch + 12, data);
		break;
	case FMEnvelopeParameter::SL4:
	case FMEnvelopeParameter::RR4:
		data = static_cast<uint8_t>(envFM_[inch]->getParameterValue(FMEnvelopeParameter::SL4) << 4);
		data |= envFM_[inch]->getParameterValue(FMEnvelopeParameter::RR4);
		opna_->setRegister(0x80 + bch + 12, data);
		break;
	case FMEnvelopeParameter::SSGEG4:
		tmp = envFM_[inch]->getParameterValue(FMEnvelopeParameter::SSGEG4);
		data = judgeSSEGRegisterValue(tmp);
		opna_->setRegister(0x90 + bch + 12, data);
		break;
	}
}

void OPNAController::restoreFMEnvelopeFromReset(int ch)
{
	int inch = toInternalFMChannel(ch);

	if (hasResetEnvFM_[ch] == false || !refInstFM_[inch]) return;

	switch (mode_) {
	case SongType::Standard:
	{
		if (refInstFM_[inch]->getEnvelopeResetEnabled(FMOperatorType::All)) {
			hasResetEnvFM_[inch] = false;
			writeFMEnveropeParameterToRegister(inch, FMEnvelopeParameter::RR1,
											   refInstFM_[inch]->getEnvelopeParameter(FMEnvelopeParameter::RR1));
			writeFMEnveropeParameterToRegister(inch, FMEnvelopeParameter::RR2,
											   refInstFM_[inch]->getEnvelopeParameter(FMEnvelopeParameter::RR2));
			writeFMEnveropeParameterToRegister(inch, FMEnvelopeParameter::RR3,
											   refInstFM_[inch]->getEnvelopeParameter(FMEnvelopeParameter::RR3));
			writeFMEnveropeParameterToRegister(inch, FMEnvelopeParameter::RR4,
											   refInstFM_[inch]->getEnvelopeParameter(FMEnvelopeParameter::RR4));
		}
		break;
	}
	case SongType::FM3chExpanded:
	{
		if (refInstFM_[inch]->getEnvelopeResetEnabled(toChannelOperatorType(ch))) {
			if (inch == 2) {
				FMEnvelopeParameter param;
				switch (ch) {
				case 2:	param = FMEnvelopeParameter::RR1;	break;
				case 6:	param = FMEnvelopeParameter::RR2;	break;
				case 7:	param = FMEnvelopeParameter::RR3;	break;
				case 8:	param = FMEnvelopeParameter::RR4;	break;
				default:	throw std::out_of_range("out of range.");
				}
				hasResetEnvFM_[ch] = false;
				writeFMEnveropeParameterToRegister(2, param, refInstFM_[2]->getEnvelopeParameter(param));
			}
			else {
				hasResetEnvFM_[inch] = false;
				writeFMEnveropeParameterToRegister(inch, FMEnvelopeParameter::RR1,
												   refInstFM_[inch]->getEnvelopeParameter(FMEnvelopeParameter::RR1));
				writeFMEnveropeParameterToRegister(inch, FMEnvelopeParameter::RR2,
												   refInstFM_[inch]->getEnvelopeParameter(FMEnvelopeParameter::RR2));
				writeFMEnveropeParameterToRegister(inch, FMEnvelopeParameter::RR3,
												   refInstFM_[inch]->getEnvelopeParameter(FMEnvelopeParameter::RR3));
				writeFMEnveropeParameterToRegister(inch, FMEnvelopeParameter::RR4,
												   refInstFM_[inch]->getEnvelopeParameter(FMEnvelopeParameter::RR4));
			}
		}
		break;
	}
	}
}

void OPNAController::writeFMLFOAllRegisters(int inch)
{
	if (!refInstFM_[inch]->getLFOEnabled() || lfoStartCntFM_[inch] > 0) {	// Clear data
		uint32_t bch = getFMChannelOffset(inch);	// Bank and channel offset
		opna_->setRegister(0xb4 + bch, static_cast<uint8_t>(panFM_[inch] << 6));
		opna_->setRegister(0x60 + bch, static_cast<uint8_t>(envFM_[inch]->getParameterValue(FMEnvelopeParameter::DR1)));
		opna_->setRegister(0x60 + bch + 8, static_cast<uint8_t>(envFM_[inch]->getParameterValue(FMEnvelopeParameter::DR2)));
		opna_->setRegister(0x60 + bch + 4, static_cast<uint8_t>(envFM_[inch]->getParameterValue(FMEnvelopeParameter::DR3)));
		opna_->setRegister(0x60 + bch + 12, static_cast<uint8_t>(envFM_[inch]->getParameterValue(FMEnvelopeParameter::DR4)));
	}
	else {
		writeFMLFORegister(inch, FMLFOParameter::FREQ);
		writeFMLFORegister(inch, FMLFOParameter::PMS);
		writeFMLFORegister(inch, FMLFOParameter::AMS);
		writeFMLFORegister(inch, FMLFOParameter::AM1);
		writeFMLFORegister(inch, FMLFOParameter::AM2);
		writeFMLFORegister(inch, FMLFOParameter::AM3);
		writeFMLFORegister(inch, FMLFOParameter::AM4);
		lfoStartCntFM_[inch] = -1;
	}
}

void OPNAController::writeFMLFORegister(int inch, FMLFOParameter param)
{
	uint32_t bch = getFMChannelOffset(inch);	// Bank and channel offset
	uint8_t data;

	switch (param) {
	case FMLFOParameter::FREQ:
		lfoFreq_ = refInstFM_[inch]->getLFOParameter(FMLFOParameter::FREQ);
		opna_->setRegister(0x22, static_cast<uint8_t>(lfoFreq_ | (1 << 3)));
		break;
	case FMLFOParameter::PMS:
	case FMLFOParameter::AMS:
		data = static_cast<uint8_t>(panFM_[inch] << 6);
		data |= (refInstFM_[inch]->getLFOParameter(FMLFOParameter::AMS) << 4);
		data |= refInstFM_[inch]->getLFOParameter(FMLFOParameter::PMS);
		opna_->setRegister(0xb4 + bch, data);
		break;
	case FMLFOParameter::AM1:
		data = static_cast<uint8_t>(refInstFM_[inch]->getLFOParameter(FMLFOParameter::AM1) << 7);
		data |= envFM_[inch]->getParameterValue(FMEnvelopeParameter::DR1);
		opna_->setRegister(0x60 + bch, data);
		break;
	case FMLFOParameter::AM2:
		data = static_cast<uint8_t>(refInstFM_[inch]->getLFOParameter(FMLFOParameter::AM2) << 7);
		data |= envFM_[inch]->getParameterValue(FMEnvelopeParameter::DR2);
		opna_->setRegister(0x60 + bch + 8, data);
		break;
	case FMLFOParameter::AM3:
		data = static_cast<uint8_t>(refInstFM_[inch]->getLFOParameter(FMLFOParameter::AM3) << 7);
		data |= envFM_[inch]->getParameterValue(FMEnvelopeParameter::DR3);
		opna_->setRegister(0x60 + bch + 4, data);
		break;
	case FMLFOParameter::AM4:
		data = static_cast<uint8_t>(refInstFM_[inch]->getLFOParameter(FMLFOParameter::AM4) << 7);
		data |= envFM_[inch]->getParameterValue(FMEnvelopeParameter::DR4);
		opna_->setRegister(0x60 + bch + 12, data);
		break;
	default:
		break;
	}
}

void OPNAController::checkLFOUsed()
{
	for (int inch = 0; inch < 6; ++inch) {
		if (refInstFM_[inch] && refInstFM_[inch]->getLFOEnabled()) return;
	}

	if (lfoFreq_ != -1) {
		lfoFreq_ = -1;
		opna_->setRegister(0x22, 0);	// LFO off
	}
}

void OPNAController::setFrontFMSequences(int ch)
{
	if (isMuteFM_[ch]) return;

	int inch = toInternalFMChannel(ch);
	if (refInstFM_[inch] && refInstFM_[inch]->getLFOEnabled()) {
		lfoStartCntFM_[inch] = refInstFM_[inch]->getLFOParameter(FMLFOParameter::Count);
		writeFMLFOAllRegisters(inch);
	}
	else {
		lfoStartCntFM_[inch] = -1;
	}

	checkOperatorSequenceFM(ch, 1);

	if (auto& treIt = treItFM_[ch]) treIt->front();
	sumVolSldFM_[ch] += volSldFM_[ch];
	checkVolumeEffectFM(ch);

	if (auto& arpIt = arpItFM_[ch]) {
		arpIt->front();
		checkRealToneFMByArpeggio(ch);
	}
	checkPortamentoFM(ch);

	if (auto& ptIt = ptItFM_[ch]) {
		ptIt->front();
		checkRealToneFMByPitch(ch);
	}
	if (auto& vibIt = vibItFM_[ch]) {
		vibIt->front();
		needToneSetFM_[ch] = true;
	}
	if (auto& nsIt = nsItFM_[ch]) {
		nsIt->front();
		if (!nsIt->hasEnded()) {
			sumNoteSldFM_[ch] += nsIt->data().data;
			needToneSetFM_[ch] = true;
		}
	}

	writePitchFM(ch);
}

void OPNAController::releaseStartFMSequences(int ch)
{
	if (isMuteFM_[ch]) return;

	int inch = toInternalFMChannel(ch);
	if (lfoStartCntFM_[inch] > 0) {
		--lfoStartCntFM_[inch];
		writeFMLFOAllRegisters(inch);
	}

	checkOperatorSequenceFM(ch, 2);

	if (auto& treIt = treItFM_[ch]) treIt->release();
	sumVolSldFM_[ch] += volSldFM_[ch];
	checkVolumeEffectFM(ch);

	if (auto& arpIt = arpItFM_[ch]) {
		arpIt->release();
		checkRealToneFMByArpeggio(ch);
	}
	checkPortamentoFM(ch);

	if (auto& ptIt = ptItFM_[ch]) {
		ptIt->release();
		checkRealToneFMByPitch(ch);
	}
	if (auto& vibIt = vibItFM_[ch]) {
		vibIt->release();
		needToneSetFM_[ch] = true;
	}
	if (auto& nsIt = nsItFM_[ch]) {
		nsIt->release();
		if (!nsIt->hasEnded()) {
			sumNoteSldFM_[ch] += nsIt->data().data;
			needToneSetFM_[ch] = true;
		}
	}

	if (needToneSetFM_[ch]) writePitchFM(ch);
}

void OPNAController::tickEventFM(int ch)
{
	if (hasPreSetTickEventFM_[ch]) {
		hasPreSetTickEventFM_[ch] = false;
	}
	else {
		if (isMuteFM_[ch]) return;

		int inch = toInternalFMChannel(ch);
		if (lfoStartCntFM_[inch] > 0) {
			--lfoStartCntFM_[inch];
			writeFMLFOAllRegisters(inch);
		}

		checkOperatorSequenceFM(ch, 0);

		if (auto& treIt = treItFM_[ch]) treIt->next();
		sumVolSldFM_[ch] += volSldFM_[ch];
		checkVolumeEffectFM(ch);

		if (auto& arpIt = arpItFM_[ch]) {
			arpIt->next();
			checkRealToneFMByArpeggio(ch);
		}
		checkPortamentoFM(ch);

		if (auto& ptIt = ptItFM_[ch]) {
			ptIt->next();
			checkRealToneFMByPitch(ch);
		}
		if (auto& vibIt = vibItFM_[ch]) {
			vibIt->next();
			needToneSetFM_[ch] = true;
		}
		if (auto& nsIt = nsItFM_[ch]) {
			nsIt->next();
			if (!nsIt->hasEnded()) {
				sumNoteSldFM_[ch] += nsIt->data().data;
				needToneSetFM_[ch] = true;
			}
		}

		if (needToneSetFM_[ch]) writePitchFM(ch);
	}
}

void OPNAController::checkOperatorSequenceFM(int ch, int type)
{
	int inch = toInternalFMChannel(ch);
	for (auto& p : FM_ENV_PARAMS_OP_.at(toChannelOperatorType(ch))) {
		if (auto& it = opSeqItFM_[inch].at(p)) {
			switch (type) {
			case 0:	it->next();		break;
			case 1:	it->front();	break;
			case 2:	it->release();	break;
			default:	throw std::out_of_range("The range of type is 0-2.");
			}
			if (!it->hasEnded()) {
				int d = it->data().data;
				if (d != envFM_[inch]->getParameterValue(p)) {
					writeFMEnveropeParameterToRegister(inch, p, d);
				}
			}
		}
	}
}

void OPNAController::checkVolumeEffectFM(int ch)
{
	int v;
	if (auto& treIt = treItFM_[ch]) {
		v = treIt->data().data + sumVolSldFM_[ch];
	}
	else {
		if (volSldFM_[ch]) v = sumVolSldFM_[ch];
		else return;
	}

	uint32_t bch = getFMChannelOffset(ch);
	int inch = toInternalFMChannel(ch);
	switch (toChannelOperatorType(ch)) {
	case FMOperatorType::All:
	{
		int al = envFM_[ch]->getParameterValue(FMEnvelopeParameter::AL);
		if (isCarrier(0, al)) {	// Operator 1
			int data = envFM_[ch]->getParameterValue(FMEnvelopeParameter::TL1) + v;
			opna_->setRegister(0x40 + bch, static_cast<uint8_t>(clamp(data, 0 ,127)));
		}
		if (isCarrier(1, al)) {	// Operator 2
			int data = envFM_[ch]->getParameterValue(FMEnvelopeParameter::TL2) + v;
			opna_->setRegister(0x40 + bch + 8, static_cast<uint8_t>(clamp(data, 0 ,127)));
		}
		if (isCarrier(2, al)) {	// Operator 3
			int data = envFM_[ch]->getParameterValue(FMEnvelopeParameter::TL3) + v;
			opna_->setRegister(0x40 + bch + 4, static_cast<uint8_t>(clamp(data, 0 ,127)));
		}
		if (isCarrier(3, al)) {	// Operator 4
			int data = envFM_[ch]->getParameterValue(FMEnvelopeParameter::TL4) + v;
			opna_->setRegister(0x40 + bch + 12, static_cast<uint8_t>(clamp(data, 0 ,127)));
		}
		break;
	}
	case FMOperatorType::Op1:
	{
		int data = envFM_[inch]->getParameterValue(FMEnvelopeParameter::TL1) + v;
		opna_->setRegister(0x40 + bch, static_cast<uint8_t>(clamp(data, 0 ,127)));
		break;
	}
	case FMOperatorType::Op2:
	{
		int data = envFM_[inch]->getParameterValue(FMEnvelopeParameter::TL2) + v;
		opna_->setRegister(0x40 + bch + 8, static_cast<uint8_t>(clamp(data, 0 ,127)));
		break;
	}
	case FMOperatorType::Op3:
	{
		int data = envFM_[inch]->getParameterValue(FMEnvelopeParameter::TL3) + v;
		opna_->setRegister(0x40 + bch + 4, static_cast<uint8_t>(clamp(data, 0 ,127)));
		break;
	}
	case FMOperatorType::Op4:
	{
		int data = envFM_[inch]->getParameterValue(FMEnvelopeParameter::TL4) + v;
		opna_->setRegister(0x40 + bch + 12, static_cast<uint8_t>(clamp(data, 0 ,127)));
		break;
	}
	}
}

void OPNAController::writePitchFM(int ch)
{
	if (keyToneFM_[ch].octave == -1) return;	// Not set note yet

	uint16_t p = calc_pitch::calculateFNumber(
					 keyToneFM_[ch].note,
					 keyToneFM_[ch].octave,
					 keyToneFM_[ch].pitch
					 + sumPitchFM_[ch]
					 + (vibItFM_[ch] ? vibItFM_[ch]->data().data : 0)
					 + detuneFM_[ch]
					 + sumNoteSldFM_[ch]
					 + transposeFM_[ch],
					 fdetuneFM_[ch]);
	uint32_t offset = getFMChannelOffset(ch, true);
	opna_->setRegister(0xa4 + offset, p >> 8);
	opna_->setRegister(0xa0 + offset, p & 0x00ff);

	needToneSetFM_[ch] = false;
}

void OPNAController::setInstrumentFMProperties(int ch)
{
	int inch = toInternalFMChannel(ch);
	FMOperatorType opType = toChannelOperatorType(ch);
	enableEnvResetFM_[ch] = refInstFM_[inch]->getEnvelopeResetEnabled(opType);
}

std::vector<int> OPNAController::getOperatorsInLevel(int level, int al)
{
	switch (level) {
	case 0:
		switch (al) {
		case 0:
		case 1:
		case 2:
		case 3:
			return { 3 };
		case 4:
			return { 1, 3 };
		case 5:
		case 6:
			return { 1, 2, 3 };
		case 7:
			return { 0, 1, 2, 3 };
		default:
			throw std::invalid_argument("Invalid algorithm.");
		}
	case 1:
		switch (al) {
		case 0:
		case 1:
			return { 2 };
		case 2:
			return { 0, 2 };
		case 3:
			return { 1, 2 };
		case 4:
			return { 0, 2 };
		case 5:
		case 6:
			return { 0 };
		case 7:
			return {};
		default:
			throw std::invalid_argument("Invalid algorithm.");
		}
	case 2:
		switch (al) {
		case 0:
			return { 1 };
		case 1:
			return { 0, 1 };
		case 2:
			return { 1 };
		case 3:
			return { 0 };
		case 4:
		case 5:
		case 6:
		case 7:
			return {};
		default:
			throw std::invalid_argument("Invalid algorithm.");
		}
	case 3:
		switch (al) {
		case 0:
			return { 0 };
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
			return {};
		default:
			throw std::invalid_argument("Invalid algorithm.");
		}
	default:
		throw std::invalid_argument("Invalid operator level.");
	}
}

//---------- SSG ----------//
/********** Key on-off **********/
void OPNAController::keyOnSSG(int ch, Note note, int octave, int pitch, bool isJam)
{
	if (isMuteSSG_[ch]) return;

	updateEchoBufferSSG(ch, octave, note, pitch);

	if (isTonePrtmSSG_[ch] && hasKeyOnBeforeSSG_[ch]) {
		keyToneSSG_[ch].pitch += (sumNoteSldSSG_[ch] +transposeSSG_[ch]);
	}
	else {
		keyToneSSG_[ch] = baseToneSSG_[ch].front();
		sumPitchSSG_[ch] = 0;
		sumVolSldSSG_[ch] = 0;
		tmpVolSSG_[ch] = -1;
	}
	if (!noteSldSSGSetFlag_) {
		nsItSSG_[ch].reset();
	}
	noteSldSSGSetFlag_ = false;
	needToneSetSSG_[ch] = true;
	sumNoteSldSSG_[ch] = 0;
	transposeSSG_[ch] = 0;

	isKeyOnSSG_[ch] = false;	// For first tick check

	setFrontSSGSequences(ch);

	hasPreSetTickEventSSG_[ch] = isJam;
	isKeyOnSSG_[ch] = true;
	hasKeyOnBeforeSSG_[ch] = true;
}

void OPNAController::keyOnSSG(int ch, int echoBuf)
{
	ToneDetail& td = baseToneSSG_[ch].at(static_cast<size_t>(echoBuf));
	if (td.octave == -1) return;
	keyOnSSG(ch, td.note, td.octave, td.pitch);
}

void OPNAController::keyOffSSG(int ch, bool isJam)
{
	if (!isKeyOnSSG_[ch]) {
		tickEventSSG(ch);
		return;
	}
	releaseStartSSGSequences(ch);
	hasPreSetTickEventSSG_[ch] = isJam;
	isKeyOnSSG_[ch] = false;
}

void OPNAController::updateEchoBufferSSG(int ch, int octave, Note note, int pitch)
{
	baseToneSSG_[ch].pop_back();
	baseToneSSG_[ch].push_front({ octave, note, pitch });
}

/********** Set instrument **********/
/// NOTE: inst != nullptr
void OPNAController::setInstrumentSSG(int ch, std::shared_ptr<InstrumentSSG> inst)
{
	refInstSSG_[ch] = inst;

	if (inst->getWaveformEnabled())
		wfItSSG_[ch] = inst->getWaveformSequenceIterator();
	else
		wfItSSG_[ch].reset();
	if (inst->getToneNoiseEnabled())
		tnItSSG_[ch] = inst->getToneNoiseSequenceIterator();
	else
		tnItSSG_[ch].reset();
	if (inst->getEnvelopeEnabled())
		envItSSG_[ch] = inst->getEnvelopeSequenceIterator();
	else
		envItSSG_[ch].reset();
	if (!isArpEffSSG_[ch]) {
		if (inst->getArpeggioEnabled())
			arpItSSG_[ch] = inst->getArpeggioSequenceIterator();
		else
			arpItSSG_[ch].reset();
	}
	if (inst->getPitchEnabled())
		ptItSSG_[ch] = inst->getPitchSequenceIterator();
	else
		ptItSSG_[ch].reset();
}

void OPNAController::updateInstrumentSSG(int instNum)
{
	for (int ch = 0; ch < 3; ++ch) {
		if (refInstSSG_[ch] && refInstSSG_[ch]->isRegisteredWithManager()
				&& refInstSSG_[ch]->getNumber() == instNum) {
			if (!refInstSSG_[ch]->getWaveformEnabled()) wfItSSG_[ch].reset();
			if (!refInstSSG_[ch]->getToneNoiseEnabled()) tnItSSG_[ch].reset();
			if (!refInstSSG_[ch]->getEnvelopeEnabled()) envItSSG_[ch].reset();
			if (!refInstSSG_[ch]->getArpeggioEnabled()) arpItSSG_[ch].reset();
			if (!refInstSSG_[ch]->getPitchEnabled()) ptItSSG_[ch].reset();
		}
	}
}

/********** Set volume **********/
void OPNAController::setVolumeSSG(int ch, int volume)
{
	if (volume < NSTEP_SSG_VOLUME) {
		baseVolSSG_[ch] = volume;
		tmpVolSSG_[ch] = -1;

		if (isKeyOnSSG_[ch]) setRealVolumeSSG(ch);
	}
}

void OPNAController::setTemporaryVolumeSSG(int ch, int volume)
{
	if (volume < NSTEP_SSG_VOLUME) {
		tmpVolSSG_[ch] = volume;

		if (isKeyOnSSG_[ch]) setRealVolumeSSG(ch);
	}
}

void OPNAController::setRealVolumeSSG(int ch)
{
	if (isBuzzEffSSG_[ch] || isHardEnvSSG_[ch]) return;

	int volume = (tmpVolSSG_[ch] == -1) ? baseVolSSG_[ch] : tmpVolSSG_[ch];
	if (envItSSG_[ch]) {
		int type = envItSSG_[ch]->getCommandType();
		if (0 <= type && type < 16) {
			volume -= (15 - type);
		}
	}
	if (auto& treIt = treItSSG_[ch]) volume += treIt->data().data;
	volume += sumVolSldSSG_[ch];

	volume = clamp(volume, 0, 15);

	opna_->setRegister(0x08 + static_cast<uint32_t>(ch), static_cast<uint8_t>(volume));
	needEnvSetSSG_[ch] = false;
}

void OPNAController::setMasterVolumeSSG(double dB)
{
	opna_->setVolumeSSG(dB);
}

/********** Set effect **********/
void OPNAController::setArpeggioEffectSSG(int ch, int second, int third)
{
	if (second || third) {
		arpItSSG_[ch] = std::make_unique<ArpeggioEffectIterator>(second, third);
		isArpEffSSG_[ch] = true;
	}
	else {
		if (!refInstSSG_[ch] || !refInstSSG_[ch]->getArpeggioEnabled()) arpItSSG_[ch].reset();
		else arpItSSG_[ch] = refInstSSG_[ch]->getArpeggioSequenceIterator();
		isArpEffSSG_[ch] = false;
	}
}

void OPNAController::setPortamentoEffectSSG(int ch, int depth, bool isTonePortamento)
{
	prtmSSG_[ch] = depth;
	isTonePrtmSSG_[ch] =  depth ? isTonePortamento : false;
}

void OPNAController::setVibratoEffectSSG(int ch, int period, int depth)
{
	if (period && depth) vibItSSG_[ch] = std::make_unique<WavingEffectIterator>(period, depth);
	else vibItSSG_[ch].reset();
}

void OPNAController::setTremoloEffectSSG(int ch, int period, int depth)
{
	if (period && depth) treItSSG_[ch] = std::make_unique<WavingEffectIterator>(period, depth);
	else treItSSG_[ch].reset();
}

void OPNAController::setVolumeSlideSSG(int ch, int depth, bool isUp)
{
	volSldSSG_[ch] = isUp ? depth : -depth;
}

void OPNAController::setDetuneSSG(int ch, int pitch)
{
	detuneSSG_[ch] = pitch;
	needToneSetSSG_[ch] = true;
}

void OPNAController::setFineDetuneSSG(int ch, int pitch)
{
	fdetuneSSG_[ch] = pitch;
	needToneSetSSG_[ch] = true;
}

void OPNAController::setNoteSlideSSG(int ch, int speed, int seminote)
{
	if (seminote) {
		nsItSSG_[ch] = std::make_unique<NoteSlideEffectIterator>(speed, seminote);
		noteSldSSGSetFlag_ = true;
	}
	else nsItSSG_[ch].reset();
}

void OPNAController::setTransposeEffectSSG(int ch, int seminote)
{
	transposeSSG_[ch] += (seminote * calc_pitch::SEMINOTE_PITCH);
	needToneSetSSG_[ch] = true;
}

void OPNAController::setToneNoiseMixSSG(int ch, int value)
{
	toneNoiseMixSSG_[ch] = value;

	// Tone
	if ((tnSSG_[ch].isTone = (0x01 & value))) mixerSSG_ &= ~SSGToneFlag(ch);
	else mixerSSG_ |= SSGToneFlag(ch);
	// Noise
	if ((tnSSG_[ch].isNoise = (0x02 & value))) mixerSSG_ &= ~SSGNoiseFlag(ch);
	else mixerSSG_ |= SSGNoiseFlag(ch);
	opna_->setRegister(0x07, mixerSSG_);

	tnItSSG_[ch].reset();
}

void OPNAController::setNoisePitchSSG(int ch, int pitch)
{
	noisePitchSSG_ = pitch;
	tnSSG_[ch].noisePeriod_ = pitch;
	opna_->setRegister(0x06, static_cast<uint8_t>(31 - pitch));	// Reverse order
}

void OPNAController::setHardEnvelopePeriod(int ch, bool high, int period)
{
	bool sendable = isHardEnvSSG_[ch]
					&& (CommandSequenceUnit::checkDataType(envSSG_[ch].data) == CommandSequenceUnit::RAW);
	if (high) {
		hardEnvPeriodHighSSG_ = period;
		if (sendable) {
			envSSG_[ch].data = (period << 8) | (envSSG_[ch].data & 0x00ff);
			envSSG_[ch].data |= ~(1 << 16);	// raw data flag
			opna_->setRegister(0x0c, static_cast<uint8_t>(period));
		}
	}
	else {
		hardEnvPeriodLowSSG_ = period;
		if (sendable) {
			envSSG_[ch].data = (envSSG_[ch].data & 0xff00) | period;
			envSSG_[ch].data |= ~(1 << 16);	// raw data flag
			opna_->setRegister(0x0b, static_cast<uint8_t>(period));
		}
	}
}

void OPNAController::setAutoEnvelopeSSG(int ch, int shift, int shape)
{
	if (shape) {
		opna_->setRegister(0x0d, static_cast<uint8_t>(shape));
		envSSG_[ch].type = AUTO_ENV_SHAPE_TYPE_[shape - 1];
		opna_->setRegister(0x08 + static_cast<uint32_t>(ch), 0x10);
		isHardEnvSSG_[ch] = true;
		if (shift == -8) {	// Raw
			envSSG_[ch].data = (hardEnvPeriodHighSSG_ << 8) | hardEnvPeriodLowSSG_;
			opna_->setRegister(0x0c, static_cast<uint8_t>(hardEnvPeriodHighSSG_));
			opna_->setRegister(0x0b, static_cast<uint8_t>(hardEnvPeriodLowSSG_));
		}
		else {
			envSSG_[ch].data = CommandSequenceUnit::shift2data(shift);
		}
	}
	else {
		isHardEnvSSG_[ch] = false;
		envSSG_[ch] = { -1, CommandSequenceUnit::NODATA };
		// Clear hard envelope in setRealVolumeSSG
	}
	needEnvSetSSG_[ch] = true;
	envItSSG_[ch].reset();
}

/********** For state retrieve **********/
void OPNAController::haltSequencesSSG(int ch)
{
	if (auto& wfIt = wfItSSG_[ch]) wfIt->end();
	if (auto& treIt = treItSSG_[ch]) treIt->end();
	if (auto& envIt = envItSSG_[ch]) envIt->end();
	if (auto& tnIt = tnItSSG_[ch]) tnIt->end();
	if (auto& arpIt = arpItSSG_[ch]) arpIt->end();
	if (auto& ptIt = ptItSSG_[ch]) ptIt->end();
	if (auto& vibIt = vibItSSG_[ch]) vibIt->end();
	if (auto& nsIt = nsItSSG_[ch]) nsIt->end();
}

/********** Chip details **********/
bool OPNAController::isKeyOnSSG(int ch) const
{
	return isKeyOnSSG_[ch];
}

bool OPNAController::isTonePortamentoSSG(int ch) const
{
	return isTonePrtmSSG_[ch];
}

ToneDetail OPNAController::getSSGTone(int ch) const
{
	return baseToneSSG_[ch].front();
}

/***********************************/
void OPNAController::initSSG()
{
	mixerSSG_ = 0xff;
	opna_->setRegister(0x07, mixerSSG_);	// SSG mix
	noisePitchSSG_ = 0;
	hardEnvPeriodHighSSG_ = 0;
	hardEnvPeriodLowSSG_ = 0;

	for (int ch = 0; ch < 3; ++ch) {
		isKeyOnSSG_[ch] = false;
		hasKeyOnBeforeSSG_[ch] = false;

		refInstSSG_[ch].reset();	// Init envelope

		// Init echo buffer
		baseToneSSG_[ch] = std::deque<ToneDetail>(4);
		for (auto& td : baseToneSSG_[ch]) {
			td.octave = -1;
		}

		keyToneSSG_[ch].note = Note::C;	// Dummy
		keyToneSSG_[ch].octave = -1;
		keyToneSSG_[ch].pitch = 0;	// Dummy
		sumPitchSSG_[ch] = 0;
		tnSSG_[ch] = { false, false, CommandSequenceUnit::NODATA };
		baseVolSSG_[ch] = NSTEP_SSG_VOLUME - 1;	// Init volume
		tmpVolSSG_[ch] = -1;
		isHardEnvSSG_[ch] = false;
		isBuzzEffSSG_[ch] = false;

		// Init sequence
		hasPreSetTickEventSSG_[ch] = false;
		wfItSSG_[ch].reset();
		wfSSG_[ch] = { SSGWaveformType::UNSET, CommandSequenceUnit::NODATA };
		envItSSG_[ch].reset();
		envSSG_[ch] = { -1, CommandSequenceUnit::NODATA };
		tnItSSG_[ch].reset();
		arpItSSG_[ch].reset();
		ptItSSG_[ch].reset();
		needEnvSetSSG_[ch] = false;
		setHardEnvIfNecessary_[ch] = false;
		needMixSetSSG_[ch] = false;
		needToneSetSSG_[ch] = false;
		needSqMaskFreqSetSSG_[ch] = false;

		// Effect
		isArpEffSSG_[ch] = false;
		prtmSSG_[ch] = 0;
		isTonePrtmSSG_[ch] = false;
		vibItSSG_[ch].reset();
		treItSSG_[ch].reset();
		volSldSSG_[ch] = 0;
		sumVolSldSSG_[ch] = 0;
		detuneSSG_[ch] = 0;
		fdetuneSSG_[ch] = 0;
		nsItSSG_[ch].reset();
		sumNoteSldSSG_[ch] = 0;
		noteSldSSGSetFlag_ = false;
		transposeSSG_[ch] = 0;
		toneNoiseMixSSG_[ch] = 0;
	}
}

void OPNAController::setMuteSSGState(int ch, bool isMute)
{
	isMuteSSG_[ch] = isMute;

	if (isMute) {
		opna_->setRegister(0x08 + static_cast<uint32_t>(ch), 0);
		isKeyOnSSG_[ch] = false;
	}
}

bool OPNAController::isMuteSSG(int ch)
{
	return isMuteSSG_[ch];
}

void OPNAController::setFrontSSGSequences(int ch)
{
	if (isMuteSSG_[ch]) return;

	setHardEnvIfNecessary_[ch] = false;

	if (wfItSSG_[ch]) writeWaveformSSGToRegister(ch, wfItSSG_[ch]->front());
	else writeSquareWaveform(ch);

	if (auto& treIt = treItSSG_[ch]) {
		treIt->front();
		needEnvSetSSG_[ch] = true;
	}
	if (volSldSSG_[ch]) {
		sumVolSldSSG_[ch] += volSldSSG_[ch];
		needEnvSetSSG_[ch] = true;
	}
	if (envItSSG_[ch]) writeEnvelopeSSGToRegister(ch, envItSSG_[ch]->front());
	else setRealVolumeSSG(ch);

	if (tnItSSG_[ch]) writeToneNoiseSSGToRegister(ch, tnItSSG_[ch]->front());
	else if (needMixSetSSG_[ch]) writeToneNoiseSSGToRegisterNoReference(ch);

	if (auto& arpIt = arpItSSG_[ch]) {
		arpIt->front();
		checkRealToneSSGByArpeggio(ch);
	}
	checkPortamentoSSG(ch);

	if (auto& ptIt = ptItSSG_[ch]) {
		ptIt->front();
		checkRealToneSSGByPitch(ch);
	}
	if (auto& vibIt = vibItSSG_[ch]) {
		vibIt->front();
		needToneSetSSG_[ch] = true;
	}
	if (auto& nsIt = nsItSSG_[ch]) {
		nsIt->front();
		if (!nsIt->hasEnded()) {
			sumNoteSldSSG_[ch] += nsIt->data().data;
			needToneSetSSG_[ch] = true;
		}
	}

	writePitchSSG(ch);
}

void OPNAController::releaseStartSSGSequences(int ch)
{
	if (isMuteSSG_[ch]) return;

	setHardEnvIfNecessary_[ch] = false;

	if (wfItSSG_[ch]) writeWaveformSSGToRegister(ch, wfItSSG_[ch]->next(true));

	if (auto& treIt = treItSSG_[ch]) {
		treIt->release();
		needEnvSetSSG_[ch] = true;
	}
	if (volSldSSG_[ch]) {
		sumVolSldSSG_[ch] += volSldSSG_[ch];
		needEnvSetSSG_[ch] = true;
	}
	if (envItSSG_[ch]) {
		int pos = envItSSG_[ch]->next(true);
		if (pos == -1) {
			opna_->setRegister(0x08 + static_cast<uint32_t>(ch), 0);
			isHardEnvSSG_[ch] = false;
		}
		else writeEnvelopeSSGToRegister(ch, pos);
	}
	else {
		if (!hasPreSetTickEventSSG_[ch]) {
			opna_->setRegister(0x08 + static_cast<uint32_t>(ch), 0);
			isHardEnvSSG_[ch] = false;
		}
	}

	if (tnItSSG_[ch]) writeToneNoiseSSGToRegister(ch, tnItSSG_[ch]->next(true));
	else if (needMixSetSSG_[ch]) writeToneNoiseSSGToRegisterNoReference(ch);

	if (auto& arpIt = arpItSSG_[ch]) {
		arpIt->release();
		checkRealToneSSGByArpeggio(ch);
	}
	checkPortamentoSSG(ch);

	if (auto& ptIt = ptItSSG_[ch]) {
		ptIt->release();
		checkRealToneSSGByPitch(ch);
	}
	if (auto& vibIt = vibItSSG_[ch]) {
		vibIt->release();
		needToneSetSSG_[ch] = true;
	}
	if (auto& nsIt = nsItSSG_[ch]) {
		nsIt->release();
		if (!nsIt->hasEnded()) {
			sumNoteSldSSG_[ch] += nsIt->data().data;
			needToneSetSSG_[ch] = true;
		}
	}

	if (needToneSetSSG_[ch] || (isHardEnvSSG_[ch] && needEnvSetSSG_[ch]) || needSqMaskFreqSetSSG_[ch])
		writePitchSSG(ch);
}

void OPNAController::tickEventSSG(int ch)
{
	if (hasPreSetTickEventSSG_[ch]) {
		hasPreSetTickEventSSG_[ch] = false;
	}
	else {
		if (isMuteSSG_[ch]) return;

		setHardEnvIfNecessary_[ch] = false;

		if (wfItSSG_[ch]) writeWaveformSSGToRegister(ch, wfItSSG_[ch]->next());

		if (auto& treIt = treItSSG_[ch]) {
			treIt->next();
			needEnvSetSSG_[ch] = true;
		}
		if (volSldSSG_[ch]) {
			sumVolSldSSG_[ch] += volSldSSG_[ch];
			needEnvSetSSG_[ch] = true;
		}
		if (envItSSG_[ch]) {
			writeEnvelopeSSGToRegister(ch, envItSSG_[ch]->next());
		}
		else if (needToneSetSSG_[ch] || needEnvSetSSG_[ch]) {
			setRealVolumeSSG(ch);
		}

		if (tnItSSG_[ch]) writeToneNoiseSSGToRegister(ch, tnItSSG_[ch]->next());
		else if (needMixSetSSG_[ch]) writeToneNoiseSSGToRegisterNoReference(ch);

		if (auto& arpIt = arpItSSG_[ch]) {
			arpIt->next();
			checkRealToneSSGByArpeggio(ch);
		}
		checkPortamentoSSG(ch);

		if (auto& ptIt = ptItSSG_[ch]) {
			ptIt->next();
			checkRealToneSSGByPitch(ch);
		}
		if (auto& vibIt = vibItSSG_[ch]) {
			vibIt->next();
			needToneSetSSG_[ch] = true;
		}
		if (auto& nsIt = nsItSSG_[ch]) {
			nsIt->next();
			if (!nsIt->hasEnded()) {
				sumNoteSldSSG_[ch] += nsIt->data().data;
				needToneSetSSG_[ch] = true;
			}
		}

		if (needToneSetSSG_[ch] || (isHardEnvSSG_[ch] && needEnvSetSSG_[ch]) || needSqMaskFreqSetSSG_[ch])
			writePitchSSG(ch);
	}
}

void OPNAController::writeWaveformSSGToRegister(int ch, int seqPos)
{
	if (seqPos == -1) return;

	switch (static_cast<SSGWaveformType>(wfItSSG_[ch]->getCommandType())) {
	case SSGWaveformType::SQUARE:
	{
		writeSquareWaveform(ch);
		return;
	}
	case SSGWaveformType::TRIANGLE:
	{
		if (wfSSG_[ch].type == SSGWaveformType::TRIANGLE && isKeyOnSSG_[ch]) return;

		switch (wfSSG_[ch].type) {
		case SSGWaveformType::UNSET:
		case SSGWaveformType::SQUARE:
		case SSGWaveformType::SQM_TRIANGLE:
		case SSGWaveformType::SQM_SAW:
		case SSGWaveformType::SQM_INVSAW:
			needMixSetSSG_[ch] = true;
			break;
		default:
			break;
		}

		switch (wfSSG_[ch].type) {
		case SSGWaveformType::UNSET:
		case SSGWaveformType::SQUARE:
		case SSGWaveformType::SAW:
		case SSGWaveformType::INVSAW:
		case SSGWaveformType::SQM_SAW:
		case SSGWaveformType::SQM_INVSAW:
			opna_->setRegister(0x0d, 0x0e);
			break;
		default:
			if (!isKeyOnSSG_[ch]) opna_->setRegister(0x0d, 0x0e);	// First key on
			break;
		}

		if (isHardEnvSSG_[ch]) {
			isBuzzEffSSG_[ch] = true;
			isHardEnvSSG_[ch] = false;
		}
		else if (!isBuzzEffSSG_[ch] || !isKeyOnSSG_[ch]) {
			isBuzzEffSSG_[ch] = true;
			opna_->setRegister(0x08 + static_cast<uint32_t>(ch), 0x10);
		}

		if (envSSG_[ch].type == 0) envSSG_[ch] = { -1, CommandSequenceUnit::NODATA };

		needEnvSetSSG_[ch] = false;
		needToneSetSSG_[ch] = true;
		needSqMaskFreqSetSSG_[ch] = false;
		wfSSG_[ch] = { SSGWaveformType::TRIANGLE, CommandSequenceUnit::NODATA };
		return;
	}
	case SSGWaveformType::SAW:
	{
		if (wfSSG_[ch].type == SSGWaveformType::SAW && isKeyOnSSG_[ch]) return;

		switch (wfSSG_[ch].type) {
		case SSGWaveformType::UNSET:
		case SSGWaveformType::SQUARE:
		case SSGWaveformType::SQM_TRIANGLE:
		case SSGWaveformType::SQM_SAW:
		case SSGWaveformType::SQM_INVSAW:
			needMixSetSSG_[ch] = true;
			break;
		default:
			break;
		}

		switch (wfSSG_[ch].type) {
		case SSGWaveformType::UNSET:
		case SSGWaveformType::SQUARE:
		case SSGWaveformType::TRIANGLE:
		case SSGWaveformType::INVSAW:
		case SSGWaveformType::SQM_TRIANGLE:
		case SSGWaveformType::SQM_INVSAW:
			opna_->setRegister(0x0d, 0x0c);
			break;
		default:
			if (!isKeyOnSSG_[ch]) opna_->setRegister(0x0d, 0x0c);	// First key on
			break;
		}

		if (isHardEnvSSG_[ch]) {
			isBuzzEffSSG_[ch] = true;
			isHardEnvSSG_[ch] = false;
		}
		else if (!isBuzzEffSSG_[ch] || !isKeyOnSSG_[ch]) {
			isBuzzEffSSG_[ch] = true;
			opna_->setRegister(0x08 + static_cast<uint32_t>(ch), 0x10);
		}

		if (envSSG_[ch].type == 0) envSSG_[ch] = { -1, CommandSequenceUnit::NODATA };

		needEnvSetSSG_[ch] = false;
		needToneSetSSG_[ch] = true;
		needSqMaskFreqSetSSG_[ch] = false;
		wfSSG_[ch] = { SSGWaveformType::SAW, CommandSequenceUnit::NODATA };
		return;
	}
	case SSGWaveformType::INVSAW:
	{
		if (wfSSG_[ch].type == SSGWaveformType::INVSAW && isKeyOnSSG_[ch]) return;

		switch (wfSSG_[ch].type) {
		case SSGWaveformType::UNSET:
		case SSGWaveformType::SQUARE:
		case SSGWaveformType::SQM_TRIANGLE:
		case SSGWaveformType::SQM_SAW:
		case SSGWaveformType::SQM_INVSAW:
			needMixSetSSG_[ch] = true;
			break;
		default:
			break;
		}

		switch (wfSSG_[ch].type) {
		case SSGWaveformType::UNSET:
		case SSGWaveformType::SQUARE:
		case SSGWaveformType::TRIANGLE:
		case SSGWaveformType::SAW:
		case SSGWaveformType::SQM_TRIANGLE:
		case SSGWaveformType::SQM_SAW:
			opna_->setRegister(0x0d, 0x08);
			break;
		default:
			if (!isKeyOnSSG_[ch]) opna_->setRegister(0x0d, 0x08);	// First key on
			break;
		}

		if (isHardEnvSSG_[ch]) {
			isBuzzEffSSG_[ch] = true;
			isHardEnvSSG_[ch] = false;
		}
		else if (!isBuzzEffSSG_[ch] || !isKeyOnSSG_[ch]) {
			isBuzzEffSSG_[ch] = true;
			opna_->setRegister(0x08 + static_cast<uint32_t>(ch), 0x10);
		}

		if (envSSG_[ch].type == 0) envSSG_[ch] = { -1, CommandSequenceUnit::NODATA };

		needEnvSetSSG_[ch] = false;
		needToneSetSSG_[ch] = true;
		needSqMaskFreqSetSSG_[ch] = false;
		wfSSG_[ch] = { SSGWaveformType::INVSAW, CommandSequenceUnit::NODATA };
		return;
	}
	case SSGWaveformType::SQM_TRIANGLE:
	{
		int data = wfItSSG_[ch]->getCommandData();
		if (wfSSG_[ch].type == SSGWaveformType::SQM_TRIANGLE && wfSSG_[ch].data == data && isKeyOnSSG_[ch]) return;

		switch (wfSSG_[ch].type) {
		case SSGWaveformType::UNSET:
		case SSGWaveformType::TRIANGLE:
		case SSGWaveformType::SAW:
		case SSGWaveformType::INVSAW:
			needMixSetSSG_[ch] = true;
			break;
		default:
			break;
		}

		if (wfSSG_[ch].data != data) {
			if (CommandSequenceUnit::checkDataType(data) == CommandSequenceUnit::RATIO) {
				needSqMaskFreqSetSSG_[ch] = true;
			}
			else {
				uint16_t pitch = static_cast<uint16_t>(data);
				uint8_t offset = static_cast<uint8_t>(ch << 1);
				opna_->setRegister(0x00 + offset, pitch & 0xff);
				opna_->setRegister(0x01 + offset, pitch >> 8);
				needSqMaskFreqSetSSG_[ch] = false;
			}
		}
		else {
			needSqMaskFreqSetSSG_[ch] = false;
		}

		switch (wfSSG_[ch].type) {
		case SSGWaveformType::UNSET:
		case SSGWaveformType::SQUARE:
		case SSGWaveformType::SAW:
		case SSGWaveformType::INVSAW:
		case SSGWaveformType::SQM_SAW:
		case SSGWaveformType::SQM_INVSAW:
			opna_->setRegister(0x0d, 0x0e);
			break;
		default:
			if (!isKeyOnSSG_[ch]) opna_->setRegister(0x0d, 0x0e);	// First key on
			break;
		}

		if (isHardEnvSSG_[ch]) {
			isBuzzEffSSG_[ch] = true;
			isHardEnvSSG_[ch] = false;
		}
		else if (!isBuzzEffSSG_[ch] || !isKeyOnSSG_[ch]) {
			isBuzzEffSSG_[ch] = true;
			opna_->setRegister(0x08 + static_cast<uint32_t>(ch), 0x10);
		}

		if (envSSG_[ch].type == 0) envSSG_[ch] = { -1, CommandSequenceUnit::NODATA };

		needEnvSetSSG_[ch] = false;
		needToneSetSSG_[ch] = true;
		wfSSG_[ch] = { SSGWaveformType::SQM_TRIANGLE, data };
		return;
	}
	case SSGWaveformType::SQM_SAW:
	{
		int data = wfItSSG_[ch]->getCommandData();
		if (wfSSG_[ch].type == SSGWaveformType::SQM_SAW && wfSSG_[ch].data == data && isKeyOnSSG_[ch]) return;

		switch (wfSSG_[ch].type) {
		case SSGWaveformType::UNSET:
		case SSGWaveformType::TRIANGLE:
		case SSGWaveformType::SAW:
		case SSGWaveformType::INVSAW:
			needMixSetSSG_[ch] = true;
			break;
		default:
			break;
		}

		if (wfSSG_[ch].data != data) {
			if (CommandSequenceUnit::checkDataType(data) == CommandSequenceUnit::RATIO) {
				needSqMaskFreqSetSSG_[ch] = true;
			}
			else {
				uint16_t pitch = static_cast<uint16_t>(data);
				uint8_t offset = static_cast<uint8_t>(ch << 1);
				opna_->setRegister(0x00 + offset, pitch & 0xff);
				opna_->setRegister(0x01 + offset, pitch >> 8);
				needSqMaskFreqSetSSG_[ch] = false;
			}
		}
		else {
			needSqMaskFreqSetSSG_[ch] = false;
		}

		switch (wfSSG_[ch].type) {
		case SSGWaveformType::UNSET:
		case SSGWaveformType::SQUARE:
		case SSGWaveformType::TRIANGLE:
		case SSGWaveformType::INVSAW:
		case SSGWaveformType::SQM_TRIANGLE:
		case SSGWaveformType::SQM_INVSAW:
			opna_->setRegister(0x0d, 0x0c);
			break;
		default:
			if (!isKeyOnSSG_[ch]) opna_->setRegister(0x0d, 0x0c);	// First key on
			break;
		}

		if (isHardEnvSSG_[ch]) {
			isBuzzEffSSG_[ch] = true;
			isHardEnvSSG_[ch] = false;
		}
		else if (!isBuzzEffSSG_[ch] || !isKeyOnSSG_[ch]) {
			isBuzzEffSSG_[ch] = true;
			opna_->setRegister(0x08 + static_cast<uint32_t>(ch), 0x10);
		}

		if (envSSG_[ch].type == 0) envSSG_[ch] = { -1, CommandSequenceUnit::NODATA };

		needEnvSetSSG_[ch] = false;
		needToneSetSSG_[ch] = true;
		wfSSG_[ch] = { SSGWaveformType::SQM_SAW, data };
		return;
	}
	case SSGWaveformType::SQM_INVSAW:
	{
		int data = wfItSSG_[ch]->getCommandData();
		if (wfSSG_[ch].type == SSGWaveformType::SQM_INVSAW && wfSSG_[ch].data == data && isKeyOnSSG_[ch]) return;

		switch (wfSSG_[ch].type) {
		case SSGWaveformType::UNSET:
		case SSGWaveformType::TRIANGLE:
		case SSGWaveformType::SAW:
		case SSGWaveformType::INVSAW:
			needMixSetSSG_[ch] = true;
			break;
		default:
			break;
		}

		if (wfSSG_[ch].data != data) {
			if (CommandSequenceUnit::checkDataType(data) == CommandSequenceUnit::RATIO) {
				needSqMaskFreqSetSSG_[ch] = true;
			}
			else {
				uint16_t pitch = static_cast<uint16_t>(data);
				uint8_t offset = static_cast<uint8_t>(ch << 1);
				opna_->setRegister(0x00 + offset, pitch & 0xff);
				opna_->setRegister(0x01 + offset, pitch >> 8);
				needSqMaskFreqSetSSG_[ch] = false;
			}
		}
		else {
			needSqMaskFreqSetSSG_[ch] = false;
		}

		switch (wfSSG_[ch].type) {
		case SSGWaveformType::UNSET:
		case SSGWaveformType::SQUARE:
		case SSGWaveformType::TRIANGLE:
		case SSGWaveformType::SAW:
		case SSGWaveformType::SQM_TRIANGLE:
		case SSGWaveformType::SQM_SAW:
			opna_->setRegister(0x0d, 0x08);
			break;
		default:
			if (!isKeyOnSSG_[ch]) opna_->setRegister(0x0d, 0x08);	// First key on
			break;
		}

		if (isHardEnvSSG_[ch]) {
			isBuzzEffSSG_[ch] = true;
			isHardEnvSSG_[ch] = false;
		}
		else if (!isBuzzEffSSG_[ch] || !isKeyOnSSG_[ch]) {
			isBuzzEffSSG_[ch] = true;
			opna_->setRegister(0x08 + static_cast<uint32_t>(ch), 0x10);
		}

		if (envSSG_[ch].type == 0) envSSG_[ch] = { -1, CommandSequenceUnit::NODATA };

		needEnvSetSSG_[ch] = false;
		needToneSetSSG_[ch] = true;
		wfSSG_[ch] = { SSGWaveformType::SQM_INVSAW, data };
		return;
	}
	default:
		break;
	}
}

void OPNAController::writeSquareWaveform(int ch)
{
	if (wfSSG_[ch].type == SSGWaveformType::SQUARE) {
		if (!isKeyOnSSG_[ch]) {
			needEnvSetSSG_[ch] = true;
			needToneSetSSG_[ch] = true;
		}
		return;
	}

	switch (wfSSG_[ch].type) {
	case SSGWaveformType::SQM_TRIANGLE:
	case SSGWaveformType::SQM_SAW:
	case SSGWaveformType::SQM_INVSAW:
		break;
	default:
	{
		needMixSetSSG_[ch] = true;
		break;
	}
	}

	if (isBuzzEffSSG_[ch]) {
		isBuzzEffSSG_[ch] = false;
		setHardEnvIfNecessary_[ch] = true;
	}

	needEnvSetSSG_[ch] = true;
	needToneSetSSG_[ch] = true;
	needSqMaskFreqSetSSG_[ch] = false;
	wfSSG_[ch] = { SSGWaveformType::SQUARE, CommandSequenceUnit::NODATA };
}

void OPNAController::writeToneNoiseSSGToRegister(int ch, int seqPos)
{
	if (seqPos == -1) {
		if (needMixSetSSG_[ch]) writeToneNoiseSSGToRegisterNoReference(ch);
		return;
	}

	int type = tnItSSG_[ch]->getCommandType();
	if (type == -1) return;

	uint8_t prevMixer = mixerSSG_;
	if (!type) {	// tone
		switch (wfSSG_[ch].type) {
		case SSGWaveformType::TRIANGLE:
		case SSGWaveformType::SAW:
		case SSGWaveformType::INVSAW:
			if (tnSSG_[ch].isTone) {
				mixerSSG_ |= SSGToneFlag(ch);
				tnSSG_[ch].isTone = false;
			}
			break;
		default:
			if (!tnSSG_[ch].isTone) {
				mixerSSG_ &= ~SSGToneFlag(ch);
				tnSSG_[ch].isTone = true;
			}
			break;
		}

		if (tnSSG_[ch].isNoise) {
			mixerSSG_ |= SSGNoiseFlag(ch);
			tnSSG_[ch].isNoise = false;
			tnSSG_[ch].noisePeriod_ = CommandSequenceUnit::NODATA;
		}
	}
	else if (type == 65) {	// None
		if (tnSSG_[ch].isTone) {
			mixerSSG_ |= SSGToneFlag(ch);
			tnSSG_[ch].isTone = false;
		}

		if (tnSSG_[ch].isNoise) {
			mixerSSG_ |= SSGNoiseFlag(ch);
			tnSSG_[ch].isNoise = false;
			tnSSG_[ch].noisePeriod_ = CommandSequenceUnit::NODATA;
		}
	}
	else if (type > 32) {	// Tone&Noise
		switch (wfSSG_[ch].type) {
		case SSGWaveformType::TRIANGLE:
		case SSGWaveformType::SAW:
		case SSGWaveformType::INVSAW:
			if (tnSSG_[ch].isTone) {
				mixerSSG_ |= SSGToneFlag(ch);
				tnSSG_[ch].isTone = false;
			}
			break;
		default:
			if (!tnSSG_[ch].isTone) {
				mixerSSG_ &= ~SSGToneFlag(ch);
				tnSSG_[ch].isTone = true;
			}
			break;
		}

		if (!tnSSG_[ch].isNoise) {
			mixerSSG_ &= ~SSGNoiseFlag(ch);
			tnSSG_[ch].isNoise = true;
		}

		int p = type - 33;
		if (tnSSG_[ch].noisePeriod_ != p) {
			opna_->setRegister(0x06, static_cast<uint8_t>(31 - p));	// Reverse order
			tnSSG_->noisePeriod_ = p;
		}
	}
	else {	// Noise
		if (tnSSG_[ch].isTone) {
			mixerSSG_ |= SSGToneFlag(ch);
			tnSSG_[ch].isTone = false;
		}

		if (!tnSSG_[ch].isNoise) {
			mixerSSG_ &= ~SSGNoiseFlag(ch);
			tnSSG_[ch].isNoise = true;
		}

		int p = type - 1;
		if (tnSSG_[ch].noisePeriod_ != p) {
			opna_->setRegister(0x06, static_cast<uint8_t>(31 - p));	// Reverse order
			tnSSG_->noisePeriod_ = p;
		}
	}

	if (mixerSSG_ != prevMixer) opna_->setRegister(0x07, mixerSSG_);
	needMixSetSSG_[ch] = false;
}

void OPNAController::writeToneNoiseSSGToRegisterNoReference(int ch)
{
	switch (wfSSG_[ch].type) {
	case SSGWaveformType::TRIANGLE:
	case SSGWaveformType::SAW:
	case SSGWaveformType::INVSAW:
		mixerSSG_ |= SSGToneFlag(ch);
		tnSSG_[ch].isNoise = false;
		break;
	default:
		mixerSSG_ &= ~SSGToneFlag(ch);
		tnSSG_[ch].isTone = true;
		break;
	}
	opna_->setRegister(0x07, mixerSSG_);

	needMixSetSSG_[ch] = false;
}

void OPNAController::writeEnvelopeSSGToRegister(int ch, int seqPos)
{
	if (isBuzzEffSSG_[ch]) return;
	if (seqPos == -1) {
		if (needEnvSetSSG_[ch]) {
			setRealVolumeSSG(ch);
			needEnvSetSSG_[ch] = false;
		}
		return;
	}

	int type = envItSSG_[ch]->getCommandType();
	if (type == -1) return;
	else if (type < 16) {	// Software envelope
		isHardEnvSSG_[ch] = false;
		envSSG_[ch] = { type, CommandSequenceUnit::NODATA };
		setRealVolumeSSG(ch);
		needEnvSetSSG_[ch] = false;
	}
	else {	// Hardware envelope
		int data = envItSSG_[ch]->getCommandData();
		if (envSSG_[ch].data != data || setHardEnvIfNecessary_[ch]) {
			envSSG_[ch].data = data;
			if (CommandSequenceUnit::checkDataType(data) == CommandSequenceUnit::RATIO) {
				/* Envelope period is set in writePitchSSG */
				needEnvSetSSG_[ch] = true;
			}
			else {
				opna_->setRegister(0x0b, 0x00ff & envSSG_[ch].data);
				opna_->setRegister(0x0c, static_cast<uint8_t>(envSSG_[ch].data >> 8));
				needEnvSetSSG_[ch] = false;
			}
		}
		else {
			needEnvSetSSG_[ch] = false;
		}
		if (envSSG_[ch].type != type || !isKeyOnSSG_[ch] || setHardEnvIfNecessary_[ch]) {
			opna_->setRegister(0x0d, static_cast<uint8_t>(type - 16 + 8));
			envSSG_[ch].type = type;
			if (CommandSequenceUnit::checkDataType(data) == CommandSequenceUnit::RATIO)
				needEnvSetSSG_[ch] = true;
		}
		if (!isHardEnvSSG_[ch]) {
			opna_->setRegister(static_cast<uint32_t>(0x08 + ch), 0x10);
			isHardEnvSSG_[ch] = true;
		}
		// setHardEnvIfNecessary_[ch] = false;
	}
}

void OPNAController::writePitchSSG(int ch)
{
	if (keyToneSSG_[ch].octave == -1) return;	// Not set note yet

	int p = keyToneSSG_[ch].pitch
			+ sumPitchSSG_[ch]
			+ (vibItSSG_[ch] ? vibItSSG_[ch]->data().data : 0)
			+ detuneSSG_[ch]
			+ sumNoteSldSSG_[ch]
			+ transposeSSG_[ch];

	switch (wfSSG_[ch].type) {
	case SSGWaveformType::SQUARE:
	{
		uint16_t pitch = calc_pitch::calculateSSGSquareTP(
							 keyToneSSG_[ch].note, keyToneSSG_[ch].octave, p, fdetuneSSG_[ch]);
		if (needToneSetSSG_[ch]) {
			uint8_t offset = static_cast<uint8_t>(ch << 1);
			opna_->setRegister(0x00 + offset, pitch & 0xff);
			opna_->setRegister(0x01 + offset, pitch >> 8);
			writeAutoEnvelopePitchSSG(ch, pitch);
		}
		else if (isHardEnvSSG_[ch] && needEnvSetSSG_[ch]) {
			writeAutoEnvelopePitchSSG(ch, pitch);
		}
		break;
	}
	case SSGWaveformType::TRIANGLE:
		if (needToneSetSSG_[ch]) {
			uint16_t pitch = calc_pitch::calculateSSGTriangleEP(
								 keyToneSSG_[ch].note, keyToneSSG_[ch].octave, p, fdetuneSSG_[ch]);
			opna_->setRegister(0x0b, pitch & 0x00ff);
			opna_->setRegister(0x0c, pitch >> 8);
		}
		break;
	case SSGWaveformType::SAW:
	case SSGWaveformType::INVSAW:
		if (needToneSetSSG_[ch]){
			uint16_t pitch = calc_pitch::calculateSSGSawEP(
								 keyToneSSG_[ch].note, keyToneSSG_[ch].octave, p, fdetuneSSG_[ch]);
			opna_->setRegister(0x0b, pitch & 0x00ff);
			opna_->setRegister(0x0c, pitch >> 8);
		}
		break;
	case SSGWaveformType::SQM_TRIANGLE:
	{
		uint16_t pitch = calc_pitch::calculateSSGTriangleEP(
							 keyToneSSG_[ch].note, keyToneSSG_[ch].octave, p, fdetuneSSG_[ch]);
		if (needToneSetSSG_[ch]) {
			opna_->setRegister(0x0b, pitch & 0x00ff);
			opna_->setRegister(0x0c, pitch >> 8);
			if (CommandSequenceUnit::checkDataType(wfSSG_[ch].data) == CommandSequenceUnit::RATIO) {
				writeSquareMaskPitchSSG(ch, pitch, true);
			}
		}
		else if (needSqMaskFreqSetSSG_[ch]) {
			if (CommandSequenceUnit::checkDataType(wfSSG_[ch].data) == CommandSequenceUnit::RATIO) {
				writeSquareMaskPitchSSG(ch, pitch, true);
			}
		}
		break;
	}
	case SSGWaveformType::SQM_SAW:
	case SSGWaveformType::SQM_INVSAW:
	{
		uint16_t pitch = calc_pitch::calculateSSGSawEP(
							 keyToneSSG_[ch].note, keyToneSSG_[ch].octave, p, fdetuneSSG_[ch]);
		if (needToneSetSSG_[ch]) {
			opna_->setRegister(0x0b, pitch & 0x00ff);
			opna_->setRegister(0x0c, pitch >> 8);
			if (CommandSequenceUnit::checkDataType(wfSSG_[ch].data) == CommandSequenceUnit::RATIO) {
				writeSquareMaskPitchSSG(ch, pitch, false);
			}
		}
		else if (needSqMaskFreqSetSSG_[ch]) {
			if (CommandSequenceUnit::checkDataType(wfSSG_[ch].data) == CommandSequenceUnit::RATIO) {
				writeSquareMaskPitchSSG(ch, pitch, false);
			}
		}
		break;
	}
	default:
		break;
	}

	needToneSetSSG_[ch] = false;
	needEnvSetSSG_[ch] = false;
	needSqMaskFreqSetSSG_[ch] = false;
}

void OPNAController::writeAutoEnvelopePitchSSG(int ch, double tonePitch)
{
	// Multiple frequency if triangle
	int div = (envSSG_[ch].type == 18 || envSSG_[ch].type == 22) ? 32 : 16;

	CommandSequenceUnit::DataType type = CommandSequenceUnit::checkDataType(envSSG_[ch].data);
	switch (type) {
	case CommandSequenceUnit::RATIO:
	{
		auto ratio = CommandSequenceUnit::data2ratio(envSSG_[ch].data);
		uint16_t period = static_cast<uint16_t>(std::round(tonePitch * ratio.first / (ratio.second * div)));
		opna_->setRegister(0x0b, 0x00ff & period);
		opna_->setRegister(0x0c, static_cast<uint8_t>(period >> 8));
		break;
	}
	case CommandSequenceUnit::LSHIFT:
	case CommandSequenceUnit::RSHIFT:
	{
		uint16_t period = static_cast<uint16_t>(std::round(tonePitch / div));
		int shift = CommandSequenceUnit::data2shift(envSSG_[ch].data);
		shift = (type == CommandSequenceUnit::LSHIFT) ? -shift : shift;
		shift -= 4;	// Adjust rate to that of 0CC-FamiTracker
		if (shift < 0) period <<= -shift;
		else period >>= shift;
		opna_->setRegister(0x0b, 0x00ff & period);
		opna_->setRegister(0x0c, static_cast<uint8_t>(period >> 8));
		break;
	}
	default:
		break;
	}
}

void OPNAController::writeSquareMaskPitchSSG(int ch, double tonePitch, bool isTriangle)
{
	int mul = isTriangle ? 32 : 16;	// Multiple frequency if triangle
	auto ratio = CommandSequenceUnit::data2ratio(wfSSG_[ch].data);
	// Calculate mask period
	uint16_t period = static_cast<uint16_t>(std::round(ratio.first * mul * tonePitch / ratio.second));
	uint8_t offset = static_cast<uint8_t>(ch << 1);
	opna_->setRegister(0x00 + offset, period & 0x00ff);
	opna_->setRegister(0x01 + offset, period >> 8);
}

//---------- Rhythm ----------//
/********** Key on/off **********/
void OPNAController::setKeyOnFlagRhythm(int ch)
{
	if (isMuteRhythm_[ch]) return;

	if (tmpVolRhythm_[ch] != -1)
		setVolumeRhythm(ch, volRhythm_[ch]);

	keyOnFlagRhythm_ |= static_cast<uint8_t>(1 << ch);
}

void OPNAController::setKeyOffFlagRhythm(int ch)
{
	keyOffFlagRhythm_ |= static_cast<uint8_t>(1 << ch);
}

/********** Set volume **********/
void OPNAController::setVolumeRhythm(int ch, int volume)
{
	if (volume < NSTEP_RHYTHM_VOLUME) {
		volRhythm_[ch] = volume;
		tmpVolRhythm_[ch] = -1;
		opna_->setRegister(0x18 + static_cast<uint32_t>(ch), static_cast<uint8_t>((panRhythm_[ch] << 6) | volume));
	}
}

void OPNAController::setMasterVolumeRhythm(int volume)
{
	mVolRhythm_ = volume;
	opna_->setRegister(0x11, static_cast<uint8_t>(volume));
}

void OPNAController::setTemporaryVolumeRhythm(int ch, int volume)
{
	if (volume < NSTEP_RHYTHM_VOLUME) {
		tmpVolRhythm_[ch] = volume;
		opna_->setRegister(0x18 + static_cast<uint32_t>(ch), static_cast<uint8_t>((panRhythm_[ch] << 6) | volume));
	}
}

/********** Set effect **********/
void OPNAController::setPanRhythm(int ch, int value)
{
	panRhythm_[ch] = static_cast<uint8_t>(value);
	opna_->setRegister(0x18 + static_cast<uint32_t>(ch), static_cast<uint8_t>((value << 6) | volRhythm_[ch]));
}

/***********************************/
void OPNAController::initRhythm()
{
	keyOnFlagRhythm_ = 0;
	keyOffFlagRhythm_ = 0;
	mVolRhythm_ = 0x3f;
	opna_->setRegister(0x11, 0x3f);	// Rhythm total volume

	for (int ch = 0; ch < 6; ++ch) {
		volRhythm_[ch] = NSTEP_RHYTHM_VOLUME - 1;	// Init volume
		tmpVolRhythm_[ch] = -1;

		// Init pan
		panRhythm_[ch] = 3;
		opna_->setRegister(0x18 + static_cast<uint32_t>(ch), 0xdf);
	}
}

void OPNAController::setMuteRhythmState(int ch, bool isMute)
{
	isMuteRhythm_[ch] = isMute;

	if (isMute) {
		setKeyOffFlagRhythm(ch);
		updateKeyOnOffStatusRhythm();
	}
}

bool OPNAController::isMuteRhythm(int ch)
{
	return isMuteRhythm_[ch];
}

void OPNAController::updateKeyOnOffStatusRhythm()
{
	if (keyOnFlagRhythm_) {
		opna_->setRegister(0x10, keyOnFlagRhythm_);
		keyOnFlagRhythm_ = 0;
	}
	if (keyOffFlagRhythm_) {
		opna_->setRegister(0x10, 0x80 | keyOffFlagRhythm_);
		keyOffFlagRhythm_ = 0;
	}
}

//---------- ADPCM ----------//
/********** Key on-off **********/
void OPNAController::keyOnADPCM(Note note, int octave, int pitch, bool isJam)
{
	if (isMuteADPCM_ || (!refInstADPCM_ && !refInstKit_)) return;

	updateEchoBufferADPCM(octave, note, pitch);

	bool isTonePrtm = isTonePrtmADPCM_ && hasKeyOnBeforeADPCM_;
	if (isTonePrtm) {
		keyToneADPCM_.pitch += (sumNoteSldADPCM_ + transposeADPCM_);
	}
	else {
		keyToneADPCM_ = baseToneADPCM_.front();
		sumPitchADPCM_ = 0;
		sumVolSldADPCM_ = 0;
		tmpVolADPCM_ = -1;
	}
	if (!noteSldADPCMSetFlag_) {
		nsItADPCM_.reset();
	}
	noteSldADPCMSetFlag_ = false;
	needToneSetADPCM_ = true;
	sumNoteSldADPCM_ = 0;
	transposeADPCM_ = 0;

	setFrontADPCMSequences();
	hasPreSetTickEventADPCM_ = isJam;

	if (!isTonePrtm) {
		opna_->setRegister(0x101, 0x02);
		opna_->setRegister(0x100, 0xa1);

		if (refInstADPCM_) {
			triggerSamplePlayADPCM(refInstADPCM_->getSampleStartAddress(),
								   refInstADPCM_->getSampleStopAddress(),
								   refInstADPCM_->isSampleRepeatable());
		}
		else if (hasStartRequestedKit_) {	// valid key in refInstKit_
			int key = octaveAndNoteToNoteNumber(keyToneADPCM_.octave, keyToneADPCM_.note);
			triggerSamplePlayADPCM(refInstKit_->getSampleStartAddress(key),
								   refInstKit_->getSampleStopAddress(key),
								   refInstKit_->isSampleRepeatable(key));
			hasStartRequestedKit_ = false;
		}

		isKeyOnADPCM_ = true;
	}

	hasKeyOnBeforeADPCM_ = true;
}

void OPNAController::keyOnADPCM(int echoBuf)
{
	ToneDetail& td = baseToneADPCM_.at(static_cast<size_t>(echoBuf));
	if (td.octave == -1) return;
	keyOnADPCM( td.note, td.octave, td.pitch);
}

void OPNAController::keyOffADPCM(bool isJam)
{
	if (!isKeyOnADPCM_) {
		tickEventADPCM();
		return;
	}
	releaseStartADPCMSequences();
	hasPreSetTickEventADPCM_ = isJam;
	isKeyOnADPCM_ = false;
}

void OPNAController::updateEchoBufferADPCM(int octave, Note note, int pitch)
{
	baseToneADPCM_.pop_back();
	baseToneADPCM_.push_front({ octave, note, pitch });
}

/********** Set instrument **********/
/// NOTE: inst != nullptr
void OPNAController::setInstrumentADPCM(std::shared_ptr<InstrumentADPCM> inst)
{
	refInstADPCM_ = inst;
	refInstKit_.reset();

	if (inst->getEnvelopeEnabled())
		envItADPCM_ = inst->getEnvelopeSequenceIterator();
	else
		envItADPCM_.reset();
	if (!isArpEffADPCM_) {
		if (inst->getArpeggioEnabled())
			arpItADPCM_ = inst->getArpeggioSequenceIterator();
		else
			arpItADPCM_.reset();
	}
	if (inst->getPitchEnabled())
		ptItADPCM_ = inst->getPitchSequenceIterator();
	else
		ptItADPCM_.reset();
}

void OPNAController::updateInstrumentADPCM(int instNum)
{
	if (refInstADPCM_ && refInstADPCM_->isRegisteredWithManager()
			&& refInstADPCM_->getNumber() == instNum) {
		if (!refInstADPCM_->getEnvelopeEnabled()) envItADPCM_.reset();
		if (!refInstADPCM_->getArpeggioEnabled()) arpItADPCM_.reset();
		if (!refInstADPCM_->getPitchEnabled()) ptItADPCM_.reset();
	}
}

/// NOTE: inst != nullptr
void OPNAController::setInstrumentDrumkit(std::shared_ptr<InstrumentDrumkit> inst)
{
	refInstKit_ = inst;
	refInstADPCM_.reset();

	envItADPCM_.reset();
	arpItADPCM_.reset();
	ptItADPCM_.reset();
}

void OPNAController::updateInstrumentDrumkit(int instNum, int key)
{
	(void)instNum;
	(void)key;
}

void OPNAController::clearSamplesADPCM()
{
	storePointADPCM_ = 0;
	startAddrADPCM_ = std::numeric_limits<size_t>::max();
	stopAddrADPCM_ = startAddrADPCM_;
}

std::vector<size_t> OPNAController::storeSampleADPCM(std::vector<uint8_t> sample)
{
	std::vector<size_t> addrs(2);

	opna_->setRegister(0x110, 0x80);
	opna_->setRegister(0x100, 0x61);
	opna_->setRegister(0x100, 0x60);
	opna_->setRegister(0x101, 0x02);

	size_t dramLim = (opna_->getDRAMSize() - 1) >> 5;	// By 32 bytes
	opna_->setRegister(0x10c, dramLim & 0xff);
	opna_->setRegister(0x10d, (dramLim >> 8) & 0xff);

	if (storePointADPCM_ < dramLim) {
		size_t startAddress = storePointADPCM_;
		opna_->setRegister(0x102, startAddress & 0xff);
		opna_->setRegister(0x103, (startAddress >> 8) & 0xff);

		size_t stopAddress = startAddress + ((sample.size() - 1) >> 5);	// By 32 bytes
		stopAddress = std::min(stopAddress, dramLim);
		opna_->setRegister(0x104, stopAddress & 0xff);
		opna_->setRegister(0x105, (stopAddress >> 8) & 0xff);
		storePointADPCM_ = stopAddress + 1;

		size_t size = sample.size();
		for (size_t i = 0; i < size; ++i) {
			opna_->setRegister(0x108, sample[i]);
		}

		addrs = { startAddress, stopAddress };
	}

	opna_->setRegister(0x100, 0x00);
	opna_->setRegister(0x110, 0x80);

	return addrs;
}

/********** Set volume **********/
void OPNAController::setVolumeADPCM(int volume)
{
	if (volume < NSTEP_ADPCM_VOLUME) {
		baseVolADPCM_ = volume;
		tmpVolADPCM_ = -1;

		if (isKeyOnADPCM_) setRealVolumeADPCM();
	}
}

void OPNAController::setTemporaryVolumeADPCM(int volume)
{
	if (volume < NSTEP_ADPCM_VOLUME) {
		tmpVolADPCM_ = volume;

		if (isKeyOnADPCM_) setRealVolumeADPCM();
	}
}

void OPNAController::setRealVolumeADPCM()
{
	int volume = (tmpVolADPCM_ == -1) ? baseVolADPCM_ : tmpVolADPCM_;
	if (envItADPCM_) {
		int type = envItADPCM_->data().data;
		if (type >= 0) volume -= (NSTEP_ADPCM_VOLUME - 1 - type);
	}
	if (treItADPCM_) volume += treItADPCM_->data().data;
	volume += sumVolSldADPCM_;

	volume = clamp(volume, 0, NSTEP_ADPCM_VOLUME - 1);

	opna_->setRegister(0x10b, static_cast<uint8_t>(volume));
	needEnvSetADPCM_ = false;
}

/********** Set effect **********/
void OPNAController::setPanADPCM(int value)
{
	panADPCM_ = static_cast<uint8_t>(value << 6);
	opna_->setRegister(0x101, panADPCM_ | 0x02);
}

void OPNAController::setArpeggioEffectADPCM(int second, int third)
{
	if (refInstKit_) return;

	if (second || third) {
		arpItADPCM_ = std::make_unique<ArpeggioEffectIterator>(second, third);
		isArpEffADPCM_ = true;
	}
	else {
		if (!refInstADPCM_ || !refInstADPCM_->getArpeggioEnabled()) arpItADPCM_.reset();
		else arpItADPCM_ = refInstADPCM_->getArpeggioSequenceIterator();
		isArpEffADPCM_ = false;
	}
}

void OPNAController::setPortamentoEffectADPCM(int depth, bool isTonePortamento)
{
	if (refInstKit_) return;

	prtmADPCM_ = depth;
	isTonePrtmADPCM_ =  depth ? isTonePortamento : false;
}

void OPNAController::setVibratoEffectADPCM(int period, int depth)
{
	if (refInstKit_) return;

	if (period && depth) vibItADPCM_ = std::make_unique<WavingEffectIterator>(period, depth);
	else vibItADPCM_.reset();
}

void OPNAController::setTremoloEffectADPCM(int period, int depth)
{
	if (period && depth) treItADPCM_ = std::make_unique<WavingEffectIterator>(period, depth);
	else treItADPCM_.reset();
}

void OPNAController::setVolumeSlideADPCM(int depth, bool isUp)
{
	volSldADPCM_ = isUp ? depth : -depth;
}

void OPNAController::setDetuneADPCM(int pitch)
{
	if (refInstKit_) return;

	detuneADPCM_ = pitch;
	needToneSetADPCM_ = true;
}

void OPNAController::setFineDetuneADPCM(int pitch)
{
	if (refInstKit_) return;

	fdetuneADPCM_ = pitch;
	needToneSetADPCM_ = true;
}

void OPNAController::setNoteSlideADPCM(int speed, int seminote)
{
	if (refInstKit_) return;

	if (seminote) {
		nsItADPCM_ = std::make_unique<NoteSlideEffectIterator>(speed, seminote);
		noteSldADPCMSetFlag_ = true;
	}
	else nsItADPCM_.reset();
}

void OPNAController::setTransposeEffectADPCM(int seminote)
{
	transposeADPCM_ += (seminote * calc_pitch::SEMINOTE_PITCH);
	needToneSetADPCM_ = true;
}

/********** For state retrieve **********/
void OPNAController::haltSequencesADPCM()
{
	if (treItADPCM_) treItADPCM_->end();
	if (envItADPCM_) envItADPCM_->end();
	if (arpItADPCM_) arpItADPCM_->end();
	if (ptItADPCM_) ptItADPCM_->end();
	if (vibItADPCM_) vibItADPCM_->end();
	if (nsItADPCM_) nsItADPCM_->end();
}

/********** Chip details **********/
bool OPNAController::isKeyOnADPCM() const
{
	return isKeyOnADPCM_;
}

bool OPNAController::isTonePortamentoADPCM() const
{
	return isTonePrtmADPCM_;
}

ToneDetail OPNAController::getADPCMTone() const
{
	return baseToneADPCM_.front();
}

size_t OPNAController::getADPCMStoredSize() const
{
	return storePointADPCM_ << 5;
}

/***********************************/
void OPNAController::initADPCM()
{
	isKeyOnADPCM_ = false;
	hasKeyOnBeforeADPCM_ = false;

	refInstADPCM_.reset();	// Init envelope
	refInstKit_.reset();

	// Init echo buffer
	baseToneADPCM_ = std::deque<ToneDetail>(4);
	for (auto& td : baseToneADPCM_) {
		td.octave = -1;
	}

	keyToneADPCM_.note = Note::C;	// Dummy
	keyToneADPCM_.octave = -1;
	keyToneADPCM_.pitch = 0;	// Dummy
	sumPitchADPCM_ = 0;
	baseVolADPCM_ = NSTEP_ADPCM_VOLUME - 1;	// Init volume
	tmpVolADPCM_ = -1;
	panADPCM_ = 0xc0;
	startAddrADPCM_ = std::numeric_limits<size_t>::max();
	stopAddrADPCM_ = startAddrADPCM_;
	hasStartRequestedKit_ = false;

	// Init sequence
	hasPreSetTickEventADPCM_ = false;
	envItADPCM_.reset();
	arpItADPCM_.reset();
	ptItADPCM_.reset();
	needEnvSetADPCM_ = false;
	needToneSetADPCM_ = false;

	// Effect
	isArpEffADPCM_ = false;
	prtmADPCM_ = 0;
	isTonePrtmADPCM_ = false;
	vibItADPCM_.reset();
	treItADPCM_.reset();
	volSldADPCM_ = 0;
	sumVolSldADPCM_ = 0;
	detuneADPCM_ = 0;
	fdetuneADPCM_ = 0;
	nsItADPCM_.reset();
	sumNoteSldADPCM_ = 0;
	noteSldADPCMSetFlag_ = false;
	transposeADPCM_ = 0;

	opna_->setRegister(0x100, 0xa1);	// Stop synthesis
	// Limit address
	size_t dramLim = (opna_->getDRAMSize() - 1) >> 5;	// By 32 bytes
	opna_->setRegister(0x10c, dramLim & 0xff);
	opna_->setRegister(0x10d, (dramLim >> 8) & 0xff);
}

void OPNAController::setMuteADPCMState(bool isMute)
{
	isMuteADPCM_ = isMute;

	if (isMute) {
		opna_->setRegister(0x10b, 0);
		isKeyOnADPCM_ = false;
	}
}

bool OPNAController::isMuteADPCM()
{
	return isMuteADPCM_;
}

void OPNAController::setFrontADPCMSequences()
{
	if (isMuteADPCM_ || (!refInstADPCM_ && !refInstKit_)) return;

	if (treItADPCM_) {
		treItADPCM_->front();
		needEnvSetADPCM_ = true;
	}
	if (volSldADPCM_) {
		sumVolSldADPCM_ += volSldADPCM_;
		needEnvSetADPCM_ = true;
	}
	if (envItADPCM_) {
		envItADPCM_->front();
		writeEnvelopeADPCMToRegister();
	}
	else setRealVolumeADPCM();

	if (arpItADPCM_) {
		arpItADPCM_->front();
		checkRealToneADPCMByArpeggio();
	}
	checkPortamentoADPCM();

	if (ptItADPCM_) {
		ptItADPCM_->front();
		checkRealToneADPCMByPitch();
	}
	if (vibItADPCM_) {
		vibItADPCM_->front();
		needToneSetADPCM_ = true;
	}
	if (nsItADPCM_) {
		nsItADPCM_->front();
		if (!nsItADPCM_->hasEnded()) {
			sumNoteSldADPCM_ += nsItADPCM_->data().data;
			needToneSetADPCM_ = true;
		}
	}

	writePitchADPCM();
}

void OPNAController::releaseStartADPCMSequences()
{
	if (isMuteADPCM_ || (!refInstADPCM_ && !refInstKit_)) return;

	if (treItADPCM_) {
		treItADPCM_->release();
		needEnvSetADPCM_ = true;
	}
	if (volSldADPCM_) {
		sumVolSldADPCM_ += volSldADPCM_;
		needEnvSetADPCM_ = true;
	}
	if (envItADPCM_) {
		envItADPCM_->release();
		if (!envItADPCM_->hasEnded()) opna_->setRegister(0x10b, 0);
		else writeEnvelopeADPCMToRegister();
	}
	else {
		if (!hasPreSetTickEventADPCM_) {
			opna_->setRegister(0x10b, 0);
		}
	}

	if (arpItADPCM_) {
		arpItADPCM_->release();
		checkRealToneADPCMByArpeggio();
	}
	checkPortamentoADPCM();

	if (ptItADPCM_) {
		ptItADPCM_->release();
		checkRealToneADPCMByPitch();
	}
	if (vibItADPCM_) {
		vibItADPCM_->release();
		needToneSetADPCM_ = true;
	}
	if (nsItADPCM_) {
		nsItADPCM_->release();
		if (!nsItADPCM_->hasEnded()) {
			sumNoteSldADPCM_ += nsItADPCM_->data().data;
			needToneSetADPCM_ = true;
		}
	}

	if (needToneSetADPCM_) writePitchADPCM();

	hasStartRequestedKit_ = false;	// Always silent at relase in drumkit
}

void OPNAController::tickEventADPCM()
{
	if (hasPreSetTickEventADPCM_) {
		hasPreSetTickEventADPCM_ = false;
	}
	else {
		if (isMuteADPCM_ || (!refInstADPCM_ && !refInstKit_)) return;

		if (treItADPCM_) {
			treItADPCM_->next();
			needEnvSetADPCM_ = true;
		}
		if (volSldADPCM_) {
			sumVolSldADPCM_ += volSldADPCM_;
			needEnvSetADPCM_ = true;
		}
		if (envItADPCM_) {
			envItADPCM_->next();
			writeEnvelopeADPCMToRegister();
		}
		else if (needEnvSetADPCM_) {
			setRealVolumeADPCM();
		}

		if (arpItADPCM_) {
			arpItADPCM_->next();
			checkRealToneADPCMByArpeggio();
		}
		checkPortamentoADPCM();

		if (ptItADPCM_) {
			ptItADPCM_->next();
			checkRealToneADPCMByPitch();
		}
		if (vibItADPCM_) {
			vibItADPCM_->next();
			needToneSetADPCM_ = true;
		}
		if (nsItADPCM_) {
			nsItADPCM_->next();
			if (!nsItADPCM_->hasEnded()) {
				sumNoteSldADPCM_ += nsItADPCM_->data().data;
				needToneSetADPCM_ = true;
			}
		}

		if (needToneSetADPCM_) writePitchADPCM();

		if (hasStartRequestedKit_) {
			opna_->setRegister(0x101, 0x02);
			opna_->setRegister(0x100, 0xa1);

			int key = octaveAndNoteToNoteNumber(keyToneADPCM_.octave, keyToneADPCM_.note);
			triggerSamplePlayADPCM(refInstKit_->getSampleStartAddress(key),
								   refInstKit_->getSampleStopAddress(key),
								   refInstKit_->isSampleRepeatable(key));
			hasStartRequestedKit_ = false;
		}
	}
}

void OPNAController::writeEnvelopeADPCMToRegister()
{
	if (!envItADPCM_->hasEnded() || needEnvSetADPCM_) {
		setRealVolumeADPCM();
		needEnvSetADPCM_ = false;
	}
}

void OPNAController::writePitchADPCM()
{
	if (keyToneADPCM_.octave == -1) return;	// Not set note yet

	if (refInstADPCM_) {
		int p = keyToneADPCM_.pitch
				+ sumPitchADPCM_
				+ (vibItADPCM_ ? vibItADPCM_->data().data : 0)
				+ detuneADPCM_
				+ sumNoteSldADPCM_
				+ transposeADPCM_;
		p = calc_pitch::calculatePitchIndex(keyToneADPCM_.octave, keyToneADPCM_.note, p);

		int diff = p - calc_pitch::SEMINOTE_PITCH * refInstADPCM_->getSampleRootKeyNumber();
		writePitchADPCMToRegister(diff, refInstADPCM_->getSampleRootDeltaN());
	}
	else if (refInstKit_) {
		int key = clamp(octaveAndNoteToNoteNumber(keyToneADPCM_.octave, keyToneADPCM_.note)
						+ transposeADPCM_ / calc_pitch::SEMINOTE_PITCH, 0, 95);
		if (refInstKit_->getSampleEnabled(key)) {
			int diff = calc_pitch::SEMINOTE_PITCH * refInstKit_->getPitch(key);
			writePitchADPCMToRegister(diff, refInstKit_->getSampleRootDeltaN(key));
			hasStartRequestedKit_ = true;
		}
	}

	needToneSetADPCM_ = false;
	needEnvSetADPCM_ = false;
}

void OPNAController::writePitchADPCMToRegister(int pitchDiff, int rtDeltaN)
{
	int deltan = static_cast<int>(std::round(rtDeltaN * std::pow(2., pitchDiff / 384.))) + fdetuneADPCM_;
	opna_->setRegister(0x109, deltan & 0xff);
	opna_->setRegister(0x10a, (deltan >> 8) & 0xff);
}

void OPNAController::triggerSamplePlayADPCM(size_t startAddress, size_t stopAddress, bool repeatable)
{
	uint8_t repeatFlag = repeatable ? 0x10 : 0;
	opna_->setRegister(0x100, 0x21 | repeatFlag);

	if (startAddress != startAddrADPCM_) {
		opna_->setRegister(0x102, startAddress & 0xff);
		opna_->setRegister(0x103, (startAddress >> 8) & 0xff);
		startAddrADPCM_ = startAddress;
	}

	if (stopAddress != stopAddrADPCM_) {
		opna_->setRegister(0x104, stopAddress & 0xff);
		opna_->setRegister(0x105, (stopAddress >> 8) & 0xff);
		stopAddrADPCM_ = stopAddress;
	}

	opna_->setRegister(0x100, 0xa0 | repeatFlag);
	opna_->setRegister(0x101, panADPCM_ | 0x02);
}
