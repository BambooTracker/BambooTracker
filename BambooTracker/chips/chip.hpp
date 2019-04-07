#pragma once

#include "chip_def.h"
#include <cstdint>
#include <memory>
#include <mutex>
#include "resampler.hpp"
#include "export_container.hpp"

namespace chip
{
	class Chip
	{
	public:
		// [rate]
		// 0 = auto-set mode (set internal chip rate)
		Chip(int id, int clock, int rate, int autoRate, size_t maxDuration,
			 std::unique_ptr<AbstractResampler> resampler1, std::unique_ptr<AbstractResampler> resampler2,
			 std::shared_ptr<ExportContainerInterface> exportContainer);
		virtual ~Chip();

		virtual void reset() = 0;
		virtual void setRegister(uint32_t offset, uint8_t value) = 0;
		virtual uint8_t getRegister(uint32_t offset) const = 0;

		virtual void setRate(int rate);
		int getRate() const;

		int getClock() const;

		void setMaxDuration(size_t maxDuration);
		size_t getMaxDuration() const;
		
		void setExportContainer(std::shared_ptr<ExportContainerInterface> cntr = nullptr);

		void setMasterVolume(int percentage);

		virtual void mix(int16_t* stream, size_t nSamples) = 0;

	protected:
		const int id_;
		std::mutex mutex_;

		int rate_, clock_;
		const int autoRate_;
		int internalRate_[2];
		size_t maxDuration_;

		double masterVolumeRatio_;
		double volumeRatio_[2];

		sample* buffer_[2][2];
		std::unique_ptr<AbstractResampler> resampler_[2];

		std::shared_ptr<ExportContainerInterface> exCntr_;

		void initResampler();

		void funcSetRate(int rate);
	};
}
