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

#include "chip_def.h"
#include <cstdint>
#include <memory>
#include <mutex>

namespace chip
{
class AbstractResampler;
class AbstractRegisterWriteLogger;

class Chip
{
public:
	// [rate]
	// 0 = auto-set mode (set internal chip rate)
	Chip(int id, int clock, int rate, int autoRate, size_t maxDuration,
		 std::unique_ptr<AbstractResampler> resampler1, std::unique_ptr<AbstractResampler> resampler2,
		 std::shared_ptr<AbstractRegisterWriteLogger> logger);
	virtual ~Chip();

	virtual void reset() = 0;
	virtual void setRegister(uint32_t offset, uint8_t value) = 0;
	virtual uint8_t getRegister(uint32_t offset) const = 0;

	virtual void setRate(int rate);
	int getRate() const noexcept;

	int getClock() const noexcept;

	void setMaxDuration(size_t maxDuration);
	size_t getMaxDuration() const noexcept;

	void setRegisterWriteLogger(std::shared_ptr<AbstractRegisterWriteLogger> logger = nullptr);

	void setMasterVolume(int percentage);

	virtual void mix(int16_t* stream, size_t nSamples) = 0;

protected:
	const int id_;
	std::mutex mutex_;

	int rate_, clock_;
	const int autoRate_;
	int internalRate_[2];
	size_t maxDuration_;

	double masterVolumeRatio_;
	double volumeRatio_[2];

	sample* buffer_[2][2];
	std::unique_ptr<AbstractResampler> resampler_[2];

	std::shared_ptr<AbstractRegisterWriteLogger> logger_;

	void initResampler();

	void funcSetRate(int rate) noexcept;
};
}
