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

	static inline void assert(bool f) { if (!f) throw FileInputError(FileIO::FileType::WAV); }
};
