#pragma once

#include "chip_def.h"
#include <vector>
#include <cmath>
#include <cstddef>

namespace chip
{
	class AbstractResampler
	{
	public:
		AbstractResampler();
		virtual ~AbstractResampler();
		virtual void init(int srcRate, int destRate, size_t maxDuration);
		virtual void setDestributionRate(int destRate);
		virtual void setMaxDuration(size_t maxDuration);
		virtual sample** interpolate(sample** src, size_t nSamples, size_t intrSize) = 0;

		inline size_t calculateInternalSampleSize(size_t nSamples)
		{
			return static_cast<size_t>(std::ceil(nSamples * rateRatio_));
		}

	protected:
		inline void updateRateRatio() {
			rateRatio_ = static_cast<float>(srcRate_) / destRate_;
		}

	protected:
		int srcRate_, destRate_;
		size_t maxDuration_;
		float rateRatio_;
		sample* destBuf_[2];
	};


	class LinearResampler : public AbstractResampler
	{
	public:
		sample** interpolate(sample** src, size_t nSamples, size_t intrSize) override;
	};
}
