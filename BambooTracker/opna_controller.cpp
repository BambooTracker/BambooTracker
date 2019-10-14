#include "opna_controller.hpp"
#include <stdexcept>
#include "pitch_converter.hpp"

OPNAController::OPNAController(chip::Emu emu, int clock, int rate, int duration)
	: mode_(SongType::STD)
{
	opna_ = std::make_unique<chip::OPNA>(emu, clock, rate, duration,
										 std::make_unique<chip::LinearResampler>(),
										 std::make_unique<chip::LinearResampler>());

	for (int ch = 0; ch < 6; ++ch) {
		fmOpEnables_[ch] = 0xf;
		isMuteFM_[ch] = false;
		for (auto ep : getFMEnvelopeParametersForOperator(FMOperatorType::All))
			opSeqItFM_[ch].emplace(ep, nullptr);
	}

	for (int ch = 0; ch < 3; ++ch) {
		isMuteSSG_[ch] = false;
	}

	for (int ch = 0; ch < 6; ++ch) {
		isMuteDrum_[ch] = false;
	}

	initChip();

	outputHistory_.reset(new int16_t[2 * OutputHistorySize]{});
	outputHistoryReady_.reset(new int16_t[2 * OutputHistorySize]{});
	outputHistoryIndex_ = 0;
}

/********** Reset and initialize **********/
void OPNAController::reset()
{
	opna_->reset();
	initChip();
	std::fill(&outputHistory_[0], &outputHistory_[2 * OutputHistorySize], 0);
	std::fill(&outputHistoryReady_[0], &outputHistoryReady_[2 * OutputHistorySize], 0);
}

void OPNAController::initChip()
{
	opna_->setRegister(0x29, 0x80);		// Init interrupt / YM2608 mode

	initFM();
	initSSG();
	initDrum();
}

/********** Forward instrument sequence **********/
void OPNAController::tickEvent(SoundSource src, int ch)
{
	switch (src) {
	case SoundSource::FM:	tickEventFM(ch);	break;
	case SoundSource::SSG:	tickEventSSG(ch);	break;
	case SoundSource::Drum:	break;
	}
}

/********** Stream type **********/
void OPNAController::useSCCI(SoundInterfaceManager* manager)
{
	opna_->useSCCI(manager);
}

bool OPNAController::isUsedSCCI() const
{
	return opna_->isUsedSCCI();
}

/********** Stream samples **********/
void OPNAController::getStreamSamples(int16_t* container, size_t nSamples)
{
	opna_->mix(container, nSamples);

	size_t nHistory = std::min<size_t>(nSamples, OutputHistorySize);
	fillOutputHistory(&container[2 * (nSamples - nHistory)], nHistory);
}

void OPNAController::getOutputHistory(int16_t* container)
{
	std::lock_guard<std::mutex> lock(outputHistoryReadyMutex_);
	int16_t *history = outputHistoryReady_.get();
	std::copy(history, &history[2 * OutputHistorySize], container);
}

void OPNAController::fillOutputHistory(const int16_t* outputs, size_t nSamples)
{
	int16_t *history = outputHistory_.get();
	size_t historyIndex = outputHistoryIndex_;

	// copy as many as possible to the back
	size_t backCapacity = OutputHistorySize - historyIndex;
	size_t nBack = std::min(nSamples, backCapacity);
	std::copy(outputs, &outputs[2 * nBack], &history[2 * historyIndex]);

	// copy the rest to the front
	std::copy(&outputs[2 * nBack], &outputs[2 * nSamples], history);

	// update the write position
	historyIndex = (historyIndex + nSamples) % OutputHistorySize;
	outputHistoryIndex_ = historyIndex;

	// if no one holds the ready buffer, update the contents
	std::unique_lock<std::mutex> lock(outputHistoryReadyMutex_, std::try_to_lock);
	if (lock.owns_lock())
		transferReadyHistory();
}

void OPNAController::transferReadyHistory()
{
	const int16_t *src = outputHistory_.get();
	int16_t *dst = outputHistoryReady_.get();
	size_t index = outputHistoryIndex_;

	// copy the back, and then the front
	std::copy(&src[2 * index], &src[2 * OutputHistorySize], dst);
	std::copy(&src[0], &src[2 * index], &dst[2 * (OutputHistorySize - index)]);
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

void OPNAController::setExportContainer(std::shared_ptr<chip::ExportContainerInterface> cntr)
{
	opna_->setExportContainer(cntr);
}

//---------- FM ----------//
/********** Key on-off **********/
void OPNAController::keyOnFM(int ch, Note note, int octave, int pitch, bool isJam)
{
	if (isMuteFM(ch)) return;

	updateEchoBufferFM(ch, octave, note, pitch);

	if (isTonePortamentoFM(ch)) {
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

	if (!isTonePortamentoFM(ch)) {
		uint8_t chdata = getFMKeyOnOffChannelMask(ch);
		switch (mode_) {
		case SongType::STD:
		{
			if (isKeyOnFM_[ch]) opna_->setRegister(0x28, chdata);	// Key off
			else isKeyOnFM_[ch] = true;
			opna_->setRegister(0x28, static_cast<uint8_t>(fmOpEnables_[ch] << 4) | chdata);
			break;
		}
		case SongType::FMEX:
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
				slot = fmOpEnables_[2] & (
						   static_cast<uint8_t>(isKeyOnFM_[2])
					   | (static_cast<uint8_t>(isKeyOnFM_[6]) << 1)
						| (static_cast<uint8_t>(isKeyOnFM_[7]) << 2)
						| (static_cast<uint8_t>(isKeyOnFM_[8]) << 3)
						);
				if (prev) {	// Key off
					uint8_t mask = 0;
					switch (ch)
					{
					case 2:	mask = 0xe;	break;
					case 6:	mask = 0xd;	break;
					case 7:	mask = 0xb;	break;
					case 8:	mask = 0x7;	break;
					default:
						break;
					}
					opna_->setRegister(0x28, static_cast<uint8_t>(((slot & mask) << 4)) | chdata);
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
}

void OPNAController::keyOnFM(int ch, int echoBuf)
{
	ToneDetail& td = baseToneFM_[ch].at(static_cast<size_t>(echoBuf));
	if (td.octave == -1) return;
	keyOnFM(ch, td.note, td.octave, td.pitch);
}

void OPNAController::keyOffFM(int ch, bool isJam)
{
	if (!isKeyOnFM_[ch]) return;
	releaseStartFMSequences(ch);
	hasPreSetTickEventFM_[ch] = isJam;

	isKeyOnFM_[ch] = false;

	uint8_t chdata = getFMKeyOnOffChannelMask(ch);
	switch (mode_) {
	case SongType::STD:
	{
		opna_->setRegister(0x28, chdata);
		break;
	}
	case SongType::FMEX:
	{
		switch (ch) {
		case 2:
		case 6:
		case 7:
		case 8:
		{
			uint8_t slot = fmOpEnables_[2] & (
							   static_cast<uint8_t>(isKeyOnFM_[2])
						   | (static_cast<uint8_t>(isKeyOnFM_[6]) << 1)
					| (static_cast<uint8_t>(isKeyOnFM_[7]) << 2)
					| (static_cast<uint8_t>(isKeyOnFM_[8]) << 3)
					);
			opna_->setRegister(0x28, static_cast<uint8_t>(slot << 4) | chdata);
			break;
		}
		default:
			opna_->setRegister(0x28, chdata);
			break;
		}

		break;
	}
	}
}

// Change register only
void OPNAController::resetFMChannelEnvelope(int ch)
{
	keyOffFM(ch);

	if (mode_ == SongType::FMEX && toInternalFMChannel(ch) == 2) {
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
		fmOpEnables_[inch] = static_cast<uint8_t>(refInstFM_[inch]->getOperatorEnabled(0))
							 | static_cast<uint8_t>(refInstFM_[inch]->getOperatorEnabled(1) << 1)
							 | static_cast<uint8_t>(refInstFM_[inch]->getOperatorEnabled(2) << 2)
							 | static_cast<uint8_t>(refInstFM_[inch]->getOperatorEnabled(3) << 3);
	}
	else {
		switch (mode_) {
		case SongType::STD:
		{
			if (refInstFM_[inch]->getEnvelopeResetEnabled(FMOperatorType::All)) {	// Restore envelope from reset
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
		case SongType::FMEX:
		{
			if (refInstFM_[inch]->getEnvelopeResetEnabled(opType)) {	// Restore envelope from reset
				if (inch == 2) {
					FMEnvelopeParameter param;
					switch (ch) {
					case 2:	param = FMEnvelopeParameter::RR1;	break;
					case 6:	param = FMEnvelopeParameter::RR2;	break;
					case 7:	param = FMEnvelopeParameter::RR3;	break;
					case 8:	param = FMEnvelopeParameter::RR4;	break;
					default:	throw std::out_of_range("out of range.");
					}
					writeFMEnveropeParameterToRegister(2, param, refInstFM_[2]->getEnvelopeParameter(param));
				}
				else {
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

	if (isKeyOnFM_[ch] && lfoStartCntFM_[inch] == -1) writeFMLFOAllRegisters(inch);
	for (auto& p : getFMEnvelopeParametersForOperator(opType)) {
		if (refInstFM_[inch]->getOperatorSequenceEnabled(p))
			opSeqItFM_[inch].at(p) = refInstFM_[inch]->getOperatorSequenceSequenceIterator(p);
		else
			opSeqItFM_[inch].at(p).reset();
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
			for (auto& p : getFMEnvelopeParametersForOperator(opType)) {
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
				case SongType::STD:
				{
					opna_->setRegister(0x28, static_cast<uint8_t>(fmOpEnables_[inch] << 4) | chdata);
					break;
				}
				case SongType::FMEX:
				{
					uint8_t slot;
					if (inch == 2) {
						slot = fmOpEnables_[2] & (
								   static_cast<uint8_t>(isKeyOnFM_[2])
							   | (static_cast<uint8_t>(isKeyOnFM_[6]) << 1)
								| (static_cast<uint8_t>(isKeyOnFM_[7]) << 2)
								| (static_cast<uint8_t>(isKeyOnFM_[8]) << 3)
								);
					}
					else {
						slot = fmOpEnables_[inch];
					}
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
		if (refInstFM_[ch] && refInstFM_[ch]->getLFOEnabled()
				&& refInstFM_[ch]->getLFONumber() == lfoNum) {
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

/********** Set pan **********/
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

/********** Set effect **********/
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
	volSldFM_[ch] = depth * (isUp ? -1 : 1);
}

void OPNAController::setDetuneFM(int ch, int pitch)
{
	detuneFM_[ch] = pitch;
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
	transposeFM_[ch] += (seminote * 32);
	needToneSetFM_[ch] = true;
}

/********** For state retrieve **********/
void OPNAController::haltSequencesFM(int ch)
{
	int inch = toInternalFMChannel(ch);
	for (auto& p : getFMEnvelopeParametersForOperator(toChannelOperatorType(ch))) {
		if (auto& it = opSeqItFM_[inch].at(p)) it->end();
	}
	if (treItFM_[ch]) treItFM_[ch]->end();
	if (arpItFM_[ch]) arpItFM_[ch]->end();
	if (ptItFM_[ch]) ptItFM_[ch]->end();
	if (vibItFM_[ch]) vibItFM_[ch]->next();
	if (nsItFM_[ch]) nsItFM_[ch]->end();
}

/********** Mute **********/
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
	case SongType::STD:		mode = 0;		break;
	case SongType::FMEX:	mode = 0x40;	break;
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
	}

	size_t fmch = getFMChannelCount(mode_);
	for (size_t ch = 0; ch < fmch; ++ch) {
		// Init operators key off
		isKeyOnFM_[ch] = false;

		// Init echo buffer
		baseToneFM_[ch] = std::deque<ToneDetail>(4);
		for (auto& td : baseToneFM_[ch]) {
			td.octave = -1;
		}

		keyToneFM_[ch].octave = -1;
		sumPitchFM_[ch] = 0;
		baseVolFM_[ch] = 0;	// Init volume
		tmpVolFM_[ch] = -1;
		enableEnvResetFM_[ch] = false;

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
		nsItFM_[ch].reset();
		sumNoteSldFM_[ch] = 0;
		noteSldFMSetFlag_[ch] = false;
		transposeFM_[ch] = 0;
	}
}

int OPNAController::toInternalFMChannel(int ch) const
{
	if (0 <= ch && ch < 6) return ch;
	else if (mode_ == SongType::FMEX && 6 <= ch && ch < 9) return 2;
	else throw std::out_of_range("Out of channel range.");
}

uint8_t OPNAController::getFMKeyOnOffChannelMask(int ch) const
{
	switch (toInternalFMChannel(ch)) {
	case 0: return 0x00;
	case 1: return 0x01;
	case 2: return 0x02;
	case 3: return 0x04;
	case 4: return 0x05;
	case 5: return 0x06;
	default: return 0;
	}
}

uint32_t OPNAController::getFMChannelOffset(int ch, bool forPitch) const
{
	if (mode_ == SongType::FMEX && forPitch) {
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
	if (mode_ == SongType::FMEX && toInternalFMChannel(ch) == 2) {
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

std::vector<FMEnvelopeParameter> OPNAController::getFMEnvelopeParametersForOperator(FMOperatorType op) const
{
	std::vector<FMEnvelopeParameter> params;
	switch (op) {
	case FMOperatorType::All:
		params = {
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
		};
		break;
	case FMOperatorType::Op1:
		params = {
			FMEnvelopeParameter::AL, FMEnvelopeParameter::FB,
			FMEnvelopeParameter::AR1, FMEnvelopeParameter::DR1, FMEnvelopeParameter::SR1, FMEnvelopeParameter::RR1,
			FMEnvelopeParameter::SL1, FMEnvelopeParameter::TL1, FMEnvelopeParameter::KS1, FMEnvelopeParameter::ML1,
			FMEnvelopeParameter::DT1
		};
		break;
	case FMOperatorType::Op2:
		params = {
			FMEnvelopeParameter::AR2, FMEnvelopeParameter::DR2, FMEnvelopeParameter::SR2, FMEnvelopeParameter::RR2,
			FMEnvelopeParameter::SL2, FMEnvelopeParameter::TL2, FMEnvelopeParameter::KS2, FMEnvelopeParameter::ML2,
			FMEnvelopeParameter::DT2
		};
		break;
	case FMOperatorType::Op3:
		params = {
			FMEnvelopeParameter::AR3, FMEnvelopeParameter::DR3, FMEnvelopeParameter::SR3, FMEnvelopeParameter::RR3,
			FMEnvelopeParameter::SL3, FMEnvelopeParameter::TL3, FMEnvelopeParameter::KS3, FMEnvelopeParameter::ML3,
			FMEnvelopeParameter::DT3
		};
		break;
	case FMOperatorType::Op4:
		params = {
			FMEnvelopeParameter::AR4, FMEnvelopeParameter::DR4, FMEnvelopeParameter::SR4, FMEnvelopeParameter::RR4,
			FMEnvelopeParameter::SL4, FMEnvelopeParameter::TL4, FMEnvelopeParameter::KS4, FMEnvelopeParameter::ML4,
			FMEnvelopeParameter::DT4
		};
		break;
	}
	return params;
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
	if (mode_ == SongType::FMEX && inch == 2) data1 = calculateTL(2, data1);
	else if (isCareer(0, al)) data1 = calculateTL(inch, data1);
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
	if (mode_ == SongType::FMEX && inch == 2) data1 = calculateTL(6, data1);
	else if (isCareer(1, al)) data1 = calculateTL(inch, data1);
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
	if (mode_ == SongType::FMEX && inch == 2) data1 = calculateTL(7, data1);
	else if (isCareer(2, al)) data1 = calculateTL(inch, data1);
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
	if (mode_ == SongType::FMEX && inch == 2) data1 = calculateTL(8, data1);
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
		if (mode_ == SongType::FMEX && inch == 2) {
			data = calculateTL(2, data);
			envFM_[inch]->setParameterValue(FMEnvelopeParameter::TL1, data);	// Update
		}
		else if (isCareer(0, envFM_[inch]->getParameterValue(FMEnvelopeParameter::AL))) {
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
		data = refInstFM_[inch]->getLFOEnabled() ? static_cast<uint8_t>(refInstFM_[inch]->getLFOParameter(FMLFOParameter::AM1)) : 0;
		data <<= 7;
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
		if (mode_ == SongType::FMEX && inch == 2) {
			data = calculateTL(6, data);
			envFM_[inch]->setParameterValue(FMEnvelopeParameter::TL2, data);	// Update
		}
		else if (isCareer(1, envFM_[inch]->getParameterValue(FMEnvelopeParameter::AL))) {
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
		data = refInstFM_[inch]->getLFOEnabled() ? static_cast<uint8_t>(refInstFM_[inch]->getLFOParameter(FMLFOParameter::AM2)) : 0;
		data <<= 7;
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
		if (mode_ == SongType::FMEX && inch == 2) {
			data = calculateTL(7, data);
			envFM_[inch]->setParameterValue(FMEnvelopeParameter::TL3, data);	// Update
		}
		else if (isCareer(2, envFM_[inch]->getParameterValue(FMEnvelopeParameter::AL))) {
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
		data = refInstFM_[inch]->getLFOEnabled() ? static_cast<uint8_t>(refInstFM_[inch]->getLFOParameter(FMLFOParameter::AM3)) : 0;
		data <<= 7;
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
		if (mode_ == SongType::FMEX && inch == 2) {
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
		data = refInstFM_[inch]->getLFOEnabled() ? static_cast<uint8_t>(refInstFM_[inch]->getLFOParameter(FMLFOParameter::AM4)) : 0;
		data <<= 7;
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
	if (isMuteFM(ch)) return;

	int inch = toInternalFMChannel(ch);
	if (refInstFM_[inch] && refInstFM_[inch]->getLFOEnabled()) {
		lfoStartCntFM_[inch] = refInstFM_[inch]->getLFOParameter(FMLFOParameter::COUNT);
		writeFMLFOAllRegisters(inch);
	}
	else {
		lfoStartCntFM_[inch] = -1;
	}

	checkOperatorSequenceFM(ch, 1);

	if (treItFM_[ch]) treItFM_[ch]->front();
	sumVolSldFM_[ch] += volSldFM_[ch];
	checkVolumeEffectFM(ch);

	if (arpItFM_[ch]) checkRealToneFMByArpeggio(ch, arpItFM_[ch]->front());
	checkPortamentoFM(ch);

	if (ptItFM_[ch]) checkRealToneFMByPitch(ch, ptItFM_[ch]->front());
	if (vibItFM_[ch]) {
		vibItFM_[ch]->front();
		needToneSetFM_[ch] = true;
	}
	if (nsItFM_[ch] && nsItFM_[ch]->front() != -1) {
		sumNoteSldFM_[ch] += nsItFM_[ch]->getCommandType();
		needToneSetFM_[ch] = true;
	}

	writePitchFM(ch);
}

void OPNAController::releaseStartFMSequences(int ch)
{
	if (isMuteFM(ch)) return;

	int inch = toInternalFMChannel(ch);
	if (lfoStartCntFM_[inch] > 0) {
		--lfoStartCntFM_[inch];
		writeFMLFOAllRegisters(inch);
	}

	checkOperatorSequenceFM(ch, 2);

	if (treItFM_[ch]) treItFM_[ch]->next(true);
	sumVolSldFM_[ch] += volSldFM_[ch];
	checkVolumeEffectFM(ch);

	if (arpItFM_[ch]) checkRealToneFMByArpeggio(ch, arpItFM_[ch]->next(true));
	checkPortamentoFM(ch);

	if (ptItFM_[ch]) checkRealToneFMByPitch(ch, ptItFM_[ch]->next(true));
	if (vibItFM_[ch]) {
		vibItFM_[ch]->next(true);
		needToneSetFM_[ch] = true;
	}
	if (nsItFM_[ch] && nsItFM_[ch]->next(true) != -1) {
		sumNoteSldFM_[ch] += nsItFM_[ch]->getCommandType();
		needToneSetFM_[ch] = true;
	}

	if (needToneSetFM_[ch]) writePitchFM(ch);
}

void OPNAController::tickEventFM(int ch)
{
	if (hasPreSetTickEventFM_[ch]) {
		hasPreSetTickEventFM_[ch] = false;
	}
	else {
		if (isMuteFM(ch)) return;

		int inch = toInternalFMChannel(ch);
		if (lfoStartCntFM_[inch] > 0) {
			--lfoStartCntFM_[inch];
			writeFMLFOAllRegisters(inch);
		}

		checkOperatorSequenceFM(ch, 0);

		if (treItFM_[ch]) treItFM_[ch]->next();
		sumVolSldFM_[ch] += volSldFM_[ch];
		checkVolumeEffectFM(ch);

		if (arpItFM_[ch]) checkRealToneFMByArpeggio(ch, arpItFM_[ch]->next());
		checkPortamentoFM(ch);

		if (ptItFM_[ch]) checkRealToneFMByPitch(ch, ptItFM_[ch]->next());
		if (vibItFM_[ch]) {
			vibItFM_[ch]->next();
			needToneSetFM_[ch] = true;
		}
		if (nsItFM_[ch] && nsItFM_[ch]->next() != -1) {
			sumNoteSldFM_[ch] += nsItFM_[ch]->getCommandType();
			needToneSetFM_[ch] = true;
		}

		if (needToneSetFM_[ch]) writePitchFM(ch);
	}
}

void OPNAController::checkOperatorSequenceFM(int ch, int type)
{
	int inch = toInternalFMChannel(ch);
	for (auto& p : getFMEnvelopeParametersForOperator(toChannelOperatorType(ch))) {
		if (auto& it = opSeqItFM_[inch].at(p)) {
			int t;
			switch (type) {
			case 0:	t = it->next();		break;
			case 1:	t = it->front();	break;
			case 2:	t = it->next(true);	break;
			default:	throw std::out_of_range("The range of type is 0-2.");
			}
			if (t != -1) {
				int d = it->getCommandType();
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
	if (treItFM_[ch]) {
		v = treItFM_[ch]->getCommandType() + sumVolSldFM_[ch];
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
		if (isCareer(0, al)) {	// Operator 1
			int data = envFM_[ch]->getParameterValue(FMEnvelopeParameter::TL1) + v;
			if (data > 127) data = 127;
			else if (data < 0) data = 0;
			opna_->setRegister(0x40 + bch, static_cast<uint8_t>(data));
		}
		if (isCareer(1, al)) {	// Operator 2
			int data = envFM_[ch]->getParameterValue(FMEnvelopeParameter::TL2) + v;
			if (data > 127) data = 127;
			else if (data < 0) data = 0;
			opna_->setRegister(0x40 + bch + 8, static_cast<uint8_t>(data));
		}
		if (isCareer(2, al)) {	// Operator 3
			int data = envFM_[ch]->getParameterValue(FMEnvelopeParameter::TL3) + v;
			if (data > 127) data = 127;
			else if (data < 0) data = 0;
			opna_->setRegister(0x40 + bch + 4, static_cast<uint8_t>(data));
		}
		if (isCareer(3, al)) {	// Operator 4
			int data = envFM_[ch]->getParameterValue(FMEnvelopeParameter::TL4) + v;
			if (data > 127) data = 127;
			else if (data < 0) data = 0;
			opna_->setRegister(0x40 + bch + 12, static_cast<uint8_t>(data));
		}
		break;
	}
	case FMOperatorType::Op1:
	{
		int data = envFM_[inch]->getParameterValue(FMEnvelopeParameter::TL1) + v;
		if (data > 127) data = 127;
		else if (data < 0) data = 0;
		opna_->setRegister(0x40 + bch, static_cast<uint8_t>(data));
		break;
	}
	case FMOperatorType::Op2:
	{
		int data = envFM_[inch]->getParameterValue(FMEnvelopeParameter::TL2) + v;
		if (data > 127) data = 127;
		else if (data < 0) data = 0;
		opna_->setRegister(0x40 + bch + 8, static_cast<uint8_t>(data));
		break;
	}
	case FMOperatorType::Op3:
	{
		int data = envFM_[inch]->getParameterValue(FMEnvelopeParameter::TL3) + v;
		if (data > 127) data = 127;
		else if (data < 0) data = 0;
		opna_->setRegister(0x40 + bch + 4, static_cast<uint8_t>(data));
		break;
	}
	case FMOperatorType::Op4:
	{
		int data = envFM_[inch]->getParameterValue(FMEnvelopeParameter::TL4) + v;
		if (data > 127) data = 127;
		else if (data < 0) data = 0;
		opna_->setRegister(0x40 + bch + 12, static_cast<uint8_t>(data));
		break;
	}
	}
}

void OPNAController::checkRealToneFMByArpeggio(int ch, int seqPos)
{
	if (seqPos == -1) return;

	switch (arpItFM_[ch]->getSequenceType()) {
	case 0:	// Absolute
	{
		std::pair<int, Note> pair = noteNumberToOctaveAndNote(
										octaveAndNoteToNoteNumber(baseToneFM_[ch].front().octave,
																  baseToneFM_[ch].front().note)
										+ arpItFM_[ch]->getCommandType() - 48);
		keyToneFM_[ch].octave = pair.first;
		keyToneFM_[ch].note = pair.second;
		break;
	}
	case 1:	// Fixed
	{
		std::pair<int, Note> pair = noteNumberToOctaveAndNote(arpItFM_[ch]->getCommandType());
		keyToneFM_[ch].octave = pair.first;
		keyToneFM_[ch].note = pair.second;
		break;
	}
	case 2:	// Relative
	{
		std::pair<int, Note> pair = noteNumberToOctaveAndNote(
										octaveAndNoteToNoteNumber(keyToneFM_[ch].octave, keyToneFM_[ch].note)
										+ arpItFM_[ch]->getCommandType() - 48);
		keyToneFM_[ch].octave = pair.first;
		keyToneFM_[ch].note = pair.second;
		break;
	}
	}

	needToneSetFM_[ch] = true;
}

void OPNAController::checkPortamentoFM(int ch)
{
	if ((!arpItFM_[ch] || arpItFM_[ch]->getPosition() == -1) && prtmFM_[ch]) {
		if (isTonePrtmFM_[ch]) {
			int dif = ( octaveAndNoteToNoteNumber(baseToneFM_[ch].front().octave, baseToneFM_[ch].front().note) * 32
						+ baseToneFM_[ch].front().pitch )
					  - ( octaveAndNoteToNoteNumber(keyToneFM_[ch].octave, keyToneFM_[ch].note) * 32
						  + keyToneFM_[ch].pitch );
			if (dif > 0) {
				if (dif - prtmFM_[ch] < 0) {
					keyToneFM_[ch] = baseToneFM_[ch].front();
				}
				else {
					keyToneFM_[ch].pitch += prtmFM_[ch];
				}
				needToneSetFM_[ch] = true;
			}
			else if (dif < 0) {
				if (dif + prtmFM_[ch] > 0) {
					keyToneFM_[ch] = baseToneFM_[ch].front();
				}
				else {
					keyToneFM_[ch].pitch -= prtmFM_[ch];
				}
				needToneSetFM_[ch] = true;
			}
		}
		else {
			keyToneFM_[ch].pitch += prtmFM_[ch];
			needToneSetFM_[ch] = true;
		}
	}
}

void OPNAController::checkRealToneFMByPitch(int ch, int seqPos)
{
	if (seqPos == -1) return;

	switch (ptItFM_[ch]->getSequenceType()) {
	case 0:	// Absolute
		sumPitchFM_[ch] = ptItFM_[ch]->getCommandType() - 127;
		break;
	case 2:	// Relative
		sumPitchFM_[ch] += (ptItFM_[ch]->getCommandType() - 127);
		break;
	}

	needToneSetFM_[ch] = true;
}

void OPNAController::writePitchFM(int ch)
{
	uint16_t p = PitchConverter::getPitchFM(
					 keyToneFM_[ch].note,
					 keyToneFM_[ch].octave,
					 keyToneFM_[ch].pitch
					 + sumPitchFM_[ch]
					 + (vibItFM_[ch] ? vibItFM_[ch]->getCommandType() : 0)
					 + detuneFM_[ch]
					 + sumNoteSldFM_[ch]
					 + transposeFM_[ch]);
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

bool OPNAController::isCareer(int op, int al)
{
	switch (op) {
	case 0:
		return (al == 7);
	case 1:
		switch (al) {
		case 4:
		case 5:
		case 6:
		case 7:
			return true;
		default:
			return false;
		}
	case 2:
		switch (al) {
		case 5:
		case 6:
		case 7:
			return true;
		default:
			return false;
		}
	case 3:
		return true;
	default:
		return false;
	}
}

//---------- SSG ----------//
/********** Key on-off **********/
void OPNAController::keyOnSSG(int ch, Note note, int octave, int pitch, bool isJam)
{
	if (isMuteSSG(ch)) return;

	updateEchoBufferSSG(ch, octave, note, pitch);

	if (isTonePortamentoSSG(ch)) {
		keyToneSSG_[ch].pitch += (sumNoteSldSSG_[ch] +transposeSSG_[ch]);
	}
	else {
		keyToneSSG_[ch] = baseToneSSG_[ch].front();
		sumPitchSSG_[ch] = 0;
		sumVolSldSSG_[ch] = 0;
	}
	if (tmpVolSSG_[ch] != -1 && !volSldSSG_[ch]) {
		setVolumeSSG(ch, baseVolSSG_[ch]);
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
}

void OPNAController::keyOnSSG(int ch, int echoBuf)
{
	ToneDetail& td = baseToneSSG_[ch].at(static_cast<size_t>(echoBuf));
	if (td.octave == -1) return;
	keyOnSSG(ch, td.note, td.octave, td.pitch);
}

void OPNAController::keyOffSSG(int ch, bool isJam)
{
	if (!isKeyOnSSG_[ch]) return;
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

	if (refInstSSG_[ch]->getWaveFormEnabled())
		wfItSSG_[ch] = refInstSSG_[ch]->getWaveFormSequenceIterator();
	else
		wfItSSG_[ch].reset();
	if (refInstSSG_[ch]->getToneNoiseEnabled())
		tnItSSG_[ch] = refInstSSG_[ch]->getToneNoiseSequenceIterator();
	else
		tnItSSG_[ch].reset();
	if (refInstSSG_[ch]->getEnvelopeEnabled())
		envItSSG_[ch] = refInstSSG_[ch]->getEnvelopeSequenceIterator();
	else
		envItSSG_[ch].reset();
	if (!isArpEffSSG_[ch]) {
		if (refInstSSG_[ch]->getArpeggioEnabled())
			arpItSSG_[ch] = refInstSSG_[ch]->getArpeggioSequenceIterator();
		else
			arpItSSG_[ch].reset();
	}
	if (refInstSSG_[ch]->getPitchEnabled())
		ptItSSG_[ch] = refInstSSG_[ch]->getPitchSequenceIterator();
	else
		ptItSSG_[ch].reset();
}

void OPNAController::updateInstrumentSSG(int instNum)
{
	for (int ch = 0; ch < 3; ++ch) {
		if (refInstSSG_[ch] && refInstSSG_[ch]->isRegisteredWithManager()
				&& refInstSSG_[ch]->getNumber() == instNum) {
			if (!refInstSSG_[ch]->getWaveFormEnabled()) wfItSSG_[ch].reset();
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
	if (volume > 0xf) return;	// Out of range

	baseVolSSG_[ch] = volume;
	tmpVolSSG_[ch] = -1;

	if (isKeyOnSSG(ch)) setRealVolumeSSG(ch);
}

void OPNAController::setTemporaryVolumeSSG(int ch, int volume)
{
	if (volume > 0xf) return;	// Out of range

	tmpVolSSG_[ch] = volume;

	if (isKeyOnSSG(ch)) setRealVolumeSSG(ch);
}

void OPNAController::setRealVolumeSSG(int ch)
{
	if (isBuzzEffSSG_[ch] || isHardEnvSSG_[ch]) return;

	int volume = (tmpVolSSG_[ch] == -1) ? baseVolSSG_[ch] : tmpVolSSG_[ch];
	if (envItSSG_[ch]) {
		int type = envItSSG_[ch]->getCommandType();
		if (0 <= type && type < 16) {
			volume = volume - (15 - type);
			if (volume < 0) volume = 0;
		}
	}
	if (treItSSG_[ch]) volume += treItSSG_[ch]->getCommandType();
	volume += sumVolSldSSG_[ch];

	if (volume > 15) volume = 15;
	else if (volume < 0) volume = 0;

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
		if (!refInstSSG_[ch]->getArpeggioEnabled()) arpItSSG_[ch].reset();
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
	volSldSSG_[ch] = depth * (isUp ? 1 : -1);
}

void OPNAController::setDetuneSSG(int ch, int pitch)
{
	detuneSSG_[ch] = pitch;
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
	transposeSSG_[ch] += (seminote * 32);
	needToneSetSSG_[ch] = true;
}

void OPNAController::setToneNoiseMixSSG(int ch, int value)
{
	toneNoiseMixSSG_[ch] = value;

	// Tone
	if ((tnSSG_[ch].isTone_ = (0x01 & value))) mixerSSG_ &= ~(1 << ch);
	else mixerSSG_ |= (1 << ch);
	// Noise
	if ((tnSSG_[ch].isNoise_ = (0x02 & value))) mixerSSG_ &= ~(1 << (ch + 3));
	else mixerSSG_ |= (1 << (ch + 3));
	opna_->setRegister(0x07, mixerSSG_);

	if (tnItSSG_[ch]) tnItSSG_[ch].reset();
}

void OPNAController::setNoiseFrequencySSG(int ch, int freq)
{
	noiseFreqSSG_ = freq;
	tnSSG_[ch].noisePeriod_ = freq;
	opna_->setRegister(0x06, static_cast<uint8_t>(freq));
	if (tnItSSG_[ch]) tnItSSG_[ch].reset();
}

/********** For state retrieve **********/
void OPNAController::haltSequencesSSG(int ch)
{
	if (wfItSSG_[ch]) wfItSSG_[ch]->end();
	if (treItSSG_[ch]) treItSSG_[ch]->end();
	if (envItSSG_[ch]) envItSSG_[ch]->end();
	if (tnItSSG_[ch]) tnItSSG_[ch]->end();
	if (arpItSSG_[ch]) arpItSSG_[ch]->end();
	if (ptItSSG_[ch]) ptItSSG_[ch]->end();
	if (vibItSSG_[ch]) vibItSSG_[ch]->next();
	if (nsItSSG_[ch]) nsItSSG_[ch]->end();
}

/********** Mute **********/
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
	noiseFreqSSG_ = 0;

	for (int ch = 0; ch < 3; ++ch) {
		isKeyOnSSG_[ch] = false;

		refInstSSG_[ch].reset();	// Init envelope

		// Init echo buffer
		baseToneSSG_[ch] = std::deque<ToneDetail>(4);
		for (auto& td : baseToneSSG_[ch]) {
			td.octave = -1;
		}

		keyToneSSG_[ch].octave = -1;
		sumPitchSSG_[ch] = 0;
		tnSSG_[ch] = { false, false, -1 };
		baseVolSSG_[ch] = 0xf;	// Init volume
		tmpVolSSG_[ch] = -1;
		isHardEnvSSG_[ch] = false;
		isBuzzEffSSG_[ch] = false;

		// Init sequence
		hasPreSetTickEventSSG_[ch] = false;
		wfItSSG_[ch].reset();
		wfSSG_[ch] = { SSGWaveFormType::UNSET, -1 };
		envItSSG_[ch].reset();
		envSSG_[ch] = { -1, -1 };
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
		nsItSSG_[ch].reset();
		sumNoteSldSSG_[ch] = 0;
		noteSldSSGSetFlag_ = false;
		transposeSSG_[ch] = 0;
		toneNoiseMixSSG_[ch] = 0;
	}
}

void OPNAController::setFrontSSGSequences(int ch)
{
	if (isMuteSSG(ch)) return;

	setHardEnvIfNecessary_[ch] = false;

	if (wfItSSG_[ch]) writeWaveFormSSGToRegister(ch, wfItSSG_[ch]->front());
	else writeSquareWaveForm(ch);

	if (treItSSG_[ch]) {
		treItSSG_[ch]->front();
		needEnvSetSSG_[ch] = true;
	}
	if (volSldSSG_[ch]) {
		sumVolSldSSG_[ch] += volSldSSG_[ch];
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

	if (arpItSSG_[ch]) checkRealToneSSGByArpeggio(ch, arpItSSG_[ch]->front());
	checkPortamentoSSG(ch);

	if (ptItSSG_[ch]) checkRealToneSSGByPitch(ch, ptItSSG_[ch]->front());
	if (vibItSSG_[ch]) {
		vibItSSG_[ch]->front();
		needToneSetSSG_[ch] = true;
	}
	if (nsItSSG_[ch] && nsItSSG_[ch]->front() != -1) {
		sumNoteSldSSG_[ch] += nsItSSG_[ch]->getCommandType();
		needToneSetSSG_[ch] = true;
	}

	writePitchSSG(ch);
}

void OPNAController::releaseStartSSGSequences(int ch)
{
	if (isMuteSSG(ch)) return;

	setHardEnvIfNecessary_[ch] = false;

	if (wfItSSG_[ch]) writeWaveFormSSGToRegister(ch, wfItSSG_[ch]->next(true));

	if (treItSSG_[ch]) {
		treItSSG_[ch]->next(true);
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

	if (arpItSSG_[ch]) checkRealToneSSGByArpeggio(ch, arpItSSG_[ch]->next(true));
	checkPortamentoSSG(ch);

	if (ptItSSG_[ch]) checkRealToneSSGByPitch(ch, ptItSSG_[ch]->next(true));
	if (vibItSSG_[ch]) {
		vibItSSG_[ch]->next(true);
		needToneSetSSG_[ch] = true;
	}
	if (nsItSSG_[ch] && nsItSSG_[ch]->next(true) != -1) {
		sumNoteSldSSG_[ch] += nsItSSG_[ch]->getCommandType();
		needToneSetSSG_[ch] = true;
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
		if (isMuteSSG(ch)) return;

		setHardEnvIfNecessary_[ch] = false;

		if (wfItSSG_[ch]) writeWaveFormSSGToRegister(ch, wfItSSG_[ch]->next());

		if (treItSSG_[ch]) {
			treItSSG_[ch]->next();
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

		if (arpItSSG_[ch]) checkRealToneSSGByArpeggio(ch, arpItSSG_[ch]->next());
		checkPortamentoSSG(ch);

		if (ptItSSG_[ch]) checkRealToneSSGByPitch(ch, ptItSSG_[ch]->next());
		if (vibItSSG_[ch]) {
			vibItSSG_[ch]->next();
			needToneSetSSG_[ch] = true;
		}
		if (nsItSSG_[ch] && nsItSSG_[ch]->next() != -1) {
			sumNoteSldSSG_[ch] += nsItSSG_[ch]->getCommandType();
			needToneSetSSG_[ch] = true;
		}

		if (needToneSetSSG_[ch] || (isHardEnvSSG_[ch] && needEnvSetSSG_[ch]) || needSqMaskFreqSetSSG_[ch])
			writePitchSSG(ch);
	}
}

void OPNAController::writeWaveFormSSGToRegister(int ch, int seqPos)
{
	if (seqPos == -1) return;

	switch (static_cast<SSGWaveFormType>(wfItSSG_[ch]->getCommandType())) {
	case SSGWaveFormType::SQUARE:
	{
		writeSquareWaveForm(ch);
		return;
	}
	case SSGWaveFormType::TRIANGLE:
	{
		if (wfSSG_[ch].type == SSGWaveFormType::TRIANGLE && isKeyOnSSG_[ch]) return;

		switch (wfSSG_[ch].type) {
		case SSGWaveFormType::UNSET:
		case SSGWaveFormType::SQUARE:
		case SSGWaveFormType::SQM_TRIANGLE:
		case SSGWaveFormType::SQM_SAW:
		case SSGWaveFormType::SQM_INVSAW:
			needMixSetSSG_[ch] = true;
			break;
		default:
			break;
		}

		switch (wfSSG_[ch].type) {
		case SSGWaveFormType::UNSET:
		case SSGWaveFormType::SQUARE:
		case SSGWaveFormType::SAW:
		case SSGWaveFormType::INVSAW:
		case SSGWaveFormType::SQM_SAW:
		case SSGWaveFormType::SQM_INVSAW:
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

		if (envSSG_[ch].type == 0) envSSG_[ch] = { -1, -1 };

		needEnvSetSSG_[ch] = false;
		needToneSetSSG_[ch] = true;
		needSqMaskFreqSetSSG_[ch] = false;
		wfSSG_[ch] = { SSGWaveFormType::TRIANGLE, -1 };
		return;
	}
	case SSGWaveFormType::SAW:
	{
		if (wfSSG_[ch].type == SSGWaveFormType::SAW && isKeyOnSSG_[ch]) return;

		switch (wfSSG_[ch].type) {
		case SSGWaveFormType::UNSET:
		case SSGWaveFormType::SQUARE:
		case SSGWaveFormType::SQM_TRIANGLE:
		case SSGWaveFormType::SQM_SAW:
		case SSGWaveFormType::SQM_INVSAW:
			needMixSetSSG_[ch] = true;
			break;
		default:
			break;
		}

		switch (wfSSG_[ch].type) {
		case SSGWaveFormType::UNSET:
		case SSGWaveFormType::SQUARE:
		case SSGWaveFormType::TRIANGLE:
		case SSGWaveFormType::INVSAW:
		case SSGWaveFormType::SQM_TRIANGLE:
		case SSGWaveFormType::SQM_INVSAW:
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

		if (envSSG_[ch].type == 0) envSSG_[ch] = { -1, -1 };

		needEnvSetSSG_[ch] = false;
		needToneSetSSG_[ch] = true;
		needSqMaskFreqSetSSG_[ch] = false;
		wfSSG_[ch] = { SSGWaveFormType::SAW, -1 };
		return;
	}
	case SSGWaveFormType::INVSAW:
	{
		if (wfSSG_[ch].type == SSGWaveFormType::INVSAW && isKeyOnSSG_[ch]) return;

		switch (wfSSG_[ch].type) {
		case SSGWaveFormType::UNSET:
		case SSGWaveFormType::SQUARE:
		case SSGWaveFormType::SQM_TRIANGLE:
		case SSGWaveFormType::SQM_SAW:
		case SSGWaveFormType::SQM_INVSAW:
			needMixSetSSG_[ch] = true;
			break;
		default:
			break;
		}

		switch (wfSSG_[ch].type) {
		case SSGWaveFormType::UNSET:
		case SSGWaveFormType::SQUARE:
		case SSGWaveFormType::TRIANGLE:
		case SSGWaveFormType::SAW:
		case SSGWaveFormType::SQM_TRIANGLE:
		case SSGWaveFormType::SQM_SAW:
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

		if (envSSG_[ch].type == 0) envSSG_[ch] = { -1, -1 };

		needEnvSetSSG_[ch] = false;
		needToneSetSSG_[ch] = true;
		needSqMaskFreqSetSSG_[ch] = false;
		wfSSG_[ch] = { SSGWaveFormType::INVSAW, -1 };
		return;
	}
	case SSGWaveFormType::SQM_TRIANGLE:
	{
		int data = wfItSSG_[ch]->getCommandData();
		if (wfSSG_[ch].type == SSGWaveFormType::SQM_TRIANGLE && wfSSG_[ch].data == data && isKeyOnSSG_[ch]) return;

		switch (wfSSG_[ch].type) {
		case SSGWaveFormType::UNSET:
		case SSGWaveFormType::TRIANGLE:
		case SSGWaveFormType::SAW:
		case SSGWaveFormType::INVSAW:
			needMixSetSSG_[ch] = true;
			break;
		default:
			break;
		}

		if (wfSSG_[ch].data != data) {
			if (CommandSequenceUnit::isRatioData(data)) {
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
		case SSGWaveFormType::UNSET:
		case SSGWaveFormType::SQUARE:
		case SSGWaveFormType::SAW:
		case SSGWaveFormType::INVSAW:
		case SSGWaveFormType::SQM_SAW:
		case SSGWaveFormType::SQM_INVSAW:
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

		if (envSSG_[ch].type == 0) envSSG_[ch] = { -1, -1 };

		needEnvSetSSG_[ch] = false;
		needToneSetSSG_[ch] = true;
		wfSSG_[ch] = { SSGWaveFormType::SQM_TRIANGLE, data };
		return;
	}
	case SSGWaveFormType::SQM_SAW:
	{
		int data = wfItSSG_[ch]->getCommandData();
		if (wfSSG_[ch].type == SSGWaveFormType::SQM_SAW && wfSSG_[ch].data == data && isKeyOnSSG_[ch]) return;

		switch (wfSSG_[ch].type) {
		case SSGWaveFormType::UNSET:
		case SSGWaveFormType::TRIANGLE:
		case SSGWaveFormType::SAW:
		case SSGWaveFormType::INVSAW:
			needMixSetSSG_[ch] = true;
			break;
		default:
			break;
		}

		if (wfSSG_[ch].data != data) {
			if (CommandSequenceUnit::isRatioData(data)) {
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
		case SSGWaveFormType::UNSET:
		case SSGWaveFormType::SQUARE:
		case SSGWaveFormType::TRIANGLE:
		case SSGWaveFormType::INVSAW:
		case SSGWaveFormType::SQM_TRIANGLE:
		case SSGWaveFormType::SQM_INVSAW:
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

		if (envSSG_[ch].type == 0) envSSG_[ch] = { -1, -1 };

		needEnvSetSSG_[ch] = false;
		needToneSetSSG_[ch] = true;
		wfSSG_[ch] = { SSGWaveFormType::SQM_SAW, data };
		return;
	}
	case SSGWaveFormType::SQM_INVSAW:
	{
		int data = wfItSSG_[ch]->getCommandData();
		if (wfSSG_[ch].type == SSGWaveFormType::SQM_INVSAW && wfSSG_[ch].data == data && isKeyOnSSG_[ch]) return;

		switch (wfSSG_[ch].type) {
		case SSGWaveFormType::UNSET:
		case SSGWaveFormType::TRIANGLE:
		case SSGWaveFormType::SAW:
		case SSGWaveFormType::INVSAW:
			needMixSetSSG_[ch] = true;
			break;
		default:
			break;
		}

		if (wfSSG_[ch].data != data) {
			if (CommandSequenceUnit::isRatioData(data)) {
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
		case SSGWaveFormType::UNSET:
		case SSGWaveFormType::SQUARE:
		case SSGWaveFormType::TRIANGLE:
		case SSGWaveFormType::SAW:
		case SSGWaveFormType::SQM_TRIANGLE:
		case SSGWaveFormType::SQM_SAW:
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

		if (envSSG_[ch].type == 0) envSSG_[ch] = { -1, -1 };

		needEnvSetSSG_[ch] = false;
		needToneSetSSG_[ch] = true;
		wfSSG_[ch] = { SSGWaveFormType::SQM_INVSAW, data };
		return;
	}
	default:
		break;
	}
}

void OPNAController::writeSquareWaveForm(int ch)
{
	if (wfSSG_[ch].type == SSGWaveFormType::SQUARE) {
		if (!isKeyOnSSG_[ch]) {
			needEnvSetSSG_[ch] = true;
			needToneSetSSG_[ch] = true;
		}
		return;
	}

	switch (wfSSG_[ch].type) {
	case SSGWaveFormType::SQM_TRIANGLE:
	case SSGWaveFormType::SQM_SAW:
	case SSGWaveFormType::SQM_INVSAW:
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
	wfSSG_[ch] = { SSGWaveFormType::SQUARE, -1 };
}

void OPNAController::writeToneNoiseSSGToRegister(int ch, int seqPos)
{
	if (seqPos == -1) {
		if (needMixSetSSG_[ch]) writeToneNoiseSSGToRegisterNoReference(ch);
		return;
	}

	int type = tnItSSG_[ch]->getCommandType();
	if (type == 0) {	// tone
		if (tnSSG_[ch].isTone_) {
			if (tnSSG_[ch].isNoise_) {
				mixerSSG_ |= (1 << (ch + 3));
				switch (wfSSG_[ch].type) {
				case SSGWaveFormType::TRIANGLE:
				case SSGWaveFormType::SAW:
				case SSGWaveFormType::INVSAW:
					mixerSSG_ |= (1 << ch);
					opna_->setRegister(0x07, mixerSSG_);
					tnSSG_[ch] = { false, false, -1 };
					break;
				default:
					opna_->setRegister(0x07, mixerSSG_);
					tnSSG_[ch] = { true, false, -1 };
					break;
				}
			}
			else {
				switch (wfSSG_[ch].type) {
				case SSGWaveFormType::TRIANGLE:
				case SSGWaveFormType::SAW:
				case SSGWaveFormType::INVSAW:
					mixerSSG_ |= (1 << ch);
					opna_->setRegister(0x07, mixerSSG_);
					tnSSG_[ch] = { false, false, -1 };
					break;
				default:
					break;
				}
			}
		}
		else {
			if (tnSSG_[ch].isNoise_) {
				mixerSSG_ |= (1 << (ch + 3));
				switch (wfSSG_[ch].type) {
				case SSGWaveFormType::TRIANGLE:
				case SSGWaveFormType::SAW:
				case SSGWaveFormType::INVSAW:
					opna_->setRegister(0x07, mixerSSG_);
					tnSSG_[ch] = { false, false, -1 };
					break;
				default:
					mixerSSG_ &= ~(1 << ch);
					opna_->setRegister(0x07, mixerSSG_);
					tnSSG_[ch] = { true, false, -1 };
					break;
				}
			}
			else {
				switch (wfSSG_[ch].type) {
				case SSGWaveFormType::TRIANGLE:
				case SSGWaveFormType::SAW:
				case SSGWaveFormType::INVSAW:
					break;
				default:
					mixerSSG_ &= ~(1 << ch);
					opna_->setRegister(0x07, mixerSSG_);
					tnSSG_[ch] = { true, false, -1 };
					break;
				}
			}
		}
	}
	else {
		if (type > 32) {	// Tone&Noise
			if (tnSSG_[ch].isNoise_) {
				if (tnSSG_[ch].isTone_) {
					switch (wfSSG_[ch].type) {
					case SSGWaveFormType::TRIANGLE:
					case SSGWaveFormType::SAW:
					case SSGWaveFormType::INVSAW:
						mixerSSG_ |= (1 << ch);
						tnSSG_[ch].isTone_ = false;
						opna_->setRegister(0x07, mixerSSG_);
						break;
					default:
						break;
					}
				}
				else {
					switch (wfSSG_[ch].type) {
					case SSGWaveFormType::TRIANGLE:
					case SSGWaveFormType::SAW:
					case SSGWaveFormType::INVSAW:
						break;
					default:
						mixerSSG_ &= ~(1 << ch);
						tnSSG_[ch].isTone_ = true;
						opna_->setRegister(0x07, mixerSSG_);
						break;
					}
				}
			}
			else {
				if (tnSSG_[ch].isTone_) {
					switch (wfSSG_[ch].type) {
					case SSGWaveFormType::TRIANGLE:
					case SSGWaveFormType::SAW:
					case SSGWaveFormType::INVSAW:
						mixerSSG_ |= (1 << ch);
						tnSSG_[ch].isTone_ = false;
						break;
					default:
						break;
					}
				}
				else {
					switch (wfSSG_[ch].type) {
					case SSGWaveFormType::TRIANGLE:
					case SSGWaveFormType::SAW:
					case SSGWaveFormType::INVSAW:
						break;
					default:
						mixerSSG_ &= ~(1 << ch);
						tnSSG_[ch].isTone_ = true;
						break;
					}
				}
				mixerSSG_ &= ~(1 << (ch + 3));
				opna_->setRegister(0x07, mixerSSG_);
				tnSSG_[ch].isNoise_ = true;
			}

			if (!tnSSG_[ch].isTone_ || !tnSSG_[ch].isNoise_) {
				mixerSSG_ &= ~(0x1001 << ch);
				opna_->setRegister(0x07, mixerSSG_);
				tnSSG_[ch].isTone_ = true;
				tnSSG_[ch].isNoise_ = true;
			}


			int p = type - 33;
			if (tnSSG_[ch].noisePeriod_ != p) {
				opna_->setRegister(0x06, static_cast<uint8_t>(p));
				tnSSG_->noisePeriod_ = p;
			}
		}
		else {	// Noise
			if (tnSSG_[ch].isNoise_) {
				if (tnSSG_[ch].isTone_) {
					mixerSSG_ |= (1 << ch);
					opna_->setRegister(0x07, mixerSSG_);
					tnSSG_[ch].isTone_ = false;
				}
			}
			else {
				if (tnSSG_[ch].isTone_) {
					mixerSSG_ |= (1 << ch);
					tnSSG_[ch].isTone_ = false;
				}
				mixerSSG_ &= ~(1 << (ch + 3));
				opna_->setRegister(0x07, mixerSSG_);
				tnSSG_[ch].isNoise_ = true;
			}

			int p = type - 1;
			if (tnSSG_[ch].noisePeriod_ != p) {
				opna_->setRegister(0x06, static_cast<uint8_t>(p));
				tnSSG_->noisePeriod_ = p;
			}
		}
	}

	needMixSetSSG_[ch] = false;
}

void OPNAController::writeToneNoiseSSGToRegisterNoReference(int ch)
{
	switch (wfSSG_[ch].type) {
	case SSGWaveFormType::SQUARE:
	case SSGWaveFormType::SQM_TRIANGLE:
	case SSGWaveFormType::SQM_SAW:
	case SSGWaveFormType::SQM_INVSAW:
		mixerSSG_ &= ~(1 << ch);
		tnSSG_[ch].isTone_ = true;
		break;
	case SSGWaveFormType::TRIANGLE:
	case SSGWaveFormType::SAW:
	case SSGWaveFormType::INVSAW:
		mixerSSG_ |= (1 << ch);
		tnSSG_[ch].isNoise_ = false;
		break;
	default:
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
	if (type < 16) {	// Software envelope
		isHardEnvSSG_[ch] = false;
		envSSG_[ch] = { type, -1 };
		setRealVolumeSSG(ch);
		needEnvSetSSG_[ch] = false;
	}
	else {	// Hardware envelope
		int data = envItSSG_[ch]->getCommandData();
		if (envSSG_[ch].data != data || setHardEnvIfNecessary_[ch]) {
			envSSG_[ch].data = data;
			if (CommandSequenceUnit::isRatioData(data)) {
				/* Envelope frequency is set in writePitchSSG */
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
			if (CommandSequenceUnit::isRatioData(data)) needEnvSetSSG_[ch] = true;
		}
		if (!isHardEnvSSG_[ch]) {
			opna_->setRegister(static_cast<uint32_t>(0x08 + ch), 0x10);
			isHardEnvSSG_[ch] = true;
		}
		// setHardEnvIfNecessary_[ch] = false;
	}
}

void OPNAController::checkRealToneSSGByArpeggio(int ch, int seqPos)
{
	if (seqPos == -1) return;

	switch (arpItSSG_[ch]->getSequenceType()) {
	case 0:	// Absolute
	{
		std::pair<int, Note> pair = noteNumberToOctaveAndNote(
										octaveAndNoteToNoteNumber(baseToneSSG_[ch].front().octave,
																  baseToneSSG_[ch].front().note)
										+ arpItSSG_[ch]->getCommandType() - 48);
		keyToneSSG_[ch].octave = pair.first;
		keyToneSSG_[ch].note = pair.second;
		break;
	}
	case 1:	// Fixed
	{
		std::pair<int, Note> pair = noteNumberToOctaveAndNote(arpItSSG_[ch]->getCommandType());
		keyToneSSG_[ch].octave = pair.first;
		keyToneSSG_[ch].note = pair.second;
		break;
	}
	case 2:	// Relative
	{
		std::pair<int, Note> pair = noteNumberToOctaveAndNote(
										octaveAndNoteToNoteNumber(keyToneSSG_[ch].octave, keyToneSSG_[ch].note)
										+ arpItSSG_[ch]->getCommandType() - 48);
		keyToneSSG_[ch].octave = pair.first;
		keyToneSSG_[ch].note = pair.second;
		break;
	}
	}

	needToneSetSSG_[ch] = true;
}

void OPNAController::checkPortamentoSSG(int ch)
{
	if ((!arpItSSG_[ch] || arpItSSG_[ch]->getPosition() == -1) && prtmSSG_[ch]) {
		if (isTonePrtmSSG_[ch]) {
			int dif = ( octaveAndNoteToNoteNumber(baseToneSSG_[ch].front().octave, baseToneSSG_[ch].front().note) * 32
						+ baseToneSSG_[ch].front().pitch )
					  - ( octaveAndNoteToNoteNumber(keyToneSSG_[ch].octave, keyToneSSG_[ch].note) * 32
						  + keyToneSSG_[ch].pitch );
			if (dif > 0) {
				if (dif - prtmSSG_[ch] < 0) {
					keyToneSSG_[ch] = baseToneSSG_[ch].front();
				}
				else {
					keyToneSSG_[ch].pitch += prtmSSG_[ch];
				}
				needToneSetSSG_[ch] = true;
			}
			else if (dif < 0) {
				if (dif + prtmSSG_[ch] > 0) {
					keyToneSSG_[ch] = baseToneSSG_[ch].front();
				}
				else {
					keyToneSSG_[ch].pitch -= prtmSSG_[ch];
				}
				needToneSetSSG_[ch] = true;
			}
		}
		else {
			keyToneSSG_[ch].pitch += prtmSSG_[ch];
			needToneSetSSG_[ch] = true;
		}
	}
}

void OPNAController::checkRealToneSSGByPitch(int ch, int seqPos)
{
	if (seqPos == -1) return;

	switch (ptItSSG_[ch]->getSequenceType()) {
	case 0:	// Absolute
		sumPitchSSG_[ch] = ptItSSG_[ch]->getCommandType() - 127;
		break;
	case 2:	// Relative
		sumPitchSSG_[ch] += (ptItSSG_[ch]->getCommandType() - 127);
		break;
	}

	needToneSetSSG_[ch] = true;
}

void OPNAController::writePitchSSG(int ch)
{
	int p = keyToneSSG_[ch].pitch
			+ sumPitchSSG_[ch]
			+ (vibItSSG_[ch] ? vibItSSG_[ch]->getCommandType() : 0)
			+ detuneSSG_[ch]
			+ sumNoteSldSSG_[ch]
			+ transposeSSG_[ch];

	switch (wfSSG_[ch].type) {
	case SSGWaveFormType::SQUARE:
	{
		uint16_t pitch = PitchConverter::getPitchSSGSquare(
							 keyToneSSG_[ch].note, keyToneSSG_[ch].octave, p);
		if (needToneSetSSG_[ch]) {
			uint8_t offset = static_cast<uint8_t>(ch << 1);
			opna_->setRegister(0x00 + offset, pitch & 0xff);
			opna_->setRegister(0x01 + offset, pitch >> 8);
			if (CommandSequenceUnit::isRatioData(envSSG_[ch].data)) {	// Hard envelope ratio
				double hz = opna_->getClock() / 64.0 / pitch;
				// Multiple frequency if triangle
				int mul = (envSSG_[ch].type == 18 || envSSG_[ch].type == 22) ? 2 : 1;
				auto ratio = CommandSequenceUnit::data2ratio(envSSG_[ch].data);
				hz = mul * hz * ratio.second / ratio.first;
				uint16_t freq = static_cast<uint16_t>(opna_->getClock() / 1024.0 / hz);
				opna_->setRegister(0x0b, 0x00ff & freq);
				opna_->setRegister(0x0c, static_cast<uint8_t>(freq >> 8));
			}
		}
		else if (isHardEnvSSG_[ch] && needEnvSetSSG_[ch]) {
			if (CommandSequenceUnit::isRatioData(envSSG_[ch].data)) {	// Hard envelope ratio
				double hz = opna_->getClock() / 64.0 / pitch;
				// Multiple frequency if triangle
				int mul = (envSSG_[ch].type == 18 || envSSG_[ch].type == 22) ? 2 : 1;
				auto ratio = CommandSequenceUnit::data2ratio(envSSG_[ch].data);
				hz = mul * hz * ratio.second / ratio.first;
				uint16_t freq = static_cast<uint16_t>(opna_->getClock() / 1024.0 / hz);
				opna_->setRegister(0x0b, 0x00ff & freq);
				opna_->setRegister(0x0c, static_cast<uint8_t>(freq >> 8));
			}
		}
		break;
	}
	case SSGWaveFormType::TRIANGLE:
	if (needToneSetSSG_[ch]) {
		uint16_t pitch = PitchConverter::getPitchSSGTriangle(
							 keyToneSSG_[ch].note, keyToneSSG_[ch].octave, p);
		opna_->setRegister(0x0b, pitch & 0x00ff);
		opna_->setRegister(0x0c, pitch >> 8);
	}
	break;
	case SSGWaveFormType::SAW:
	case SSGWaveFormType::INVSAW:
	if (needToneSetSSG_[ch]){
		uint16_t pitch = PitchConverter::getPitchSSGSaw(
							 keyToneSSG_[ch].note, keyToneSSG_[ch].octave, p);
		opna_->setRegister(0x0b, pitch & 0x00ff);
		opna_->setRegister(0x0c, pitch >> 8);
	}
	break;
	case SSGWaveFormType::SQM_TRIANGLE:
	{
		uint16_t pitch = PitchConverter::getPitchSSGTriangle(
							 keyToneSSG_[ch].note, keyToneSSG_[ch].octave, p);
		if (needToneSetSSG_[ch]) {
			opna_->setRegister(0x0b, pitch & 0x00ff);
			opna_->setRegister(0x0c, pitch >> 8);
			if (CommandSequenceUnit::isRatioData(wfSSG_[ch].data)) {
				double hz = opna_->getClock() / 2048.0 / pitch;
				auto ratio = CommandSequenceUnit::data2ratio(wfSSG_[ch].data);
				hz = hz * ratio.second / ratio.first;
				uint16_t freq = static_cast<uint16_t>(opna_->getClock() / 64.0 / hz);
				uint8_t offset = static_cast<uint8_t>(ch << 1);
				opna_->setRegister(0x00 + offset, freq & 0x00ff);
				opna_->setRegister(0x01 + offset, freq >> 8);
			}
		}
		else if (needSqMaskFreqSetSSG_[ch]) {
			if (CommandSequenceUnit::isRatioData(wfSSG_[ch].data)) {
				double hz = opna_->getClock() / 2048.0 / pitch;
				auto ratio = CommandSequenceUnit::data2ratio(wfSSG_[ch].data);
				hz = hz * ratio.second / ratio.first;
				uint16_t freq = static_cast<uint16_t>(opna_->getClock() / 64.0 / hz);
				uint8_t offset = static_cast<uint8_t>(ch << 1);
				opna_->setRegister(0x00 + offset, freq & 0x00ff);
				opna_->setRegister(0x01 + offset, freq >> 8);
			}
		}
		break;
	}
	case SSGWaveFormType::SQM_SAW:
	case SSGWaveFormType::SQM_INVSAW:
	{
		uint16_t pitch = PitchConverter::getPitchSSGSaw(
							 keyToneSSG_[ch].note, keyToneSSG_[ch].octave, p);
		if (needToneSetSSG_[ch]) {
			opna_->setRegister(0x0b, pitch & 0x00ff);
			opna_->setRegister(0x0c, pitch >> 8);
			if (CommandSequenceUnit::isRatioData(wfSSG_[ch].data)) {
				double hz = opna_->getClock() / 1024.0 / pitch;
				auto ratio = CommandSequenceUnit::data2ratio(wfSSG_[ch].data);
				hz = hz * ratio.second / ratio.first;
				uint16_t freq = static_cast<uint16_t>(opna_->getClock() / 64.0 / hz);
				opna_->setRegister(0x0b, freq & 0x00ff);
				opna_->setRegister(0x0c, freq >> 8);
			}
		}
		else if (needSqMaskFreqSetSSG_[ch]) {
			if (CommandSequenceUnit::isRatioData(wfSSG_[ch].data)) {
				double hz = opna_->getClock() / 1024.0 / pitch;
				auto ratio = CommandSequenceUnit::data2ratio(wfSSG_[ch].data);
				hz = hz * ratio.second / ratio.first;
				uint16_t freq = static_cast<uint16_t>(opna_->getClock() / 64.0 / hz);
				opna_->setRegister(0x0b, freq & 0x00ff);
				opna_->setRegister(0x0c, freq >> 8);
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

//---------- Drum ----------//
/********** Key on-off **********/
void OPNAController::setKeyOnFlagDrum(int ch)
{
	if (isMuteDrum(ch)) return;

	if (tmpVolDrum_[ch] != -1)
		setVolumeDrum(ch, volDrum_[ch]);

	keyOnFlagDrum_ |= static_cast<uint8_t>(1 << ch);
}

void OPNAController::setKeyOffFlagDrum(int ch)
{
	keyOffFlagDrum_ |= static_cast<uint8_t>(1 << ch);
}

void OPNAController::updateKeyOnOffStatusDrum()
{
	if (keyOnFlagDrum_) {
		opna_->setRegister(0x10, keyOnFlagDrum_);
		keyOnFlagDrum_ = 0;
	}
	if (keyOffFlagDrum_) {
		opna_->setRegister(0x10, 0x80 | keyOffFlagDrum_);
		keyOffFlagDrum_ = 0;
	}
}

/********** Set volume **********/
void OPNAController::setVolumeDrum(int ch, int volume)
{
	if (volume > 0x1f) return;	// Out of range

	volDrum_[ch] = volume;
	tmpVolDrum_[ch] = -1;
	opna_->setRegister(0x18 + static_cast<uint32_t>(ch), static_cast<uint8_t>((panDrum_[ch] << 6) | volume));
}

void OPNAController::setMasterVolumeDrum(int volume)
{
	mVolDrum_ = volume;
	opna_->setRegister(0x11, static_cast<uint8_t>(volume));
}

void OPNAController::setTemporaryVolumeDrum(int ch, int volume)
{
	if (volume > 0x1f) return;	// Out of range

	tmpVolDrum_[ch] = volume;
	opna_->setRegister(0x18 + static_cast<uint32_t>(ch), static_cast<uint8_t>((panDrum_[ch] << 6) | volume));
}

/********** Set pan **********/
void OPNAController::setPanDrum(int ch, int value)
{
	panDrum_[ch] = static_cast<uint8_t>(value);
	opna_->setRegister(0x18 + static_cast<uint32_t>(ch), static_cast<uint8_t>((value << 6) | volDrum_[ch]));
}

/********** Mute **********/
void OPNAController::setMuteDrumState(int ch, bool isMute)
{
	isMuteDrum_[ch] = isMute;

	if (isMute) {
		setKeyOffFlagDrum(ch);
		updateKeyOnOffStatusDrum();
	}
}

bool OPNAController::isMuteDrum(int ch)
{
	return isMuteDrum_[ch];
}

/***********************************/
void OPNAController::initDrum()
{
	keyOnFlagDrum_ = 0;
	keyOffFlagDrum_ = 0;
	mVolDrum_ = 0x3f;
	opna_->setRegister(0x11, 0x3f);	// Drum total volume

	for (int ch = 0; ch < 6; ++ch) {
		volDrum_[ch] = 0x1f;	// Init volume
		tmpVolDrum_[ch] = -1;

		// Init pan
		panDrum_[ch] = 3;
		opna_->setRegister(0x18 + static_cast<uint32_t>(ch), 0xdf);
	}
}
