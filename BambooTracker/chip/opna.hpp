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

#include "chip.hpp"
#include <memory>
#include "resampler.hpp"

class Scci;
class ScciGeneratorFunc;

class C86ctl;
class C86ctlGeneratorFunc;

namespace chip
{
enum class OpnaEmulator
{
	Mame,
	Nuked,
	First = Mame,
	Last = Nuked,
};

class OPNA final : public Chip
{
public:
	// [rate]
	// 0 = rate is 55466 (FM synthesis rate when clock is 3993600 * 2)
	OPNA(OpnaEmulator emu, int clock, int rate, size_t maxDuration, size_t dramSize,
		 std::unique_ptr<AbstractResampler> fmResampler = std::make_unique<LinearResampler>(),
		 std::unique_ptr<AbstractResampler> ssgResampler = std::make_unique<LinearResampler>(),
		 std::shared_ptr<AbstractRegisterWriteLogger> logger = nullptr);
	~OPNA() override;

	void reset() override;
	void setRegister(uint32_t offset, uint8_t value) override;
	uint8_t getRegister(uint32_t offset) const override;
	void setVolumeFM(double dB);
	void setVolumeSSG(double dB);
	size_t getDRAMSize() const noexcept;
	void mix(int16_t* stream, size_t nSamples) override;
	void setScci(ScciGeneratorFunc* f);
	bool isUsedScci() const noexcept;
	void setC86ctl(C86ctlGeneratorFunc* f);
	bool isUsedC86ctl() const noexcept;

private:
	static size_t count_;

	intf2608* intf_;
	size_t dramSize_;

	std::unique_ptr<Scci> scci_;
	std::unique_ptr<C86ctl> c86ctl_;
};
}
