/*
 * Copyright (C) 2020-2021 Rerrah
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

#include <vector>
#include <cstdint>
#include <memory>
#include <cmath>
#include "abstract_instrument_property.hpp"

class SampleADPCM final : public AbstractInstrumentProperty
{
public:
	explicit SampleADPCM(int num);

	friend bool operator==(const SampleADPCM& a, const SampleADPCM& b);
	friend bool operator!=(const SampleADPCM& a, const SampleADPCM& b) { return !(a == b); }

	std::unique_ptr<SampleADPCM> clone();

	inline void setRootKeyNumber(int n) noexcept { rootKeyNum_ = n; }
	inline int getRootKeyNumber() const noexcept {return rootKeyNum_; }
	inline void setRootDeltaN(int dn) noexcept { rootDeltaN_ = dn; }
	inline int getRootDeltaN() const noexcept { return rootDeltaN_; }
	inline void setRepeatEnabled(bool enabled) noexcept { isRepeated_ = enabled; }
	inline bool isRepeatable() const noexcept { return isRepeated_; }
	inline void storeSample(const std::vector<uint8_t>& sample) { sample_ = sample; }
	inline void storeSample(std::vector<uint8_t>&& sample) { sample_ = std::move(sample); }
	inline std::vector<uint8_t> getSamples() const { return sample_; }
	void clearSample();
	inline void setStartAddress(size_t addr) noexcept { startAddress_ = addr; }
	inline size_t getStartAddress() const noexcept { return startAddress_; }
	inline void setStopAddress(size_t addr) noexcept { stopAddress_ = addr; }
	inline size_t getStopAddress() const noexcept { return stopAddress_; }

	bool isEdited() const override;
	void clearParameters() override;

	static constexpr int DEF_ROOT_KEY = 60;	// C5

	inline static int calcADPCMDeltaN(unsigned int rate)
	{
		return static_cast<int>(std::round((rate << 16) / 55500.));
	}

private:
	int rootKeyNum_, rootDeltaN_;
	bool isRepeated_;
	std::vector<uint8_t> sample_;
	size_t startAddress_, stopAddress_;
};
