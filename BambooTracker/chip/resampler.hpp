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

#include "chip_def.h"
#include <cmath>
#include <cstddef>

namespace chip
{
class AbstractResampler
{
public:
	AbstractResampler();
	virtual ~AbstractResampler();
	virtual void init(int srcRate, int destRate, size_t maxDuration);
	virtual void setDestributionRate(int destRate);
	virtual void setMaxDuration(size_t maxDuration) noexcept;
	virtual sample** interpolate(sample** src, size_t nSamples, size_t intrSize) = 0;

	inline size_t calculateInternalSampleSize(size_t nSamples)
	{
		return static_cast<size_t>(std::ceil(nSamples * rateRatio_));
	}

protected:
	int srcRate_, destRate_;
	size_t maxDuration_;
	float rateRatio_;
	sample* destBuf_[2];

	inline void updateRateRatio()
	{
		rateRatio_ = static_cast<float>(srcRate_) / destRate_;
	}
};


class LinearResampler : public AbstractResampler
{
public:
	sample** interpolate(sample** src, size_t nSamples, size_t intrSize) override;
};
}
