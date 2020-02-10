#pragma once

#include "chip.hpp"
#include "chip_misc.h"
#include "scci/scci.h"
#include "scci/SCCIDefines.h"

namespace chip
{
	class OPNA : public Chip
	{
	public:
		// [rate]
		// 0 = rate is 55466 (FM synthesis rate when clock is 3993600 * 2)
		OPNA(Emu emu, int clock, int rate, size_t maxDuration,
			 std::unique_ptr<AbstractResampler> fmResampler = std::make_unique<LinearResampler>(),
			 std::unique_ptr<AbstractResampler> ssgResampler = std::make_unique<LinearResampler>(),
			 std::shared_ptr<ExportContainerInterface> exportContainer = nullptr);
		~OPNA() override;

		void reset() override;
		void setRegister(uint32_t offset, uint8_t value) override;
		uint8_t getRegister(uint32_t offset) const override;
		void setVolumeFM(double dB);
		void setVolumeSSG(double dB);
		void mix(int16_t* stream, size_t nSamples) override;
		void useSCCI(SoundInterfaceManager* manager);
		bool isUsedSCCI() const;
		size_t getDRAMSize() const;

	private:
		static size_t count_;

		intf2608* intf_;

		// For SCCI
		SoundInterfaceManager* scciManager_;
		SoundChip* scciChip_;

		static constexpr size_t DRAM_SIZE_ = 262144;	// 256KiB
		static constexpr double VOL_REDUC_ = 7.5;

		enum SoundSource : int
		{
			FM  = 0,
			SSG = 1
		};
	};
}
