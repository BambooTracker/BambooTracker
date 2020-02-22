#include "wav_container.hpp"
#include <cmath>
#include <memory>
#include "misc.hpp"

WavContainer::WavContainer(size_t defCapacity, uint32_t rate, uint16_t nCh, uint16_t bitSize)
	: nCh_(nCh),
	  bitSize_(bitSize),
	  rate_(rate),
	  buf_(defCapacity)
{
}

WavContainer::WavContainer(const BinaryContainer& bc)
{
	size_t p = 0;
	assertFormat(bc.readString(p, 4) == "RIFF");
	p += 4;
	uint32_t chunkSize = bc.readUint32(p);
	p += 4;
	assertFormat(bc.readString(p, 4) == "WAVE");
	p += 4;

	assertFormat(bc.readString(p, 4) == "fmt ");
	p += 4;
	assertFormat(bc.readUint32(p) == 16);
	p += 4;
	assertFormat(bc.readUint16(p) == 1);	// Only support linear PCM
	p += 2;
	nCh_ = bc.readUint16(p);
	p += 2;
	rate_ = bc.readUint32(p);
	p += 4;
	uint32_t byteRate = bc.readUint32(p);
	p += 4;
	uint16_t blockSize = bc.readUint16(p);
	p += 2;
	bitSize_ = bc.readUint16(p);
	assertFormat(bitSize_ == 16);	// Only support 16-bit
	p += 2;
	assertFormat(byteRate == blockSize * rate_);
	assertFormat(blockSize == nCh_ * bitSize_ / 8);

	assertFormat(bc.readString(p, 4) == "data");
	p += 4;
	uint32_t dataSize = bc.readUint32(p);
	assertFormat(chunkSize == dataSize + 36);
	p += 4;
	assertFormat(p + dataSize <= bc.size());
	buf_.appendBinaryContainer(bc.getSubcontainer(p, dataSize));
}

void WavContainer::setChannelCount(uint16_t n)
{
	nCh_ = n;
}

uint16_t WavContainer::getChannelCount() const
{
	return nCh_;
}

void WavContainer::setBitSize(uint16_t size)
{
	bitSize_ = size;
}

uint16_t WavContainer::getBitSize() const
{
	return bitSize_;
}

void WavContainer::setSampleRate(uint32_t rate)
{
	rate_ = rate;
}

uint32_t WavContainer::getSampleRate() const
{
	return rate_;
}

size_t WavContainer::getSampleCount() const
{
	return buf_.size() * bitSize_ / 8 / nCh_;
}

void WavContainer::storeSample(std::vector<int16_t> sample)
{
	uint32_t dataSize = sample.size() * sizeof(int16_t);
	buf_.appendArray(reinterpret_cast<uint8_t*>(&sample[0]), dataSize);
}

void WavContainer::storeSample(BinaryContainer sample)
{
	buf_.appendBinaryContainer(sample);
}

BinaryContainer WavContainer::getSample() const
{
	return buf_;
}

BinaryContainer WavContainer::createWavBinary()
{
	BinaryContainer bc;

	// RIFF header
	bc.appendString("RIFF");
	uint16_t byteSize = bitSize_ / 8;
	uint32_t dataSize = buf_.size() / byteSize;
	uint32_t offset = dataSize + 36;
	bc.appendUint32(offset);
	bc.appendString("WAVE");

	// fmt chunk
	bc.appendString("fmt ");
	uint32_t chunkOfs = 16;
	bc.appendUint32(chunkOfs);
	uint16_t fmtId = 1;	// Raw linear PCM
	bc.appendUint16(fmtId);
	bc.appendUint16(nCh_);
	bc.appendUint32(rate_);
	uint16_t blockSize = byteSize * nCh_;
	uint32_t byteRate = blockSize * rate_;
	bc.appendUint32(byteRate);
	bc.appendUint16(blockSize);
	bc.appendUint16(bitSize_);

	// Data chunk
	bc.appendString("data");
	bc.appendUint32(dataSize);
	bc.appendBinaryContainer(buf_);

	return bc;
}

//WavContainer* WavContainer::resample(const WavContainer* src, uint32_t rate)
//{
//	std::unique_ptr<WavContainer> tgt
//			= std::make_unique<WavContainer>(0, rate, src->getChannelCount(), src->getBitSize());
//	assert(src->getBitSize() == 16);	// Only support int16_t

//	size_t nCh = src->getChannelCount();
//	size_t tsize = src->getSampleCount() * tgt->getSampleRate() / src->getSampleRate();
//	BinaryContainer tbc(tsize * 2 * nCh);
//	BinaryContainer sbc = src->getSample();
//	double r = static_cast<double>(src->getSampleRate()) / tgt->getSampleRate();

//	for (size_t n = 0; n < tsize; ++n) {
//		double curnf = n * r;
//		int curni = static_cast<int>(curnf);
//		double sub = curnf - curni;
//		for (size_t ch = 0; ch < nCh; ++ch) {
//			double a = sbc.readInt16((curni * nCh + ch) * 2);
//			if (sub == 0.) {
//				double b = sbc.readInt16(((curni + 1) * nCh + ch) * 2);
//				tbc.appendInt16(static_cast<int16_t>(std::round(a + (b - a) * sub)));
//			}
//			else {
//				tbc.appendInt16(static_cast<int16_t>(std::round(a)));
//			}
//		}
//	}

//	tgt->storeSample(tbc);
//	return tgt.release();
//}

//WavContainer* WavContainer::mono(const WavContainer* src)
//{
//	std::unique_ptr<WavContainer> tgt
//			= std::make_unique<WavContainer>(0, src->getSampleRate(), 1, src->getBitSize());
//	assert(src->getBitSize() == 16);	// Only support int16_t

//	BinaryContainer tbc;
//	BinaryContainer sbc = src->getSample();
//	uint16_t nCh = src->getChannelCount();
//	size_t size = src->getSampleCount();
//	for (size_t i = 0; i < size; ++i) {
//		int32_t v = 0;
//		for (size_t ch = 0; ch < nCh; ++ch) {
//			v += sbc.readInt16((i * nCh + ch) * 2);
//		}
//		tbc.writeInt16(i, static_cast<int16_t>(clamp(v, -32768, 32767)));
//	}
//	tgt->storeSample(tbc);

//	return tgt.release();
//}
