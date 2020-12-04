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
class AbstractRegisterWriteLogger
{
public:
	explicit AbstractRegisterWriteLogger(int target);
	virtual ~AbstractRegisterWriteLogger() = default;
	virtual void recordRegisterChange(uint32_t offset, uint8_t value) = 0;
	void elapse(size_t count) noexcept;
	bool empty() const noexcept;
	void clear() noexcept;
	std::vector<uint8_t> getData();
	size_t getSampleLength() const noexcept;
	size_t setLoopPoint();
	size_t forceMoveLoopPoint() noexcept;

protected:
	int target_;
	std::vector<uint8_t> buf_;
	uint64_t lastWait_;
	bool isSetLoop_;
	uint32_t loopPoint_;

	virtual void setWait() = 0;

private:
	uint64_t totalSampCnt_;
};

class VgmLogger final : public AbstractRegisterWriteLogger
{
public:
	VgmLogger(int target, uint32_t intrRate);
	void recordRegisterChange(uint32_t offset, uint8_t value) override;
	void setDataBlock(std::vector<uint8_t> data);

private:
	uint32_t intrRate_;

	void setWait() override;
};

class S98Logger final : public AbstractRegisterWriteLogger
{
public:
	explicit S98Logger(int target);
	void recordRegisterChange(uint32_t offset, uint8_t value) override;

private:
	void setWait() override;
};
}
