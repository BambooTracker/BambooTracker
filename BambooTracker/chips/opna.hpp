#pragma once

#include "chip.hpp"
#include <memory>
#include "chip_misc.hpp"
#include "scci/scci.hpp"
#include "scci/SCCIDefines.hpp"
#include "c86ctl/c86ctl_wrapper.hpp"

namespace chip
{
	class OPNA : public Chip
	{
	public:
		// [rate]
		// 0 = rate is 55466 (FM synthesis rate when clock is 3993600 * 2)
		OPNA(Emu emu, int clock, int rate, size_t maxDuration, size_t dramSize,
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
		void useSCCI(scci::SoundInterfaceManager* manager);
		bool isUsedSCCI() const;
		void useC86CTL(C86ctlBase* base);
		bool isUsedC86CTL() const;
		size_t getDRAMSize() const;

	private:
		static size_t count_;

		intf2608* intf_;

		// For SCCI
		scci::SoundInterfaceManager* scciManager_;
		scci::SoundChip* scciChip_;

		// For C86CTL
		std::unique_ptr<C86ctlBase> c86ctlBase_;
		std::unique_ptr<C86ctlRealChip> c86ctlRC_;
		std::unique_ptr<C86ctlGimic> c86ctlGm_;

		static constexpr double VOL_REDUC_ = 7.5;

		enum SoundSource : int
		{
			FM  = 0,
			SSG = 1
		};
	};
}
