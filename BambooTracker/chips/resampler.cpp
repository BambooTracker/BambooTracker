#include "resampler.hpp"
#include "chip_misc.hpp"

namespace chip
{
	AbstractResampler::AbstractResampler()
	{
		for (int pan = LEFT; pan <= RIGHT; ++pan) {
			destBuf_[pan] = new sample[SMPL_BUF_SIZE_]();
		}
	}

	AbstractResampler::~AbstractResampler()
	{
		for (int pan = LEFT; pan <= RIGHT; ++pan) {
			delete[] destBuf_[pan];
		}
	}

	void AbstractResampler::init(int srcRate, int destRate, size_t maxDuration)
	{
		srcRate_ = srcRate;
		maxDuration_ = maxDuration;
		destRate_ = destRate;
		updateRateRatio();
	}

	void AbstractResampler::setDestributionRate(int destRate)
	{
		destRate_ = destRate;
		updateRateRatio();
	}

	void AbstractResampler::setMaxDuration(size_t maxDuration)
	{
		maxDuration_ = maxDuration;
	}

	/****************************************/
	sample** LinearResampler::interpolate(sample** src, size_t nSamples, size_t intrSize)
	{
		(void)intrSize;

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

		return destBuf_;
	}
}
