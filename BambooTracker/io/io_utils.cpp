#include "io_utils.hpp"
#include <stdexcept>
#include "binary_container.hpp"

namespace io
{
const std::unordered_map<FMOperatorParameter, FMEnvelopeParameter> FM_OP_PARAMS[4] = {
	{
		{ FMOperatorParameter::AR, FMEnvelopeParameter::AR1 },
		{ FMOperatorParameter::DR, FMEnvelopeParameter::DR1 },
		{ FMOperatorParameter::SR, FMEnvelopeParameter::SR1 },
		{ FMOperatorParameter::RR, FMEnvelopeParameter::RR1 },
		{ FMOperatorParameter::SL, FMEnvelopeParameter::SL1 },
		{ FMOperatorParameter::TL, FMEnvelopeParameter::TL1 },
		{ FMOperatorParameter::KS, FMEnvelopeParameter::KS1 },
		{ FMOperatorParameter::ML, FMEnvelopeParameter::ML1 },
		{ FMOperatorParameter::DT, FMEnvelopeParameter::DT1 },
		{ FMOperatorParameter::SSGEG, FMEnvelopeParameter::SSGEG1 }
	},
	{
		{ FMOperatorParameter::AR, FMEnvelopeParameter::AR2 },
		{ FMOperatorParameter::DR, FMEnvelopeParameter::DR2 },
		{ FMOperatorParameter::SR, FMEnvelopeParameter::SR2 },
		{ FMOperatorParameter::RR, FMEnvelopeParameter::RR2 },
		{ FMOperatorParameter::SL, FMEnvelopeParameter::SL2 },
		{ FMOperatorParameter::TL, FMEnvelopeParameter::TL2 },
		{ FMOperatorParameter::KS, FMEnvelopeParameter::KS2 },
		{ FMOperatorParameter::ML, FMEnvelopeParameter::ML2 },
		{ FMOperatorParameter::DT, FMEnvelopeParameter::DT2 },
		{ FMOperatorParameter::SSGEG, FMEnvelopeParameter::SSGEG2 }
	},
	{
		{ FMOperatorParameter::AR, FMEnvelopeParameter::AR3 },
		{ FMOperatorParameter::DR, FMEnvelopeParameter::DR3 },
		{ FMOperatorParameter::SR, FMEnvelopeParameter::SR3 },
		{ FMOperatorParameter::RR, FMEnvelopeParameter::RR3 },
		{ FMOperatorParameter::SL, FMEnvelopeParameter::SL3 },
		{ FMOperatorParameter::TL, FMEnvelopeParameter::TL3 },
		{ FMOperatorParameter::KS, FMEnvelopeParameter::KS3 },
		{ FMOperatorParameter::ML, FMEnvelopeParameter::ML3 },
		{ FMOperatorParameter::DT, FMEnvelopeParameter::DT3 },
		{ FMOperatorParameter::SSGEG, FMEnvelopeParameter::SSGEG3 }
	},
	{
		{ FMOperatorParameter::AR, FMEnvelopeParameter::AR4 },
		{ FMOperatorParameter::DR, FMEnvelopeParameter::DR4 },
		{ FMOperatorParameter::SR, FMEnvelopeParameter::SR4 },
		{ FMOperatorParameter::RR, FMEnvelopeParameter::RR4 },
		{ FMOperatorParameter::SL, FMEnvelopeParameter::SL4 },
		{ FMOperatorParameter::TL, FMEnvelopeParameter::TL4 },
		{ FMOperatorParameter::KS, FMEnvelopeParameter::KS4 },
		{ FMOperatorParameter::ML, FMEnvelopeParameter::ML4 },
		{ FMOperatorParameter::DT, FMEnvelopeParameter::DT4 },
		{ FMOperatorParameter::SSGEG, FMEnvelopeParameter::SSGEG4 }
	},
};

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

void extractADPCMSamples(const BinaryContainer& ctr, size_t addrPos, size_t sampOffs,
						 int maxCnt, std::vector<int>& ids,
						 std::vector<std::vector<uint8_t>>& samples)
{
	size_t ofs = 0;
	for (int i = 0; i < maxCnt; ++i) {
		uint16_t start = ctr.readUint16(addrPos);
		addrPos += 2;
		uint16_t stop = ctr.readUint16(addrPos);
		addrPos += 2;

		if (stop && start <= stop) {
			if (ids.empty()) ofs = start;
			ids.push_back(i);
			size_t st = sampOffs + static_cast<size_t>((start - ofs) << 5);
			size_t sampSize = std::min(static_cast<size_t>((stop + 1 - start) << 5), ctr.size() - st);
			samples.push_back(ctr.getSubcontainer(st, sampSize).toVector());
		}
	}
}
}
