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

#include "resampler.hpp"

namespace chip
{
AbstractResampler::AbstractResampler()
{
	for (int pan = STEREO_LEFT; pan <= STEREO_RIGHT; ++pan) {
		destBuf_[pan] = new sample[CHIP_SMPL_BUF_SIZE_]();
	}
}

AbstractResampler::~AbstractResampler()
{
	for (int pan = STEREO_LEFT; pan <= STEREO_RIGHT; ++pan) {
		delete[] destBuf_[pan];
	}
}

void AbstractResampler::init(int srcRate, int destRate, size_t maxDuration)
{
	srcRate_ = srcRate;
	maxDuration_ = maxDuration;
	destRate_ = destRate;
	updateRateRatio();
}

void AbstractResampler::setDestributionRate(int destRate)
{
	destRate_ = destRate;
	updateRateRatio();
}

void AbstractResampler::setMaxDuration(size_t maxDuration) noexcept
{
	maxDuration_ = maxDuration;
}

/****************************************/
sample** LinearResampler::interpolate(sample** src, size_t nSamples, size_t intrSize)
{
	(void)intrSize;

	// Linear interplation
	for (int pan = STEREO_LEFT; pan <= STEREO_RIGHT; ++pan) {
		for (size_t n = 0; n < nSamples; ++n) {
			float curnf = n * rateRatio_;
			int curni = static_cast<int>(curnf);
			float sub = curnf - curni;
			if (sub) {
				destBuf_[pan][n] = static_cast<sample>(src[pan][curni] + (src[pan][curni + 1] - src[pan][curni]) * sub);
			}
			else /* if (sub == 0) */ {
				destBuf_[pan][n] = src[pan][curni];
			}
		}
	}

	return destBuf_;
}
}
