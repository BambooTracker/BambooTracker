#pragma once

#include "chip.hpp"

namespace chip
{
	class OPNA : public Chip
	{
	public:
		// [rate]
		// 0 = rate is 110933 (internal FM sample rate in 3993600 * 2 clock)
		OPNA(int clock, int rate, size_t maxDuration,
			 std::unique_ptr<AbstractResampler> fmResampler = std::make_unique<LinearResampler>(),
			 std::unique_ptr<AbstractResampler> ssgResampler = std::make_unique<LinearResampler>(),
			 std::shared_ptr<ExportContainerInterface> exportContainer = nullptr);
		~OPNA() override;

		void reset() override;
		void setRegister(uint32_t offset, uint8_t value) override;
		uint8_t getRegister(uint32_t offset) const override;
		void setVolume(float dBFM, float dBSSG);	// NOT work
		void mix(int16_t* stream, size_t nSamples) override;

	private:
		static size_t count_;

		/*static const int DEF_AMP_FM_, DEF_AMP_SSG_;*/

		enum SoundSource : int
		{
			FM  = 0,
			SSG = 1
		};
	};
}
