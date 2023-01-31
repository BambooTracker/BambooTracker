/*
 * Copyright (C) 2018-2022 Rerrah
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
#include <deque>
#include "resampler.hpp"
#include "2608_interface.hpp"
#include "real_chip_interface.hpp"

namespace chip
{
enum class OpnaEmulator
{
	Mame,
	Nuked,
	Ymfm,
	First = Mame,
	Last = Ymfm,
};

class OPNA final : public Chip
{
public:
	// [rate]
	// 0 = rate is 55466 (FM synthesis rate when clock is 3993600 * 2)
	OPNA(OpnaEmulator emu, int clock, int rate, size_t maxDuration, size_t dramSize,
		 std::unique_ptr<AbstractResampler> fmResampler = std::make_unique<BlipResampler>(),
		 std::unique_ptr<AbstractResampler> ssgResampler = std::make_unique<BlipResampler>(),
		 std::shared_ptr<AbstractRegisterWriteLogger> logger = nullptr);
	~OPNA() override;

	void setImmediateWriteMode(bool enabled) noexcept;
	bool isImmediateWriteMode() const noexcept;
	void setForcedWriteMode(bool enabled) noexcept { isForcedRegWrite_ = enabled; }
	void setRegister(uint32_t offset, uint8_t value) override;
	uint8_t getRegister(uint32_t offset) const override;
	void setVolumeFM(double dB);
	double getVolumeFM() const noexcept { return volumeFm_; }
	void setVolumeSSG(double dB);
	double getVolumeSSG() const noexcept { return volumeSsg_; }
	size_t getDRAMSize() const noexcept;
	void mix(int16_t* stream, size_t nSamples) override;

	void setFmResampler(std::unique_ptr<AbstractResampler> resampler);
	void setSsgResampler(std::unique_ptr<AbstractResampler> resampler);

	void connectToRealChip(RealChipInterfaceType type, RealChipInterfaceGeneratorFunc* f);
	RealChipInterfaceType getRealChipInterfaceType() const;
	bool hasConnectedToRealChip() const;

private:
	static size_t count_;

	std::unique_ptr<Ym2608Interface> intf_;
	double volumeFm_, volumeSsg_;
	constexpr static int VOLUME_RATIO_MOD_ = 2;
	size_t dramSize_;

	std::unique_ptr<SimpleRealChipInterface> rcIntf_;

	void resetSpecific() override;

	struct RegisterWrite
	{
		uint32_t address;
		uint8_t data;
		bool isPortA_;
	};

	bool isForcedRegWrite_;
	std::deque<RegisterWrite> regWrites_, forcedRegWrites_;

	void enqueueData(uint32_t offset, uint8_t value);
	void writeDataImmediately(uint32_t offset, uint8_t value);

	size_t dequeueData();

	size_t waitRestFm_, waitRestSsg2_;
	size_t rate2_;
	sample* tmpBuf_[2];
	void storeBufferForImmediate(size_t nSamples, size_t& pointFm, size_t& pointSsg);
	void storeBufferForWait(size_t nSamples, size_t& pointFm, size_t& pointSsg);
	void flushWait(size_t& pointFm, size_t maxFm, size_t& pointSsg2, size_t maxSsg2);

	struct WriteModeFuncs
	{
		void (OPNA::*setRegister)(uint32_t, uint8_t);
		void (OPNA::*storeBuffer)(size_t, size_t&, size_t&);
	} writeFuncs[2];
	WriteModeFuncs* writeFunc;
};
}
