#pragma once

#include "chip.hpp"
#include "chip_misc.h"
#include "scci/scci.h"
#include "scci/SCCIDefines.h"
#include "c86ctl/c86ctl.h"

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
		void useSCCI(scci::SoundInterfaceManager* manager);
		bool isUsedSCCI() const;
		void useC86CTL(c86ctl::IRealChipBase* base);
		bool isUsedC86CTL() const;

	private:
		static size_t count_;

		intf2608* intf_;

		// For SCCI
		scci::SoundInterfaceManager* scciManager_;
		scci::SoundChip* scciChip_;

		// For C86CTL
		c86ctl::IRealChipBase* c86ctlBase_;
		c86ctl::IRealChip2* c86ctlRC_;
		c86ctl::IGimic2* c86ctlGm_;

		static const double VOL_REDUC;

		enum SoundSource : int
		{
			FM  = 0,
			SSG = 1
		};
	};
}
