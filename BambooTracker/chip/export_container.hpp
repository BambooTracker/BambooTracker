/*
 * Copyright (C) 2018-2020 Rerrah
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
#include <cstddef>
#include <vector>

namespace chip
{
class ExportContainerInterface
{
public:
	virtual ~ExportContainerInterface();
	virtual bool isNeedSampleGeneration() const = 0;
	virtual void recordRegisterChange(uint32_t offset, uint8_t value) = 0;
	virtual void recordStream(int16_t* stream, size_t nSamples) = 0;
	virtual bool empty() const = 0;
	virtual void clear() = 0;
};

class WavExportContainer : public ExportContainerInterface
{
public:
	WavExportContainer();
	bool isNeedSampleGeneration() const override { return true; }
	void recordRegisterChange(uint32_t offset, uint8_t value) override;
	void recordStream(int16_t* stream, size_t nSamples) override;
	bool empty() const override;
	void clear() override;
	std::vector<int16_t> getStream() const;

private:
	std::vector<int16_t> samples_;
};

class VgmExportContainer : public ExportContainerInterface
{
public:
	VgmExportContainer(int target, uint32_t intrRate);
	bool isNeedSampleGeneration() const override { return false; }
	void recordRegisterChange(uint32_t offset, uint8_t value) override;
	void recordStream(int16_t* stream, size_t nSamples) override;
	void clear() override;
	bool empty() const override;
	std::vector<uint8_t> getData();
	size_t getSampleLength() const;
	size_t setLoopPoint();
	size_t forceMoveLoopPoint();
	void setDataBlock(std::vector<uint8_t> data);

private:
	std::vector<uint8_t> buf_;
	int target_;
	uint64_t lastWait_, totalSampCnt_;
	uint32_t intrRate_;
	bool isSetLoop_;
	uint32_t loopPoint_;

	void setWait();
};

class S98ExportContainer : public ExportContainerInterface
{
public:
	explicit S98ExportContainer(int target);
	bool isNeedSampleGeneration() const override { return false; }
	void recordRegisterChange(uint32_t offset, uint8_t value) override;
	void recordStream(int16_t* stream, size_t nSamples) override;
	void clear() override;
	bool empty() const override;
	std::vector<uint8_t> getData();
	size_t getSampleLength() const;
	size_t setLoopPoint();
	size_t forceMoveLoopPoint();

private:
	std::vector<uint8_t> buf_;
	int target_;
	uint64_t lastWait_, totalSampCnt_;
	bool isSetLoop_;
	uint32_t loopPoint_;

	void setWait();
};
}
