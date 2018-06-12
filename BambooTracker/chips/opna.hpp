#pragma once

#include "chip.hpp"

namespace chip
{
	class OPNA : public Chip
	{
	public:
		// [rate]
		// 0 = rate is 110933 (internal FM sample rate in 3993600 * 2 clock)
		#ifdef SINC_INTERPOLATION
		OPNA(int clock, int rate, size_t maxDuration);
		#else
		OPNA(int clock, int rate);
		#endif
		~OPNA();

		void reset() override;
		void setRegister(uint32_t offset, uint8_t value) override;
		uint8_t getRegister(uint32_t offset) const override;
		void setVolume(float dBFM, float dBPSG);	// NOT work
		void mix(int16_t* stream, size_t nSamples) override;

	private:
		static size_t count_;

		/*static const int DEF_AMP_FM_, DEF_AMP_PSG_;*/

		enum SoundSource : int
		{
			FM  = 0,
			PSG = 1
		};
	};
}
