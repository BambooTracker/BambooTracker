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

#include "io_utils.hpp"
#include <stdexcept>
#include "binary_container.hpp"
#include "instrument/envelope_fm.hpp"
#include "instrument/instrument_property_defs.hpp"

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
}
