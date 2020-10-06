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

#include <cstdint>
#include <vector>
#include "binary_container.hpp"
#include "file_io_error.hpp"

class WavContainer
{
public:
	explicit WavContainer(size_t defCapacity = 0, uint32_t rate = 44100, uint16_t nCh = 2, uint16_t getBitSize = 16);
	explicit WavContainer(const BinaryContainer& bc);

	void setChannelCount(uint16_t n);
	uint16_t getChannelCount() const;
	void setBitSize(uint16_t size);
	uint16_t getBitSize() const;
	void setSampleRate(uint32_t rate);
	uint32_t getSampleRate() const;

	size_t getSampleCount() const;

	void storeSample(std::vector<int16_t> sample);
	void storeSample(BinaryContainer sample);
	BinaryContainer getSample() const;

	BinaryContainer createWavBinary();

//	static WavContainer* resample(const WavContainer* src, uint32_t rate);
//	static WavContainer* mono(const WavContainer* src);

private:
	uint16_t nCh_, bitSize_;
	uint32_t rate_;
	BinaryContainer buf_;

	static inline void assertFormat(bool f) { if (!f) throw FileInputError(FileIO::FileType::WAV); }
};
