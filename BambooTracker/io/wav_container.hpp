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

#include <cstdint>
#include <vector>
#include "binary_container.hpp"

namespace io
{
class WavContainer
{
public:
	using value_type = BinaryContainer::value_type;
	using size_type = BinaryContainer::size_type;
	using iterator = BinaryContainer::iterator;
	using const_iterator = BinaryContainer::const_iterator;
	using reverse_iterator = BinaryContainer::reverse_iterator;
	using const_reverse_iterator = BinaryContainer::const_reverse_iterator;

	explicit WavContainer(uint32_t rate = 44100, uint16_t nCh = 2, uint16_t getBitSize = 16);
	explicit WavContainer(const BinaryContainer& bc);

	inline iterator begin() noexcept { return buf_.begin(); }
	inline const_iterator begin() const noexcept { return buf_.begin(); }

	inline iterator end() noexcept { return buf_.end(); }
	inline const_iterator end() const noexcept { return buf_.end(); }

	inline const_iterator cbegin() const noexcept { return buf_.cbegin(); }
	inline const_iterator cend() const noexcept { return buf_.cend(); }

	inline reverse_iterator rbegin() noexcept { return buf_.rbegin(); }
	inline const_reverse_iterator rbegin() const noexcept { return buf_.rbegin(); }

	inline reverse_iterator rend() noexcept { return buf_.rend(); }
	inline const_reverse_iterator rend() const noexcept { return buf_.rend(); }

	inline const_reverse_iterator crbegin() const noexcept { return buf_.crbegin(); }
	inline const_reverse_iterator crend() const noexcept { return buf_.crend(); }

	inline size_type size() const { return buf_.size(); }

	void setChannelCount(uint16_t n);
	inline uint16_t getChannelCount() const noexcept { return nCh_; }
	void setBitSize(uint16_t size);
	inline uint16_t getBitSize() const noexcept { return bitSize_; }
	void setSampleRate(uint32_t rate);
	inline uint32_t getSampleRate() const noexcept { return rate_; }

	size_type getSampleCount() const;

	void appendSample(const int16_t* sample, size_type nSamples);
	void appendSample(const std::vector<int16_t>& sample);
	void appendSample(const BinaryContainer& sample);
	BinaryContainer getSample() const noexcept;

	//		static WavContainer* resample(const WavContainer* src, uint32_t rate);
	//		static WavContainer* mono(const WavContainer* src);

private:
	uint16_t nCh_, bitSize_, blockSize_;
	uint32_t rate_, byteRate_;
	BinaryContainer buf_;

	void updateBlockSize();
	void updateByteRate();
	void updateSizeDataAfterAppendSample();
};
}
