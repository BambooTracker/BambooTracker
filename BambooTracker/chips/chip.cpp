#include "chip.hpp"
#include <utility>
#include "chip_misc.h"

#ifdef  __cplusplus
extern "C"
{
#endif //  __cplusplus

	#include "mame/mamedef.h"

	UINT8 CHIP_SAMPLING_MODE = 0x00;
	INT32 CHIP_SAMPLE_RATE;
	stream_sample_t* DUMMYBUF[] = { nullptr, nullptr };

#ifdef __cplusplus
}
#endif // __cplusplus

namespace chip
{
	Chip::Chip(int id, int clock, int rate, int autoRate, size_t maxDuration,
			   std::unique_ptr<AbstractResampler> resampler1, std::unique_ptr<AbstractResampler> resampler2,
			   std::shared_ptr<ExportContainerInterface> exportContainer)
		: id_(id),
		  rate_(rate),	// Dummy set
		  clock_(clock),
		  autoRate_(autoRate),
		  maxDuration_(maxDuration),
		  masterVolumeRatio_(100),
		  exCntr_(exportContainer)
	{
		resampler_[0] = std::move(resampler1);
		resampler_[1] = std::move(resampler2);

		for (int pan = LEFT; pan <= RIGHT; ++pan) {
			for (auto& buf : buffer_) {
				buf[pan] = new stream_sample_t[SMPL_BUF_SIZE_];
			}
		}
	}

	Chip::~Chip()
	{
		for (int pan = LEFT; pan <= RIGHT; ++pan) {
			for (auto& buf : buffer_) {
				delete[] buf[pan];
			}
		}
	}

	void Chip::initResampler()
	{
		for (int snd = 0; snd < 2; ++snd) {
			resampler_[snd]->init(internalRate_[snd], rate_, maxDuration_);
		}
	}

	void Chip::setRate(int rate)
	{
		std::lock_guard<std::mutex> lg(mutex_);

		funcSetRate(rate);

		for (auto& rsmp : resampler_) {
			rsmp->setDestributionRate(rate);
		}
	}

	void Chip::funcSetRate(int rate)
	{
		rate_ = CHIP_SAMPLE_RATE = ((rate) ? rate : autoRate_);
	}

	int Chip::getClock() const
	{
		return clock_;
	}

	int Chip::getRate() const
	{
		return rate_;
	}

	void Chip::setMaxDuration(size_t maxDuration)
	{
		maxDuration_ = maxDuration;
		for (int snd = 0; snd < 2; ++snd) {
			resampler_[snd]->setMaxDuration(maxDuration);
		}
	}

	size_t Chip::getMaxDuration() const
	{
		return maxDuration_;
	}

	void Chip::setExportContainer(std::shared_ptr<ExportContainerInterface> cntr)
	{
		exCntr_ = cntr;
	}

	void Chip::setMasterVolume(int percentage)
	{
		masterVolumeRatio_ = percentage / 100.0;
	}
}
