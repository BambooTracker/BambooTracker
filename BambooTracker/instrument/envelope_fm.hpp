/*
 * Copyright (C) 2018-2020 Rerrah
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
#include <memory>
#include "abstract_instrument_property.hpp"
#include "enum_hash.hpp"

enum class FMEnvelopeParameter;

class EnvelopeFM : public AbstractInstrumentProperty
{
public:
	explicit EnvelopeFM(int num);
	EnvelopeFM(const EnvelopeFM& other);

	friend bool operator==(const EnvelopeFM& a, const EnvelopeFM& b);
	friend bool operator!=(const EnvelopeFM& a, const EnvelopeFM& b) { return !(a == b); }

	std::unique_ptr<EnvelopeFM> clone();

	bool getOperatorEnabled(int num) const;
	void setOperatorEnabled(int num, bool enabled);

	int getParameterValue(FMEnvelopeParameter param) const;
	void setParameterValue(FMEnvelopeParameter param, int value);

	bool isEdited() const override;
	void clearParameters() override;

private:
	int al_;
	int fb_;
	struct FMOperator
	{
		bool enabled_;
		int ar_;
		int dr_;
		int sr_;
		int rr_;
		int sl_;
		int tl_;
		int ks_;
		int ml_;
		int dt_;
		int ssgeg_;	// -1: No use

		friend bool operator==(const FMOperator& a, const FMOperator& b) {
			return (a.enabled_ == b.enabled_ && a.ar_ == b.ar_ && a.dr_ == b.dr_
					&& a.sr_ == b.sr_ && a.rr_ == b.rr_ && a.sl_ == b.sl_ && a.tl_ == b.tl_
					&& a.ks_ == b.ks_ && a.ml_ == b.ml_ && a.dt_ == b.dt_ && a.ssgeg_ == b.ssgeg_);
		}
		friend bool operator!=(const FMOperator& a, const FMOperator& b) { return !(a == b); }
	};
	FMOperator op_[4];

	static constexpr int DEF_AL = 4;
	static constexpr int DEF_FB = 0;
	static constexpr FMOperator DEF_OP[4] = {
		{ true, 31, 0, 0, 7, 0, 32, 0, 0, 0, -1 },
		{ true, 31, 0, 0, 7, 0,  0, 0, 0, 0, -1 },
		{ true, 31, 0, 0, 7, 0, 32, 0, 0, 0, -1 },
		{ true, 31, 0, 0, 7, 0,  0, 0, 0, 0, -1 }
	};

	std::unordered_map<FMEnvelopeParameter, int&> paramMap_;

	void initParamMap();
};

enum class FMEnvelopeParameter
{
	AL, FB,
	AR1, DR1, SR1, RR1, SL1, TL1, KS1, ML1, DT1,
	AR2, DR2, SR2, RR2, SL2, TL2, KS2, ML2, DT2,
	AR3, DR3, SR3, RR3, SL3, TL3, KS3, ML3, DT3,
	AR4, DR4, SR4, RR4, SL4, TL4, KS4, ML4, DT4,
	SSGEG1, SSGEG2, SSGEG3, SSGEG4
};
