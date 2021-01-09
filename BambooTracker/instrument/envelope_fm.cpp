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

#include "envelope_fm.hpp"

namespace
{
const std::unordered_map<FMEnvelopeParameter, int> DEF_PARAMS = {
	{ FMEnvelopeParameter::AL, 4 },
	{ FMEnvelopeParameter::FB, 0 },
	{ FMEnvelopeParameter::AR1, 31 },
	{ FMEnvelopeParameter::DR1, 0 },
	{ FMEnvelopeParameter::SR1, 0 },
	{ FMEnvelopeParameter::RR1, 7 },
	{ FMEnvelopeParameter::SL1, 0 },
	{ FMEnvelopeParameter::TL1, 32 },
	{ FMEnvelopeParameter::KS1, 0 },
	{ FMEnvelopeParameter::ML1, 0 },
	{ FMEnvelopeParameter::DT1, 0 },
	{ FMEnvelopeParameter::SSGEG1, -1 },
	{ FMEnvelopeParameter::AR2, 31 },
	{ FMEnvelopeParameter::DR2, 0 },
	{ FMEnvelopeParameter::SR2, 0 },
	{ FMEnvelopeParameter::RR2, 7 },
	{ FMEnvelopeParameter::SL2, 0 },
	{ FMEnvelopeParameter::TL2, 0 },
	{ FMEnvelopeParameter::KS2, 0 },
	{ FMEnvelopeParameter::ML2, 0 },
	{ FMEnvelopeParameter::DT2, 0 },
	{ FMEnvelopeParameter::SSGEG2, -1 },
	{ FMEnvelopeParameter::AR3, 31 },
	{ FMEnvelopeParameter::DR3, 0 },
	{ FMEnvelopeParameter::SR3, 0 },
	{ FMEnvelopeParameter::RR3, 7 },
	{ FMEnvelopeParameter::SL3, 0 },
	{ FMEnvelopeParameter::TL3, 32 },
	{ FMEnvelopeParameter::KS3, 0 },
	{ FMEnvelopeParameter::ML3, 0 },
	{ FMEnvelopeParameter::DT3, 0 },
	{ FMEnvelopeParameter::SSGEG3, -1 },
	{ FMEnvelopeParameter::AR4, 31 },
	{ FMEnvelopeParameter::DR4, 0 },
	{ FMEnvelopeParameter::SR4, 0 },
	{ FMEnvelopeParameter::RR4, 7 },
	{ FMEnvelopeParameter::SL4, 0 },
	{ FMEnvelopeParameter::TL4, 0 },
	{ FMEnvelopeParameter::KS4, 0 },
	{ FMEnvelopeParameter::ML4, 0 },
	{ FMEnvelopeParameter::DT4, 0 },
	{ FMEnvelopeParameter::SSGEG4, -1 }
};
}

EnvelopeFM::EnvelopeFM(int num)
	: AbstractInstrumentProperty(num)
{
	clearParameters();
}

std::unique_ptr<EnvelopeFM> EnvelopeFM::clone()
{
	std::unique_ptr<EnvelopeFM> clone = std::make_unique<EnvelopeFM>(*this);
	clone->clearUserInstruments();
	return clone;
}

bool EnvelopeFM::getOperatorEnabled(int num) const
{
	return isEnabledOp_.test(num);
}

void EnvelopeFM::setOperatorEnabled(int num, bool enabled)
{
	isEnabledOp_.set(num, enabled);
}

int EnvelopeFM::getParameterValue(FMEnvelopeParameter param) const
{
	return params_.at(param);
}

void EnvelopeFM::setParameterValue(FMEnvelopeParameter param, int value)
{
	params_.at(param) = value;
}

bool EnvelopeFM::isEdited() const
{
	return (params_ != DEF_PARAMS || !isEnabledOp_.all());
}

void EnvelopeFM::clearParameters()
{
	params_ = DEF_PARAMS;
	isEnabledOp_.set();
}
