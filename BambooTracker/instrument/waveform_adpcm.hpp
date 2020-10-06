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

#pragma once

#include <vector>
#include <cmath>
#include <cstdint>
#include "abstract_instrument_property.hpp"

class SampleADPCM : public AbstractInstrumentProperty
{
public:
	explicit SampleADPCM(int num);
	SampleADPCM(const SampleADPCM& other);

	friend bool operator==(const SampleADPCM& a, const SampleADPCM& b);
	friend bool operator!=(const SampleADPCM& a, const SampleADPCM& b) { return !(a == b); }

	std::unique_ptr<SampleADPCM> clone();

	void setRootKeyNumber(int n);
	int getRootKeyNumber() const;
	void setRootDeltaN(int dn);
	int getRootDeltaN() const;
	void setRepeatEnabled(bool enabled);
	bool isRepeatable() const;
	void storeSample(std::vector<uint8_t> sample);
	std::vector<uint8_t> getSamples() const;
	void clearSample();
	void setStartAddress(size_t addr);
	size_t getStartAddress() const;
	void setStopAddress(size_t addr);
	size_t getStopAddress() const;

	bool isEdited() const override;
	void clearParameters() override;

private:
	int rootKeyNum_, rootDeltaN_;
	bool isRepeated_;
	std::vector<uint8_t> sample_;
	size_t startAddress_, stopAddress_;

	static constexpr int DEF_RT_KEY_ = 60;	// C5
	static constexpr int DEF_RT_DELTAN_ = 0x49cd;	// 16000Hz
	static constexpr bool DEF_REPET_ = false;
};
