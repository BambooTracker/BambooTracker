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
		instFM_[i] = std::make_unique<InstrumentFM>(-1, u8"Dummy", im);
		setInstrumentFM(i, nullptr);
	}
	for (int i = 0; i < 3; ++i) {
		instPSG_[i] = std::make_unique<InstrumentPSG>(-1, u8"Dummy", im);
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
void OPNAController::keyOnFM(int id, Note note, int octave, int fine)
{
	uint16_t pitch = PitchConverter::getPitchFM(note, octave, fine);
	uint8_t offset = getFMChannelOffset(id);
	opna_.setRegister(0xa4 + offset, pitch >> 8);
	opna_.setRegister(0xa0 + offset, pitch & 0x00ff);
	uint32_t ch = getFmChannelMask(id);
	opna_.setRegister(0x28, (fmOpEnables_[id] << 4) | ch);

	isKeyOnFM_[id] = true;
}

void OPNAController::keyOnPSG(int id, Note note, int octave, int fine)
{
	uint16_t pitch = PitchConverter::getPitchPSG(note, octave, fine);
	uint8_t offset = id << 1;
	opna_.setRegister(0x00 + offset, pitch & 0xff);
	opna_.setRegister(0x01 + offset, pitch >> 8);

	uint8_t mask = ~(1 << id);
	mixerPSG_ &= mask;
	opna_.setRegister(0x07, mixerPSG_);

	// Dummy volume
	setVolumePSG(id, 0xf);
	//*********************
}

void OPNAController::keyOffFM(int id)
{	
	uint8_t ch = getFmChannelMask(id);
	opna_.setRegister(0x28, ch);
	isKeyOnFM_[id] = false;
}

void OPNAController::keyOffPSG(int id)
{
	uint8_t flag = 1 << id;
	mixerPSG_ |= flag;
	opna_.setRegister(0x07, mixerPSG_);
	setVolumePSG(id, 0);
}

/********** Set instrument **********/
void OPNAController::setInstrumentFM(int id, InstrumentFM* inst)
{
	if (inst == nullptr) {	// Error set ()
		instFM_[id]->setNumber(-1);
	}
	else {
		instFM_[id] = std::unique_ptr<InstrumentFM>(inst);
	}

	uint32_t bch = getFMChannelOffset(id);	// Bank and channel offset
	uint8_t data;

	data = instFM_[id]->getEnvelopeParameter(FMParameter::FB) << 3;
	data += instFM_[id]->getEnvelopeParameter(FMParameter::AL);
	opna_.setRegister(0xb0 + bch, data);

	uint32_t offset = bch;	// Operator 1

	data = instFM_[id]->getEnvelopeParameter(FMParameter::DT1) << 4;
	data |= instFM_[id]->getEnvelopeParameter(FMParameter::ML1);
	opna_.setRegister(0x30 + offset, data);

	data = instFM_[id]->getEnvelopeParameter(FMParameter::TL1);
	opna_.setRegister(0x40 + offset, data);

	data = instFM_[id]->getEnvelopeParameter(FMParameter::KS1) << 6;
	data |= instFM_[id]->getEnvelopeParameter(FMParameter::AR1);
	opna_.setRegister(0x50 + offset, data);

	data = 0 << 7;	// For AM1
	data |= instFM_[id]->getEnvelopeParameter(FMParameter::DR1);
	opna_.setRegister(0x60 + offset, data);

	data = instFM_[id]->getEnvelopeParameter(FMParameter::SR1);
	opna_.setRegister(0x70 + offset, data);

	data = instFM_[id]->getEnvelopeParameter(FMParameter::SL1) << 4;
	data |= instFM_[id]->getEnvelopeParameter(FMParameter::RR1);
	opna_.setRegister(0x80 + offset, data);

	int tmp = instFM_[id]->getEnvelopeParameter(FMParameter::SSGEG1);
	data = (tmp == -1)? 0 : (0x08 + tmp);
	opna_.setRegister(0x90 + offset, data);

	offset = bch + 8;	// Operator 2

	data = instFM_[id]->getEnvelopeParameter(FMParameter::DT2) << 4;
	data |= instFM_[id]->getEnvelopeParameter(FMParameter::ML2);
	opna_.setRegister(0x30 + offset, data);

	data = instFM_[id]->getEnvelopeParameter(FMParameter::TL2);
	opna_.setRegister(0x40 + offset, data);

	data = instFM_[id]->getEnvelopeParameter(FMParameter::KS2) << 6;
	data |= instFM_[id]->getEnvelopeParameter(FMParameter::AR2);
	opna_.setRegister(0x50 + offset, data);

	data = 0 << 7;	// For AM2
	data |= instFM_[id]->getEnvelopeParameter(FMParameter::DR2);
	opna_.setRegister(0x60 + offset, data);

	data = instFM_[id]->getEnvelopeParameter(FMParameter::SR2);
	opna_.setRegister(0x70 + offset, data);

	data = instFM_[id]->getEnvelopeParameter(FMParameter::SL2) << 4;
	data |= instFM_[id]->getEnvelopeParameter(FMParameter::RR2);
	opna_.setRegister(0x80 + offset, data);

	tmp = instFM_[id]->getEnvelopeParameter(FMParameter::SSGEG2);
	data = (tmp == -1)? 0 : (0x08 + tmp);
	opna_.setRegister(0x90 + offset, data);

	offset = bch + 4;	// Operator 3

	data = instFM_[id]->getEnvelopeParameter(FMParameter::DT3) << 4;
	data |= instFM_[id]->getEnvelopeParameter(FMParameter::ML3);
	opna_.setRegister(0x30 + offset, data);

	data = instFM_[id]->getEnvelopeParameter(FMParameter::TL3);
	opna_.setRegister(0x40 + offset, data);

	data = instFM_[id]->getEnvelopeParameter(FMParameter::KS3) << 6;
	data |= instFM_[id]->getEnvelopeParameter(FMParameter::AR3);
	opna_.setRegister(0x50 + offset, data);

	data = 0 << 7;	// For AM3
	data |= instFM_[id]->getEnvelopeParameter(FMParameter::DR3);
	opna_.setRegister(0x60 + offset, data);

	data = instFM_[id]->getEnvelopeParameter(FMParameter::SR3);
	opna_.setRegister(0x70 + offset, data);

	data = instFM_[id]->getEnvelopeParameter(FMParameter::SL3) << 4;
	data |= instFM_[id]->getEnvelopeParameter(FMParameter::RR3);
	opna_.setRegister(0x80 + offset, data);

	tmp = instFM_[id]->getEnvelopeParameter(FMParameter::SSGEG3);
	data = (tmp == -1)? 0 : (0x08 + tmp);
	opna_.setRegister(0x90 + offset, data);

	offset = bch + 12;	// Operator 4

	data = instFM_[id]->getEnvelopeParameter(FMParameter::DT4) << 4;
	data |= instFM_[id]->getEnvelopeParameter(FMParameter::ML4);
	opna_.setRegister(0x30 + offset, data);

	data = instFM_[id]->getEnvelopeParameter(FMParameter::TL4);
	opna_.setRegister(0x40 + offset, data);

	data = instFM_[id]->getEnvelopeParameter(FMParameter::KS4) << 6;
	data |= instFM_[id]->getEnvelopeParameter(FMParameter::AR4);
	opna_.setRegister(0x50 + offset, data);

	data = 0 << 7;	// For AM4
	data |= instFM_[id]->getEnvelopeParameter(FMParameter::DR4);
	opna_.setRegister(0x60 + offset, data);

	data = instFM_[id]->getEnvelopeParameter(FMParameter::SR4);
	opna_.setRegister(0x70 + offset, data);

	data = instFM_[id]->getEnvelopeParameter(FMParameter::SL4) << 4;
	data |= instFM_[id]->getEnvelopeParameter(FMParameter::RR4);
	opna_.setRegister(0x80 + offset, data);

	tmp = instFM_[id]->getEnvelopeParameter(FMParameter::SSGEG4);
	data = (tmp == -1)? 0 : (0x08 + tmp);
	opna_.setRegister(0x90 + offset, data);
}

void OPNAController::setInstrumentPSG(int id, InstrumentPSG* inst)
{
	if (inst == nullptr) {	// Error set ()
		instPSG_[id]->setNumber(-1);
	}
	else {
		instPSG_[id] = std::unique_ptr<InstrumentPSG>(inst);
	}

	// UNDONE: implement
}

void OPNAController::setInstrumentFMParameter(int instNum, FMParameter param)
{
	for (int i = 0; i < 6; ++i) {
		if (instFM_[i]->getNumber() == instNum) {
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

void OPNAController::setInstrumentFMOperatorEnable(int instNum, int opNum)
{
	for (int i = 0; i < 6; ++i) {
		if (instFM_[i]->getNumber() == instNum) {
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
void OPNAController::setVolumePSG(int id, int level)
{
	opna_.setRegister(0x08 + id, level);
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

uint32_t OPNAController::getFmChannelMask(int id)
{
	// UNDONE: change channel type by Effect mode
	switch (id) {
	case 0: return 0x00;
	case 1: return 0x01;
	case 2: return 0x02;
	case 3: return 0x04;
	case 4: return 0x05;
	case 5: return 0x06;
	default: return 0;
	}
}

uint32_t OPNAController::getFMChannelOffset(int id)
{
	switch (id) {
	case 0:
	case 1:
	case 2:
		return id;
	case 3:
	case 4:
	case 5:
		return 0x100 + id % 3;
	default:
		return 0;
	}
}
