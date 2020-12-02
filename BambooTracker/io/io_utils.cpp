#include "io_utils.hpp"
#include <stdexcept>

namespace io
{
const FMEnvelopeParameter FM_OPSEQ_PARAMS[38] = {
	FMEnvelopeParameter::AL, FMEnvelopeParameter::FB,
	FMEnvelopeParameter::AR1, FMEnvelopeParameter::DR1, FMEnvelopeParameter::SR1,
	FMEnvelopeParameter::RR1, FMEnvelopeParameter::SL1, FMEnvelopeParameter::TL1,
	FMEnvelopeParameter::KS1, FMEnvelopeParameter::ML1, FMEnvelopeParameter::DT1,
	FMEnvelopeParameter::AR2, FMEnvelopeParameter::DR2, FMEnvelopeParameter::SR2,
	FMEnvelopeParameter::RR2, FMEnvelopeParameter::SL2, FMEnvelopeParameter::TL2,
	FMEnvelopeParameter::KS2, FMEnvelopeParameter::ML2, FMEnvelopeParameter::DT2,
	FMEnvelopeParameter::AR3, FMEnvelopeParameter::DR3, FMEnvelopeParameter::SR3,
	FMEnvelopeParameter::RR3, FMEnvelopeParameter::SL3, FMEnvelopeParameter::TL3,
	FMEnvelopeParameter::KS3, FMEnvelopeParameter::ML3, FMEnvelopeParameter::DT3,
	FMEnvelopeParameter::AR4, FMEnvelopeParameter::DR4, FMEnvelopeParameter::SR4,
	FMEnvelopeParameter::RR4, FMEnvelopeParameter::SL4, FMEnvelopeParameter::TL4,
	FMEnvelopeParameter::KS4, FMEnvelopeParameter::ML4, FMEnvelopeParameter::DT4
};

const FMOperatorType FM_OP_TYPES[4] = {
	FMOperatorType::Op1, FMOperatorType::Op2, FMOperatorType::Op3, FMOperatorType::Op4
};

int convertDtFromDmpTfiVgi(int dt)
{
	switch (dt) {
	case 0:		return 7;
	case 1:		return 6;
	case 2:		return 5;
	case 3:		return 0;
	case 4:		return 1;
	case 5:		return 2;
	case 6:		return 3;
	case 7:		return 3;
	default:	throw std::out_of_range("Out of range dt");
	}
}
}
