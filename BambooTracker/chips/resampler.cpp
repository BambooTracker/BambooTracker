#include "resampler.hpp"
#include "chip_misc.h"

namespace chip
{
	#ifdef SINC_INTERPOLATION
	const float Resampler::F_PI_ = 3.14159265f;
	const int Resampler::SINC_OFFSET_ = 16;
	#endif

	Resampler::Resampler()
	{
		for (int pan = LEFT; pan <= RIGHT; ++pan) {
			destBuf_[pan] = new sample[SMPL_BUF_SIZE_]();
		}
	}

	Resampler::~Resampler()
	{
		for (int pan = LEFT; pan <= RIGHT; ++pan) {
			delete[] destBuf_[pan];
		}
	}

	#ifdef SINC_INTERPOLATION
	void Resampler::init(int srcRate, int destRate, size_t maxDuration)
	#else
	void Resampler::init(int srcRate, int destRate)
	#endif
	{
		srcRate_ = srcRate;
		#ifdef SINC_INTERPOLATION
		maxDuration_ = maxDuration;
		#endif
		setDestRate(destRate);
	}

	void Resampler::setDestRate(int destRate)
	{
		destRate_ = destRate;
		rateRatio_ = static_cast<float>(srcRate_) / destRate;

		#ifdef SINC_INTERPOLATION
		initSincTables();
		#endif
	}

	#ifdef SINC_INTERPOLATION
	void Resampler::setMaxDuration(size_t maxDuration)
	{
		maxDuration_ = maxDuration;
		initSincTables();
	}

	void Resampler::initSincTables()
	{
		size_t maxSamples = destRate_ * maxDuration_ / 1000;

		if (srcRate_ != destRate_) {
			size_t intrSize = calculateInternalSampleSize(maxSamples);
			size_t offsetx2 = SINC_OFFSET_ << 1;
			sincTable_.resize(maxSamples * offsetx2);
			for (size_t n = 0; n < maxSamples; ++n) {
				size_t seg = n * offsetx2;
				float rcurn = n * rateRatio_;
				int curn = static_cast<int>(rcurn);
				int k = curn - SINC_OFFSET_;
				if (k < 0) k = 0;
				int end = curn + SINC_OFFSET_;
				if (static_cast<size_t>(end) > intrSize) end = static_cast<int>(intrSize);
				for (; k < end; ++k) {
					sincTable_[seg + SINC_OFFSET_ + (k - curn)] = sinc(F_PI_ * (rcurn - k));
				}
			}
		}
	}
	#endif

	sample** Resampler::interpolate(sample** src, size_t nSamples, size_t intrSize)
	{
		#ifdef SINC_INTERPOLATION
		// Sinc interpolation
		size_t offsetx2 = SINC_OFFSET_ << 1;
		
		for (int pan = LEFT; pan <= RIGHT; ++pan) {
			for (size_t n = 0; n < nSamples; ++n) {
				size_t seg = n * offsetx2;
				int curn = static_cast<int>(n * rateRatio_);
				int k = curn - SINC_OFFSET_;
				if (k < 0) k = 0;
				int end = curn + SINC_OFFSET_;
				if (static_cast<size_t>(end) > intrSize) end = static_cast<int>(intrSize);
				sample samp = 0;
				for (; k < end; ++k) {
					samp += static_cast<sample>(src[pan][k] * sincTable_[seg + SINC_OFFSET_ + (k - curn)]);
				}
				destBuf_[pan][n] = samp;
			}
		}
		#else
		// Linear interplation
		for (int pan = LEFT; pan <= RIGHT; ++pan) {
			for (size_t n = 0; n < nSamples; ++n) {
				float curnf = n * rateRatio_;
				int curni = static_cast<int>(curnf);
				float sub = curnf - curni;
				if (sub) {
					destBuf_[pan][n] = static_cast<sample>(src[pan][curni] + (src[pan][curni + 1] - src[pan][curni]) * sub);
				}
				else /* if (sub == 0) */ {
					destBuf_[pan][n] = src[pan][curni];
				}
			}
		}
		#endif

		return destBuf_;
	}
}
