/*
 * Copyright (C) 2018-2023 Rerrah
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
#include <algorithm>
#include <iterator>
#include "./blip_buf/blip_buf.h"

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

void AbstractResampler::setMaxDuration(size_t maxDuration) noexcept
{
	maxDuration_ = maxDuration;
}

/****************************************/
sample** LinearResampler::interpolate(sample** src, size_t nSamples, size_t)
{
	if (srcRate_ == destRate_) return src;

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

/****************************************/
BlipResampler::BlipResampler(bool fast)
{
	addDelta = fast ? blip_add_delta_fast : blip_add_delta;

	for (int pan = STEREO_LEFT; pan <= STEREO_RIGHT; ++pan) {
		ch_[pan].blipBuf_ = blip_new(CHIP_SMPL_BUF_SIZE_);
	}
}

BlipResampler::~BlipResampler()
{
	for (int pan = STEREO_LEFT; pan <= STEREO_RIGHT; ++pan) {
		blip_delete(ch_[pan].blipBuf_);
	}
}

void BlipResampler::init(int srcRate, int destRate, size_t maxDuration)
{
	AbstractResampler::init(srcRate, destRate, maxDuration);

	for (int pan = STEREO_LEFT; pan <= STEREO_RIGHT; ++pan) {
		auto& ch = ch_[pan];
		blip_clear(ch.blipBuf_);
		blip_set_rates(ch.blipBuf_, srcRate, destRate);
		ch.prevSample_ = 0;
	}
}

void BlipResampler::reset()
{
	for (int pan = STEREO_LEFT; pan <= STEREO_RIGHT; ++pan) {
		blip_clear(ch_[pan].blipBuf_);
	}
}

size_t BlipResampler::calculateInternalSampleSize(size_t nSamples, bool& ok)
{
	if (srcRate_ == destRate_) {
		ok = true;
		return nSamples;
	}
	else {
		int clocks = blip_clocks_needed(ch_[0].blipBuf_, nSamples - blip_samples_avail(ch_[0].blipBuf_));
		ok = clocks >= 0;
		return clocks;
	}
}

sample** BlipResampler::interpolate(sample** src, size_t nSamples, size_t intrSize)
{
	if (srcRate_ == destRate_) return src;

	short tmpBuf[CHIP_SMPL_BUF_SIZE_];
	for (int pan = STEREO_LEFT; pan <= STEREO_RIGHT; ++pan) {
		auto& ch = ch_[pan];
		if (ch.blipBuf_ == nullptr) continue;

		auto srcBuf = src[pan];
		for (size_t i = 0; i < intrSize; ++i) {
			addDelta(ch.blipBuf_, i, srcBuf[i] - ch.prevSample_);
			ch.prevSample_ = srcBuf[i];
		}
		blip_end_frame(ch.blipBuf_, intrSize);

		size_t size = std::min<size_t>(nSamples, blip_samples_avail(ch.blipBuf_));
		blip_read_samples(ch.blipBuf_, tmpBuf, size, 0);
		std::copy_n(std::begin(tmpBuf), size, destBuf_[pan]);
	}

	return destBuf_;
}
}
