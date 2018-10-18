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
			float f = nSamples * rateRatio_;
			size_t i = static_cast<size_t>(f);
			return ((f - i) ? (i + 1) : i);
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


	class SincResampler : public AbstractResampler
	{
	public:
		void init(int srcRate, int destRate, size_t maxDuration) override;
		void setDestributionRate(int destRate) override;
		void setMaxDuration(size_t maxDuration) override;
		sample** interpolate(sample** src, size_t nSamples, size_t intrSize) override;

	private:
		std::vector<float> sincTable_;

		static const float F_PI_;
		static const int SINC_OFFSET_;

		void initSincTables();

		static inline float sinc(float x)
		{
			return ((!x) ? 1.0f : (std::sin(x) / x));
		}
	};
}
