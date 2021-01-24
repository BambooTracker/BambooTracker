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
#include "note.hpp"
#include "utils.hpp"

namespace
{
constexpr int UNUSED_VALUE = -1;

const std::unordered_map<FMOperatorType, std::vector<FMEnvelopeParameter>> FM_ENV_PARAMS_OP = {
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
};
}

OPNAController::OPNAController(chip::OpnaEmulator emu, int clock, int rate, int duration)
	: mode_(SongType::Standard),
	  storePointADPCM_(0)
{
	constexpr size_t DRAM_SIZE = 262144;	// 256KiB
	opna_ = std::make_unique<chip::OPNA>(emu, clock, rate, duration, DRAM_SIZE,
										 std::make_unique<chip::LinearResampler>(),
										 std::make_unique<chip::LinearResampler>());

	for (int ch = 0; ch < 6; ++ch) {
		fmOpEnables_[ch] = 0xf;
		isMuteFM_[ch] = false;
		for (auto ep : FM_ENV_PARAMS_OP.at(FMOperatorType::All))
			opSeqItFM_[ch].emplace(ep, nullptr);
	}

	for (auto& ssg : ssg_) ssg.isMute = false;

	for (auto& rhy : rhythm_) rhy.isMute = false;

	isMuteADPCM_ = false;

	resetState();

	outputHistory_.reset(new int16_t[2 * bt_defs::OUTPUT_HISTORY_SIZE]{});
	outputHistoryReady_.reset(new int16_t[2 * bt_defs::OUTPUT_HISTORY_SIZE]{});
	outputHistoryIndex_ = 0;
}

/********** Reset and initialize **********/
void OPNAController::reset()
{
	opna_->reset();
	resetState();
	std::fill(&outputHistory_[0], &outputHistory_[2 * bt_defs::OUTPUT_HISTORY_SIZE], 0);
	std::fill(&outputHistoryReady_[0], &outputHistoryReady_[2 * bt_defs::OUTPUT_HISTORY_SIZE], 0);
}

void OPNAController::resetState()
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
	case SoundSource::SSG:		tickEventSSG(ssg_[ch]);	break;
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

	size_t nHistory = std::min<size_t>(nSamples, bt_defs::OUTPUT_HISTORY_SIZE);
	fillOutputHistory(&container[2 * (nSamples - nHistory)], nHistory);
}

void OPNAController::getOutputHistory(int16_t* container)
{
	std::lock_guard<std::mutex> lock(outputHistoryReadyMutex_);
	int16_t *history = outputHistoryReady_.get();
	std::copy(history, &history[2 * bt_defs::OUTPUT_HISTORY_SIZE], container);
}

void OPNAController::fillOutputHistory(const int16_t* outputs, size_t nSamples)
{
	int16_t *history = outputHistory_.get();
	size_t historyIndex = outputHistoryIndex_;

	// copy as many as possible to the back
	size_t backCapacity = bt_defs::OUTPUT_HISTORY_SIZE - historyIndex;
	size_t nBack = std::min(nSamples, backCapacity);
	std::copy(outputs, &outputs[2 * nBack], &history[2 * historyIndex]);

	// copy the rest to the front
	std::copy(&outputs[2 * nBack], &outputs[2 * nSamples], history);

	// update the write position
	historyIndex = (historyIndex + nSamples) % bt_defs::OUTPUT_HISTORY_SIZE;
	outputHistoryIndex_ = historyIndex;

	// if no one holds the ready buffer, update the contents
	std::unique_lock<std::mutex> lock(outputHistoryReadyMutex_, std::try_to_lock);
	if (lock.owns_lock()) {
		int16_t* dst = outputHistoryReady_.get();

		// copy the back, and then the front
		std::copy(&history[2 * historyIndex], &history[2 * bt_defs::OUTPUT_HISTORY_SIZE], dst);
		std::copy(&history[0], &history[2 * historyIndex], &dst[2 * (bt_defs::OUTPUT_HISTORY_SIZE - historyIndex)]);
	}
}

/********** Chip mode **********/
void OPNAController::setMode(SongType mode)
{
	mode_ = mode;
	reset();
}

/********** Mute **********/
void OPNAController::setMuteState(SoundSource src, int chInSrc, bool isMute)
{
	switch (src) {
	case SoundSource::FM:		setMuteFMState(chInSrc, isMute);		break;
	case SoundSource::SSG:		setMuteSSGState(chInSrc, isMute);		break;
	case SoundSource::RHYTHM:	setMuteRhythmState(chInSrc, isMute);	break;
	case SoundSource::ADPCM:	setMuteADPCMState(isMute);				break;
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

/********** Internal common process **********/
void OPNAController::checkRealToneByArpeggio(const ArpeggioIterInterface& arpIt,
											 const EchoBuffer& echoBuf, Note& baseNote,
											 bool& shouldSetTone)
{
	if (arpIt->hasEnded()) return;

	switch (arpIt->type()) {
	case SequenceType::AbsoluteSequence:
	{
		Note ln = echoBuf.latest();
		ln.addNoteNumber(arpIt->data().data - Note::DEFAULT_NOTE_NUM);
		baseNote = std::move(ln);
		break;
	}
	case SequenceType::FixedSequence:
	{
		baseNote = Note(arpIt->data().data);
		break;
	}
	case SequenceType::RelativeSequence:
	{
		baseNote.addNoteNumber(arpIt->data().data - Note::DEFAULT_NOTE_NUM);
		break;
	}
	default:
		return ;
	}

	shouldSetTone = true;
}

void OPNAController::checkPortamento(const ArpeggioIterInterface& arpIt,
									 int prtm, bool hasKeyOnBefore, bool isTonePrtm,
									 EchoBuffer& echoBuf,
									 Note& baseNote, bool& shouldSetTone)
{
	if ((!arpIt || arpIt->hasEnded()) && prtm && hasKeyOnBefore) {
		if (isTonePrtm) {
			Note bufNote = echoBuf.latest();
			int dif = bufNote.getAbsolutePicth() - baseNote.getAbsolutePicth();
			if (dif > 0) {
				baseNote += std::min(dif, prtm);
				shouldSetTone = true;
			}
			else if (dif < 0) {
				baseNote += std::max(dif, -prtm);
				shouldSetTone = true;
			}
		}
		else {
			baseNote += prtm;
			shouldSetTone = true;
		}
	}
}

void OPNAController::checkRealToneByPitch(const std::unique_ptr<InstrumentSequenceProperty<InstrumentSequenceBaseUnit>::Iterator>& ptIt,
										  int& sumPitch, bool& shouldSetTone)
{
	if (ptIt->hasEnded()) return;

	switch (ptIt->type()) {
	case SequenceType::AbsoluteSequence:
		sumPitch = ptIt->data().data - SEQ_PITCH_CENTER;
		break;
	case SequenceType::RelativeSequence:
		sumPitch += (ptIt->data().data - SEQ_PITCH_CENTER);
		break;
	default:
		return;
	}

	shouldSetTone = true;
}

//---------- FM ----------//
namespace
{
/// IS_CARRIER[operator][algorithm]
constexpr bool IS_CARRIER[4][8] = {
	{ false, false, false, false, false, false, false, true },
	{ false, false, false, false, true, true, true, true },
	{ false, false, false, false, false, true, true, true },
	{ true, true, true, true, true, true, true, true },
};

const FMEnvelopeParameter PARAM_TL[4] = {
	FMEnvelopeParameter::TL1, FMEnvelopeParameter::TL2, FMEnvelopeParameter::TL3, FMEnvelopeParameter::TL4
};
const FMEnvelopeParameter PARAM_ML[4] = {
	FMEnvelopeParameter::ML1, FMEnvelopeParameter::ML2, FMEnvelopeParameter::ML3, FMEnvelopeParameter::ML4
};
const FMEnvelopeParameter PARAM_AR[4] = {
	FMEnvelopeParameter::AR1, FMEnvelopeParameter::AR2, FMEnvelopeParameter::AR3, FMEnvelopeParameter::AR4
};
const FMEnvelopeParameter PARAM_DR[4] = {
	FMEnvelopeParameter::DR1, FMEnvelopeParameter::DR2, FMEnvelopeParameter::DR3, FMEnvelopeParameter::DR4
};
const FMEnvelopeParameter PARAM_RR[4] = {
	FMEnvelopeParameter::RR1, FMEnvelopeParameter::RR2, FMEnvelopeParameter::RR3, FMEnvelopeParameter::RR4
};

std::vector<int> getOperatorsInLevel(int level, int al)
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

inline uint8_t judgeSSGEGRegisterValue(int v)
{
	return (v == -1) ? 0 : (0x08 + static_cast<uint8_t>(v));
}
}

/********** Key on-off **********/
void OPNAController::keyOnFM(int ch, const Note& note, bool isJam)
{
	if (isMuteFM_[ch]) return;

	echoBufFM_[ch].push(note);

	bool isTonePrtm = isTonePrtmFM_[ch] && hasKeyOnBeforeFM_[ch];
	if (isTonePrtm) {
		baseNoteFM_[ch] += (sumNoteSldFM_[ch] + transposeFM_[ch]);
	}
	else {
		baseNoteFM_[ch] = echoBufFM_[ch].latest();
		neverSetBaseNoteFM_[ch] = false;
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
					static const std::unordered_map<int, uint8_t> FM3_KEY_OFF_MASK = {
						{ 2, 0xe }, { 6, 0xd }, { 7, 0xb }, { 8, 0x7 }
					};
					uint8_t flags = static_cast<uint8_t>(((slot & FM3_KEY_OFF_MASK.at(ch)) << 4)) | chdata;
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
	if (static_cast<size_t>(echoBuf) < echoBufFM_[ch].size())
		keyOnFM(ch, echoBufFM_[ch][echoBuf]);
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
				FMEnvelopeParameter tl = PARAM_TL[op];
				writeFMEnveropeParameterToRegister(inch, tl, inst->getEnvelopeParameter(tl));
			}
			if (isMLCtrlFM_[inch][op]) {
				isMLCtrlFM_[inch][op] = false;
				FMEnvelopeParameter ml = PARAM_ML[op];
				writeFMEnveropeParameterToRegister(inch, ml, inst->getEnvelopeParameter(ml));
			}
			if (isARCtrlFM_[inch][op]) {
				isARCtrlFM_[inch][op] = false;
				FMEnvelopeParameter ar = PARAM_AR[op];
				writeFMEnveropeParameterToRegister(inch, ar, inst->getEnvelopeParameter(ar));
			}
			if (isDRCtrlFM_[inch][op]) {
				isDRCtrlFM_[inch][op] = false;
				FMEnvelopeParameter dr = PARAM_DR[op];
				writeFMEnveropeParameterToRegister(inch, dr, inst->getEnvelopeParameter(dr));
			}
			if (isRRCtrlFM_[inch][op]) {
				isRRCtrlFM_[inch][op] = false;
				FMEnvelopeParameter rr = PARAM_RR[op];
				writeFMEnveropeParameterToRegister(inch, rr, inst->getEnvelopeParameter(rr));
			}
		}
		restoreFMEnvelopeFromReset(ch);
	}

	if (isKeyOnFM_[ch] && lfoStartCntFM_[inch] == -1) writeFMLFOAllRegisters(inch);
	for (auto& p : FM_ENV_PARAMS_OP.at(opType)) {
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
	int cnt = static_cast<int>(Song::getFMChannelCount(mode_));
	for (int ch = 0; ch < cnt; ++ch) {
		int inch = toInternalFMChannel(ch);

		if (refInstFM_[inch] && refInstFM_[inch]->isRegisteredWithManager()
				&& refInstFM_[inch]->getNumber() == instNum) {
			writeFMEnvelopeToRegistersFromInstrument(inch);
			if (isKeyOnFM_[ch] && lfoStartCntFM_[inch] == -1) writeFMLFOAllRegisters(inch);
			FMOperatorType opType = toChannelOperatorType(ch);
			for (auto& p : FM_ENV_PARAMS_OP.at(opType)) {
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
	int chsize = static_cast<int>(Song::getFMChannelCount(mode_));
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
	transposeFM_[ch] += (seminote * Note::SEMINOTE_PITCH);
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
	FMEnvelopeParameter param = PARAM_TL[op];
	writeFMEnveropeParameterToRegister(inch, param, value);
	isTLCtrlFM_[inch][op] = true;
	opSeqItFM_[inch].at(param).reset();
}

void OPNAController::setMLControlFM(int ch, int op, int value)
{
	int inch = toInternalFMChannel(ch);
	FMEnvelopeParameter param = PARAM_ML[op];
	writeFMEnveropeParameterToRegister(inch, param, value);
	isMLCtrlFM_[inch][op] = true;
	opSeqItFM_[inch].at(param).reset();
}

void OPNAController::setARControlFM(int ch, int op, int value)
{
	int inch = toInternalFMChannel(ch);
	FMEnvelopeParameter param = PARAM_AR[op];
	writeFMEnveropeParameterToRegister(inch, param, value);
	isARCtrlFM_[inch][op] = true;
	opSeqItFM_[inch].at(param).reset();
}

void OPNAController::setDRControlFM(int ch, int op, int value)
{
	int inch = toInternalFMChannel(ch);
	FMEnvelopeParameter param = PARAM_DR[op];
	writeFMEnveropeParameterToRegister(inch, param, value);
	isDRCtrlFM_[inch][op] = true;
	opSeqItFM_[inch].at(param).reset();
}

void OPNAController::setRRControlFM(int ch, int op, int value)
{
	int inch = toInternalFMChannel(ch);
	FMEnvelopeParameter param = PARAM_RR[op];
	writeFMEnveropeParameterToRegister(inch, param, value);
	isRRCtrlFM_[inch][op] = true;
	opSeqItFM_[inch].at(param).reset();
}

void OPNAController::setBrightnessFM(int ch, int value)
{
	int inch = toInternalFMChannel(ch);
	std::vector<int> ops = getOperatorsInLevel(1, envFM_[inch]->getParameterValue(FMEnvelopeParameter::AL));
	for (auto& op : ops) {
		FMEnvelopeParameter param = PARAM_TL[op];
		int v = utils::clamp(envFM_[inch]->getParameterValue(param) + value, 0, 127);
		writeFMEnveropeParameterToRegister(inch, param, v);
		isBrightFM_[inch][op] = true;
		opSeqItFM_[inch].at(param).reset();
	}
}

/********** For state retrieve **********/
void OPNAController::haltSequencesFM(int ch)
{
	int inch = toInternalFMChannel(ch);
	for (auto& p : FM_ENV_PARAMS_OP.at(toChannelOperatorType(ch))) {
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

Note OPNAController::getFMLatestNote(int ch) const
{
	return echoBufFM_[ch].latest();
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

	size_t fmch = Song::getFMChannelCount(mode_);
	for (size_t ch = 0; ch < fmch; ++ch) {
		// Init operators key off
		isKeyOnFM_[ch] = false;
		hasKeyOnBeforeFM_[ch] = false;

		echoBufFM_[ch].clear();

		neverSetBaseNoteFM_[ch] = true;
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
	else if (IS_CARRIER[0][al]) data1 = calculateTL(inch, data1);
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
	data1 = judgeSSGEGRegisterValue(tmp);
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
	else if (IS_CARRIER[1][al]) data1 = calculateTL(inch, data1);
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
	data1 = judgeSSGEGRegisterValue(tmp);
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
	else if (IS_CARRIER[2][al]) data1 = calculateTL(inch, data1);
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
	data1 = judgeSSGEGRegisterValue(tmp);
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
	data1 = judgeSSGEGRegisterValue(tmp);
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
		else if (IS_CARRIER[0][envFM_[inch]->getParameterValue(FMEnvelopeParameter::AL)]) {
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
		else if (IS_CARRIER[1][envFM_[inch]->getParameterValue(FMEnvelopeParameter::AL)]) {
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
		else if (IS_CARRIER[2][envFM_[inch]->getParameterValue(FMEnvelopeParameter::AL)]) {
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
		data = judgeSSGEGRegisterValue(tmp);
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
	for (auto& p : FM_ENV_PARAMS_OP.at(toChannelOperatorType(ch))) {
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
		if (IS_CARRIER[0][al]) {	// Operator 1
			int data = envFM_[ch]->getParameterValue(FMEnvelopeParameter::TL1) + v;
			opna_->setRegister(0x40 + bch, static_cast<uint8_t>(utils::clamp(data, 0 ,127)));
		}
		if (IS_CARRIER[1][al]) {	// Operator 2
			int data = envFM_[ch]->getParameterValue(FMEnvelopeParameter::TL2) + v;
			opna_->setRegister(0x40 + bch + 8, static_cast<uint8_t>(utils::clamp(data, 0 ,127)));
		}
		if (IS_CARRIER[2][al]) {	// Operator 3
			int data = envFM_[ch]->getParameterValue(FMEnvelopeParameter::TL3) + v;
			opna_->setRegister(0x40 + bch + 4, static_cast<uint8_t>(utils::clamp(data, 0 ,127)));
		}
		{							// Operator 4 (absolutely carrier)
			int data = envFM_[ch]->getParameterValue(FMEnvelopeParameter::TL4) + v;
			opna_->setRegister(0x40 + bch + 12, static_cast<uint8_t>(utils::clamp(data, 0 ,127)));
		}
		break;
	}
	case FMOperatorType::Op1:
	{
		int data = envFM_[inch]->getParameterValue(FMEnvelopeParameter::TL1) + v;
		opna_->setRegister(0x40 + bch, static_cast<uint8_t>(utils::clamp(data, 0 ,127)));
		break;
	}
	case FMOperatorType::Op2:
	{
		int data = envFM_[inch]->getParameterValue(FMEnvelopeParameter::TL2) + v;
		opna_->setRegister(0x40 + bch + 8, static_cast<uint8_t>(utils::clamp(data, 0 ,127)));
		break;
	}
	case FMOperatorType::Op3:
	{
		int data = envFM_[inch]->getParameterValue(FMEnvelopeParameter::TL3) + v;
		opna_->setRegister(0x40 + bch + 4, static_cast<uint8_t>(utils::clamp(data, 0 ,127)));
		break;
	}
	case FMOperatorType::Op4:
	{
		int data = envFM_[inch]->getParameterValue(FMEnvelopeParameter::TL4) + v;
		opna_->setRegister(0x40 + bch + 12, static_cast<uint8_t>(utils::clamp(data, 0 ,127)));
		break;
	}
	}
}

void OPNAController::writePitchFM(int ch)
{
	if (neverSetBaseNoteFM_[ch]) return;

	Note&& note = baseNoteFM_[ch] + (sumPitchFM_[ch]
									 + (vibItFM_[ch] ? vibItFM_[ch]->data().data : 0)
									 + detuneFM_[ch]
									 + sumNoteSldFM_[ch]
									 + transposeFM_[ch]);
	uint16_t p = note_utils::calculateFNumber(note.getAbsolutePicth(), fdetuneFM_[ch]);
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

//---------- SSG ----------//
namespace
{
constexpr int AUTO_ENV_SHAPE_TYPE[15] = { 17, 17, 17, 21, 21, 21, 21, 16, 17, 18, 19, 20, 21, 22, 23 };

namespace ToneNoiseState
{
enum : uint8_t
{
	CLEAR_TN = 0,
	TONE_TN = 1,
	NOISE_TN = 2,
	ALL_TN = 3
};
}

inline uint8_t SSGToneFlag(size_t ch) { return (1u << ch); }
inline uint8_t SSGNoiseFlag(size_t ch) { return (8u << ch); }
}

/********** Key on-off **********/
void OPNAController::keyOnSSG(int ch, const Note& note, bool isJam)
{
	auto& ssg = ssg_[ch];
	if (ssg.isMute) return;

	ssg.echoBuf.push(note);

	if (ssg.isTonePrtm && ssg.hasKeyOnBefore) {
		ssg.baseNote += (ssg.nsSum + ssg.transpose);
	}
	else {
		ssg.baseNote = ssg.echoBuf.latest();
		ssg.neverSetBaseNote = false;
		ssg.ptSum = 0;
		ssg.volSldSum = 0;
		ssg.oneshotVol = UNUSED_VALUE;
	}
	if (!noteSldSSGSetFlag_) {
		ssg.nsItr.reset();
	}
	noteSldSSGSetFlag_ = false;
	ssg.shouldSetTone = true;
	ssg.nsSum = 0;
	ssg.transpose = 0;

	{
		ssg.isInKeyOnProcess_ = true;
		setFrontSSGSequences(ssg);
		ssg.isInKeyOnProcess_ = false;
	}

	ssg.shouldSkip1stTickExec = isJam;
	ssg.isKeyOn = true;
	ssg.hasKeyOnBefore = true;
}

void OPNAController::keyOnSSG(int ch, int echoBuf)
{
	auto& ssg = ssg_[ch];
	if (static_cast<size_t>(echoBuf) < ssg.echoBuf.size())
		keyOnSSG(ch, ssg.echoBuf[echoBuf]);
}

void OPNAController::keyOffSSG(int ch, bool isJam)
{
	auto& ssg = ssg_[ch];
	if (!ssg.isKeyOn) {
		tickEventSSG(ssg);
		return;
	}
	releaseStartSSGSequences(ssg);
	ssg.shouldSkip1stTickExec = isJam;
	ssg.isKeyOn = false;
}

/********** Set instrument **********/
/// NOTE: inst != nullptr
void OPNAController::setInstrumentSSG(int ch, std::shared_ptr<InstrumentSSG> inst)
{
	auto& ssg = ssg_[ch];

	ssg.refInst = inst;

	if (inst->getWaveformEnabled())
		ssg.wfItr = inst->getWaveformSequenceIterator();
	else
		ssg.wfItr.reset();
	if (inst->getToneNoiseEnabled())
		ssg.tnItr = inst->getToneNoiseSequenceIterator();
	else
		ssg.tnItr.reset();
	if (inst->getEnvelopeEnabled())
		ssg.envItr = inst->getEnvelopeSequenceIterator();
	else
		ssg.envItr.reset();
	if (!ssg.isArpEff) {
		if (inst->getArpeggioEnabled())
			ssg.arpItr = inst->getArpeggioSequenceIterator();
		else
			ssg.arpItr.reset();
	}
	if (inst->getPitchEnabled())
		ssg.ptItr = inst->getPitchSequenceIterator();
	else
		ssg.ptItr.reset();
}

void OPNAController::updateInstrumentSSG(int instNum)
{
	for (auto& ssg : ssg_) {
		if (ssg.refInst && ssg.refInst->isRegisteredWithManager()
				&& ssg.refInst->getNumber() == instNum) {
			if (!ssg.refInst->getWaveformEnabled()) ssg.wfItr.reset();
			if (!ssg.refInst->getToneNoiseEnabled()) ssg.tnItr.reset();
			if (!ssg.refInst->getEnvelopeEnabled()) ssg.envItr.reset();
			if (!ssg.refInst->getArpeggioEnabled()) ssg.arpItr.reset();
			if (!ssg.refInst->getPitchEnabled()) ssg.ptItr.reset();
		}
	}
}

/********** Set volume **********/
void OPNAController::setVolumeSSG(int ch, int volume)
{
	if (volume < bt_defs::NSTEP_SSG_VOLUME) {
		auto& ssg = ssg_[ch];
		ssg.baseVol = volume;
		ssg.oneshotVol = UNUSED_VALUE;

		if (ssg.isKeyOn) setRealVolumeSSG(ssg);
	}
}

void OPNAController::setOneshotVolumeSSG(int ch, int volume)
{
	if (volume < bt_defs::NSTEP_SSG_VOLUME) {
		auto& ssg = ssg_[ch];
		ssg.oneshotVol = volume;

		if (ssg.isKeyOn) setRealVolumeSSG(ssg);
	}
}

void OPNAController::setRealVolumeSSG(SSGChannel& ssg)
{
	if (SSGWaveformType::testHardEnvelopeOccupancity(ssg.wfChState.data) || ssg.isHardEnv) {
		ssg.shouldSetEnv = false;
		return;
	}

	int volume = (ssg.oneshotVol == UNUSED_VALUE) ? ssg.baseVol : ssg.oneshotVol;
	if (auto& envItr = ssg.envItr) {
		int d = envItr->data().data;
		if (0 <= d && d < 16) {
			volume -= (15 - d);
		}
	}
	if (auto& treItr = ssg.treItr) volume += treItr->data().data;
	volume += ssg.volSldSum;

	volume = utils::clamp(volume, 0, 15);

	opna_->setRegister(0x08 + ssg.ch, static_cast<uint8_t>(volume));
	ssg.shouldSetEnv = false;
}

void OPNAController::setMasterVolumeSSG(double dB)
{
	opna_->setVolumeSSG(dB);
}

/********** Set effect **********/
void OPNAController::setArpeggioEffectSSG(int ch, int second, int third)
{
	auto& ssg = ssg_[ch];
	if (second || third) {
		ssg.arpItr = std::make_unique<ArpeggioEffectIterator>(second, third);
		ssg.isArpEff = true;
	}
	else {
		if (!ssg.refInst || !ssg.refInst->getArpeggioEnabled()) ssg.arpItr.reset();
		else ssg.arpItr = ssg.refInst->getArpeggioSequenceIterator();
		ssg.isArpEff = false;
	}
}

void OPNAController::setPortamentoEffectSSG(int ch, int depth, bool isTonePortamento)
{
	auto& ssg = ssg_[ch];
	ssg.prtmDepth = depth;
	ssg.isTonePrtm =  depth ? isTonePortamento : false;
}

void OPNAController::setVibratoEffectSSG(int ch, int period, int depth)
{
	auto& ssg = ssg_[ch];
	if (period && depth) ssg.vibItr = std::make_unique<WavingEffectIterator>(period, depth);
	else ssg.vibItr.reset();
}

void OPNAController::setTremoloEffectSSG(int ch, int period, int depth)
{
	auto& ssg = ssg_[ch];
	if (period && depth) ssg.treItr = std::make_unique<WavingEffectIterator>(period, depth);
	else ssg.treItr.reset();
}

void OPNAController::setVolumeSlideSSG(int ch, int depth, bool isUp)
{
	ssg_[ch].volSld = isUp ? depth : -depth;
}

void OPNAController::setDetuneSSG(int ch, int pitch)
{
	auto& ssg = ssg_[ch];
	ssg.detune = pitch;
	ssg.shouldSetTone = true;
}

void OPNAController::setFineDetuneSSG(int ch, int pitch)
{
	auto& ssg = ssg_[ch];
	ssg.fdetune = pitch;
	ssg.shouldSetTone = true;
}

void OPNAController::setNoteSlideSSG(int ch, int speed, int seminote)
{
	auto& ssg = ssg_[ch];
	if (seminote) {
		ssg.nsItr = std::make_unique<NoteSlideEffectIterator>(speed, seminote);
		noteSldSSGSetFlag_ = true;
	}
	else ssg.nsItr.reset();
}

void OPNAController::setTransposeEffectSSG(int ch, int seminote)
{
	auto& ssg = ssg_[ch];
	ssg.transpose += (seminote * Note::SEMINOTE_PITCH);
	ssg.shouldSetTone = true;
}

void OPNAController::setToneNoiseMixSSG(int ch, int value)
{
	auto& ssg = ssg_[ch];

	// Tone
	if (value & ToneNoiseState::TONE_TN) mixerSSG_ &= ~SSGToneFlag(ssg.ch);
	else mixerSSG_ |= SSGToneFlag(ssg.ch);
	// Noise
	if (value & ToneNoiseState::NOISE_TN) mixerSSG_ &= ~SSGNoiseFlag(ssg.ch);
	else mixerSSG_ |= SSGNoiseFlag(ssg.ch);
	opna_->setRegister(0x07, mixerSSG_);

	ssg.tnItr.reset();
}

void OPNAController::setNoisePitchSSG(int ch, int pitch)
{
	(void)ch;
	noisePeriodSSG_ = 31 - static_cast<uint8_t>(pitch);	// Reverse order
	opna_->setRegister(0x06, noisePeriodSSG_);
}

void OPNAController::setHardEnvelopePeriod(int ch, bool high, int period)
{
	auto& ssg = ssg_[ch];
	bool sendable = ssg.isHardEnv
					&& (ssg.envState.type == SSGEnvelopeUnit::RawSubdata);
	if (high) {
		hardEnvPeriodHighSSG_ = period;
		if (sendable) {
			int sub = (period << 8) | (ssg.envState.subdata & 0x00ff);
			ssg.envState = SSGEnvelopeUnit::makeRawUnit(ssg.envState.data, sub);
			opna_->setRegister(0x0c, static_cast<uint8_t>(period));
		}
	}
	else {
		hardEnvPeriodLowSSG_ = period;
		if (sendable) {
			int sub = (ssg.envState.subdata & 0xff00) | period;
			ssg.envState = SSGEnvelopeUnit::makeRawUnit(ssg.envState.data, sub);
			opna_->setRegister(0x0b, static_cast<uint8_t>(period));
		}
	}
}

void OPNAController::setAutoEnvelopeSSG(int ch, int shift, int shape)
{
	auto& ssg = ssg_[ch];
	if (shape) {
		opna_->setRegister(0x0d, static_cast<uint8_t>(shape));
		int d = AUTO_ENV_SHAPE_TYPE[shape - 1];
		opna_->setRegister(0x08 + ssg.ch, 0x10);
		ssg.isHardEnv = true;
		if (shift == -8) {	// Raw
			ssg.envState = SSGEnvelopeUnit::makeRawUnit(d, (hardEnvPeriodHighSSG_ << 8) | hardEnvPeriodLowSSG_);
			opna_->setRegister(0x0c, static_cast<uint8_t>(hardEnvPeriodHighSSG_));
			opna_->setRegister(0x0b, static_cast<uint8_t>(hardEnvPeriodLowSSG_));
			ssg.shouldSetEnv = false;
			ssg.shouldSetHardEnvFreq = false;
		}
		else {
			ssg.envState = SSGEnvelopeUnit::makeShiftUnit(d, shift);
			ssg.shouldSetEnv = true;
			ssg.shouldSetHardEnvFreq = true;
		}
	}
	else {
		ssg.isHardEnv = false;
		ssg.envState = SSGEnvelopeUnit();
		// Clear hard envelope in setRealVolumeSSG
		ssg.shouldSetEnv = true;
		ssg.shouldSetHardEnvFreq = false;
	}
	ssg.envItr.reset();
}

/********** For state retrieve **********/
void OPNAController::haltSequencesSSG(int ch)
{
	auto& ssg = ssg_[ch];
	if (auto& wfItr = ssg.wfItr) wfItr->end();
	if (auto& treItr = ssg.treItr) treItr->end();
	if (auto& envItr = ssg.envItr) envItr->end();
	if (auto& tnItr = ssg.tnItr) tnItr->end();
	if (auto& arpItr = ssg.arpItr) arpItr->end();
	if (auto& ptItr = ssg.ptItr) ptItr->end();
	if (auto& vibItr = ssg.vibItr) vibItr->end();
	if (auto& nsItr = ssg.nsItr) nsItr->end();
}

/********** Chip details **********/
bool OPNAController::isKeyOnSSG(int ch) const
{
	return ssg_[ch].isKeyOn;
}

bool OPNAController::isTonePortamentoSSG(int ch) const
{
	return ssg_[ch].isTonePrtm;
}

Note OPNAController::getSSGLatestNote(int ch) const
{
	return ssg_[ch].echoBuf.latest();
}

/***********************************/
void OPNAController::initSSG()
{
	mixerSSG_ = 0xff;
	opna_->setRegister(0x07, mixerSSG_);
	noisePeriodSSG_ = 0;
	opna_->setRegister(0x06, noisePeriodSSG_);
	hardEnvPeriodHighSSG_ = 0;
	hardEnvPeriodLowSSG_ = 0;

	for (size_t ch = 0; ch < 3; ++ch) {
		auto& ssg = ssg_[ch];
		ssg.ch = ch;

		ssg.isKeyOn = false;
		ssg.hasKeyOnBefore = false;
		ssg.isInKeyOnProcess_ = false;

		ssg.refInst.reset();	// Init envelope

		ssg.echoBuf.clear();

		ssg.neverSetBaseNote = true;
		ssg.baseVol = bt_defs::NSTEP_SSG_VOLUME - 1;	// Init volume
		ssg.oneshotVol = UNUSED_VALUE;
		ssg.isHardEnv = false;

		// Init sequence
		ssg.shouldSkip1stTickExec = false;
		ssg.wfItr.reset();
		ssg.wfChState = SSGWaveformUnit::makeOnlyDataUnit(SSGWaveformType::UNSET);
		ssg.envItr.reset();
		ssg.envState = SSGEnvelopeUnit();
		ssg.tnItr.reset();
		ssg.arpItr.reset();
		ssg.ptItr.reset();
		ssg.ptSum = 0;
		ssg.shouldSetEnv = false;
		ssg.shouldSetSqMaskFreq = false;
		ssg.shouldSetHardEnvFreq = false;
		ssg.shouldUpdateMixState = false;
		ssg.shouldSetTone = false;

		// Effect
		ssg.isArpEff = false;
		ssg.prtmDepth = 0;
		ssg.isTonePrtm = false;
		ssg.vibItr.reset();
		ssg.treItr.reset();
		ssg.volSld = 0;
		ssg.volSldSum = 0;
		ssg.detune = 0;
		ssg.fdetune = 0;
		ssg.nsItr.reset();
		ssg.nsSum = 0;
		noteSldSSGSetFlag_ = false;
		ssg.transpose = 0;
	}
}

void OPNAController::setMuteSSGState(int ch, bool isMute)
{
	auto& ssg = ssg_[ch];
	ssg.isMute = isMute;

	if (isMute) {
		opna_->setRegister(0x08 + ssg.ch, 0);
		ssg.isKeyOn = false;
	}
}

bool OPNAController::isMuteSSG(int ch)
{
	return ssg_[ch].isMute;
}

void OPNAController::setFrontSSGSequences(SSGChannel& ssg)
{
	if (ssg.isMute) return;

	if (auto& wfItr = ssg.wfItr) {
		wfItr->front();
		writeWaveformSSGToRegister(ssg);
	}
	else writeSquareWaveform(ssg);

	if (auto& treItr = ssg.treItr) {
		treItr->front();
		ssg.shouldSetEnv = true;
	}
	if (ssg.volSld) {
		ssg.volSldSum += ssg.volSld;
		ssg.shouldSetEnv = true;
	}
	if (auto& envItr = ssg.envItr) {
		envItr->front();
		writeEnvelopeSSGToRegister(ssg);
	}
	else setRealVolumeSSG(ssg);

	if (auto& tnItr = ssg.tnItr) {
		tnItr->front();
		writeMixerSSGToRegisterBySequence(ssg);
	}
	else if (ssg.shouldUpdateMixState) writeMixerSSGToRegisterByNoReference(ssg);

	if (auto& arpItr = ssg.arpItr) {
		arpItr->front();
		checkRealToneSSGByArpeggio(ssg);
	}
	checkPortamentoSSG(ssg);

	if (auto& ptItr = ssg.ptItr) {
		ptItr->front();
		checkRealToneSSGByPitch(ssg);
	}
	if (auto& vibItr = ssg.vibItr) {
		vibItr->front();
		ssg.shouldSetTone = true;
	}
	if (auto& nsItr = ssg.nsItr) {
		nsItr->front();
		if (!nsItr->hasEnded()) {
			ssg.nsSum += nsItr->data().data;
			ssg.shouldSetTone = true;
		}
	}

	writePitchSSG(ssg);
}

void OPNAController::releaseStartSSGSequences(SSGChannel& ssg)
{
	if (ssg.isMute) return;

	if (auto& wfItr = ssg.wfItr) {
		wfItr->release();
		writeWaveformSSGToRegister(ssg);
	}

	if (auto& treItr = ssg.treItr) {
		treItr->release();
		ssg.shouldSetEnv = true;
	}
	if (ssg.volSld) {
		ssg.volSldSum += ssg.volSld;
		ssg.shouldSetEnv = true;
	}
	if (auto& envItr = ssg.envItr) {
		envItr->release();
		if (envItr->hasEnded()) {
			// Silence
			opna_->setRegister(0x08 + ssg.ch, 0);
			ssg.shouldSetEnv = false;
			ssg.isHardEnv = false;
		}
		else writeEnvelopeSSGToRegister(ssg);
	}
	else {
		// Silence
		opna_->setRegister(0x08 + ssg.ch, 0);
		ssg.shouldSetEnv = false;
		ssg.isHardEnv = false;
	}

	if (auto& tnItr = ssg.tnItr) {
		tnItr->release();
		writeMixerSSGToRegisterBySequence(ssg);
	}
	else if (ssg.shouldUpdateMixState) writeMixerSSGToRegisterByNoReference(ssg);

	if (auto& arpItr = ssg.arpItr) {
		arpItr->release();
		checkRealToneSSGByArpeggio(ssg);
	}
	checkPortamentoSSG(ssg);

	if (auto& ptItr = ssg.ptItr) {
		ptItr->release();
		checkRealToneSSGByPitch(ssg);
	}
	if (auto& vibItr = ssg.vibItr) {
		vibItr->release();
		ssg.shouldSetTone = true;
	}
	if (auto& nsItr = ssg.nsItr) {
		nsItr->release();
		if (!nsItr->hasEnded()) {
			ssg.nsSum += nsItr->data().data;
			ssg.shouldSetTone = true;
		}
	}

	if (ssg.shouldSetTone || ssg.shouldSetHardEnvFreq || ssg.shouldSetSqMaskFreq)
		writePitchSSG(ssg);
}

void OPNAController::tickEventSSG(SSGChannel& ssg)
{
	if (ssg.shouldSkip1stTickExec) {
		ssg.shouldSkip1stTickExec = false;
	}
	else {
		if (ssg.isMute) return;

		if (auto& wfItr = ssg.wfItr) {
			wfItr->next();
			writeWaveformSSGToRegister(ssg);
		}

		if (auto& treItr = ssg.treItr) {
			treItr->next();
			ssg.shouldSetEnv = true;
		}
		if (ssg.volSld) {
			ssg.volSldSum += ssg.volSld;
			ssg.shouldSetEnv = true;
		}
		if (auto& envItr = ssg.envItr) {
			envItr->next();
			writeEnvelopeSSGToRegister(ssg);
		}
		else if (ssg.shouldSetEnv) {
			setRealVolumeSSG(ssg);
		}

		if (auto& tnItr = ssg.tnItr) {
			tnItr->next();
			writeMixerSSGToRegisterBySequence(ssg);
		}
		else if (ssg.shouldUpdateMixState) writeMixerSSGToRegisterByNoReference(ssg);

		if (auto& arpItr = ssg.arpItr) {
			arpItr->next();
			checkRealToneSSGByArpeggio(ssg);
		}
		checkPortamentoSSG(ssg);

		if (auto& ptItr = ssg.ptItr) {
			ptItr->next();
			checkRealToneSSGByPitch(ssg);
		}
		if (auto& vibItr = ssg.vibItr) {
			vibItr->next();
			ssg.shouldSetTone = true;
		}
		if (auto& nsItr = ssg.nsItr) {
			nsItr->next();
			if (!nsItr->hasEnded()) {
				ssg.nsSum += nsItr->data().data;
				ssg.shouldSetTone = true;
			}
		}

		if (ssg.shouldSetTone || ssg.shouldSetHardEnvFreq || ssg.shouldSetSqMaskFreq)
			writePitchSSG(ssg);
	}
}

void OPNAController::writeWaveformSSGToRegister(SSGChannel& ssg)
{
	auto& wfItr = ssg.wfItr;
	if (wfItr->hasEnded()) return;

	SSGWaveformUnit&& data = wfItr->data();
	switch (data.data) {
	case SSGWaveformType::SQUARE:
	{
		writeSquareWaveform(ssg);
		return;
	}
	case SSGWaveformType::TRIANGLE:
	{
		if (ssg.wfChState.data == SSGWaveformType::TRIANGLE && !ssg.isInKeyOnProcess_ && ssg.isKeyOn) {
			ssg.shouldSetEnv = false;
			return;
		}

		switch (ssg.wfChState.data) {
		case SSGWaveformType::TRIANGLE:
		case SSGWaveformType::SAW:
		case SSGWaveformType::INVSAW:
			break;
		default:
			ssg.shouldUpdateMixState = true;
			break;
		}

		// Reset phase
		switch (ssg.wfChState.data) {
		case SSGWaveformType::TRIANGLE:
		case SSGWaveformType::SQM_TRIANGLE:
			if (ssg.isInKeyOnProcess_) opna_->setRegister(0x0d, 0x0e);
			break;
		default:
			opna_->setRegister(0x0d, 0x0e);
			break;
		}

		if (ssg.isHardEnv) {
			ssg.isHardEnv = false;
		}
		else if (!SSGWaveformType::testHardEnvelopeOccupancity(ssg.wfChState.data) || ssg.isInKeyOnProcess_) {
			opna_->setRegister(0x08 + ssg.ch, 0x10);
		}

		ssg.shouldSetEnv = false;
		ssg.shouldSetTone = true;
		ssg.shouldSetSqMaskFreq = false;
		break;
	}
	case SSGWaveformType::SAW:
	{
		if (ssg.wfChState.data == SSGWaveformType::SAW && !ssg.isInKeyOnProcess_ && ssg.isKeyOn) {
			ssg.shouldSetEnv = false;
			return;
		}

		switch (ssg.wfChState.data) {
		case SSGWaveformType::TRIANGLE:
		case SSGWaveformType::SAW:
		case SSGWaveformType::INVSAW:
			break;
		default:
			ssg.shouldUpdateMixState = true;
			break;
		}

		// Reset phase
		switch (ssg.wfChState.data) {
		case SSGWaveformType::SAW:
		case SSGWaveformType::SQM_SAW:
			if (ssg.isInKeyOnProcess_) opna_->setRegister(0x0d, 0x0c);
			break;
		default:
			opna_->setRegister(0x0d, 0x0c);
			break;
		}

		if (ssg.isHardEnv) {
			ssg.isHardEnv = false;
		}
		else if (!SSGWaveformType::testHardEnvelopeOccupancity(ssg.wfChState.data) || ssg.isInKeyOnProcess_) {
			opna_->setRegister(0x08 + ssg.ch, 0x10);
		}

		ssg.shouldSetEnv = false;
		ssg.shouldSetTone = true;
		ssg.shouldSetSqMaskFreq = false;
		break;
	}
	case SSGWaveformType::INVSAW:
	{
		if (ssg.wfChState.data == SSGWaveformType::INVSAW && !ssg.isInKeyOnProcess_ && ssg.isKeyOn) {
			ssg.shouldSetEnv = false;
			return;
		}

		switch (ssg.wfChState.data) {
		case SSGWaveformType::TRIANGLE:
		case SSGWaveformType::SAW:
		case SSGWaveformType::INVSAW:
			break;
		default:
			ssg.shouldUpdateMixState = true;
			break;
		}

		// Reset phase
		switch (ssg.wfChState.data) {
		case SSGWaveformType::INVSAW:
		case SSGWaveformType::SQM_INVSAW:
			if (ssg.isInKeyOnProcess_) opna_->setRegister(0x0d, 0x08);
			break;
		default:
			opna_->setRegister(0x0d, 0x08);
			break;
		}

		if (ssg.isHardEnv) {
			ssg.isHardEnv = false;
		}
		else if (!SSGWaveformType::testHardEnvelopeOccupancity(ssg.wfChState.data) || ssg.isInKeyOnProcess_) {
			opna_->setRegister(0x08 + ssg.ch, 0x10);
		}

		ssg.shouldSetEnv = false;
		ssg.shouldSetTone = true;
		ssg.shouldSetSqMaskFreq = false;
		break;
	}
	case SSGWaveformType::SQM_TRIANGLE:
	{
		if (ssg.wfChState == data && !ssg.isInKeyOnProcess_ && ssg.isKeyOn) {
			ssg.shouldSetEnv = false;
			return;
		}

		switch (ssg.wfChState.data) {
		case SSGWaveformType::UNSET:
		case SSGWaveformType::TRIANGLE:
		case SSGWaveformType::SAW:
		case SSGWaveformType::INVSAW:
			ssg.shouldUpdateMixState = true;
			break;
		default:
			break;
		}

		if (ssg.wfChState.subdata != data.subdata) {
			if (data.type == SSGWaveformUnit::RatioSubdata) {
				// Set frequency of square mask in pitch process since it depends on pitch
				ssg.shouldSetSqMaskFreq = true;
			}
			else {	// Raw data
				uint16_t pitch = static_cast<uint16_t>(data.subdata);
				size_t offset = ssg.ch << 1;
				opna_->setRegister(0x00 + offset, pitch & 0xff);
				opna_->setRegister(0x01 + offset, pitch >> 8);
				ssg.shouldSetSqMaskFreq = false;
			}
		}
		else {
			ssg.shouldSetSqMaskFreq = false;
		}

		// Reset phase
		switch (ssg.wfChState.data) {
		case SSGWaveformType::TRIANGLE:
		case SSGWaveformType::SQM_TRIANGLE:
			if (ssg.isInKeyOnProcess_) opna_->setRegister(0x0d, 0x0e);
			break;
		default:
			opna_->setRegister(0x0d, 0x0e);
			break;
		}

		if (ssg.isHardEnv) {
			ssg.isHardEnv = false;
		}
		else if (!SSGWaveformType::testHardEnvelopeOccupancity(ssg.wfChState.data) || ssg.isInKeyOnProcess_) {
			opna_->setRegister(0x08 + ssg.ch, 0x10);
		}

		ssg.shouldSetEnv = false;
		ssg.shouldSetTone = true;
		break;
	}
	case SSGWaveformType::SQM_SAW:
	{
		if (ssg.wfChState == data && !ssg.isInKeyOnProcess_ && ssg.isKeyOn) {
			ssg.shouldSetEnv = false;
			return;
		}

		switch (ssg.wfChState.data) {
		case SSGWaveformType::UNSET:
		case SSGWaveformType::TRIANGLE:
		case SSGWaveformType::SAW:
		case SSGWaveformType::INVSAW:
			ssg.shouldUpdateMixState = true;
			break;
		default:
			break;
		}

		if (ssg.wfChState.subdata != data.subdata) {
			if (data.type == SSGWaveformUnit::RatioSubdata) {
				// Set frequency of square mask in pitch process since it depends on pitch
				ssg.shouldSetSqMaskFreq = true;
			}
			else {	// Raw data
				uint16_t pitch = static_cast<uint16_t>(data.subdata);
				size_t offset = ssg.ch << 1;
				opna_->setRegister(0x00 + offset, pitch & 0xff);
				opna_->setRegister(0x01 + offset, pitch >> 8);
				ssg.shouldSetSqMaskFreq = false;
			}
		}
		else {
			ssg.shouldSetSqMaskFreq = false;
		}

		// Reset phase
		switch (ssg.wfChState.data) {
		case SSGWaveformType::SAW:
		case SSGWaveformType::SQM_SAW:
			if (ssg.isInKeyOnProcess_) opna_->setRegister(0x0d, 0x0c);
			break;
		default:
			opna_->setRegister(0x0d, 0x0c);
			break;
		}

		if (ssg.isHardEnv) {
			ssg.isHardEnv = false;
		}
		else if (!SSGWaveformType::testHardEnvelopeOccupancity(ssg.wfChState.data) || ssg.isInKeyOnProcess_) {
			opna_->setRegister(0x08 + ssg.ch, 0x10);
		}

		ssg.shouldSetEnv = false;
		ssg.shouldSetTone = true;
		break;
	}
	case SSGWaveformType::SQM_INVSAW:
	{
		if (ssg.wfChState == data && !ssg.isInKeyOnProcess_ && ssg.isKeyOn) {
			ssg.shouldSetEnv = false;
			return;
		}

		switch (ssg.wfChState.data) {
		case SSGWaveformType::UNSET:
		case SSGWaveformType::TRIANGLE:
		case SSGWaveformType::SAW:
		case SSGWaveformType::INVSAW:
			ssg.shouldUpdateMixState = true;
			break;
		default:
			break;
		}

		if (ssg.wfChState.subdata != data.subdata) {
			if (data.type == SSGWaveformUnit::RatioSubdata) {
				// Set frequency of square mask in pitch process since it depends on pitch
				ssg.shouldSetSqMaskFreq = true;
			}
			else {	// Raw data
				uint16_t pitch = static_cast<uint16_t>(data.subdata);
				size_t offset = ssg.ch << 1;
				opna_->setRegister(0x00 + offset, pitch & 0xff);
				opna_->setRegister(0x01 + offset, pitch >> 8);
				ssg.shouldSetSqMaskFreq = false;
			}
		}
		else {
			ssg.shouldSetSqMaskFreq = false;
		}

		// Reset phase
		switch (ssg.wfChState.data) {
		case SSGWaveformType::INVSAW:
		case SSGWaveformType::SQM_INVSAW:
			if (ssg.isInKeyOnProcess_) opna_->setRegister(0x0d, 0x08);
			break;
		default:
			opna_->setRegister(0x0d, 0x08);
			break;
		}

		if (ssg.isHardEnv) {
			ssg.isHardEnv = false;
		}
		else if (!SSGWaveformType::testHardEnvelopeOccupancity(ssg.wfChState.data) || ssg.isInKeyOnProcess_) {
			opna_->setRegister(0x08 + ssg.ch, 0x10);
		}

		ssg.shouldSetEnv = false;
		ssg.shouldSetTone = true;
		break;
	}
	default:
		return;
	}
	ssg.wfChState = std::move(data);

	// Clear current envelope state
	// since the register of volume and hardware envelope frequency is used by waveform sequence
	ssg.envState = SSGEnvelopeUnit();
}

void OPNAController::writeSquareWaveform(SSGChannel& ssg)
{
	if (ssg.wfChState.data == SSGWaveformType::SQUARE) {
		if (ssg.isInKeyOnProcess_) {
			ssg.shouldSetEnv = true;
			ssg.shouldSetTone = true;
		}
		return;
	}

	switch (ssg.wfChState.data) {
	case SSGWaveformType::SQM_TRIANGLE:
	case SSGWaveformType::SQM_SAW:
	case SSGWaveformType::SQM_INVSAW:
		break;
	default:
		ssg.shouldUpdateMixState = true;
		break;
	}

	ssg.shouldSetEnv = true;
	ssg.shouldSetTone = true;
	ssg.shouldSetSqMaskFreq = false;
	ssg.wfChState = SSGWaveformUnit::makeOnlyDataUnit(SSGWaveformType::SQUARE);
}

void OPNAController::writeEnvelopeSSGToRegister(SSGChannel& ssg)
{
	// Skip if waveform settings use hardware envelope
	if (SSGWaveformType::testHardEnvelopeOccupancity(ssg.wfChState.data)) return;

	auto& envItr = ssg.envItr;
	if (envItr->hasEnded()) {
		if (ssg.shouldSetEnv) setRealVolumeSSG(ssg);
		return;
	}

	SSGEnvelopeUnit&& data = envItr->data();
	if (data.data < 16) {	// Software envelope
		ssg.isHardEnv = false;
		ssg.envState = std::move(data);
		setRealVolumeSSG(ssg);
	}
	else {	// Hardware envelope
		if (!ssg.isHardEnv) {
			opna_->setRegister(0x08 + ssg.ch, 0x10);
			ssg.isHardEnv = true;
		}
		if (ssg.envState.subdata != data.subdata) {
			ssg.envState.type = data.type;
			ssg.envState.subdata = data.subdata;
			if (data.type == SSGEnvelopeUnit::RatioSubdata) {
				// Set frequency of hardware envelope in pitch process since it depends on pitch
				ssg.shouldSetHardEnvFreq = true;
			}
			else {	// Raw data
				opna_->setRegister(0x0b, 0x00ff & ssg.envState.subdata);
				opna_->setRegister(0x0c, static_cast<uint8_t>(ssg.envState.subdata >> 8));
				ssg.shouldSetHardEnvFreq = false;
			}
		}
		if (ssg.envState.data != data.data || ssg.isInKeyOnProcess_) {
			opna_->setRegister(0x0d, static_cast<uint8_t>(data.data - 16 + 8));	// Reset phase
			ssg.envState.data = data.data;
			if (data.type == SSGEnvelopeUnit::RatioSubdata) {
				// Set frequency of hardware envelope in pitch process since it depends on pitch
				ssg.shouldSetHardEnvFreq = true;
			}
		}
	}
	ssg.shouldSetEnv = false;
}

void OPNAController::writeMixerSSGToRegisterBySequence(SSGChannel& ssg)
{
	auto& tnItr = ssg.tnItr;
	if (tnItr->hasEnded()) {
		if (ssg.shouldUpdateMixState) writeMixerSSGToRegisterByNoReference(ssg);
		return;
	}

	int type = tnItr->data().data;

	uint8_t prevMixer = mixerSSG_;
	if (!type) {	// tone
		switch (ssg.wfChState.data) {
		case SSGWaveformType::TRIANGLE:
		case SSGWaveformType::SAW:
		case SSGWaveformType::INVSAW:
			mixerSSG_ |= SSGToneFlag(ssg.ch);	// Off for buzzer effects
			break;
		default:
			mixerSSG_ &= ~SSGToneFlag(ssg.ch);
			break;
		}
		mixerSSG_ |= SSGNoiseFlag(ssg.ch);
	}
	else if (type == 65) {	// None
		mixerSSG_ |= SSGToneFlag(ssg.ch);
		mixerSSG_ |= SSGNoiseFlag(ssg.ch);
	}
	else if (type > 32) {	// Tone&Noise
		switch (ssg.wfChState.data) {
		case SSGWaveformType::TRIANGLE:
		case SSGWaveformType::SAW:
		case SSGWaveformType::INVSAW:
			mixerSSG_ |= SSGToneFlag(ssg.ch);
			break;
		default:
			mixerSSG_ &= ~SSGToneFlag(ssg.ch);
			break;
		}
		mixerSSG_ &= ~SSGNoiseFlag(ssg.ch);

		uint8_t p = static_cast<uint8_t>(64 - type - 33);	// Reverse order
		if (noisePeriodSSG_ != p) {
			noisePeriodSSG_ = p;
			opna_->setRegister(0x06, p);
		}
	}
	else {	// Noise
		mixerSSG_ |= SSGToneFlag(ssg.ch);
		mixerSSG_ &= ~SSGNoiseFlag(ssg.ch);

		uint8_t p = static_cast<uint8_t>(32 - type);	// Reverse order
		if (noisePeriodSSG_ != p) {
			noisePeriodSSG_ = p;
			opna_->setRegister(0x06, p);
		}
	}

	if (mixerSSG_ != prevMixer) opna_->setRegister(0x07, mixerSSG_);
	ssg.shouldUpdateMixState = false;
}

void OPNAController::writeMixerSSGToRegisterByNoReference(SSGChannel& ssg)
{
	switch (ssg.wfChState.data) {
	case SSGWaveformType::TRIANGLE:
	case SSGWaveformType::SAW:
	case SSGWaveformType::INVSAW:
		mixerSSG_ |= SSGToneFlag(ssg.ch);
		break;
	default:
		mixerSSG_ &= ~SSGToneFlag(ssg.ch);
		break;
	}
	opna_->setRegister(0x07, mixerSSG_);
	ssg.shouldUpdateMixState = false;
}

void OPNAController::writePitchSSG(SSGChannel& ssg)
{
	if (ssg.neverSetBaseNote) return;

	int p = (ssg.baseNote + (ssg.ptSum
							 + (ssg.vibItr ? ssg.vibItr->data().data : 0)
							 + ssg.detune
							 + ssg.nsSum
							 + ssg.transpose)).getAbsolutePicth();

	switch (ssg.wfChState.data) {
	case SSGWaveformType::SQUARE:
	{
		uint16_t pitch = note_utils::calculateSSGSquareTP(p, ssg.fdetune);
		if (ssg.shouldSetTone) {
			size_t offset = ssg.ch << 1;
			opna_->setRegister(0x00 + offset, pitch & 0xff);
			opna_->setRegister(0x01 + offset, pitch >> 8);
			// Forced call in case of changes in tone processing
			writeAutoEnvelopePitchSSG(ssg, pitch);
		}
		else if (ssg.shouldSetHardEnvFreq) {
			writeAutoEnvelopePitchSSG(ssg, pitch);
		}
		break;
	}
	case SSGWaveformType::TRIANGLE:
		if (ssg.shouldSetTone) {
			uint16_t pitch = note_utils::calculateSSGTriangleEP(p, ssg.fdetune);
			opna_->setRegister(0x0b, pitch & 0x00ff);
			opna_->setRegister(0x0c, pitch >> 8);
		}
		break;
	case SSGWaveformType::SAW:
	case SSGWaveformType::INVSAW:
		if (ssg.shouldSetTone){
			uint16_t pitch = note_utils::calculateSSGSawEP(p, ssg.fdetune);
			opna_->setRegister(0x0b, pitch & 0x00ff);
			opna_->setRegister(0x0c, pitch >> 8);
		}
		break;
	case SSGWaveformType::SQM_TRIANGLE:
	{
		uint16_t pitch = note_utils::calculateSSGTriangleEP(p, ssg.fdetune);
		if (ssg.shouldSetTone) {
			opna_->setRegister(0x0b, pitch & 0x00ff);
			opna_->setRegister(0x0c, pitch >> 8);
			// Forced call in case of changes in tone processing
			if (ssg.wfChState.type == SSGWaveformUnit::RatioSubdata) {
				writeSquareMaskPitchSSG(ssg, pitch, true);
			}
		}
		else if (ssg.shouldSetSqMaskFreq) {
			if (ssg.wfChState.type == SSGWaveformUnit::RatioSubdata) {
				writeSquareMaskPitchSSG(ssg, pitch, true);
			}
		}
		break;
	}
	case SSGWaveformType::SQM_SAW:
	case SSGWaveformType::SQM_INVSAW:
	{
		uint16_t pitch = note_utils::calculateSSGSawEP(p, ssg.fdetune);
		if (ssg.shouldSetTone) {
			opna_->setRegister(0x0b, pitch & 0x00ff);
			opna_->setRegister(0x0c, pitch >> 8);
			// Forced call in case of changes in tone processing
			if (ssg.wfChState.type == SSGWaveformUnit::RatioSubdata) {
				writeSquareMaskPitchSSG(ssg, pitch, false);
			}
		}
		else if (ssg.shouldSetSqMaskFreq) {
			if (ssg.wfChState.type == SSGWaveformUnit::RatioSubdata) {
				writeSquareMaskPitchSSG(ssg, pitch, false);
			}
		}
		break;
	}
	default:
		break;
	}

	ssg.shouldSetTone = false;
	ssg.shouldSetEnv = false;
	ssg.shouldSetHardEnvFreq = false;
	ssg.shouldSetSqMaskFreq = false;
}

void OPNAController::writeAutoEnvelopePitchSSG(SSGChannel& ssg, double tonePitch)
{
	// Multiple frequency if triangle
	int div = (ssg.envState.data == 18 || ssg.envState.data == 22) ? 32 : 16;

	switch (ssg.envState.type) {
	case SSGEnvelopeUnit::RatioSubdata:
	{
		int r1, r2;
		ssg.envState.getSubdataAsRatio(r1, r2);
		uint16_t period = static_cast<uint16_t>(std::round(tonePitch * r1 / (r2 * div)));
		opna_->setRegister(0x0b, 0x00ff & period);
		opna_->setRegister(0x0c, static_cast<uint8_t>(period >> 8));
		break;
	}
	case SSGEnvelopeUnit::ShiftSubdata:
	{
		uint16_t period = static_cast<uint16_t>(std::round(tonePitch / div));
		int rshift;
		ssg.envState.getSubdataAsShift(rshift);
		rshift -= 4;	// Adjust rate to that of 0CC-FamiTracker
		if (rshift < 0) period <<= -rshift;
		else period >>= rshift;
		opna_->setRegister(0x0b, 0x00ff & period);
		opna_->setRegister(0x0c, static_cast<uint8_t>(period >> 8));
		break;
	}
	default:
		break;
	}
}

void OPNAController::writeSquareMaskPitchSSG(SSGChannel& ssg, double tonePitch, bool isTriangle)
{
	int mul = isTriangle ? 32 : 16;	// Multiple frequency if triangle
	int r1, r2;
	ssg.wfChState.getSubdataAsRatio(r1, r2);
	// Calculate mask period
	uint16_t period = static_cast<uint16_t>(std::round(r1 * mul * tonePitch / r2));
	size_t offset = ssg.ch << 1;
	opna_->setRegister(0x00 + offset, period & 0x00ff);
	opna_->setRegister(0x01 + offset, period >> 8);
}

//---------- Rhythm ----------//
namespace
{
inline uint8_t makePanAndVolumeRegVal(uint8_t panState, int volume)
{
	return static_cast<uint8_t>((panState << 6) | volume);
}
}

/********** Key on/off **********/
void OPNAController::setKeyOnFlagRhythm(int ch)
{
	auto& rhy = rhythm_[ch];
	if (rhy.isMute) return;

	if (rhy.oneshotVol != UNUSED_VALUE)
		setVolumeRhythm(ch, rhy.baseVol);

	keyOnRequestFlagsRhythm_ |= static_cast<uint8_t>(1 << ch);
}

void OPNAController::setKeyOffFlagRhythm(int ch)
{
	keyOffRequestFlagsRhythm_ |= static_cast<uint8_t>(1 << ch);
}

/********** Set volume **********/
void OPNAController::setVolumeRhythm(int ch, int volume)
{
	if (volume < bt_defs::NSTEP_RHYTHM_VOLUME) {
		auto& rhy = rhythm_[ch];
		rhy.baseVol = volume;
		rhy.oneshotVol = UNUSED_VALUE;
		opna_->setRegister(0x18 + static_cast<uint32_t>(ch), makePanAndVolumeRegVal(rhy.panState, volume));
	}
}

void OPNAController::setOneshotVolumeRhythm(int ch, int volume)
{
	if (volume < bt_defs::NSTEP_RHYTHM_VOLUME) {
		auto& rhy = rhythm_[ch];
		rhy.oneshotVol = volume;
		opna_->setRegister(0x18 + static_cast<uint32_t>(ch), makePanAndVolumeRegVal(rhy.panState, volume));
	}
}

void OPNAController::setMasterVolumeRhythm(int volume)
{
	masterVolRhythm_ = volume;
	opna_->setRegister(0x11, static_cast<uint8_t>(volume));
}

/********** Set effect **********/
void OPNAController::setPanRhythm(int ch, int value)
{
	auto& rhy = rhythm_[ch];
	rhy.panState = static_cast<uint8_t>(value);
	int volume = (rhy.oneshotVol == UNUSED_VALUE) ? rhy.baseVol : rhy.oneshotVol;
	opna_->setRegister(0x18 + static_cast<uint32_t>(ch), makePanAndVolumeRegVal(value, volume));
}

/***********************************/
void OPNAController::initRhythm()
{
	keyOnRequestFlagsRhythm_ = 0;
	keyOffRequestFlagsRhythm_ = 0;
	masterVolRhythm_ = 0x3f;
	opna_->setRegister(0x11, 0x3f);	// Rhythm total volume

	for (size_t ch = 0; ch < 6; ++ch) {
		auto& rhy = rhythm_[ch];
		rhy.baseVol = bt_defs::NSTEP_RHYTHM_VOLUME - 1;
		rhy.oneshotVol = UNUSED_VALUE;

		// Init pan
		rhy.panState = 3;
		opna_->setRegister(0x18 + ch, 0xdf);
	}
}

void OPNAController::setMuteRhythmState(int ch, bool isMute)
{
	rhythm_[ch].isMute = isMute;

	if (isMute) {
		setKeyOffFlagRhythm(ch);
		updateKeyOnOffStatusRhythm();
	}
}

bool OPNAController::isMuteRhythm(int ch)
{
	return rhythm_[ch].isMute;
}

void OPNAController::updateKeyOnOffStatusRhythm()
{
	if (keyOnRequestFlagsRhythm_) {
		opna_->setRegister(0x10, keyOnRequestFlagsRhythm_);
		keyOnRequestFlagsRhythm_ = 0;
	}
	if (keyOffRequestFlagsRhythm_) {
		opna_->setRegister(0x10, 0x80 | keyOffRequestFlagsRhythm_);
		keyOffRequestFlagsRhythm_ = 0;
	}
}

//---------- ADPCM ----------//
/********** Key on-off **********/
void OPNAController::keyOnADPCM(const Note& note, bool isJam)
{
	if (isMuteADPCM_ || (!refInstADPCM_ && !refInstKit_)) return;

	echoBufADPCM_.push(note);

	bool isTonePrtm = hasTonePrtmADPCM_ && hasKeyOnBeforeADPCM_;
	if (isTonePrtm) {
		baseNoteADPCM_ += (nsSumADPCM_ + transposeADPCM_);
	}
	else {
		baseNoteADPCM_ = echoBufADPCM_.latest();
		neverSetBaseNoteADPCM_ = false;
		ptSumADPCM_ = 0;
		volSldSumADPCM_ = 0;
		oneshotVolADPCM_ = UNUSED_VALUE;
	}

	if (noteSldADPCMSetFlag_) noteSldADPCMSetFlag_ = false;
	else nsItADPCM_.reset();

	shouldSetToneADPCM_ = true;
	nsSumADPCM_ = 0;
	transposeADPCM_ = 0;

	setFrontADPCMSequences();
	shouldSkip1stTickExecADPCM_ = isJam;

	if (!isTonePrtm) {
		opna_->setRegister(0x101, 0x02);
		opna_->setRegister(0x100, 0xa1);

		if (refInstADPCM_) {
			triggerSamplePlayADPCM(refInstADPCM_->getSampleStartAddress(),
								   refInstADPCM_->getSampleStopAddress(),
								   refInstADPCM_->isSampleRepeatable());
		}
		else if (hasStartRequestedKit_) {	// valid key in refInstKit_
			int key = baseNoteADPCM_.getNoteNumber();
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
	if (static_cast<size_t>(echoBuf) < echoBufADPCM_.size())
		keyOnADPCM(echoBufADPCM_[echoBuf]);
}

void OPNAController::keyOffADPCM(bool isJam)
{
	if (!isKeyOnADPCM_) {
		tickEventADPCM();
		return;
	}
	releaseStartADPCMSequences();
	shouldSkip1stTickExecADPCM_ = isJam;
	isKeyOnADPCM_ = false;
}

/********** Set instrument **********/
/// NOTE: inst != nullptr
void OPNAController::setInstrumentADPCM(std::shared_ptr<InstrumentADPCM> inst)
{
	refInstADPCM_ = inst;
	refInstKit_.reset();

	if (inst->getEnvelopeEnabled())
		envItrADPCM_ = inst->getEnvelopeSequenceIterator();
	else
		envItrADPCM_.reset();
	if (!hasArpEffADPCM_) {
		if (inst->getArpeggioEnabled())
			arpItrADPCM_ = inst->getArpeggioSequenceIterator();
		else
			arpItrADPCM_.reset();
	}
	if (inst->getPitchEnabled())
		ptItrADPCM_ = inst->getPitchSequenceIterator();
	else
		ptItrADPCM_.reset();
}

void OPNAController::updateInstrumentADPCM(int instNum)
{
	if (refInstADPCM_ && refInstADPCM_->isRegisteredWithManager()
			&& refInstADPCM_->getNumber() == instNum) {
		if (!refInstADPCM_->getEnvelopeEnabled()) envItrADPCM_.reset();
		if (!refInstADPCM_->getArpeggioEnabled()) arpItrADPCM_.reset();
		if (!refInstADPCM_->getPitchEnabled()) ptItrADPCM_.reset();
	}
}

/// NOTE: inst != nullptr
void OPNAController::setInstrumentDrumkit(std::shared_ptr<InstrumentDrumkit> inst)
{
	refInstKit_ = inst;
	refInstADPCM_.reset();

	envItrADPCM_.reset();
	arpItrADPCM_.reset();
	ptItrADPCM_.reset();
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

bool OPNAController::storeSampleADPCM(const std::vector<uint8_t>& sample, size_t& startAddr, size_t& stopAddr)
{
	opna_->setRegister(0x110, 0x80);
	opna_->setRegister(0x100, 0x61);
	opna_->setRegister(0x100, 0x60);
	opna_->setRegister(0x101, 0x02);

	size_t dramLim = (opna_->getDRAMSize() - 1) >> 5;	// By 32 bytes
	opna_->setRegister(0x10c, dramLim & 0xff);
	opna_->setRegister(0x10d, (dramLim >> 8) & 0xff);

	bool stored = false;
	if (storePointADPCM_ < dramLim) {
		startAddr = storePointADPCM_;
		opna_->setRegister(0x102, startAddr & 0xff);
		opna_->setRegister(0x103, (startAddr >> 8) & 0xff);

		stopAddr = startAddr + ((sample.size() - 1) >> 5);	// By 32 bytes
		stopAddr = std::min(stopAddr, dramLim);
		opna_->setRegister(0x104, stopAddr & 0xff);
		opna_->setRegister(0x105, (stopAddr >> 8) & 0xff);
		storePointADPCM_ = stopAddr + 1;

		size_t size = sample.size();
		for (size_t i = 0; i < size; ++i) {
			opna_->setRegister(0x108, sample[i]);
		}
		stored = true;
	}

	opna_->setRegister(0x100, 0x00);
	opna_->setRegister(0x110, 0x80);

	return stored;
}

/********** Set volume **********/
void OPNAController::setVolumeADPCM(int volume)
{
	if (volume < bt_defs::NSTEP_ADPCM_VOLUME) {
		baseVolADPCM_ = volume;
		oneshotVolADPCM_ = UNUSED_VALUE;

		if (isKeyOnADPCM_) setRealVolumeADPCM();
	}
}

void OPNAController::setOneshotVolumeADPCM(int volume)
{
	if (volume < bt_defs::NSTEP_ADPCM_VOLUME) {
		oneshotVolADPCM_ = volume;

		if (isKeyOnADPCM_) setRealVolumeADPCM();
	}
}

void OPNAController::setRealVolumeADPCM()
{
	int volume = (oneshotVolADPCM_ == UNUSED_VALUE) ? baseVolADPCM_ : oneshotVolADPCM_;
	if (envItrADPCM_) {
		int type = envItrADPCM_->data().data;
		if (type >= 0) volume -= (bt_defs::NSTEP_ADPCM_VOLUME - 1 - type);
	}
	if (treItrADPCM_) volume += treItrADPCM_->data().data;
	volume += volSldSumADPCM_;

	volume = utils::clamp(volume, 0, bt_defs::NSTEP_ADPCM_VOLUME - 1);

	opna_->setRegister(0x10b, static_cast<uint8_t>(volume));
	shouldWriteEnvADPCM_ = false;
}

/********** Set effect **********/
void OPNAController::setPanADPCM(int value)
{
	panStateADPCM_ = static_cast<uint8_t>(value << 6);
	opna_->setRegister(0x101, panStateADPCM_ | 0x02);
}

void OPNAController::setArpeggioEffectADPCM(int second, int third)
{
	if (refInstKit_) return;

	if (second || third) {
		arpItrADPCM_ = std::make_unique<ArpeggioEffectIterator>(second, third);
		hasArpEffADPCM_ = true;
	}
	else {
		if (!refInstADPCM_ || !refInstADPCM_->getArpeggioEnabled()) arpItrADPCM_.reset();
		else arpItrADPCM_ = refInstADPCM_->getArpeggioSequenceIterator();
		hasArpEffADPCM_ = false;
	}
}

void OPNAController::setPortamentoEffectADPCM(int depth, bool isTonePortamento)
{
	if (refInstKit_) return;

	prtmDepthADPCM_ = depth;
	hasTonePrtmADPCM_ = depth ? isTonePortamento : false;
}

void OPNAController::setVibratoEffectADPCM(int period, int depth)
{
	if (refInstKit_) return;

	if (period && depth) vibItrADPCM_ = std::make_unique<WavingEffectIterator>(period, depth);
	else vibItrADPCM_.reset();
}

void OPNAController::setTremoloEffectADPCM(int period, int depth)
{
	if (period && depth) treItrADPCM_ = std::make_unique<WavingEffectIterator>(period, depth);
	else treItrADPCM_.reset();
}

void OPNAController::setVolumeSlideADPCM(int depth, bool isUp)
{
	volSldADPCM_ = isUp ? depth : -depth;
}

void OPNAController::setDetuneADPCM(int pitch)
{
	if (refInstKit_) return;

	detuneADPCM_ = pitch;
	shouldSetToneADPCM_ = true;
}

void OPNAController::setFineDetuneADPCM(int pitch)
{
	if (refInstKit_) return;

	fdetuneADPCM_ = pitch;
	shouldSetToneADPCM_ = true;
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
	transposeADPCM_ += (seminote * Note::SEMINOTE_PITCH);
	shouldSetToneADPCM_ = true;
}

/********** For state retrieve **********/
void OPNAController::haltSequencesADPCM()
{
	if (treItrADPCM_) treItrADPCM_->end();
	if (envItrADPCM_) envItrADPCM_->end();
	if (arpItrADPCM_) arpItrADPCM_->end();
	if (ptItrADPCM_) ptItrADPCM_->end();
	if (vibItrADPCM_) vibItrADPCM_->end();
	if (nsItADPCM_) nsItADPCM_->end();
}

/********** Chip details **********/
bool OPNAController::isKeyOnADPCM() const
{
	return isKeyOnADPCM_;
}

bool OPNAController::isTonePortamentoADPCM() const
{
	return hasTonePrtmADPCM_;
}

Note OPNAController::getADPCMLatestNote() const
{
	return echoBufADPCM_.latest();
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

	echoBufADPCM_.clear();

	neverSetBaseNoteADPCM_ = true;
	baseVolADPCM_ = bt_defs::NSTEP_ADPCM_VOLUME - 1;	// Init volume
	oneshotVolADPCM_ = UNUSED_VALUE;
	panStateADPCM_ = 0xc0;
	shouldWriteEnvADPCM_ = false;
	shouldSetToneADPCM_ = false;
	startAddrADPCM_ = std::numeric_limits<size_t>::max();
	stopAddrADPCM_ = startAddrADPCM_;
	hasStartRequestedKit_ = false;

	// Init sequence
	shouldSkip1stTickExecADPCM_ = false;
	envItrADPCM_.reset();
	arpItrADPCM_.reset();
	ptItrADPCM_.reset();
	ptSumADPCM_ = 0;

	// Effect
	hasArpEffADPCM_ = false;
	prtmDepthADPCM_ = 0;
	hasTonePrtmADPCM_ = false;
	vibItrADPCM_.reset();
	treItrADPCM_.reset();
	volSldADPCM_ = 0;
	volSldSumADPCM_ = 0;
	detuneADPCM_ = 0;
	fdetuneADPCM_ = 0;
	nsItADPCM_.reset();
	nsSumADPCM_ = 0;
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

	if (treItrADPCM_) {
		treItrADPCM_->front();
		shouldWriteEnvADPCM_ = true;
	}
	if (volSldADPCM_) {
		volSldSumADPCM_ += volSldADPCM_;
		shouldWriteEnvADPCM_ = true;
	}
	if (envItrADPCM_) {
		envItrADPCM_->front();
		writeEnvelopeADPCMToRegister();
	}
	else setRealVolumeADPCM();

	if (arpItrADPCM_) {
		arpItrADPCM_->front();
		checkRealToneADPCMByArpeggio();
	}
	checkPortamentoADPCM();

	if (ptItrADPCM_) {
		ptItrADPCM_->front();
		checkRealToneADPCMByPitch();
	}
	if (vibItrADPCM_) {
		vibItrADPCM_->front();
		/* shouldSetToneADPCM_ = true; */
	}
	if (nsItADPCM_) {
		nsItADPCM_->front();
		if (!nsItADPCM_->hasEnded()) {
			nsSumADPCM_ += nsItADPCM_->data().data;
			/* shouldSetToneADPCM_ = true; */
		}
	}

	writePitchADPCM();
}

void OPNAController::releaseStartADPCMSequences()
{
	if (isMuteADPCM_ || (!refInstADPCM_ && !refInstKit_)) return;

	if (treItrADPCM_) {
		treItrADPCM_->release();
		shouldWriteEnvADPCM_ = true;
	}
	if (volSldADPCM_) {
		volSldSumADPCM_ += volSldADPCM_;
		shouldWriteEnvADPCM_ = true;
	}
	if (envItrADPCM_) {
		envItrADPCM_->release();
		if (!envItrADPCM_->hasEnded()) {
			opna_->setRegister(0x10b, 0);	// Silence
			shouldWriteEnvADPCM_ = false;
		}
		else writeEnvelopeADPCMToRegister();
	}
	else {
		opna_->setRegister(0x10b, 0);	// Silence
		shouldWriteEnvADPCM_ = false;
	}

	if (arpItrADPCM_) {
		arpItrADPCM_->release();
		checkRealToneADPCMByArpeggio();
	}
	checkPortamentoADPCM();

	if (ptItrADPCM_) {
		ptItrADPCM_->release();
		checkRealToneADPCMByPitch();
	}
	if (vibItrADPCM_) {
		vibItrADPCM_->release();
		shouldSetToneADPCM_ = true;
	}
	if (nsItADPCM_) {
		nsItADPCM_->release();
		if (!nsItADPCM_->hasEnded()) {
			nsSumADPCM_ += nsItADPCM_->data().data;
			shouldSetToneADPCM_ = true;
		}
	}

	if (shouldSetToneADPCM_) writePitchADPCM();

	hasStartRequestedKit_ = false;	// Always silent at relase in drumkit
}

void OPNAController::tickEventADPCM()
{
	if (shouldSkip1stTickExecADPCM_) {
		shouldSkip1stTickExecADPCM_ = false;
	}
	else {
		if (isMuteADPCM_ || (!refInstADPCM_ && !refInstKit_)) return;

		if (treItrADPCM_) {
			treItrADPCM_->next();
			shouldWriteEnvADPCM_ = true;
		}
		if (volSldADPCM_) {
			volSldSumADPCM_ += volSldADPCM_;
			shouldWriteEnvADPCM_ = true;
		}
		if (envItrADPCM_) {
			envItrADPCM_->next();
			writeEnvelopeADPCMToRegister();
		}
		else if (shouldWriteEnvADPCM_) {
			setRealVolumeADPCM();
		}

		if (arpItrADPCM_) {
			arpItrADPCM_->next();
			checkRealToneADPCMByArpeggio();
		}
		checkPortamentoADPCM();

		if (ptItrADPCM_) {
			ptItrADPCM_->next();
			checkRealToneADPCMByPitch();
		}
		if (vibItrADPCM_) {
			vibItrADPCM_->next();
			shouldSetToneADPCM_ = true;
		}
		if (nsItADPCM_) {
			nsItADPCM_->next();
			if (!nsItADPCM_->hasEnded()) {
				nsSumADPCM_ += nsItADPCM_->data().data;
				shouldSetToneADPCM_ = true;
			}
		}

		if (shouldSetToneADPCM_) writePitchADPCM();

		if (hasStartRequestedKit_) {
			opna_->setRegister(0x101, 0x02);
			opna_->setRegister(0x100, 0xa1);

			int key = baseNoteADPCM_.getNoteNumber();
			triggerSamplePlayADPCM(refInstKit_->getSampleStartAddress(key),
								   refInstKit_->getSampleStopAddress(key),
								   refInstKit_->isSampleRepeatable(key));
			hasStartRequestedKit_ = false;
		}
	}
}

void OPNAController::writeEnvelopeADPCMToRegister()
{
	if (!envItrADPCM_->hasEnded() || shouldWriteEnvADPCM_) {
		setRealVolumeADPCM();
	}
}

void OPNAController::writePitchADPCM()
{
	if (neverSetBaseNoteADPCM_) return;

	if (refInstADPCM_) {
		int p = (baseNoteADPCM_ + (ptSumADPCM_
								   + (vibItrADPCM_ ? vibItrADPCM_->data().data : 0)
								   + detuneADPCM_
								   + nsSumADPCM_
								   + transposeADPCM_)).getAbsolutePicth();
		int diff = p - Note::SEMINOTE_PITCH * refInstADPCM_->getSampleRootKeyNumber();
		writePitchADPCMToRegister(diff, refInstADPCM_->getSampleRootDeltaN());
	}
	else if (refInstKit_) {
		int key = utils::clamp(baseNoteADPCM_.getNoteNumber()
							   + transposeADPCM_ / Note::SEMINOTE_PITCH, 0, Note::NOTE_NUMBER_RANGE - 1);
		if (refInstKit_->getSampleEnabled(key)) {
			int diff = Note::SEMINOTE_PITCH * refInstKit_->getPitch(key);
			writePitchADPCMToRegister(diff, refInstKit_->getSampleRootDeltaN(key));
			hasStartRequestedKit_ = true;
		}
	}

	shouldSetToneADPCM_ = false;
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
	opna_->setRegister(0x101, panStateADPCM_ | 0x02);
}
