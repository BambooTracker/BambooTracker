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

#include "opna.hpp"
#include <cstdint>
#include <cmath>
#include <algorithm>
#include "register_write_logger.hpp"
#include "mame/mame_2608.hpp"
#include "nuked/nuked_2608.hpp"
#include "ymfm/ymfm_2608.hpp"

#ifdef USE_REAL_CHIP
#include "scci/scci_wrapper.hpp"
#include "c86ctl/c86ctl_wrapper.hpp"
#endif

namespace chip
{
namespace
{
constexpr double VOL_REDUC_ = 7.5;

// 55466Hz: FM internal rate
constexpr int DEFAULT_AUTO_RATE = 55466;

enum SoundSourceIndex : int { FM = 0, SSG = 1 };

inline double clamp(double value, double low, double high)
{
	return std::min<double>(std::max<double>(value, low), high);
}

void gainSamples(sample** samples, size_t nSamples, double gain)
{
	for (int pan = STEREO_LEFT; pan <= STEREO_RIGHT; ++pan) {
		std::transform(samples[pan], samples[pan] + nSamples, samples[pan],
					   [gain](sample s) { return static_cast<sample>(s * gain); });
	}
}
}

size_t OPNA::count_ = 0;

OPNA::OPNA(OpnaEmulator emu, int clock, int rate, size_t maxDuration, size_t dramSize,
		   std::unique_ptr<AbstractResampler> fmResampler, std::unique_ptr<AbstractResampler> ssgResampler,
		   std::shared_ptr<AbstractRegisterWriteLogger> logger)
	: Chip(count_++, clock, rate, DEFAULT_AUTO_RATE, maxDuration,
		   std::move(fmResampler), std::move(ssgResampler),
		   logger),
	  dramSize_(dramSize),
	  rcIntf_(std::make_unique<SimpleRealChipInterface>())
{
	switch (emu) {
	default:
		fprintf(stderr, "Unknown emulator choice. Using the default.\n");
		/* fall through */
	case OpnaEmulator::Mame:
		fprintf(stderr, "Using emulator: MAME YM2608\n");
		intf_ = std::make_unique<Mame2608>();
		break;
	case OpnaEmulator::Nuked:
		fprintf(stderr, "Using emulator: Nuked OPN-Mod\n");
		intf_ = std::make_unique<Nuked2608>();
		break;
	case OpnaEmulator::Ymfm:
		fprintf(stderr, "Using emulator: ymfm\n");
		intf_ = std::make_unique<Ymfm2608>();
		break;
	}

	funcSetRate(rate);

	internalRate_[FM] = intf_->startDevice(clock, internalRate_[SSG], dramSize);

	initResampler();

	setVolumeFM(0);
	setVolumeSSG(0);

	reset();
}

OPNA::~OPNA()
{
	intf_->stopDevice();
	--count_;
}

void OPNA::resetSpecific()
{
	intf_->resetDevice();
	rcIntf_->reset();
}

void OPNA::setRegister(uint32_t offset, uint8_t value)
{
	std::lock_guard<std::mutex> lg(mutex_);

	if (logger_) {
		logger_->recordRegisterChange(offset, value);
	}
	else {
		if (offset & 0x100) {
			intf_->writeAddressToPortB(offset & 0xff);
			intf_->writeDataToPortB(value & 0xff);
		}
		else {
			intf_->writeAddressToPortA(offset & 0xff);
			intf_->writeDataToPortA(value & 0xff);
		}
	}

	rcIntf_->setRegister(offset, value);
}

uint8_t OPNA::getRegister(uint32_t offset) const
{
	if (offset & 0x100) intf_->writeAddressToPortB(offset & 0xff);
	else intf_->writeAddressToPortA(offset & 0xff);
	return intf_->readData();
}


void OPNA::setVolumeFM(double dB)
{
	std::lock_guard<std::mutex> lg(mutex_);
	busVolumeRatio_[FM] = std::pow(10.0, (dB - VOL_REDUC_) / 20.0);
	updateVolumeRatio(FM);
}

void OPNA::setVolumeSSG(double dB)
{
	std::lock_guard<std::mutex> lg(mutex_);
	busVolumeRatio_[SSG] = std::pow(10.0, (dB - VOL_REDUC_) / 20.0);
	updateVolumeRatio(SSG);

	rcIntf_->setSSGVolume(dB);
}

size_t OPNA::getDRAMSize() const noexcept
{
	return dramSize_;
}

void OPNA::mix(int16_t* stream, size_t nSamples)
{
	std::lock_guard<std::mutex> lg(mutex_);

	sample **bufFM, **bufSSG;

	// Set FM buffer
	if (internalRate_[FM] == rate_) {
		intf_->updateStream(buffer_[FM], nSamples);
		gainSamples(buffer_[FM], nSamples, volumeRatio_[FM]);
		bufFM = buffer_[FM];
	}
	else {
		size_t intrSize = resampler_[FM]->calculateInternalSampleSize(nSamples);
		intf_->updateStream(buffer_[FM], intrSize);
		gainSamples(buffer_[FM], intrSize, volumeRatio_[FM]);
		bufFM = resampler_[FM]->interpolate(buffer_[FM], nSamples, intrSize);
	}

	// Set SSG buffer
	if (internalRate_[SSG] == rate_) {
		intf_->updateSsgStream(buffer_[SSG], nSamples);
		gainSamples(buffer_[SSG], nSamples, volumeRatio_[SSG]);
		bufSSG = buffer_[SSG];
	}
	else {
		size_t intrSize = resampler_[SSG]->calculateInternalSampleSize(nSamples);
		intf_->updateSsgStream(buffer_[SSG], intrSize);
		gainSamples(buffer_[SSG], intrSize, volumeRatio_[SSG]);
		bufSSG = resampler_[SSG]->interpolate(buffer_[SSG], nSamples, intrSize);
	}

	int16_t* p = stream;
	for (size_t i = 0; i < nSamples; ++i) {
		for (int pan = STEREO_LEFT; pan <= STEREO_RIGHT; ++pan) {
			*p++ = static_cast<int16_t>(clamp(bufFM[pan][i] + bufSSG[pan][i], -32768, 32767));
		}
	}
}

void OPNA::setFmResampler(std::unique_ptr<AbstractResampler> resampler)
{
	std::lock_guard<std::mutex> lg(mutex_);
	resampler_[FM] = std::move(resampler);
	initResampler();
}

void OPNA::setSsgResampler(std::unique_ptr<AbstractResampler> resampler)
{
	std::lock_guard<std::mutex> lg(mutex_);
	resampler_[SSG] = std::move(resampler);
	initResampler();
}

void OPNA::connectToRealChip(RealChipInterfaceType type, RealChipInterfaceGeneratorFunc* f)
{
	switch (type) {
	default:	// Fall through
	case RealChipInterfaceType::NONE:
		if (rcIntf_->getType() != RealChipInterfaceType::NONE)
			rcIntf_ = std::make_unique<SimpleRealChipInterface>();
		rcIntf_->createInstance(f);
		break;
#ifdef USE_REAL_CHIP
	case RealChipInterfaceType::SCCI:
		if (rcIntf_->getType() != RealChipInterfaceType::SCCI)
			rcIntf_ = std::make_unique<Scci>();
		rcIntf_->createInstance(f);
		break;
	case RealChipInterfaceType::C86CTL:
		if (rcIntf_->getType() != RealChipInterfaceType::C86CTL)
			rcIntf_ = std::make_unique<C86ctl>();
		rcIntf_->createInstance(f);
		break;
#endif
	}
}

RealChipInterfaceType OPNA::getRealChipInterfaceType() const
{
	return rcIntf_->getType();
}

bool OPNA::hasConnectedToRealChip() const
{
	return rcIntf_->hasConnected();
}
}
