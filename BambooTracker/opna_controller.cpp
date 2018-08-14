#include "opna_controller.hpp"
#include "pitch_converter.hpp"

#ifdef SINC_INTERPOLATION
OPNAController::OPNAController(int clock, int rate, int duration, InstrumentsManager* im) :
	opna_(clock, rate, duration)
#else
OPNAController::OPNAController(int clock, int rate, InstrumentsManager* im) :
	opna_(clock, rate)
#endif
{
	initChip();

	for (int i = 0; i < 6; ++i) {
		instFM_[i] = std::make_shared<InstrumentFM>(-1, u8"Dummy", im);
		setInstrumentFM(i, nullptr);
		toneFM_[i].octave = -1;
	}
	for (int i = 0; i < 3; ++i) {
		instSSG_[i] = std::make_shared<InstrumentSSG>(-1, u8"Dummy", im);
		setInstrumentSSG(i, nullptr);
		toneSSG_[i].octave = -1;
	}
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

	// Dummy volume
	setVolumeSSG(ch, 0xf);
	//*********************

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
	setVolumeSSG(ch, 0);
}

/********** Set instrument **********/
void OPNAController::setInstrumentFM(int ch, std::shared_ptr<InstrumentFM> inst)
{
	if (inst == nullptr) {	// Error set ()
		instFM_[ch]->setNumber(-1);
	}
	else {
		instFM_[ch] = inst;
	}

	updateFMEnvelopeRegisters(ch);
}

void OPNAController::setInstrumentSSG(int ch, std::shared_ptr<InstrumentSSG> inst)
{
	if (inst == nullptr) {	// Error set ()
		instSSG_[ch]->setNumber(-1);
	}
	else {
		instSSG_[ch] = inst;
	}

	// UNDONE: implement
}

void OPNAController::updateInstrumentFM(int instNum)
{
	for (int i = 0; i < 6; ++i) {
		if (instFM_[i]->getNumber() == instNum) {
			updateFMEnvelopeRegisters(i);
		}
	}
}

void OPNAController::setInstrumentFMEnvelopeParameter(int envNum, FMParameter param)
{
	for (int i = 0; i < 6; ++i) {
		if (instFM_[i]->getEnvelopeNumber() == envNum) {
			uint32_t bch = getFMChannelOffset(i);	// Bank and channel offset
			uint8_t data;
			int tmp;

			switch (param) {
			case FMParameter::AL:
			case FMParameter::FB:
				data = instFM_[i]->getEnvelopeParameter(FMParameter::FB) << 3;
				data += instFM_[i]->getEnvelopeParameter(FMParameter::AL);
				opna_.setRegister(0xb0 + bch, data);
				break;
			case FMParameter::DT1:
			case FMParameter::ML1:
				data = instFM_[i]->getEnvelopeParameter(FMParameter::DT1) << 4;
				data |= instFM_[i]->getEnvelopeParameter(FMParameter::ML1);
				opna_.setRegister(0x30 + bch, data);
				break;
			case FMParameter::TL1:
				data = instFM_[i]->getEnvelopeParameter(FMParameter::TL1);
				opna_.setRegister(0x40 + bch, data);
				break;
			case FMParameter::KS1:
			case FMParameter::AR1:
				data = instFM_[i]->getEnvelopeParameter(FMParameter::KS1) << 6;
				data |= instFM_[i]->getEnvelopeParameter(FMParameter::AR1);
				opna_.setRegister(0x50 + bch, data);
				break;
			case FMParameter::AM1:
			case FMParameter::DR1:
                data = 0 << 7;	// For AM1
				data |= instFM_[i]->getEnvelopeParameter(FMParameter::DR1);
				opna_.setRegister(0x60 + bch, data);
				break;
			case FMParameter::SR1:
				data = instFM_[i]->getEnvelopeParameter(FMParameter::SR1);
				opna_.setRegister(0x70 + bch, data);
				break;
			case FMParameter::SL1:
			case FMParameter::RR1:
				data = instFM_[i]->getEnvelopeParameter(FMParameter::SL1) << 4;
				data |= instFM_[i]->getEnvelopeParameter(FMParameter::RR1);
				opna_.setRegister(0x80 + bch, data);
				break;
			case::FMParameter::SSGEG1:
				tmp = instFM_[i]->getEnvelopeParameter(FMParameter::SSGEG1);
				data = (tmp == -1) ? 0 : (0x08 + tmp);
				opna_.setRegister(0x90 + bch, data);
				break;
			case FMParameter::DT2:
			case FMParameter::ML2:
				data = instFM_[i]->getEnvelopeParameter(FMParameter::DT2) << 4;
				data |= instFM_[i]->getEnvelopeParameter(FMParameter::ML2);
				opna_.setRegister(0x30 + bch + 8, data);
				break;
			case FMParameter::TL2:
				data = instFM_[i]->getEnvelopeParameter(FMParameter::TL2);
				opna_.setRegister(0x40 + bch + 8, data);
				break;
			case FMParameter::KS2:
			case FMParameter::AR2:
				data = instFM_[i]->getEnvelopeParameter(FMParameter::KS2) << 6;
				data |= instFM_[i]->getEnvelopeParameter(FMParameter::AR2);
				opna_.setRegister(0x50 + bch + 8, data);
				break;
			case FMParameter::AM2:
			case FMParameter::DR2:
                data = 0 << 7;	// For AM2
				data |= instFM_[i]->getEnvelopeParameter(FMParameter::DR2);
				opna_.setRegister(0x60 + bch + 8, data);
				break;
			case FMParameter::SR2:
				data = instFM_[i]->getEnvelopeParameter(FMParameter::SR2);
				opna_.setRegister(0x70 + bch + 8, data);
				break;
			case FMParameter::SL2:
			case FMParameter::RR2:
				data = instFM_[i]->getEnvelopeParameter(FMParameter::SL2) << 4;
				data |= instFM_[i]->getEnvelopeParameter(FMParameter::RR2);
				opna_.setRegister(0x80 + bch + 8, data);
				break;
			case FMParameter::SSGEG2:
				tmp = instFM_[i]->getEnvelopeParameter(FMParameter::SSGEG2);
				data = (tmp == -1) ? 0 : (0x08 + tmp);
				opna_.setRegister(0x90 + bch + 8, data);
				break;
			case FMParameter::DT3:
			case FMParameter::ML3:
				data = instFM_[i]->getEnvelopeParameter(FMParameter::DT3) << 4;
				data |= instFM_[i]->getEnvelopeParameter(FMParameter::ML3);
				opna_.setRegister(0x30 + bch + 4, data);
				break;
			case FMParameter::TL3:
				data = instFM_[i]->getEnvelopeParameter(FMParameter::TL3);
				opna_.setRegister(0x40 + bch + 4, data);
				break;
			case FMParameter::KS3:
			case FMParameter::AR3:
				data = instFM_[i]->getEnvelopeParameter(FMParameter::KS3) << 6;
				data |= instFM_[i]->getEnvelopeParameter(FMParameter::AR3);
				opna_.setRegister(0x50 + bch + 4, data);
				break;
			case FMParameter::AM3:
			case FMParameter::DR3:
                data = 0 << 7;	// For AM3
				data |= instFM_[i]->getEnvelopeParameter(FMParameter::DR3);
				opna_.setRegister(0x60 + bch + 4, data);
				break;
			case FMParameter::SR3:
				data = instFM_[i]->getEnvelopeParameter(FMParameter::SR3);
				opna_.setRegister(0x70 + bch + 4, data);
				break;
			case FMParameter::SL3:
			case FMParameter::RR3:
				data = instFM_[i]->getEnvelopeParameter(FMParameter::SL3) << 4;
				data |= instFM_[i]->getEnvelopeParameter(FMParameter::RR3);
				opna_.setRegister(0x80 + bch + 4, data);
				break;
			case FMParameter::SSGEG3:
				tmp = instFM_[i]->getEnvelopeParameter(FMParameter::SSGEG3);
				data = (tmp == -1) ? 0 : (0x08 + tmp);
				opna_.setRegister(0x90 + bch + 4, data);
				break;
			case FMParameter::DT4:
			case FMParameter::ML4:
				data = instFM_[i]->getEnvelopeParameter(FMParameter::DT4) << 4;
				data |= instFM_[i]->getEnvelopeParameter(FMParameter::ML4);
				opna_.setRegister(0x30 + bch + 12, data);
				break;
			case FMParameter::TL4:
				data = instFM_[i]->getEnvelopeParameter(FMParameter::TL4);
				opna_.setRegister(0x40 + bch + 12, data);
				break;
			case FMParameter::KS4:
			case FMParameter::AR4:
				data = instFM_[i]->getEnvelopeParameter(FMParameter::KS4) << 6;
				data |= instFM_[i]->getEnvelopeParameter(FMParameter::AR4);
				opna_.setRegister(0x50 + bch + 12, data);
				break;
			case FMParameter::AM4:
			case FMParameter::DR4:
                data = 0 << 7;	// For AM4
				data |= instFM_[i]->getEnvelopeParameter(FMParameter::DR4);
				opna_.setRegister(0x60 + bch + 12, data);
				break;
			case FMParameter::SR4:
				data = instFM_[i]->getEnvelopeParameter(FMParameter::SR4);
				opna_.setRegister(0x70 + bch + 12, data);
				break;
			case FMParameter::SL4:
			case FMParameter::RR4:
				data = instFM_[i]->getEnvelopeParameter(FMParameter::SL4) << 4;
				data |= instFM_[i]->getEnvelopeParameter(FMParameter::RR4);
				opna_.setRegister(0x80 + bch + 12, data);
				break;
			case FMParameter::SSGEG4:
				tmp = instFM_[i]->getEnvelopeParameter(FMParameter::SSGEG4);
				data = (tmp == -1) ? 0 : (0x08 + tmp);
				opna_.setRegister(0x90 + bch + 12, data);
				break;
			}
		}
	}
}

void OPNAController::setInstrumentFMOperatorEnable(int envNum, int opNum)
{
	for (int i = 0; i < 6; ++i) {
		if (instFM_[i]->getEnvelopeNumber() == envNum) {
			uint32_t ch = getFmChannelMask(i);
			if (instFM_[i]->getOperatorEnable(opNum)) {
				fmOpEnables_[i] |= (1 << opNum);
			}
			else {
				fmOpEnables_[i] &= ~(1 << opNum);
			}
			if (isKeyOnFM_[i]) opna_.setRegister(0x28, (fmOpEnables_[i] << 4) | ch);
		}
	}
}

/********** Set volume **********/
void OPNAController::setVolumeSSG(int ch, int level)
{
	opna_.setRegister(0x08 + ch, level);
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

	// Init operators key off
	for (int i = 0; i < 6; ++i) {
		fmOpEnables_[i] = 0xf;
		isKeyOnFM_[i] = false;
	}

	// Init pan
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

void OPNAController::updateFMEnvelopeRegisters(int ch)
{
	uint32_t bch = getFMChannelOffset(ch);	// Bank and channel offset
	uint8_t data;

	data = instFM_[ch]->getEnvelopeParameter(FMParameter::FB) << 3;
	data += instFM_[ch]->getEnvelopeParameter(FMParameter::AL);
	opna_.setRegister(0xb0 + bch, data);

	uint32_t offset = bch;	// Operator 1

	data = instFM_[ch]->getEnvelopeParameter(FMParameter::DT1) << 4;
	data |= instFM_[ch]->getEnvelopeParameter(FMParameter::ML1);
	opna_.setRegister(0x30 + offset, data);

	data = instFM_[ch]->getEnvelopeParameter(FMParameter::TL1);
	opna_.setRegister(0x40 + offset, data);

	data = instFM_[ch]->getEnvelopeParameter(FMParameter::KS1) << 6;
	data |= instFM_[ch]->getEnvelopeParameter(FMParameter::AR1);
	opna_.setRegister(0x50 + offset, data);

	data = 0 << 7;	// For AM1
	data |= instFM_[ch]->getEnvelopeParameter(FMParameter::DR1);
	opna_.setRegister(0x60 + offset, data);

	data = instFM_[ch]->getEnvelopeParameter(FMParameter::SR1);
	opna_.setRegister(0x70 + offset, data);

	data = instFM_[ch]->getEnvelopeParameter(FMParameter::SL1) << 4;
	data |= instFM_[ch]->getEnvelopeParameter(FMParameter::RR1);
	opna_.setRegister(0x80 + offset, data);

	int tmp = instFM_[ch]->getEnvelopeParameter(FMParameter::SSGEG1);
	data = (tmp == -1) ? 0 : (0x08 + tmp);
	opna_.setRegister(0x90 + offset, data);

	offset = bch + 8;	// Operator 2

	data = instFM_[ch]->getEnvelopeParameter(FMParameter::DT2) << 4;
	data |= instFM_[ch]->getEnvelopeParameter(FMParameter::ML2);
	opna_.setRegister(0x30 + offset, data);

	data = instFM_[ch]->getEnvelopeParameter(FMParameter::TL2);
	opna_.setRegister(0x40 + offset, data);

	data = instFM_[ch]->getEnvelopeParameter(FMParameter::KS2) << 6;
	data |= instFM_[ch]->getEnvelopeParameter(FMParameter::AR2);
	opna_.setRegister(0x50 + offset, data);

	data = 0 << 7;	// For AM2
	data |= instFM_[ch]->getEnvelopeParameter(FMParameter::DR2);
	opna_.setRegister(0x60 + offset, data);

	data = instFM_[ch]->getEnvelopeParameter(FMParameter::SR2);
	opna_.setRegister(0x70 + offset, data);

	data = instFM_[ch]->getEnvelopeParameter(FMParameter::SL2) << 4;
	data |= instFM_[ch]->getEnvelopeParameter(FMParameter::RR2);
	opna_.setRegister(0x80 + offset, data);

	tmp = instFM_[ch]->getEnvelopeParameter(FMParameter::SSGEG2);
	data = (tmp == -1) ? 0 : (0x08 + tmp);
	opna_.setRegister(0x90 + offset, data);

	offset = bch + 4;	// Operator 3

	data = instFM_[ch]->getEnvelopeParameter(FMParameter::DT3) << 4;
	data |= instFM_[ch]->getEnvelopeParameter(FMParameter::ML3);
	opna_.setRegister(0x30 + offset, data);

	data = instFM_[ch]->getEnvelopeParameter(FMParameter::TL3);
	opna_.setRegister(0x40 + offset, data);

	data = instFM_[ch]->getEnvelopeParameter(FMParameter::KS3) << 6;
	data |= instFM_[ch]->getEnvelopeParameter(FMParameter::AR3);
	opna_.setRegister(0x50 + offset, data);

	data = 0 << 7;	// For AM3
	data |= instFM_[ch]->getEnvelopeParameter(FMParameter::DR3);
	opna_.setRegister(0x60 + offset, data);

	data = instFM_[ch]->getEnvelopeParameter(FMParameter::SR3);
	opna_.setRegister(0x70 + offset, data);

	data = instFM_[ch]->getEnvelopeParameter(FMParameter::SL3) << 4;
	data |= instFM_[ch]->getEnvelopeParameter(FMParameter::RR3);
	opna_.setRegister(0x80 + offset, data);

	tmp = instFM_[ch]->getEnvelopeParameter(FMParameter::SSGEG3);
	data = (tmp == -1) ? 0 : (0x08 + tmp);
	opna_.setRegister(0x90 + offset, data);

	offset = bch + 12;	// Operator 4

	data = instFM_[ch]->getEnvelopeParameter(FMParameter::DT4) << 4;
	data |= instFM_[ch]->getEnvelopeParameter(FMParameter::ML4);
	opna_.setRegister(0x30 + offset, data);

	data = instFM_[ch]->getEnvelopeParameter(FMParameter::TL4);
	opna_.setRegister(0x40 + offset, data);

	data = instFM_[ch]->getEnvelopeParameter(FMParameter::KS4) << 6;
	data |= instFM_[ch]->getEnvelopeParameter(FMParameter::AR4);
	opna_.setRegister(0x50 + offset, data);

	data = 0 << 7;	// For AM4
	data |= instFM_[ch]->getEnvelopeParameter(FMParameter::DR4);
	opna_.setRegister(0x60 + offset, data);

	data = instFM_[ch]->getEnvelopeParameter(FMParameter::SR4);
	opna_.setRegister(0x70 + offset, data);

	data = instFM_[ch]->getEnvelopeParameter(FMParameter::SL4) << 4;
	data |= instFM_[ch]->getEnvelopeParameter(FMParameter::RR4);
	opna_.setRegister(0x80 + offset, data);

	tmp = instFM_[ch]->getEnvelopeParameter(FMParameter::SSGEG4);
	data = (tmp == -1) ? 0 : (0x08 + tmp);
	opna_.setRegister(0x90 + offset, data);
}
