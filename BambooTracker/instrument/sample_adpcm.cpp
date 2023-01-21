/*
 * Copyright (C) 2020-2023 Rerrah
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

#include "sample_adpcm.hpp"
#include <algorithm>

namespace
{
constexpr int DEF_RT_DELTAN_ = 0x49cd;	// 16000Hz
constexpr bool DEF_REPET_ = false;
}

SampleADPCM::SampleADPCM(int num)
	: AbstractInstrumentProperty (num), repeatRange_(0, 0)
{
	clearParameters();
}

bool operator==(const SampleADPCM& a, const SampleADPCM& b) {
	return (a.rootKeyNum_ == b.rootKeyNum_ && a.rootDeltaN_ == b.rootDeltaN_
			&& a.isRepeated_ == b.isRepeated_ && a.sample_ == b.sample_);
}

std::unique_ptr<SampleADPCM> SampleADPCM::clone()
{
	std::unique_ptr<SampleADPCM> clone = std::make_unique<SampleADPCM>(*this);
	clone->clearUserInstruments();
	return clone;
}

void SampleADPCM::clearSample()
{
	startAddress_ = 0;
	stopAddress_ = 0;
	sample_ = std::vector<uint8_t>(1);
	repeatRange_ = SampleRepeatRange(0, (sample_.size() - 1) >> 5);	// By 32 bytes
}

bool SampleADPCM::isEdited() const
{
	if (rootKeyNum_ != DEF_ROOT_KEY
			|| rootDeltaN_ != DEF_RT_DELTAN_
			|| isRepeated_ != DEF_REPET_
			|| sample_.size() != 1
			|| sample_.front() != 0)
		return true;
	return false;
}

void SampleADPCM::clearParameters()
{
	clearSample();

	rootKeyNum_ = DEF_ROOT_KEY;
	rootDeltaN_ = DEF_RT_DELTAN_;
	isRepeated_ = DEF_REPET_;
}

bool SampleADPCM::setRepeatRange(const SampleRepeatRange& range) noexcept
{
	if (sample_.size() <= range.last()) {
		return false;
	}

	repeatRange_ = range;
	return true;
}

bool SampleADPCM::storeSample(const std::vector<uint8_t>& sample)
{
	if (sample.empty()) return false;

	repeatRange_ = repeatRange_.clampLast((sample.size() - 1) >> 5);	// By 32 bytes
	sample_ = sample;

	return true;
}

bool SampleADPCM::storeSample(std::vector<uint8_t>&& sample)
{
	if (sample.empty()) return false;

	repeatRange_ = repeatRange_.clampLast((sample.size() - 1) >> 5);	// By 32 bytes
	sample_ = std::move(sample);

	return true;
}
