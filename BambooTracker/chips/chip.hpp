#pragma once

#include "chip_def.h"
#include <cstdint>
#include <mutex>
#include "resampler.hpp"

namespace chip
{
	class Chip
	{
	public:
		// [rate]
		// 0 = auto-set mode (set internal chip rate)
		Chip(int id, int clock, int rate, int autoRate);
		virtual ~Chip();

		virtual void reset() = 0;
		virtual void setRegister(uint32_t offset, uint8_t value) = 0;
		virtual uint8_t getRegister(uint32_t offset) const = 0;

		virtual void setRate(int rate);
		int getRate() const;

		#ifdef SINC_INTERPOLATION
		void setMaxDuration(size_t maxDuration);
		#endif
		
		/*virtual void setVolume(float db) = 0;*/
		virtual void mix(int16_t* stream, size_t nSamples) = 0;

	protected:
		const int id_;
		std::mutex mutex_;

		int rate_;
		const int autoRate_;
		int internalRate_[2];

		/*float dB_[2];*/
		float volumeRatio_[2];
		/*static const int MAX_AMP_;*/

		sample* buffer_[2][2];
		Resampler resampler_[2];

		#ifdef SINC_INTERPOLATION
		void initResampler(size_t maxDuration);
		#else
		void initResampler();
		#endif

		void funcSetRate(int rate);
	};
}
