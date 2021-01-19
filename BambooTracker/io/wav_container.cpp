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

#include "wav_container.hpp"
#include <cmath>
#include <memory>
#include "file_io_error.hpp"

namespace io
{
namespace
{
inline void assertValue(bool f, size_t pos)
{
	if (!f) throw io::FileCorruptionError(io::FileType::WAV, pos);
}

enum WavOffset : size_t
{
	RIFF_OFFS = 0,
	FILE_SIZE_OFFS = 4,
	WAVE_OFFS = 8,
	FMT_OFS = 12,
	FMT_SIZE_OFFS = 16,
	FORMAT_OFFS = 20,
	NCH_OFFS = 22,
	RATE_OFFS = 24,
	BYTE_RATE_OFFS = 28,
	BLOCK_SIZE_OFFS = 32,
	BIT_SIZE_OFFS = 34,
	DATA_OFFS = 36,
	DATA_SIZE_OFFS = 40,
	PREPARED_SIZE = 44
};
}

WavContainer::WavContainer(uint32_t rate, uint16_t nCh, uint16_t bitSize)
	: nCh_(nCh),
	  bitSize_(bitSize),
	  rate_(rate)
{
	// RIFF header
	buf_.appendString("RIFF");
	uint16_t byteSize = bitSize_ / 8;
	buf_.appendUint32(36);
	buf_.appendString("WAVE");

	// fmt chunk
	buf_.appendString("fmt ");
	uint32_t chunkOfs = 16;
	buf_.appendUint32(chunkOfs);
	uint16_t fmtId = 1;	// Raw linear PCM
	buf_.appendUint16(fmtId);
	buf_.appendUint16(nCh_);
	buf_.appendUint32(rate_);
	uint16_t blockSize = byteSize * nCh_;
	uint32_t byteRate = blockSize * rate_;
	buf_.appendUint32(byteRate);
	buf_.appendUint16(blockSize);
	buf_.appendUint16(bitSize_);

	// Data chunk
	buf_.appendString("data");
	buf_.appendUint32(0);
}

WavContainer::WavContainer(const BinaryContainer& bc)
{
	buf_.resize(PREPARED_SIZE);
	size_t p = 0;
	assertValue(bc.readString(p, 4) == "RIFF", p);
	buf_.writeString(RIFF_OFFS, "RIFF");
	p += 4;
	uint32_t fileSize = bc.readUint32(p) + 8;
	assertValue(fileSize == bc.size(), p);
	p += 4;
	assertValue(bc.readString(p, 4) == "WAVE", p);
	buf_.writeString(WAVE_OFFS, "WAVE");
	p += 4;

	while (p < fileSize) {
		std::string id = bc.readString(p, 4);
		p += 4;

		if (id == "fmt ") {
			buf_.writeString(FMT_OFS, "fmt ");
			uint32_t fmtSize = bc.readUint32(p);
			buf_.writeUint32(FMT_SIZE_OFFS, 16);
			size_t fmtp = p + 4;
			p = fmtp + fmtSize;
			assertValue(bc.readUint16(fmtp) == 1, fmtp);	// Only support linear PCM
			buf_.writeUint16(FORMAT_OFFS, 1);
			fmtp += 2;
			nCh_ = bc.readUint16(fmtp);
			buf_.writeUint16(NCH_OFFS, nCh_);
			fmtp += 2;
			rate_ = bc.readUint32(fmtp);
			buf_.writeUint32(RATE_OFFS, rate_);
			fmtp += 4;
			byteRate_ = bc.readUint32(fmtp);
			buf_.writeUint32(BYTE_RATE_OFFS, byteRate_);
			fmtp += 4;
			blockSize_ = bc.readUint16(fmtp);
			assertValue(byteRate_ == blockSize_ * rate_, fmtp);
			buf_.writeUint16(BLOCK_SIZE_OFFS, blockSize_);
			fmtp += 2;
			bitSize_ = bc.readUint16(fmtp);
			assertValue(bitSize_ == 16, fmtp);	// Only support 16-bit
			assertValue(blockSize_ == nCh_ * bitSize_ / 8, fmtp);
			buf_.writeUint16(BIT_SIZE_OFFS, bitSize_);
			/* fmtp += 2; */
		}
		else if (id == "data") {
			buf_.writeString(DATA_OFFS, "data");
			uint32_t dataSize = bc.readUint32(p);
			assertValue(p + dataSize <= bc.size(), p);
			buf_.writeUint32(DATA_SIZE_OFFS, dataSize);
			p += 4;
			buf_.appendBinaryContainer(bc.getSubcontainer(p, dataSize));
			p += dataSize;
		}
		else {
			p += (bc.readUint32(p) + 4);	// Jump to next chunk
		}
	}

	buf_.writeUint32(FILE_SIZE_OFFS, buf_.size() - 8);
}

void WavContainer::setChannelCount(uint16_t n)
{
	nCh_ = n;
	buf_.writeUint16(NCH_OFFS, nCh_);
	updateBlockSize();
}

void WavContainer::setBitSize(uint16_t size)
{
	bitSize_ = size;
	buf_.writeUint16(BIT_SIZE_OFFS, bitSize_);
	updateBlockSize();
}

void WavContainer::setSampleRate(uint32_t rate)
{
	rate_ = rate;
	buf_.writeUint16(rate, rate);
	updateBlockSize();
	updateByteRate();
}

WavContainer::size_type WavContainer::getSampleCount() const
{
	return (buf_.size() - PREPARED_SIZE) * bitSize_ / 8 / nCh_;
}

void WavContainer::appendSample(const int16_t* sample, size_type nSamples)
{
	size_t dataSize = nCh_ * nSamples * sizeof(int16_t);
	buf_.appendArray(reinterpret_cast<const uint8_t*>(sample), dataSize);
	updateSizeDataAfterAppendSample();
}

void WavContainer::appendSample(const std::vector<int16_t>& sample)
{
	size_t dataSize = sample.size() * sizeof(int16_t);
	buf_.appendArray(reinterpret_cast<const uint8_t*>(sample.data()), dataSize);
	updateSizeDataAfterAppendSample();
}

void WavContainer::appendSample(const BinaryContainer& sample)
{
	buf_.appendBinaryContainer(sample);
	updateSizeDataAfterAppendSample();
}

BinaryContainer WavContainer::getSample() const noexcept
{
	return buf_.getSubcontainer(PREPARED_SIZE, buf_.size() - PREPARED_SIZE);
}

//	WavContainer* WavContainer::resample(const WavContainer* src, uint32_t rate)
//	{
//		std::unique_ptr<WavContainer> tgt
//				= std::make_unique<WavContainer>(0, rate, src->getChannelCount(), src->getBitSize());
//		assert(src->getBitSize() == 16);	// Only support int16_t

//		size_t nCh = src->getChannelCount();
//		size_t tsize = src->getSampleCount() * tgt->getSampleRate() / src->getSampleRate();
//		BinaryContainer tbc(tsize * 2 * nCh);
//		BinaryContainer sbc = src->getSample();
//		double r = static_cast<double>(src->getSampleRate()) / tgt->getSampleRate();

//		for (size_t n = 0; n < tsize; ++n) {
//			double curnf = n * r;
//			int curni = static_cast<int>(curnf);
//			double sub = curnf - curni;
//			for (size_t ch = 0; ch < nCh; ++ch) {
//				double a = sbc.readInt16((curni * nCh + ch) * 2);
//				if (sub == 0.) {
//					double b = sbc.readInt16(((curni + 1) * nCh + ch) * 2);
//					tbc.appendInt16(static_cast<int16_t>(std::round(a + (b - a) * sub)));
//				}
//				else {
//					tbc.appendInt16(static_cast<int16_t>(std::round(a)));
//				}
//			}
//		}

//		tgt->storeSample(tbc);
//		return tgt.release();
//	}

//	WavContainer* WavContainer::mono(const WavContainer* src)
//	{
//		std::unique_ptr<WavContainer> tgt
//				= std::make_unique<WavContainer>(0, src->getSampleRate(), 1, src->getBitSize());
//		assert(src->getBitSize() == 16);	// Only support int16_t

//		BinaryContainer tbc;
//		BinaryContainer sbc = src->getSample();
//		uint16_t nCh = src->getChannelCount();
//		size_t size = src->getSampleCount();
//		for (size_t i = 0; i < size; ++i) {
//			int32_t v = 0;
//			for (size_t ch = 0; ch < nCh; ++ch) {
//				v += sbc.readInt16((i * nCh + ch) * 2);
//			}
//			tbc.writeInt16(i, static_cast<int16_t>(clamp(v, -32768, 32767)));
//		}
//		tgt->storeSample(tbc);

//		return tgt.release();
//	}

void WavContainer::updateBlockSize()
{
	blockSize_ = nCh_ * bitSize_ / 8;
	buf_.writeUint16(BLOCK_SIZE_OFFS, blockSize_);
}

void WavContainer::updateByteRate()
{
	byteRate_ = blockSize_ * rate_;
	buf_.writeUint32(BYTE_RATE_OFFS, byteRate_);
}

void WavContainer::updateSizeDataAfterAppendSample()
{
	buf_.writeUint32(FILE_SIZE_OFFS, buf_.size() - 8);
	buf_.writeUint32(DATA_SIZE_OFFS, buf_.size() - PREPARED_SIZE);
}
}
