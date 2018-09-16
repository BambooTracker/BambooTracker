#include "opna_controller.hpp"
#include "pitch_converter.hpp"

#include <QDebug>

#ifdef SINC_INTERPOLATION
OPNAController::OPNAController(int clock, int rate, int duration) :
	opna_(clock, rate, duration)
#else
OPNAController::OPNAController(int clock, int rate) :
	opna_(clock, rate)
#endif
{
	initChip();

	for (int i = 0; i < 6; ++i) {
		isMuteFM_[i] = false;
	}

	for (int i = 0; i < 3; ++i) {
		isMuteSSG_[i] = false;
	}
}

/********** Reset and initialize **********/
void OPNAController::reset()
{
	opna_.reset();
	initChip();
}

void OPNAController::initChip()
{
	opna_.setRegister(0x29, 0x80);		// Init interrupt / YM2608 mode
	mixerSSG_ = 0xff;
	opna_.setRegister(0x07, mixerSSG_);	// SSG mix
	opna_.setRegister(0x11, 0x3f);		// Drum total volume

	// FM
	lfoFreq_ = -1;

	for (int ch = 0; ch < 6; ++ch) {
		// Init operators key off
		fmOpEnables_[ch] = 0xf;
		isKeyOnFM_[ch] = false;

		// Init envelope
		envFM_[ch] = std::make_unique<EnvelopeFM>(-1);
		refInstFM_[ch].reset();

		toneFM_[ch].octave = -1;	// Init key on note data
		volFM_[ch] = 0;	// Init volume
		gateCntFM_[ch] = 0;
		enableEnvResetFM_[ch] = true;

		// Init sequence
		hasPreSetTickEventFM_[ch] = false;

		// Init FM pan
		uint32_t bch = getFMChannelOffset(ch);
		panFM_[ch] = 3;
		opna_.setRegister(0xb4 + bch, 0xc0);
	}

	// SSG
	for (int ch = 0; ch < 3; ++ch) {
		isKeyOnSSG_[ch] = false;

		refInstSSG_[ch].reset();	// Init envelope
		toneSSG_[ch].octave = -1;	// Init key on note data
		baseVolSSG_[ch] = 0xf;	// Init volume
		isHardEnvSSG_[ch] = false;
		isBuzzEffSSG_[ch] = false;

		// Init sequence
		hasPreSetTickEventSSG_[ch] = false;
		wfItSSG_[ch].reset();
		wfSSG_[ch] = { -1, -1 };
		envItSSG_[ch].reset();
		envSSG_[ch] = { -1, -1 };

		gateCntSSG_[ch] = 0;
	}
}

/********** Forward instrument sequence **********/
void OPNAController::tickEvent(SoundSource src, int ch)
{
	switch (src) {
	case SoundSource::FM:
		if (hasPreSetTickEventFM_[ch]) {
			hasPreSetTickEventFM_[ch] = false;
		}
		else {
			// UNDONE
		}
		break;

	case SoundSource::SSG:
		if (hasPreSetTickEventSSG_[ch]) {
			hasPreSetTickEventSSG_[ch] = false;
		}
		else {
			if (wfItSSG_[ch]) writeWaveFormSSGToRegister(ch, wfItSSG_[ch]->next());
			if (envItSSG_[ch]) writeEnvelopeSSGToRegister(ch, envItSSG_[ch]->next());
		}
		break;
	}
}

/********** Chip details **********/
int OPNAController::getGateCount(SoundSource src, int ch) const
{
	switch (src) {
	case SoundSource::FM:	return gateCntFM_[ch];
	case SoundSource::SSG:	return gateCntSSG_[ch];
	}
}

/********** Stream samples **********/
void OPNAController::getStreamSamples(int16_t* container, size_t nSamples)
{
	opna_.mix(container, nSamples);
}

/********** Stream details **********/
int OPNAController::getRate() const
{
	return opna_.getRate();
}

int OPNAController::getDuration() const
{
	return 40;	// dummy set
}

//---------- FM ----------//
/********** Key on-off **********/
void OPNAController::keyOnFM(int ch, Note note, int octave, int fine, bool isJam)
{
	if (isMuteFM(ch)) return;

	toneFM_[ch].octave = octave;
	toneFM_[ch].note = note;
	toneFM_[ch].fine = fine;

	setFrontFMSequences(ch);
	hasPreSetTickEventFM_[ch] = isJam;

	uint16_t pitch = PitchConverter::getPitchFM(note, octave, fine);
	uint32_t offset = getFMChannelOffset(ch);
	opna_.setRegister(0xa4 + offset, pitch >> 8);
	opna_.setRegister(0xa0 + offset, pitch & 0x00ff);
	uint32_t chdata = getFmChannelMask(ch);
	opna_.setRegister(0x28, (fmOpEnables_[ch] << 4) | chdata);

	isKeyOnFM_[ch] = true;
}

void OPNAController::keyOffFM(int ch, bool isJam)
{
	releaseStartFMSequences(ch);
	hasPreSetTickEventFM_[ch] = isJam;

	uint8_t chdata = getFmChannelMask(ch);
	opna_.setRegister(0x28, chdata);
	isKeyOnFM_[ch] = false;
}

void OPNAController::resetFMChannelEnvelope(int ch)
{
	keyOffFM(ch);

	// Change register only
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

/********** Set instrument **********/
void OPNAController::setInstrumentFM(int ch, std::shared_ptr<InstrumentFM> inst)
{
	if (inst == nullptr) {	// Error set ()
		if (refInstFM_[ch] != nullptr) {	// When setted instrument has been deleted
			refInstFM_[ch]->setNumber(-1);
		}
		else {
			return;
		}
	}
	else {
		refInstFM_[ch] = inst;
	}

	writeFMEnvelopeToRegistersFromInstrument(ch);
	writeFMLFOAllRegisters(ch);
	setInstrumentFMProperties(ch);

	checkLFOUsed();
}

void OPNAController::updateInstrumentFM(int instNum)
{
	for (int ch = 0; ch < 6; ++ch) {
		if (refInstFM_[ch] != nullptr && refInstFM_[ch]->getNumber() == instNum) {
			writeFMEnvelopeToRegistersFromInstrument(ch);
			writeFMLFOAllRegisters(ch);
			setInstrumentFMProperties(ch);
		}
	}

	checkLFOUsed();
}

void OPNAController::updateInstrumentFMEnvelopeParameter(int envNum, FMEnvelopeParameter param)
{
	for (int ch = 0; ch < 6; ++ch) {
		if (refInstFM_[ch] != nullptr && refInstFM_[ch]->getEnvelopeNumber() == envNum) {
			writeFMEnveropeParameterToRegister(ch, param, refInstFM_[ch]->getEnvelopeParameter(param));
		}
	}
}

void OPNAController::setInstrumentFMOperatorEnabled(int envNum, int opNum)
{
	for (int ch = 0; ch < 6; ++ch) {
		if (refInstFM_[ch] != nullptr && refInstFM_[ch]->getEnvelopeNumber() == envNum) {
			bool enabled = refInstFM_[ch]->getOperatorEnabled(opNum);
			envFM_[ch]->setOperatorEnabled(opNum, enabled);
			if (enabled) {
				fmOpEnables_[ch] |= (1 << opNum);
			}
			else {
				fmOpEnables_[ch] &= ~(1 << opNum);
			}
			if (isKeyOnFM_[ch]) {
				uint32_t mask = getFmChannelMask(ch);
				opna_.setRegister(0x28, (fmOpEnables_[ch] << 4) | mask);
			}
		}
	}
}

void OPNAController::updateInstrumentFMLFOParameter(int lfoNum, FMLFOParamter param)
{
	for (int ch = 0; ch < 6; ++ch) {
		if (refInstFM_[ch] != nullptr && refInstFM_[ch]->getLFONumber() == lfoNum) {
			writeFMLFORegister(ch, param);
		}
	}
}

/********** Set volume **********/
void OPNAController::setVolumeFM(int ch, int volume)
{
	volFM_[ch] = volume;

	if (isKeyOnFM(ch) && refInstFM_[ch] != nullptr) {	// Change TL
		writeFMEnveropeParameterToRegister(ch, FMEnvelopeParameter::TL1,
										   refInstFM_[ch]->getEnvelopeParameter(FMEnvelopeParameter::TL1));
		writeFMEnveropeParameterToRegister(ch, FMEnvelopeParameter::TL2,
										   refInstFM_[ch]->getEnvelopeParameter(FMEnvelopeParameter::TL2));
		writeFMEnveropeParameterToRegister(ch, FMEnvelopeParameter::TL3,
										   refInstFM_[ch]->getEnvelopeParameter(FMEnvelopeParameter::TL3));
		writeFMEnveropeParameterToRegister(ch, FMEnvelopeParameter::TL4,
										   refInstFM_[ch]->getEnvelopeParameter(FMEnvelopeParameter::TL4));
	}
}

/********** Mute **********/
void OPNAController::setMuteFMState(int ch, bool isMute)
{
	isMuteFM_[ch] = isMute;

	if (isMute) resetFMChannelEnvelope(ch);
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

bool OPNAController::enableFMEnvelopeReset(int ch) const
{
	return (envFM_[ch] == nullptr)
			? true
			: enableEnvResetFM_[ch];
}

ToneDetail OPNAController::getFMTone(int ch) const
{
	return toneFM_[ch];
}

/***********************************/
uint32_t OPNAController::getFmChannelMask(int ch)
{
	// UNDONE: change channel type by Effect mode
	switch (ch) {
	case 0: return 0x00;
	case 1: return 0x01;
	case 2: return 0x02;
	case 3: return 0x04;
	case 4: return 0x05;
	case 5: return 0x06;
	default: return 0;
	}
}

uint32_t OPNAController::getFMChannelOffset(int ch)
{
	switch (ch) {
	case 0:
	case 1:
	case 2:
		return ch;
	case 3:
	case 4:
	case 5:
		return 0x100 + ch % 3;
	default:
		return 0;
	}
}

void OPNAController::writeFMEnvelopeToRegistersFromInstrument(int ch)
{
	uint32_t bch = getFMChannelOffset(ch);	// Bank and channel offset
	uint8_t data1, data2;
	int al;

	data1 = refInstFM_[ch]->getEnvelopeParameter(FMEnvelopeParameter::FB);
	envFM_[ch]->setParameterValue(FMEnvelopeParameter::FB, data1);
	data1 <<= 3;
	al = refInstFM_[ch]->getEnvelopeParameter(FMEnvelopeParameter::AL);
	envFM_[ch]->setParameterValue(FMEnvelopeParameter::AL, al);
	data1 += al;
	opna_.setRegister(0xb0 + bch, data1);

	uint32_t offset = bch;	// Operator 1

	data1 = refInstFM_[ch]->getEnvelopeParameter(FMEnvelopeParameter::DT1);
	envFM_[ch]->setParameterValue(FMEnvelopeParameter::DT1, data1);
	data1 <<= 4;
	data2 = refInstFM_[ch]->getEnvelopeParameter(FMEnvelopeParameter::ML1);
	envFM_[ch]->setParameterValue(FMEnvelopeParameter::ML1, data2);
	data1 |= data2;
	opna_.setRegister(0x30 + offset, data1);

	data1 = refInstFM_[ch]->getEnvelopeParameter(FMEnvelopeParameter::TL1);
	if (isCareer(0, al)) data1 = calculateTL(ch, data1);	// Adjust volume
	envFM_[ch]->setParameterValue(FMEnvelopeParameter::TL1, data1);
	opna_.setRegister(0x40 + offset, data1);

	data1 = refInstFM_[ch]->getEnvelopeParameter(FMEnvelopeParameter::KS1);
	envFM_[ch]->setParameterValue(FMEnvelopeParameter::KS1, data1);
	data1 <<= 6;
	data2 = refInstFM_[ch]->getEnvelopeParameter(FMEnvelopeParameter::AR1);
	envFM_[ch]->setParameterValue(FMEnvelopeParameter::AR1, data2);
	data1 |= data2;
	opna_.setRegister(0x50 + offset, data1);

	data1 = (refInstFM_[ch]->getLFONumber() == -1) ? 0 : refInstFM_[ch]->getLFOParameter(FMLFOParamter::AM1);
	data1 <<= 7;
	data2 = refInstFM_[ch]->getEnvelopeParameter(FMEnvelopeParameter::DR1);
	envFM_[ch]->setParameterValue(FMEnvelopeParameter::DR1, data2);
	data1 |= data2;
	opna_.setRegister(0x60 + offset, data1);

	data1 = refInstFM_[ch]->getEnvelopeParameter(FMEnvelopeParameter::SR1);
	envFM_[ch]->setParameterValue(FMEnvelopeParameter::SR1, data2);
	opna_.setRegister(0x70 + offset, data1);

	data1 = refInstFM_[ch]->getEnvelopeParameter(FMEnvelopeParameter::SL1);
	envFM_[ch]->setParameterValue(FMEnvelopeParameter::SL1, data1);
	data1 <<= 4;
	data2 = refInstFM_[ch]->getEnvelopeParameter(FMEnvelopeParameter::RR1);
	envFM_[ch]->setParameterValue(FMEnvelopeParameter::RR1, data2);
	data1 |= data2;
	opna_.setRegister(0x80 + offset, data1);

	int tmp = refInstFM_[ch]->getEnvelopeParameter(FMEnvelopeParameter::SSGEG1);
	envFM_[ch]->setParameterValue(FMEnvelopeParameter::SSGEG1, tmp);
	data1 = judgeSSEGRegisterValue(tmp);
	opna_.setRegister(0x90 + offset, data1);

	offset = bch + 8;	// Operator 2

	data1 = refInstFM_[ch]->getEnvelopeParameter(FMEnvelopeParameter::DT2);
	envFM_[ch]->setParameterValue(FMEnvelopeParameter::DT2, data1);
	data1 <<= 4;
	data2 = refInstFM_[ch]->getEnvelopeParameter(FMEnvelopeParameter::ML2);
	envFM_[ch]->setParameterValue(FMEnvelopeParameter::ML2, data2);
	data1 |= data2;
	opna_.setRegister(0x30 + offset, data1);

	data1 = refInstFM_[ch]->getEnvelopeParameter(FMEnvelopeParameter::TL2);
	if (isCareer(1, al)) data1 = calculateTL(ch, data1);	// Adjust volume
	envFM_[ch]->setParameterValue(FMEnvelopeParameter::TL2, data1);
	opna_.setRegister(0x40 + offset, data1);

	data1 = refInstFM_[ch]->getEnvelopeParameter(FMEnvelopeParameter::KS2);
	envFM_[ch]->setParameterValue(FMEnvelopeParameter::KS2, data1);
	data1 <<= 6;
	data2 = refInstFM_[ch]->getEnvelopeParameter(FMEnvelopeParameter::AR2);
	envFM_[ch]->setParameterValue(FMEnvelopeParameter::AR2, data2);
	data1 |= data2;
	opna_.setRegister(0x50 + offset, data1);

	data1 = (refInstFM_[ch]->getLFONumber() == -1) ? 0 : refInstFM_[ch]->getLFOParameter(FMLFOParamter::AM2);
	data1 <<= 7;
	data2 = refInstFM_[ch]->getEnvelopeParameter(FMEnvelopeParameter::DR2);
	envFM_[ch]->setParameterValue(FMEnvelopeParameter::DR2, data2);
	data1 |= data2;
	opna_.setRegister(0x60 + offset, data1);

	data1 = refInstFM_[ch]->getEnvelopeParameter(FMEnvelopeParameter::SR2);
	envFM_[ch]->setParameterValue(FMEnvelopeParameter::SR2, data2);
	opna_.setRegister(0x70 + offset, data1);

	data1 = refInstFM_[ch]->getEnvelopeParameter(FMEnvelopeParameter::SL2);
	envFM_[ch]->setParameterValue(FMEnvelopeParameter::SL2, data1);
	data1 <<= 4;
	data2 = refInstFM_[ch]->getEnvelopeParameter(FMEnvelopeParameter::RR2);
	envFM_[ch]->setParameterValue(FMEnvelopeParameter::RR2, data2);
	data1 |= data2;
	opna_.setRegister(0x80 + offset, data1);

	tmp = refInstFM_[ch]->getEnvelopeParameter(FMEnvelopeParameter::SSGEG2);
	envFM_[ch]->setParameterValue(FMEnvelopeParameter::SSGEG2, tmp);
	data1 = judgeSSEGRegisterValue(tmp);
	opna_.setRegister(0x90 + offset, data1);

	offset = bch + 4;	// Operator 3

	data1 = refInstFM_[ch]->getEnvelopeParameter(FMEnvelopeParameter::DT3);
	envFM_[ch]->setParameterValue(FMEnvelopeParameter::DT3, data1);
	data1 <<= 4;
	data2 = refInstFM_[ch]->getEnvelopeParameter(FMEnvelopeParameter::ML3);
	envFM_[ch]->setParameterValue(FMEnvelopeParameter::ML3, data2);
	data1 |= data2;
	opna_.setRegister(0x30 + offset, data1);

	data1 = refInstFM_[ch]->getEnvelopeParameter(FMEnvelopeParameter::TL3);
	if (isCareer(3, al)) data1 = calculateTL(ch, data1);	// Adjust volume
	envFM_[ch]->setParameterValue(FMEnvelopeParameter::TL3, data1);
	opna_.setRegister(0x40 + offset, data1);

	data1 = refInstFM_[ch]->getEnvelopeParameter(FMEnvelopeParameter::KS3);
	envFM_[ch]->setParameterValue(FMEnvelopeParameter::KS3, data1);
	data1 <<= 6;
	data2 = refInstFM_[ch]->getEnvelopeParameter(FMEnvelopeParameter::AR3);
	envFM_[ch]->setParameterValue(FMEnvelopeParameter::AR3, data2);
	data1 |= data2;
	opna_.setRegister(0x50 + offset, data1);

	data1 = (refInstFM_[ch]->getLFONumber() == -1) ? 0 : refInstFM_[ch]->getLFOParameter(FMLFOParamter::AM3);
	data1 <<= 7;
	data2 = refInstFM_[ch]->getEnvelopeParameter(FMEnvelopeParameter::DR3);
	envFM_[ch]->setParameterValue(FMEnvelopeParameter::DR3, data2);
	data1 |= data2;
	opna_.setRegister(0x60 + offset, data1);

	data1 = refInstFM_[ch]->getEnvelopeParameter(FMEnvelopeParameter::SR3);
	envFM_[ch]->setParameterValue(FMEnvelopeParameter::SR3, data2);
	opna_.setRegister(0x70 + offset, data1);

	data1 = refInstFM_[ch]->getEnvelopeParameter(FMEnvelopeParameter::SL3);
	envFM_[ch]->setParameterValue(FMEnvelopeParameter::SL3, data1);
	data1 <<= 4;
	data2 = refInstFM_[ch]->getEnvelopeParameter(FMEnvelopeParameter::RR3);
	envFM_[ch]->setParameterValue(FMEnvelopeParameter::RR3, data2);
	data1 |= data2;
	opna_.setRegister(0x80 + offset, data1);

	tmp = refInstFM_[ch]->getEnvelopeParameter(FMEnvelopeParameter::SSGEG3);
	envFM_[ch]->setParameterValue(FMEnvelopeParameter::SSGEG3, tmp);
	data1 = judgeSSEGRegisterValue(tmp);
	opna_.setRegister(0x90 + offset, data1);

	offset = bch + 12;	// Operator 4

	data1 = refInstFM_[ch]->getEnvelopeParameter(FMEnvelopeParameter::DT4);
	envFM_[ch]->setParameterValue(FMEnvelopeParameter::DT4, data1);
	data1 <<= 4;
	data2 = refInstFM_[ch]->getEnvelopeParameter(FMEnvelopeParameter::ML4);
	envFM_[ch]->setParameterValue(FMEnvelopeParameter::ML4, data2);
	data1 |= data2;
	opna_.setRegister(0x30 + offset, data1);

	data1 = refInstFM_[ch]->getEnvelopeParameter(FMEnvelopeParameter::TL4);
	data1 = calculateTL(ch, data1);	// Adjust volume
	envFM_[ch]->setParameterValue(FMEnvelopeParameter::TL4, data1);
	opna_.setRegister(0x40 + offset, data1);

	data1 = refInstFM_[ch]->getEnvelopeParameter(FMEnvelopeParameter::KS4);
	envFM_[ch]->setParameterValue(FMEnvelopeParameter::KS4, data1);
	data1 <<= 6;
	data2 = refInstFM_[ch]->getEnvelopeParameter(FMEnvelopeParameter::AR4);
	envFM_[ch]->setParameterValue(FMEnvelopeParameter::AR4, data2);
	data1 |= data2;
	opna_.setRegister(0x50 + offset, data1);

	data1 = (refInstFM_[ch]->getLFONumber() == -1) ? 0 : refInstFM_[ch]->getLFOParameter(FMLFOParamter::AM4);
	data1 <<= 7;
	data2 = refInstFM_[ch]->getEnvelopeParameter(FMEnvelopeParameter::DR4);
	envFM_[ch]->setParameterValue(FMEnvelopeParameter::DR4, data2);
	data1 |= data2;
	opna_.setRegister(0x60 + offset, data1);

	data1 = refInstFM_[ch]->getEnvelopeParameter(FMEnvelopeParameter::SR4);
	envFM_[ch]->setParameterValue(FMEnvelopeParameter::SR4, data2);
	opna_.setRegister(0x70 + offset, data1);

	data1 = refInstFM_[ch]->getEnvelopeParameter(FMEnvelopeParameter::SL4);
	envFM_[ch]->setParameterValue(FMEnvelopeParameter::SL4, data1);
	data1 <<= 4;
	data2 = refInstFM_[ch]->getEnvelopeParameter(FMEnvelopeParameter::RR4);
	envFM_[ch]->setParameterValue(FMEnvelopeParameter::RR4, data2);
	data1 |= data2;
	opna_.setRegister(0x80 + offset, data1);

	tmp = refInstFM_[ch]->getEnvelopeParameter(FMEnvelopeParameter::SSGEG4);
	envFM_[ch]->setParameterValue(FMEnvelopeParameter::SSGEG4, tmp);
	data1 = judgeSSEGRegisterValue(tmp);
	opna_.setRegister(0x90 + offset, data1);
}

void OPNAController::writeFMEnveropeParameterToRegister(int ch, FMEnvelopeParameter param, int value)
{
	uint32_t bch = getFMChannelOffset(ch);	// Bank and channel offset
	uint8_t data;
	int tmp;

	envFM_[ch]->setParameterValue(param, value);

	switch (param) {
	case FMEnvelopeParameter::AL:
	case FMEnvelopeParameter::FB:
		data = envFM_[ch]->getParameterValue(FMEnvelopeParameter::FB) << 3;
		data += envFM_[ch]->getParameterValue(FMEnvelopeParameter::AL);
		opna_.setRegister(0xb0 + bch, data);
		break;
	case FMEnvelopeParameter::DT1:
	case FMEnvelopeParameter::ML1:
		data = envFM_[ch]->getParameterValue(FMEnvelopeParameter::DT1) << 4;
		data |= envFM_[ch]->getParameterValue(FMEnvelopeParameter::ML1);
		opna_.setRegister(0x30 + bch, data);
		break;
	case FMEnvelopeParameter::TL1:
		data = envFM_[ch]->getParameterValue(FMEnvelopeParameter::TL1);
		// Adjust volume
		if (isCareer(0, envFM_[ch]->getParameterValue(FMEnvelopeParameter::AL))) {
			data = calculateTL(ch, data);
			envFM_[ch]->setParameterValue(param, data);	// Update
		}
		opna_.setRegister(0x40 + bch, data);
		break;
	case FMEnvelopeParameter::KS1:
	case FMEnvelopeParameter::AR1:
		data = envFM_[ch]->getParameterValue(FMEnvelopeParameter::KS1) << 6;
		data |= envFM_[ch]->getParameterValue(FMEnvelopeParameter::AR1);
		opna_.setRegister(0x50 + bch, data);
		break;
	case FMEnvelopeParameter::DR1:
		data = (refInstFM_[ch]->getLFONumber() == -1) ? 0 : refInstFM_[ch]->getLFOParameter(FMLFOParamter::AM1);
		data <<= 7;
		data |= envFM_[ch]->getParameterValue(FMEnvelopeParameter::DR1);
		opna_.setRegister(0x60 + bch, data);
		break;
	case FMEnvelopeParameter::SR1:
		data = envFM_[ch]->getParameterValue(FMEnvelopeParameter::SR1);
		opna_.setRegister(0x70 + bch, data);
		break;
	case FMEnvelopeParameter::SL1:
	case FMEnvelopeParameter::RR1:
		data = envFM_[ch]->getParameterValue(FMEnvelopeParameter::SL1) << 4;
		data |= envFM_[ch]->getParameterValue(FMEnvelopeParameter::RR1);
		opna_.setRegister(0x80 + bch, data);
		break;
	case::FMEnvelopeParameter::SSGEG1:
		tmp = envFM_[ch]->getParameterValue(FMEnvelopeParameter::SSGEG1);
		data = (tmp == -1) ? 0 : (0x08 + tmp);
		opna_.setRegister(0x90 + bch, data);
		break;
	case FMEnvelopeParameter::DT2:
	case FMEnvelopeParameter::ML2:
		data = envFM_[ch]->getParameterValue(FMEnvelopeParameter::DT2) << 4;
		data |= envFM_[ch]->getParameterValue(FMEnvelopeParameter::ML2);
		opna_.setRegister(0x30 + bch + 8, data);
		break;
	case FMEnvelopeParameter::TL2:
		data = envFM_[ch]->getParameterValue(FMEnvelopeParameter::TL2);
		// Adjust volume
		if (isCareer(1, envFM_[ch]->getParameterValue(FMEnvelopeParameter::AL))) {
			data = calculateTL(ch, data);
			envFM_[ch]->setParameterValue(param, data);	// Update
		}
		opna_.setRegister(0x40 + bch + 8, data);
		break;
	case FMEnvelopeParameter::KS2:
	case FMEnvelopeParameter::AR2:
		data = envFM_[ch]->getParameterValue(FMEnvelopeParameter::KS2) << 6;
		data |= envFM_[ch]->getParameterValue(FMEnvelopeParameter::AR2);
		opna_.setRegister(0x50 + bch + 8, data);
		break;
	case FMEnvelopeParameter::DR2:
		data = (refInstFM_[ch]->getLFONumber() == -1) ? 0 : refInstFM_[ch]->getLFOParameter(FMLFOParamter::AM2);
		data <<= 7;
		data |= envFM_[ch]->getParameterValue(FMEnvelopeParameter::DR2);
		opna_.setRegister(0x60 + bch + 8, data);
		break;
	case FMEnvelopeParameter::SR2:
		data = envFM_[ch]->getParameterValue(FMEnvelopeParameter::SR2);
		opna_.setRegister(0x70 + bch + 8, data);
		break;
	case FMEnvelopeParameter::SL2:
	case FMEnvelopeParameter::RR2:
		data = envFM_[ch]->getParameterValue(FMEnvelopeParameter::SL2) << 4;
		data |= envFM_[ch]->getParameterValue(FMEnvelopeParameter::RR2);
		opna_.setRegister(0x80 + bch + 8, data);
		break;
	case FMEnvelopeParameter::SSGEG2:
		tmp = envFM_[ch]->getParameterValue(FMEnvelopeParameter::SSGEG2);
		data = (tmp == -1) ? 0 : (0x08 + tmp);
		opna_.setRegister(0x90 + bch + 8, data);
		break;
	case FMEnvelopeParameter::DT3:
	case FMEnvelopeParameter::ML3:
		data = envFM_[ch]->getParameterValue(FMEnvelopeParameter::DT3) << 4;
		data |= envFM_[ch]->getParameterValue(FMEnvelopeParameter::ML3);
		opna_.setRegister(0x30 + bch + 4, data);
		break;
	case FMEnvelopeParameter::TL3:
		data = envFM_[ch]->getParameterValue(FMEnvelopeParameter::TL3);
		// Adjust volume
		if (isCareer(2, envFM_[ch]->getParameterValue(FMEnvelopeParameter::AL))) {
			data = calculateTL(ch, data);
			envFM_[ch]->setParameterValue(param, data);	// Update
		}
		opna_.setRegister(0x40 + bch + 4, data);
		break;
	case FMEnvelopeParameter::KS3:
	case FMEnvelopeParameter::AR3:
		data = envFM_[ch]->getParameterValue(FMEnvelopeParameter::KS3) << 6;
		data |= envFM_[ch]->getParameterValue(FMEnvelopeParameter::AR3);
		opna_.setRegister(0x50 + bch + 4, data);
		break;
	case FMEnvelopeParameter::DR3:
		data = (refInstFM_[ch]->getLFONumber() == -1) ? 0 : refInstFM_[ch]->getLFOParameter(FMLFOParamter::AM3);
		data <<= 7;
		data |= envFM_[ch]->getParameterValue(FMEnvelopeParameter::DR3);
		opna_.setRegister(0x60 + bch + 4, data);
		break;
	case FMEnvelopeParameter::SR3:
		data = envFM_[ch]->getParameterValue(FMEnvelopeParameter::SR3);
		opna_.setRegister(0x70 + bch + 4, data);
		break;
	case FMEnvelopeParameter::SL3:
	case FMEnvelopeParameter::RR3:
		data = envFM_[ch]->getParameterValue(FMEnvelopeParameter::SL3) << 4;
		data |= envFM_[ch]->getParameterValue(FMEnvelopeParameter::RR3);
		opna_.setRegister(0x80 + bch + 4, data);
		break;
	case FMEnvelopeParameter::SSGEG3:
		tmp = envFM_[ch]->getParameterValue(FMEnvelopeParameter::SSGEG3);
		data = (tmp == -1) ? 0 : (0x08 + tmp);
		opna_.setRegister(0x90 + bch + 4, data);
		break;
	case FMEnvelopeParameter::DT4:
	case FMEnvelopeParameter::ML4:
		data = envFM_[ch]->getParameterValue(FMEnvelopeParameter::DT4) << 4;
		data |= envFM_[ch]->getParameterValue(FMEnvelopeParameter::ML4);
		opna_.setRegister(0x30 + bch + 12, data);
		break;
	case FMEnvelopeParameter::TL4:
		data = envFM_[ch]->getParameterValue(FMEnvelopeParameter::TL4);
		// Adjust volume
		data = calculateTL(ch, data);
		envFM_[ch]->setParameterValue(param, data);	// Update
		opna_.setRegister(0x40 + bch + 12, data);
		break;
	case FMEnvelopeParameter::KS4:
	case FMEnvelopeParameter::AR4:
		data = envFM_[ch]->getParameterValue(FMEnvelopeParameter::KS4) << 6;
		data |= envFM_[ch]->getParameterValue(FMEnvelopeParameter::AR4);
		opna_.setRegister(0x50 + bch + 12, data);
		break;
	case FMEnvelopeParameter::DR4:
		data = (refInstFM_[ch]->getLFONumber() == -1) ? 0 : refInstFM_[ch]->getLFOParameter(FMLFOParamter::AM4);
		data <<= 7;
		data |= envFM_[ch]->getParameterValue(FMEnvelopeParameter::DR4);
		opna_.setRegister(0x60 + bch + 12, data);
		break;
	case FMEnvelopeParameter::SR4:
		data = envFM_[ch]->getParameterValue(FMEnvelopeParameter::SR4);
		opna_.setRegister(0x70 + bch + 12, data);
		break;
	case FMEnvelopeParameter::SL4:
	case FMEnvelopeParameter::RR4:
		data = envFM_[ch]->getParameterValue(FMEnvelopeParameter::SL4) << 4;
		data |= envFM_[ch]->getParameterValue(FMEnvelopeParameter::RR4);
		opna_.setRegister(0x80 + bch + 12, data);
		break;
	case FMEnvelopeParameter::SSGEG4:
		tmp = envFM_[ch]->getParameterValue(FMEnvelopeParameter::SSGEG4);
		data = judgeSSEGRegisterValue(tmp);
		opna_.setRegister(0x90 + bch + 12, data);
		break;
	}
}

void OPNAController::writeFMLFOAllRegisters(int ch)
{
	if (refInstFM_[ch]->getLFONumber() == -1) {	// Clear data
		uint32_t bch = getFMChannelOffset(ch);	// Bank and channel offset
		opna_.setRegister(0xb4 + bch, panFM_[ch] << 6);
		opna_.setRegister(0x60 + bch, envFM_[ch]->getParameterValue(FMEnvelopeParameter::DR1));
		opna_.setRegister(0x60 + bch + 8, envFM_[ch]->getParameterValue(FMEnvelopeParameter::DR2));
		opna_.setRegister(0x60 + bch + 4, envFM_[ch]->getParameterValue(FMEnvelopeParameter::DR3));
		opna_.setRegister(0x60 + bch + 12, envFM_[ch]->getParameterValue(FMEnvelopeParameter::DR4));
	}
	else {
		writeFMLFORegister(ch, FMLFOParamter::FREQ);
		writeFMLFORegister(ch, FMLFOParamter::PMS);
		writeFMLFORegister(ch, FMLFOParamter::AMS);
		writeFMLFORegister(ch, FMLFOParamter::AM1);
		writeFMLFORegister(ch, FMLFOParamter::AM2);
		writeFMLFORegister(ch, FMLFOParamter::AM3);
		writeFMLFORegister(ch, FMLFOParamter::AM4);
	}
}

void OPNAController::writeFMLFORegister(int ch, FMLFOParamter param)
{
	uint32_t bch = getFMChannelOffset(ch);	// Bank and channel offset
	uint8_t data;

	switch (param) {
	case FMLFOParamter::FREQ:
		lfoFreq_ = refInstFM_[ch]->getLFOParameter(FMLFOParamter::FREQ);
		opna_.setRegister(0x22, lfoFreq_ | (1 << 3));
		break;
	case FMLFOParamter::PMS:
	case FMLFOParamter::AMS:
		data = panFM_[ch] << 6;
		data |= (refInstFM_[ch]->getLFOParameter(FMLFOParamter::AMS) << 4);
		data |= refInstFM_[ch]->getLFOParameter(FMLFOParamter::PMS);
		opna_.setRegister(0xb4 + bch, data);
		break;
	case FMLFOParamter::AM1:
		data = refInstFM_[ch]->getLFOParameter(FMLFOParamter::AM1) << 7;
		data |= envFM_[ch]->getParameterValue(FMEnvelopeParameter::DR1);
		opna_.setRegister(0x60 + bch, data);
		break;
	case FMLFOParamter::AM2:
		data = refInstFM_[ch]->getLFOParameter(FMLFOParamter::AM2) << 7;
		data |= envFM_[ch]->getParameterValue(FMEnvelopeParameter::DR2);
		opna_.setRegister(0x60 + bch + 8, data);
		break;
	case FMLFOParamter::AM3:
		data = refInstFM_[ch]->getLFOParameter(FMLFOParamter::AM3) << 7;
		data |= envFM_[ch]->getParameterValue(FMEnvelopeParameter::DR3);
		opna_.setRegister(0x60 + bch + 4, data);
		break;
	case FMLFOParamter::AM4:
		data = refInstFM_[ch]->getLFOParameter(FMLFOParamter::AM4) << 7;
		data |= envFM_[ch]->getParameterValue(FMEnvelopeParameter::DR4);
		opna_.setRegister(0x60 + bch + 12, data);
		break;
	}
}

void OPNAController::checkLFOUsed()
{
	for (int i = 0; i < 6; ++i) {
		if (refInstFM_[i] != nullptr &&refInstFM_[i]->getLFONumber() != -1) return;
	}

	if (lfoFreq_ != -1) {
		lfoFreq_ = -1;
		opna_.setRegister(0x22, 0);	// LFO off
	}
}

void OPNAController::setFrontFMSequences(int ch)
{
	// UNDONE
}

void OPNAController::releaseStartFMSequences(int ch)
{
	// UNDONE
}

void OPNAController::setInstrumentFMProperties(int ch)
{
	gateCntFM_[ch] = refInstFM_[ch]->getGateCount();
	enableEnvResetFM_[ch] = refInstFM_[ch]->getEnvelopeResetEnabled();
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
void OPNAController::keyOnSSG(int ch, Note note, int octave, int fine, bool isJam)
{
	if (isMuteSSG(ch)) return;

	toneSSG_[ch].octave = octave;
	toneSSG_[ch].note = note;
	toneSSG_[ch].fine = fine;

	setFrontSSGSequences(ch);

	hasPreSetTickEventSSG_[ch] = isJam;
	isKeyOnSSG_[ch] = true;
}

void OPNAController::keyOffSSG(int ch, bool isJam)
{
	releaseStartSSGSequences(ch);
	hasPreSetTickEventSSG_[ch] = isJam;
	isKeyOnSSG_[ch] = false;
}

/********** Set instrument **********/
void OPNAController::setInstrumentSSG(int ch, std::shared_ptr<InstrumentSSG> inst)
{
	if (inst == nullptr) {	// Error set ()
		if (refInstSSG_[ch] != nullptr) {
			refInstSSG_[ch]->setNumber(-1);
		}
		else {
			return;
		}
	}
	else {
		refInstSSG_[ch] = inst;
	}

	if (refInstSSG_[ch]->getWaveFormNumber() != -1)
		wfItSSG_[ch] = refInstSSG_[ch]->getWaveFormSequenceIterator();
	if (refInstSSG_[ch]->getEnvelopeNumber() != -1)
		envItSSG_[ch] = refInstSSG_[ch]->getEnvelopeSequenceIterator();
	setInstrumentSSGProperties(ch);
}

void OPNAController::updateInstrumentSSG(int instNum)
{
	for (int ch = 0; ch < 3; ++ch) {
		if (refInstSSG_[ch] != nullptr && refInstSSG_[ch]->getNumber() == instNum) {
			checkWaveFormSSGNumber(ch);
			checkEnvelopeSSGNumber(ch);
			setInstrumentSSGProperties(ch);
		}
	}
}

/********** Set volume **********/
void OPNAController::setVolumeSSG(int ch, int volume)
{
	if (volume > 0xf) return;	// Out of range

	baseVolSSG_[ch] = volume;

	if (isKeyOnSSG(ch)) setRealVolumeSSG(ch);
}

void OPNAController::setRealVolumeSSG(int ch)
{
	if (isBuzzEffSSG_[ch] || isHardEnvSSG_[ch]) return;

	int volume = baseVolSSG_[ch];
	if (envItSSG_[ch]) {
		int type = envItSSG_[ch]->getCommandType();
		if (0 <= type && type < 16) {
			volume = volume - (15 - type);
			if (volume < 0) volume = 0;
		}
	}
	opna_.setRegister(0x08 + ch, volume);
}

/********** Mute **********/
void OPNAController::setMuteSSGState(int ch, bool isMute)
{
	isMuteSSG_[ch] = isMute;

	if (isMute) keyOffSSG(ch);
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

ToneDetail OPNAController::getSSGTone(int ch) const
{
	return toneSSG_[ch];
}

/***********************************/
void OPNAController::setFrontSSGSequences(int ch)
{
	if (wfItSSG_[ch]) {
		writeWaveFormSSGToRegister(ch, wfItSSG_[ch]->front());
	}
	else {
		isBuzzEffSSG_[ch] = false;
		if (wfSSG_[ch].type != 0 || !isKeyOnSSG_[ch])
			writeSquareWaveForm(ch);
	}

	if (envItSSG_[ch]) {
		writeEnvelopeSSGToRegister(ch, envItSSG_[ch]->front());
	}
	else {
		setRealVolumeSSG(ch);
	}
}

void OPNAController::releaseStartSSGSequences(int ch)
{
	if (wfItSSG_[ch]) writeWaveFormSSGToRegister(ch, wfItSSG_[ch]->next(true));

	if (envItSSG_[ch]) {
		int pos = envItSSG_[ch]->next(true);
		if (pos == -1) {
			opna_.setRegister(0x08 + ch, 0);
			isHardEnvSSG_[ch] = false;
		}
		else writeEnvelopeSSGToRegister(ch, pos);
	}
	else {
		if (!hasPreSetTickEventSSG_[ch]) {
			opna_.setRegister(0x08 + ch, 0);
			isHardEnvSSG_[ch] = false;
		}
	}
}

void OPNAController::checkWaveFormSSGNumber(int ch)
{
	if (refInstSSG_[ch]->getWaveFormNumber() == -1)
		wfItSSG_[ch].reset();
}

void OPNAController::writeWaveFormSSGToRegister(int ch, int seqPos)
{
	if (seqPos == -1) return;

	switch (wfItSSG_[ch]->getCommandType()) {
	case 0:	// Square
	{
		if (wfSSG_[ch].type == 0) return;
		isBuzzEffSSG_[ch] = false;
		writeSquareWaveForm(ch);
		return;
	}
	case 1:	// Triangle
	{
//		if (wfSSG_[ch].type == 1) return;

//		if (wfSSG_[ch].type != 2) {
//			isHardEnvSSG_[ch] = true;
//			mixerSSG_ |= (0x1 << ch);
//			opna_.setRegister(0x07, mixerSSG_);
//		}
//		opna_.setRegister(0x0d, 0x06);
//		wfSSG_[ch] = { 1, -1 };
		return;
	}
	case 2:	// Saw
	{
//		if (wfSSG_[ch].type == 2) return;

//		if (wfSSG_[ch].type != 1) {
//			mixerSSG_ |= (0x1 << ch);
//			opna_.setRegister(0x07, mixerSSG_);
//		}
//		wfSSG_[ch] = { 2, -1 };
		return;
	}
	case 3:	// Triangle with square
	{
//		int data = wfItSSG_[ch]->getCommandData();
//		if (wfSSG_[ch].type == 3 && wfSSG_[ch].data == data) return;


//		wfSSG_[ch] = { 3, data };
		return;
	}
	case 4:	// Saw with square
	{
//		int data = wfItSSG_[ch]->getCommandData();
//		if (wfSSG_[ch].type == 4 && wfSSG_[ch].data == data) return;


//		wfSSG_[ch] = { 4, data };
		return;
	}
	}
}

void OPNAController::writeSquareWaveForm(int ch)
{
	uint16_t pitch = PitchConverter::getPitchSSG(toneSSG_[ch].note,
												 toneSSG_[ch].octave,
												 toneSSG_[ch].fine);
	uint8_t offset = ch << 1;
	opna_.setRegister(0x00 + offset, pitch & 0xff);
	opna_.setRegister(0x01 + offset, pitch >> 8);
	uint8_t mask = ~(1 << ch);
	mixerSSG_ &= mask;
	opna_.setRegister(0x07, mixerSSG_);
	wfSSG_[ch] = { 0, -1 };
}

void OPNAController::checkEnvelopeSSGNumber(int ch)
{
	if (refInstSSG_[ch]->getEnvelopeNumber() == -1)
		envItSSG_[ch].reset();
}

void OPNAController::writeEnvelopeSSGToRegister(int ch, int seqPos)
{
	if (seqPos == -1 || isBuzzEffSSG_[ch]) return;

	int type = envItSSG_[ch]->getCommandType();
	if (type < 16) {	// Software envelope
		isHardEnvSSG_[ch] = false;
		envSSG_[ch] = { type, -1 };
		setRealVolumeSSG(ch);
	}
	else {	// Hardware envelope
		unsigned int data = envItSSG_[ch]->getCommandData();
		if (envSSG_[ch].data != data) {
			opna_.setRegister(0x0b, 0x00ff & data);
			opna_.setRegister(0x0c, data >> 8);
			envSSG_[ch].data = data;
		}
		if (envSSG_[ch].type != type) {
			opna_.setRegister(0x0d, type - 16 + 8);
			envSSG_[ch].type = type;
		}
		if (!isHardEnvSSG_[ch]) {
			opna_.setRegister(0x08 + ch, 0x10);
			isHardEnvSSG_[ch] = true;
		}
	}
}

void OPNAController::setInstrumentSSGProperties(int ch)
{
	gateCntSSG_[ch] = refInstSSG_[ch]->getGateCount();
}
