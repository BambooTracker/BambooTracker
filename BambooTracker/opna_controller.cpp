#include "opna_controller.hpp"
#include "pitch_converter.hpp"

#ifdef SINC_INTERPOLATION
OPNAController::OPNAController(int clock, int rate, int duration) :
	opna_(clock, rate, duration)
#else
OPNAController::OPNAController(int clock, int rate) :
	opna_(clock, rate)
#endif
{
	initChip();
}

/********** Reset and initialize **********/
void OPNAController::reset()
{
	opna_.reset();
	initChip();
}

/********** Key on-off **********/
void OPNAController::keyOnFM(int ch, Note note, int octave, int fine)
{
	uint16_t pitch = PitchConverter::getPitchFM(note, octave, fine);
	uint32_t offset = getFMChannelOffset(ch);
	opna_.setRegister(0xa4 + offset, pitch >> 8);
	opna_.setRegister(0xa0 + offset, pitch & 0x00ff);
	uint32_t chdata = getFmChannelMask(ch);
	opna_.setRegister(0x28, (fmOpEnables_[ch] << 4) | chdata);

	isKeyOnFM_[ch] = true;
	toneFM_[ch].octave = octave;
	toneFM_[ch].note = note;
	toneFM_[ch].fine = fine;
}

void OPNAController::keyOnSSG(int ch, Note note, int octave, int fine)
{
	uint16_t pitch = PitchConverter::getPitchSSG(note, octave, fine);
	uint8_t offset = ch << 1;
	opna_.setRegister(0x00 + offset, pitch & 0xff);
	opna_.setRegister(0x01 + offset, pitch >> 8);

	uint8_t mask = ~(1 << ch);
	mixerSSG_ &= mask;
	opna_.setRegister(0x07, mixerSSG_);

	setVolumeSSG(ch, volSSG_[ch]);

	toneSSG_[ch].octave = octave;
	toneSSG_[ch].note = note;
	toneSSG_[ch].fine = fine;
}

void OPNAController::keyOffFM(int ch)
{	
	uint8_t chdata = getFmChannelMask(ch);
	opna_.setRegister(0x28, chdata);
	isKeyOnFM_[ch] = false;
}

void OPNAController::keyOffSSG(int ch)
{
	uint8_t flag = 1 << ch;
	mixerSSG_ |= flag;
	opna_.setRegister(0x07, mixerSSG_);
	opna_.setRegister(0x08 + ch, 0);	// Volume 0
}

/********** Set instrument **********/
void OPNAController::setInstrumentFM(int ch, std::shared_ptr<InstrumentFM> inst)
{
	if (inst == nullptr) {	// Error set ()
		if (refInstFM_[ch] != nullptr) {	// When setted instrument has been deleted
			refInstFM_[ch]->setNumber(-1);
			writeFMEnvelopeToRegistersFromInstrument(ch);
		}
	}
	else {
		refInstFM_[ch] = inst;
		writeFMEnvelopeToRegistersFromInstrument(ch);
	}
}

void OPNAController::setInstrumentSSG(int ch, std::shared_ptr<InstrumentSSG> inst)
{
	if (inst == nullptr) {	// Error set ()
		if (refInstSSG_[ch] != nullptr)
			refInstSSG_[ch]->setNumber(-1);
	}
	else {
		refInstSSG_[ch] = inst;
	}

	// UNDONE: implement
}

void OPNAController::updateInstrumentFM(int instNum)
{
	for (int i = 0; i < 6; ++i) {
		if (refInstFM_[i] != nullptr && refInstFM_[i]->getNumber() == instNum) {
			writeFMEnvelopeToRegistersFromInstrument(i);
		}
	}
}

void OPNAController::writeFMEnveropeParameterToRegister(int ch, FMParameter param, int value)
{
	uint32_t bch = getFMChannelOffset(ch);	// Bank and channel offset
	uint8_t data;
	int tmp;

	envFM_[ch]->setParameterValue(param, value);

	switch (param) {
	case FMParameter::AL:
	case FMParameter::FB:
		data = envFM_[ch]->getParameterValue(FMParameter::FB) << 3;
		data += envFM_[ch]->getParameterValue(FMParameter::AL);
		opna_.setRegister(0xb0 + bch, data);
		break;
	case FMParameter::DT1:
	case FMParameter::ML1:
		data = envFM_[ch]->getParameterValue(FMParameter::DT1) << 4;
		data |= envFM_[ch]->getParameterValue(FMParameter::ML1);
		opna_.setRegister(0x30 + bch, data);
		break;
	case FMParameter::TL1:
		data = envFM_[ch]->getParameterValue(FMParameter::TL1);
		// Adjust volume
		if (envFM_[ch]->getParameterValue(FMParameter::AL) == 7) {
			data = calculateTL(ch, data);
			envFM_[ch]->setParameterValue(param, data);	// Update
		}
		opna_.setRegister(0x40 + bch, data);
		break;
	case FMParameter::KS1:
	case FMParameter::AR1:
		data = envFM_[ch]->getParameterValue(FMParameter::KS1) << 6;
		data |= envFM_[ch]->getParameterValue(FMParameter::AR1);
		opna_.setRegister(0x50 + bch, data);
		break;
	case FMParameter::AM1:
	case FMParameter::DR1:
		data = 0 << 7;	// For AM1
		data |= envFM_[ch]->getParameterValue(FMParameter::DR1);
		opna_.setRegister(0x60 + bch, data);
		break;
	case FMParameter::SR1:
		data = envFM_[ch]->getParameterValue(FMParameter::SR1);
		opna_.setRegister(0x70 + bch, data);
		break;
	case FMParameter::SL1:
	case FMParameter::RR1:
		data = envFM_[ch]->getParameterValue(FMParameter::SL1) << 4;
		data |= envFM_[ch]->getParameterValue(FMParameter::RR1);
		opna_.setRegister(0x80 + bch, data);
		break;
	case::FMParameter::SSGEG1:
		tmp = envFM_[ch]->getParameterValue(FMParameter::SSGEG1);
		data = (tmp == -1) ? 0 : (0x08 + tmp);
		opna_.setRegister(0x90 + bch, data);
		break;
	case FMParameter::DT2:
	case FMParameter::ML2:
		data = envFM_[ch]->getParameterValue(FMParameter::DT2) << 4;
		data |= envFM_[ch]->getParameterValue(FMParameter::ML2);
		opna_.setRegister(0x30 + bch + 8, data);
		break;
	case FMParameter::TL2:
		data = envFM_[ch]->getParameterValue(FMParameter::TL2);
		// Adjust volume
		switch (envFM_[ch]->getParameterValue(FMParameter::AL)) {
		case 4:
		case 5:
		case 6:
		case 7:
			data = calculateTL(ch, data);
			envFM_[ch]->setParameterValue(param, data);	// Update
			break;
		default:
			break;
		}
		opna_.setRegister(0x40 + bch + 8, data);
		break;
	case FMParameter::KS2:
	case FMParameter::AR2:
		data = envFM_[ch]->getParameterValue(FMParameter::KS2) << 6;
		data |= envFM_[ch]->getParameterValue(FMParameter::AR2);
		opna_.setRegister(0x50 + bch + 8, data);
		break;
	case FMParameter::AM2:
	case FMParameter::DR2:
		data = 0 << 7;	// For AM2
		data |= envFM_[ch]->getParameterValue(FMParameter::DR2);
		opna_.setRegister(0x60 + bch + 8, data);
		break;
	case FMParameter::SR2:
		data = envFM_[ch]->getParameterValue(FMParameter::SR2);
		opna_.setRegister(0x70 + bch + 8, data);
		break;
	case FMParameter::SL2:
	case FMParameter::RR2:
		data = envFM_[ch]->getParameterValue(FMParameter::SL2) << 4;
		data |= envFM_[ch]->getParameterValue(FMParameter::RR2);
		opna_.setRegister(0x80 + bch + 8, data);
		break;
	case FMParameter::SSGEG2:
		tmp = envFM_[ch]->getParameterValue(FMParameter::SSGEG2);
		data = (tmp == -1) ? 0 : (0x08 + tmp);
		opna_.setRegister(0x90 + bch + 8, data);
		break;
	case FMParameter::DT3:
	case FMParameter::ML3:
		data = envFM_[ch]->getParameterValue(FMParameter::DT3) << 4;
		data |= envFM_[ch]->getParameterValue(FMParameter::ML3);
		opna_.setRegister(0x30 + bch + 4, data);
		break;
	case FMParameter::TL3:
		data = envFM_[ch]->getParameterValue(FMParameter::TL3);
		// Adjust volume
		switch (envFM_[ch]->getParameterValue(FMParameter::AL)) {
		case 5:
		case 6:
		case 7:
			data = calculateTL(ch, data);
			envFM_[ch]->setParameterValue(param, data);	// Update
			break;
		default:
			break;
		}
		opna_.setRegister(0x40 + bch + 4, data);
		break;
	case FMParameter::KS3:
	case FMParameter::AR3:
		data = envFM_[ch]->getParameterValue(FMParameter::KS3) << 6;
		data |= envFM_[ch]->getParameterValue(FMParameter::AR3);
		opna_.setRegister(0x50 + bch + 4, data);
		break;
	case FMParameter::AM3:
	case FMParameter::DR3:
		data = 0 << 7;	// For AM3
		data |= envFM_[ch]->getParameterValue(FMParameter::DR3);
		opna_.setRegister(0x60 + bch + 4, data);
		break;
	case FMParameter::SR3:
		data = envFM_[ch]->getParameterValue(FMParameter::SR3);
		opna_.setRegister(0x70 + bch + 4, data);
		break;
	case FMParameter::SL3:
	case FMParameter::RR3:
		data = envFM_[ch]->getParameterValue(FMParameter::SL3) << 4;
		data |= envFM_[ch]->getParameterValue(FMParameter::RR3);
		opna_.setRegister(0x80 + bch + 4, data);
		break;
	case FMParameter::SSGEG3:
		tmp = envFM_[ch]->getParameterValue(FMParameter::SSGEG3);
		data = (tmp == -1) ? 0 : (0x08 + tmp);
		opna_.setRegister(0x90 + bch + 4, data);
		break;
	case FMParameter::DT4:
	case FMParameter::ML4:
		data = envFM_[ch]->getParameterValue(FMParameter::DT4) << 4;
		data |= envFM_[ch]->getParameterValue(FMParameter::ML4);
		opna_.setRegister(0x30 + bch + 12, data);
		break;
	case FMParameter::TL4:
		data = envFM_[ch]->getParameterValue(FMParameter::TL4);
		// Adjust volume
		data = calculateTL(ch, data);
		envFM_[ch]->setParameterValue(param, data);	// Update
		opna_.setRegister(0x40 + bch + 12, data);
		break;
	case FMParameter::KS4:
	case FMParameter::AR4:
		data = envFM_[ch]->getParameterValue(FMParameter::KS4) << 6;
		data |= envFM_[ch]->getParameterValue(FMParameter::AR4);
		opna_.setRegister(0x50 + bch + 12, data);
		break;
	case FMParameter::AM4:
	case FMParameter::DR4:
		data = 0 << 7;	// For AM4
		data |= envFM_[ch]->getParameterValue(FMParameter::DR4);
		opna_.setRegister(0x60 + bch + 12, data);
		break;
	case FMParameter::SR4:
		data = envFM_[ch]->getParameterValue(FMParameter::SR4);
		opna_.setRegister(0x70 + bch + 12, data);
		break;
	case FMParameter::SL4:
	case FMParameter::RR4:
		data = envFM_[ch]->getParameterValue(FMParameter::SL4) << 4;
		data |= envFM_[ch]->getParameterValue(FMParameter::RR4);
		opna_.setRegister(0x80 + bch + 12, data);
		break;
	case FMParameter::SSGEG4:
		tmp = envFM_[ch]->getParameterValue(FMParameter::SSGEG4);
		data = judgeSSEGRegisterValue(tmp);
		opna_.setRegister(0x90 + bch + 12, data);
		break;
	}
}

void OPNAController::updateInstrumentFMEnvelopeParameter(int envNum, FMParameter param)
{
	for (int ch = 0; ch < 6; ++ch) {
		if (refInstFM_[ch] != nullptr && refInstFM_[ch]->getEnvelopeNumber() == envNum) {
			writeFMEnveropeParameterToRegister(ch, param, refInstFM_[ch]->getEnvelopeParameter(param));
		}
	}
}

void OPNAController::setInstrumentFMOperatorEnable(int envNum, int opNum)
{
	for (int ch = 0; ch < 6; ++ch) {
		if (refInstFM_[ch] != nullptr && refInstFM_[ch]->getEnvelopeNumber() == envNum) {
			uint32_t ch = getFmChannelMask(ch);
			if (refInstFM_[ch]->getOperatorEnable(opNum)) {
				fmOpEnables_[ch] |= (1 << opNum);
			}
			else {
				fmOpEnables_[ch] &= ~(1 << opNum);
			}
			if (isKeyOnFM_[ch]) opna_.setRegister(0x28, (fmOpEnables_[ch] << 4) | ch);
		}
	}
}

/********** Set volume **********/
void OPNAController::setVolumeFM(int ch, int volume)
{
	volFM_[ch] = volume;

	if (isKeyOnFM(ch) && refInstFM_[ch] != nullptr) {	// Change TL
		writeFMEnveropeParameterToRegister(ch, FMParameter::TL1,
										   refInstFM_[ch]->getEnvelopeParameter(FMParameter::TL1));
		writeFMEnveropeParameterToRegister(ch, FMParameter::TL2,
										   refInstFM_[ch]->getEnvelopeParameter(FMParameter::TL2));
		writeFMEnveropeParameterToRegister(ch, FMParameter::TL3,
										   refInstFM_[ch]->getEnvelopeParameter(FMParameter::TL3));
		writeFMEnveropeParameterToRegister(ch, FMParameter::TL4,
										   refInstFM_[ch]->getEnvelopeParameter(FMParameter::TL4));
	}
}

void OPNAController::setVolumeSSG(int ch, int volume)
{
	volSSG_[ch] = volume;

	if (isKeyOnSSG(ch)) opna_.setRegister(0x08 + ch, volume);
}

/********** Chip details **********/
bool OPNAController::isKeyOnFM(int ch) const
{
	return isKeyOnFM_[ch];
}

bool OPNAController::isKeyOnSSG(int ch) const
{
	return (((0x09 << ch) & ~mixerSSG_) > 0);
}

ToneDetail OPNAController::getFMTone(int ch) const
{
	return toneFM_[ch];
}

ToneDetail OPNAController::getSSGTone(int ch) const
{
	return toneSSG_[ch];
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

/***********************************/
void OPNAController::initChip()
{	
	opna_.setRegister(0x29, 0x80);		// Init interrupt / YM2608 mode
	mixerSSG_ = 0xff;
	opna_.setRegister(0x07, mixerSSG_);	// SSG mix
	opna_.setRegister(0x11, 0x3f);		// Drum total volume

	// FM
	for (int i = 0; i < 6; ++i) {
		// Init operators key off
		fmOpEnables_[i] = 0xf;
		isKeyOnFM_[i] = false;

		// Init envelope
		envFM_[i] = std::make_unique<EnvelopeFM>(-1);
		refInstFM_[i].reset();

		toneFM_[i].octave = -1;	// Init key on note data
		volFM_[i] = 0;	// Init volume
	}

	// SSG
	for (int i = 0; i < 3; ++i) {
		refInstSSG_[i].reset();	// Init envelope
		toneSSG_[i].octave = -1;	// Init key on note data
		volSSG_[i] = 0xf;	// Init volume
	}

	// Init FM pan
	for (int i = 0; i < 0x200; i += 0x100) {
		for (int j = 0; j < 3; ++j) {
			opna_.setRegister(0xb4+i+j, 0xc0);
		}
	}
}

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

	data1 = refInstFM_[ch]->getEnvelopeParameter(FMParameter::FB);
	envFM_[ch]->setParameterValue(FMParameter::FB, data1);
	data1 <<= 3;
	al = refInstFM_[ch]->getEnvelopeParameter(FMParameter::AL);
	envFM_[ch]->setParameterValue(FMParameter::AL, al);
	data1 += al;
	opna_.setRegister(0xb0 + bch, data1);

	uint32_t offset = bch;	// Operator 1

	data1 = refInstFM_[ch]->getEnvelopeParameter(FMParameter::DT1);
	envFM_[ch]->setParameterValue(FMParameter::DT1, data1);
	data1 <<= 4;
	data2 = refInstFM_[ch]->getEnvelopeParameter(FMParameter::ML1);
	envFM_[ch]->setParameterValue(FMParameter::ML1, data2);
	data1 |= data2;
	opna_.setRegister(0x30 + offset, data1);

	data1 = refInstFM_[ch]->getEnvelopeParameter(FMParameter::TL1);
	// Adjust volume
	if (al == 7)
		data1 = calculateTL(ch, data1);
	envFM_[ch]->setParameterValue(FMParameter::TL1, data1);
	opna_.setRegister(0x40 + offset, data1);

	data1 = refInstFM_[ch]->getEnvelopeParameter(FMParameter::KS1);
	envFM_[ch]->setParameterValue(FMParameter::KS1, data1);
	data1 <<= 6;
	data2 = refInstFM_[ch]->getEnvelopeParameter(FMParameter::AR1);
	envFM_[ch]->setParameterValue(FMParameter::AR1, data2);
	data1 |= data2;
	opna_.setRegister(0x50 + offset, data1);

	data1 = 0 << 7;	// For AM1
	data2 = refInstFM_[ch]->getEnvelopeParameter(FMParameter::DR1);
	envFM_[ch]->setParameterValue(FMParameter::DR1, data2);
	data1 |= data2;
	opna_.setRegister(0x60 + offset, data1);

	data1 = refInstFM_[ch]->getEnvelopeParameter(FMParameter::SR1);
	envFM_[ch]->setParameterValue(FMParameter::SR1, data2);
	opna_.setRegister(0x70 + offset, data1);

	data1 = refInstFM_[ch]->getEnvelopeParameter(FMParameter::SL1);
	envFM_[ch]->setParameterValue(FMParameter::SL1, data1);
	data1 <<= 4;
	data2 = refInstFM_[ch]->getEnvelopeParameter(FMParameter::RR1);
	envFM_[ch]->setParameterValue(FMParameter::RR1, data2);
	data1 |= data2;
	opna_.setRegister(0x80 + offset, data1);

	int tmp = refInstFM_[ch]->getEnvelopeParameter(FMParameter::SSGEG1);
	envFM_[ch]->setParameterValue(FMParameter::SSGEG1, tmp);
	data1 = judgeSSEGRegisterValue(tmp);
	opna_.setRegister(0x90 + offset, data1);

	offset = bch + 8;	// Operator 2

	data1 = refInstFM_[ch]->getEnvelopeParameter(FMParameter::DT2);
	envFM_[ch]->setParameterValue(FMParameter::DT2, data1);
	data1 <<= 4;
	data2 = refInstFM_[ch]->getEnvelopeParameter(FMParameter::ML2);
	envFM_[ch]->setParameterValue(FMParameter::ML2, data2);
	data1 |= data2;
	opna_.setRegister(0x30 + offset, data1);

	data1 = refInstFM_[ch]->getEnvelopeParameter(FMParameter::TL2);
	// Adjust volume
	switch (al) {
	case 4:
	case 5:
	case 6:
	case 7:
		data1 = calculateTL(ch, data1);
		break;
	default:
		break;
	}
	envFM_[ch]->setParameterValue(FMParameter::TL2, data1);
	opna_.setRegister(0x40 + offset, data1);

	data1 = refInstFM_[ch]->getEnvelopeParameter(FMParameter::KS2);
	envFM_[ch]->setParameterValue(FMParameter::KS2, data1);
	data1 <<= 6;
	data2 = refInstFM_[ch]->getEnvelopeParameter(FMParameter::AR2);
	envFM_[ch]->setParameterValue(FMParameter::AR2, data2);
	data1 |= data2;
	opna_.setRegister(0x50 + offset, data1);

	data1 = 0 << 7;	// For AM2
	data2 = refInstFM_[ch]->getEnvelopeParameter(FMParameter::DR2);
	envFM_[ch]->setParameterValue(FMParameter::DR2, data2);
	data1 |= data2;
	opna_.setRegister(0x60 + offset, data1);

	data1 = refInstFM_[ch]->getEnvelopeParameter(FMParameter::SR2);
	envFM_[ch]->setParameterValue(FMParameter::SR2, data2);
	opna_.setRegister(0x70 + offset, data1);

	data1 = refInstFM_[ch]->getEnvelopeParameter(FMParameter::SL2);
	envFM_[ch]->setParameterValue(FMParameter::SL2, data1);
	data1 <<= 4;
	data2 = refInstFM_[ch]->getEnvelopeParameter(FMParameter::RR2);
	envFM_[ch]->setParameterValue(FMParameter::RR2, data2);
	data1 |= data2;
	opna_.setRegister(0x80 + offset, data1);

	tmp = refInstFM_[ch]->getEnvelopeParameter(FMParameter::SSGEG2);
	envFM_[ch]->setParameterValue(FMParameter::SSGEG2, tmp);
	data1 = judgeSSEGRegisterValue(tmp);
	opna_.setRegister(0x90 + offset, data1);

	offset = bch + 4;	// Operator 3

	data1 = refInstFM_[ch]->getEnvelopeParameter(FMParameter::DT3);
	envFM_[ch]->setParameterValue(FMParameter::DT3, data1);
	data1 <<= 4;
	data2 = refInstFM_[ch]->getEnvelopeParameter(FMParameter::ML3);
	envFM_[ch]->setParameterValue(FMParameter::ML3, data2);
	data1 |= data2;
	opna_.setRegister(0x30 + offset, data1);

	data1 = refInstFM_[ch]->getEnvelopeParameter(FMParameter::TL3);
	// Adjust volume
	switch (al) {
	case 5:
	case 6:
	case 7:
		data1 = calculateTL(ch, data1);
		break;
	default:
		break;
	}
	envFM_[ch]->setParameterValue(FMParameter::TL3, data1);
	opna_.setRegister(0x40 + offset, data1);

	data1 = refInstFM_[ch]->getEnvelopeParameter(FMParameter::KS3);
	envFM_[ch]->setParameterValue(FMParameter::KS3, data1);
	data1 <<= 6;
	data2 = refInstFM_[ch]->getEnvelopeParameter(FMParameter::AR3);
	envFM_[ch]->setParameterValue(FMParameter::AR3, data2);
	data1 |= data2;
	opna_.setRegister(0x50 + offset, data1);

	data1 = 0 << 7;	// For AM3
	data2 = refInstFM_[ch]->getEnvelopeParameter(FMParameter::DR3);
	envFM_[ch]->setParameterValue(FMParameter::DR3, data2);
	data1 |= data2;
	opna_.setRegister(0x60 + offset, data1);

	data1 = refInstFM_[ch]->getEnvelopeParameter(FMParameter::SR3);
	envFM_[ch]->setParameterValue(FMParameter::SR3, data2);
	opna_.setRegister(0x70 + offset, data1);

	data1 = refInstFM_[ch]->getEnvelopeParameter(FMParameter::SL3);
	envFM_[ch]->setParameterValue(FMParameter::SL3, data1);
	data1 <<= 4;
	data2 = refInstFM_[ch]->getEnvelopeParameter(FMParameter::RR3);
	envFM_[ch]->setParameterValue(FMParameter::RR3, data2);
	data1 |= data2;
	opna_.setRegister(0x80 + offset, data1);

	tmp = refInstFM_[ch]->getEnvelopeParameter(FMParameter::SSGEG3);
	envFM_[ch]->setParameterValue(FMParameter::SSGEG3, tmp);
	data1 = judgeSSEGRegisterValue(tmp);
	opna_.setRegister(0x90 + offset, data1);

	offset = bch + 12;	// Operator 4

	data1 = refInstFM_[ch]->getEnvelopeParameter(FMParameter::DT4);
	envFM_[ch]->setParameterValue(FMParameter::DT4, data1);
	data1 <<= 4;
	data2 = refInstFM_[ch]->getEnvelopeParameter(FMParameter::ML4);
	envFM_[ch]->setParameterValue(FMParameter::ML4, data2);
	data1 |= data2;
	opna_.setRegister(0x30 + offset, data1);

	data1 = refInstFM_[ch]->getEnvelopeParameter(FMParameter::TL4);
	// Adjust volume
	data1 = calculateTL(ch, data1);
	envFM_[ch]->setParameterValue(FMParameter::TL4, data1);
	opna_.setRegister(0x40 + offset, data1);

	data1 = refInstFM_[ch]->getEnvelopeParameter(FMParameter::KS4);
	envFM_[ch]->setParameterValue(FMParameter::KS4, data1);
	data1 <<= 6;
	data2 = refInstFM_[ch]->getEnvelopeParameter(FMParameter::AR4);
	envFM_[ch]->setParameterValue(FMParameter::AR4, data2);
	data1 |= data2;
	opna_.setRegister(0x50 + offset, data1);

	data1 = 0 << 7;	// For AM4
	data2 = refInstFM_[ch]->getEnvelopeParameter(FMParameter::DR4);
	envFM_[ch]->setParameterValue(FMParameter::DR4, data2);
	data1 |= data2;
	opna_.setRegister(0x60 + offset, data1);

	data1 = refInstFM_[ch]->getEnvelopeParameter(FMParameter::SR4);
	envFM_[ch]->setParameterValue(FMParameter::SR4, data2);
	opna_.setRegister(0x70 + offset, data1);

	data1 = refInstFM_[ch]->getEnvelopeParameter(FMParameter::SL4);
	envFM_[ch]->setParameterValue(FMParameter::SL4, data1);
	data1 <<= 4;
	data2 = refInstFM_[ch]->getEnvelopeParameter(FMParameter::RR4);
	envFM_[ch]->setParameterValue(FMParameter::RR4, data2);
	data1 |= data2;
	opna_.setRegister(0x80 + offset, data1);

	tmp = refInstFM_[ch]->getEnvelopeParameter(FMParameter::SSGEG4);
	envFM_[ch]->setParameterValue(FMParameter::SSGEG4, tmp);
	data1 = judgeSSEGRegisterValue(tmp);
	opna_.setRegister(0x90 + offset, data1);
}
