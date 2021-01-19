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

#pragma once

#include <unordered_map>
#include <bitset>
#include <memory>
#include "abstract_instrument_property.hpp"
#include "enum_hash.hpp"

enum class FMEnvelopeParameter
{
	AL, FB,
	AR1, DR1, SR1, RR1, SL1, TL1, KS1, ML1, DT1,
	AR2, DR2, SR2, RR2, SL2, TL2, KS2, ML2, DT2,
	AR3, DR3, SR3, RR3, SL3, TL3, KS3, ML3, DT3,
	AR4, DR4, SR4, RR4, SL4, TL4, KS4, ML4, DT4,
	SSGEG1, SSGEG2, SSGEG3, SSGEG4
};

class EnvelopeFM final : public AbstractInstrumentProperty
{
public:
	explicit EnvelopeFM(int num);

	friend bool operator==(const EnvelopeFM& a, const EnvelopeFM& b)
	{
		return (a.params_ == b.params_ && a.isEnabledOp_ == b.isEnabledOp_);
	}
	friend bool operator!=(const EnvelopeFM& a, const EnvelopeFM& b) { return !(a == b); }

	std::unique_ptr<EnvelopeFM> clone();

	bool getOperatorEnabled(int num) const;
	void setOperatorEnabled(int num, bool enabled);

	int getParameterValue(FMEnvelopeParameter param) const;
	void setParameterValue(FMEnvelopeParameter param, int value);

	bool isEdited() const override;
	void clearParameters() override;

private:
	std::unordered_map<FMEnvelopeParameter, int> params_;
	std::bitset<4> isEnabledOp_;
};
