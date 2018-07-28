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
	}
	for (int i = 0; i < 3; ++i) {
		instPSG_[i] = std::make_shared<InstrumentPSG>(-1, u8"Dummy", im);
		setInstrumentPSG(i, nullptr);
	}
}

/********** Reset and initialize **********/
void OPNAController::reset()
{
	opna_.reset();
	initChip();
}

/********** Key on-off **********/
void OPNAController::keyOnFM(int chId, Note note, int octave, int fine)
{
	uint16_t pitch = PitchConverter::getPitchFM(note, octave, fine);
	uint8_t offset = getFMChannelOffset(chId);
	opna_.setRegister(0xa4 + offset, pitch >> 8);
	opna_.setRegister(0xa0 + offset, pitch & 0x00ff);
	uint32_t ch = getFmChannelMask(chId);
	opna_.setRegister(0x28, (fmOpEnables_[chId] << 4) | ch);

	isKeyOnFM_[chId] = true;
}

void OPNAController::keyOnPSG(int chId, Note note, int octave, int fine)
{
	uint16_t pitch = PitchConverter::getPitchPSG(note, octave, fine);
	uint8_t offset = chId << 1;
	opna_.setRegister(0x00 + offset, pitch & 0xff);
	opna_.setRegister(0x01 + offset, pitch >> 8);

	uint8_t mask = ~(1 << chId);
	mixerPSG_ &= mask;
	opna_.setRegister(0x07, mixerPSG_);

	// Dummy volume
	setVolumePSG(chId, 0xf);
	//*********************
}

void OPNAController::keyOffFM(int chId)
{	
	uint8_t ch = getFmChannelMask(chId);
	opna_.setRegister(0x28, ch);
	isKeyOnFM_[chId] = false;
}

void OPNAController::keyOffPSG(int chId)
{
	uint8_t flag = 1 << chId;
	mixerPSG_ |= flag;
	opna_.setRegister(0x07, mixerPSG_);
	setVolumePSG(chId, 0);
}

/********** Set instrument **********/
void OPNAController::setInstrumentFM(int chId, std::shared_ptr<InstrumentFM> inst)
{
	if (inst == nullptr) {	// Error set ()
		instFM_[chId]->setNumber(-1);
	}
	else {
		instFM_[chId] = inst;
	}

	updateFMEnvelopeRegisters(chId);
}

void OPNAController::setInstrumentPSG(int chId, std::shared_ptr<InstrumentPSG> inst)
{
	if (inst == nullptr) {	// Error set ()
		instPSG_[chId]->setNumber(-1);
	}
	else {
		instPSG_[chId] = inst;
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
				data = instFM_[i]->getEnvelopeParameter(FMParameter::AM1) << 7;
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
				data = (tmp == -1)? 0 : (0x08 + tmp);
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
				data = instFM_[i]->getEnvelopeParameter(FMParameter::AM2) << 7;
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
				data = (tmp == -1)? 0 : (0x08 + tmp);
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
				data = instFM_[i]->getEnvelopeParameter(FMParameter::AM3) << 7;
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
				data = (tmp == -1)? 0 : (0x08 + tmp);
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
				data = instFM_[i]->getEnvelopeParameter(FMParameter::AM4) << 7;
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
				data = (tmp == -1)? 0 : (0x08 + tmp);
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
void OPNAController::setVolumePSG(int chId, int level)
{
	opna_.setRegister(0x08 + chId, level);
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
	mixerPSG_ = 0xff;
	opna_.setRegister(0x07, mixerPSG_);	// PSG mix
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

uint32_t OPNAController::getFmChannelMask(int chId)
{
	// UNDONE: change channel type by Effect mode
	switch (chId) {
	case 0: return 0x00;
	case 1: return 0x01;
	case 2: return 0x02;
	case 3: return 0x04;
	case 4: return 0x05;
	case 5: return 0x06;
	default: return 0;
	}
}

uint32_t OPNAController::getFMChannelOffset(int chId)
{
	switch (chId) {
	case 0:
	case 1:
	case 2:
		return chId;
	case 3:
	case 4:
	case 5:
		return 0x100 + chId % 3;
	default:
		return 0;
	}
}

void OPNAController::updateFMEnvelopeRegisters(int chId)
{
	uint32_t bch = getFMChannelOffset(chId);	// Bank and channel offset
	uint8_t data;

	data = instFM_[chId]->getEnvelopeParameter(FMParameter::FB) << 3;
	data += instFM_[chId]->getEnvelopeParameter(FMParameter::AL);
	opna_.setRegister(0xb0 + bch, data);

	uint32_t offset = bch;	// Operator 1

	data = instFM_[chId]->getEnvelopeParameter(FMParameter::DT1) << 4;
	data |= instFM_[chId]->getEnvelopeParameter(FMParameter::ML1);
	opna_.setRegister(0x30 + offset, data);

	data = instFM_[chId]->getEnvelopeParameter(FMParameter::TL1);
	opna_.setRegister(0x40 + offset, data);

	data = instFM_[chId]->getEnvelopeParameter(FMParameter::KS1) << 6;
	data |= instFM_[chId]->getEnvelopeParameter(FMParameter::AR1);
	opna_.setRegister(0x50 + offset, data);

	data = 0 << 7;	// For AM1
	data |= instFM_[chId]->getEnvelopeParameter(FMParameter::DR1);
	opna_.setRegister(0x60 + offset, data);

	data = instFM_[chId]->getEnvelopeParameter(FMParameter::SR1);
	opna_.setRegister(0x70 + offset, data);

	data = instFM_[chId]->getEnvelopeParameter(FMParameter::SL1) << 4;
	data |= instFM_[chId]->getEnvelopeParameter(FMParameter::RR1);
	opna_.setRegister(0x80 + offset, data);

	int tmp = instFM_[chId]->getEnvelopeParameter(FMParameter::SSGEG1);
	data = (tmp == -1)? 0 : (0x08 + tmp);
	opna_.setRegister(0x90 + offset, data);

	offset = bch + 8;	// Operator 2

	data = instFM_[chId]->getEnvelopeParameter(FMParameter::DT2) << 4;
	data |= instFM_[chId]->getEnvelopeParameter(FMParameter::ML2);
	opna_.setRegister(0x30 + offset, data);

	data = instFM_[chId]->getEnvelopeParameter(FMParameter::TL2);
	opna_.setRegister(0x40 + offset, data);

	data = instFM_[chId]->getEnvelopeParameter(FMParameter::KS2) << 6;
	data |= instFM_[chId]->getEnvelopeParameter(FMParameter::AR2);
	opna_.setRegister(0x50 + offset, data);

	data = 0 << 7;	// For AM2
	data |= instFM_[chId]->getEnvelopeParameter(FMParameter::DR2);
	opna_.setRegister(0x60 + offset, data);

	data = instFM_[chId]->getEnvelopeParameter(FMParameter::SR2);
	opna_.setRegister(0x70 + offset, data);

	data = instFM_[chId]->getEnvelopeParameter(FMParameter::SL2) << 4;
	data |= instFM_[chId]->getEnvelopeParameter(FMParameter::RR2);
	opna_.setRegister(0x80 + offset, data);

	tmp = instFM_[chId]->getEnvelopeParameter(FMParameter::SSGEG2);
	data = (tmp == -1)? 0 : (0x08 + tmp);
	opna_.setRegister(0x90 + offset, data);

	offset = bch + 4;	// Operator 3

	data = instFM_[chId]->getEnvelopeParameter(FMParameter::DT3) << 4;
	data |= instFM_[chId]->getEnvelopeParameter(FMParameter::ML3);
	opna_.setRegister(0x30 + offset, data);

	data = instFM_[chId]->getEnvelopeParameter(FMParameter::TL3);
	opna_.setRegister(0x40 + offset, data);

	data = instFM_[chId]->getEnvelopeParameter(FMParameter::KS3) << 6;
	data |= instFM_[chId]->getEnvelopeParameter(FMParameter::AR3);
	opna_.setRegister(0x50 + offset, data);

	data = 0 << 7;	// For AM3
	data |= instFM_[chId]->getEnvelopeParameter(FMParameter::DR3);
	opna_.setRegister(0x60 + offset, data);

	data = instFM_[chId]->getEnvelopeParameter(FMParameter::SR3);
	opna_.setRegister(0x70 + offset, data);

	data = instFM_[chId]->getEnvelopeParameter(FMParameter::SL3) << 4;
	data |= instFM_[chId]->getEnvelopeParameter(FMParameter::RR3);
	opna_.setRegister(0x80 + offset, data);

	tmp = instFM_[chId]->getEnvelopeParameter(FMParameter::SSGEG3);
	data = (tmp == -1)? 0 : (0x08 + tmp);
	opna_.setRegister(0x90 + offset, data);

	offset = bch + 12;	// Operator 4

	data = instFM_[chId]->getEnvelopeParameter(FMParameter::DT4) << 4;
	data |= instFM_[chId]->getEnvelopeParameter(FMParameter::ML4);
	opna_.setRegister(0x30 + offset, data);

	data = instFM_[chId]->getEnvelopeParameter(FMParameter::TL4);
	opna_.setRegister(0x40 + offset, data);

	data = instFM_[chId]->getEnvelopeParameter(FMParameter::KS4) << 6;
	data |= instFM_[chId]->getEnvelopeParameter(FMParameter::AR4);
	opna_.setRegister(0x50 + offset, data);

	data = 0 << 7;	// For AM4
	data |= instFM_[chId]->getEnvelopeParameter(FMParameter::DR4);
	opna_.setRegister(0x60 + offset, data);

	data = instFM_[chId]->getEnvelopeParameter(FMParameter::SR4);
	opna_.setRegister(0x70 + offset, data);

	data = instFM_[chId]->getEnvelopeParameter(FMParameter::SL4) << 4;
	data |= instFM_[chId]->getEnvelopeParameter(FMParameter::RR4);
	opna_.setRegister(0x80 + offset, data);

	tmp = instFM_[chId]->getEnvelopeParameter(FMParameter::SSGEG4);
	data = (tmp == -1)? 0 : (0x08 + tmp);
	opna_.setRegister(0x90 + offset, data);
}
