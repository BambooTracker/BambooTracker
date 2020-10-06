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

#include "waveform_adpcm.hpp"

SampleADPCM::SampleADPCM(int num)
	: AbstractInstrumentProperty (num)
{
	clearParameters();
}

SampleADPCM::SampleADPCM(const SampleADPCM& other)
	: AbstractInstrumentProperty(other)
{
	rootKeyNum_ = other.rootKeyNum_;
	rootDeltaN_ = other.rootDeltaN_;
	isRepeated_ = other.isRepeated_;
	sample_ = other.sample_;
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

void SampleADPCM::setRootKeyNumber(int n)
{
	rootKeyNum_ = n;
}

int SampleADPCM::getRootKeyNumber() const
{
	return rootKeyNum_;
}

void SampleADPCM::setRootDeltaN(int dn)
{
	rootDeltaN_ = dn;
}

int SampleADPCM::getRootDeltaN() const
{
	return rootDeltaN_;
}

void SampleADPCM::setRepeatEnabled(bool enabled)
{
	isRepeated_ = enabled;
}

bool SampleADPCM::isRepeatable() const
{
	return isRepeated_;
}

void SampleADPCM::storeSample(std::vector<uint8_t> sample)
{
	sample_ = sample;
}

std::vector<uint8_t> SampleADPCM::getSamples() const
{
	return sample_;
}

void SampleADPCM::clearSample()
{
	startAddress_ = 0;
	stopAddress_ = 0;
	sample_ = std::vector<uint8_t>(1);
}

void SampleADPCM::setStartAddress(size_t addr)
{
	startAddress_ = addr;
}

size_t SampleADPCM::getStartAddress() const
{
	return startAddress_;
}

void SampleADPCM::setStopAddress(size_t addr)
{
	stopAddress_ = addr;
}

size_t SampleADPCM::getStopAddress() const
{
	return stopAddress_;
}

bool SampleADPCM::isEdited() const
{
	if (rootKeyNum_ != DEF_RT_KEY_
			|| rootDeltaN_ != DEF_RT_DELTAN_
			|| isRepeated_ != DEF_REPET_
			|| sample_.size() != 1
			|| sample_[0] != 0)
		return true;
	return false;
}

void SampleADPCM::clearParameters()
{
	rootKeyNum_ = DEF_RT_KEY_;
	rootDeltaN_ = DEF_RT_DELTAN_;
	isRepeated_ = DEF_REPET_;
	sample_ = std::vector<uint8_t>(1);
	startAddress_ = 0;
	stopAddress_ = 0;
}
