/*
 * Copyright (C) 2020 Rerrah
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

#include "envelope_fm.hpp"
#include "misc.hpp"

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
