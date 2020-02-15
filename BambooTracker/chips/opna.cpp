#include "opna.hpp"
#include <cstdint>
#include <cmath>
#include "chip_misc.h"

#ifdef  __cplusplus
extern "C"
{
#endif //  __cplusplus

#include "mame/2608intf.h"
#include "nuked/nuke2608intf.h"

#ifdef __cplusplus
}
#endif // __cplusplus

namespace chip
{
	size_t OPNA::count_ = 0;
	
	const double OPNA::VOL_REDUC = 7.5;

	OPNA::OPNA(Emu emu, int clock, int rate, size_t maxDuration,
			   std::unique_ptr<AbstractResampler> fmResampler, std::unique_ptr<AbstractResampler> ssgResampler,
			   std::shared_ptr<ExportContainerInterface> exportContainer)
		: Chip(count_++, clock, rate, 110933, maxDuration,
			   std::move(fmResampler), std::move(ssgResampler),	// autoRate = 110933: FM internal rate
			   exportContainer),
		  scciManager_(nullptr),
		  scciChip_(nullptr),
		  c86ctlBase_(nullptr),
		  c86ctlRC_(nullptr),
		  c86ctlGm_(nullptr)
	{
		switch (emu) {
		default:
			fprintf(stderr, "Unknown emulator choice. Using the default.\n");
			/* fall through */
		case Emu::Mame:
			fprintf(stderr, "Using emulator: MAME YM2608\n");
			intf_ = &mame_intf2608;
			break;
		case Emu::Nuked:
			fprintf(stderr, "Using emulator: Nuked OPN-Mod\n");
			intf_ = &nuked_intf2608;
			break;
		}

		funcSetRate(rate);

		uint8_t EmuCore = 0;
		intf_->set_ay_emu_core(EmuCore);

		uint8_t AYDisable = 0;	// Enable
		uint8_t AYFlags = 0;		// None
		internalRate_[FM] = intf_->device_start(id_, clock, AYDisable, AYFlags, reinterpret_cast<int*>(&internalRate_[SSG]));

		initResampler();

		setVolumeFM(0);
		setVolumeSSG(0);

		reset();
	}

	OPNA::~OPNA()
	{
		intf_->device_stop(id_);

		--count_;

		useSCCI(nullptr);
		useC86CTL(nullptr);
	}

	void OPNA::reset()
	{
		std::lock_guard<std::mutex> lg(mutex_);

		intf_->device_reset(id_);

		if (scciChip_) scciChip_->init();
		if (c86ctlRC_) c86ctlRC_->resetChip();
	}

	void OPNA::setRegister(uint32_t offset, uint8_t value)
	{
		std::lock_guard<std::mutex> lg(mutex_);

		if (needSampleGen_) {
			if (offset & 0x100) {
				intf_->control_port_b_w(id_, 2, offset & 0xff);
				intf_->data_port_b_w(id_, 3, value & 0xff);
			}
			else
			{
				intf_->control_port_a_w(id_, 0, offset & 0xff);
				intf_->data_port_a_w(id_, 1, value & 0xff);
			}
		}

		if (scciChip_) scciChip_->setRegister(offset, value);
		if (c86ctlRC_) c86ctlRC_->out(offset, value);

		if (exCntr_) exCntr_->recordRegisterChange(offset, value);
	}

	uint8_t OPNA::getRegister(uint32_t offset) const
	{
		if (offset & 0x100) {
			intf_->control_port_b_w(id_, 2, offset & 0xff);
		}
		else
		{
			intf_->control_port_a_w(id_, 0, offset & 0xff);
		}
		return intf_->read_port_r(id_, 1);
	}


	void OPNA::setVolumeFM(double dB)
	{
		std::lock_guard<std::mutex> lg(mutex_);
		volumeRatio_[FM] = std::pow(10.0, (dB - VOL_REDUC) / 20.0);
	}

	void OPNA::setVolumeSSG(double dB)
	{
		std::lock_guard<std::mutex> lg(mutex_);
		volumeRatio_[SSG] = std::pow(10.0, (dB - VOL_REDUC) / 20.0);

		if (c86ctlGm_) {
			// NOTE: estimate SSG volume roughly
			uint8_t vol = static_cast<uint8_t>(std::round((dB < -3.0) ? (2.5 * dB + 45.5)
																	  : (7. * dB + 59.)));
			c86ctlGm_->setSSGVolume(vol);
		}
	}

	void OPNA::mix(int16_t* stream, size_t nSamples)
	{
		std::lock_guard<std::mutex> lg(mutex_);

		if (needSampleGen_) {
			sample **bufFM, **bufSSG;

			// Set FM buffer
			if (internalRate_[FM] == rate_) {
				intf_->stream_update(id_, buffer_[FM], nSamples);
				bufFM = buffer_[FM];
			}
			else {
				size_t intrSize = resampler_[FM]->calculateInternalSampleSize(nSamples);
				intf_->stream_update(id_, buffer_[FM], intrSize);
				bufFM = resampler_[FM]->interpolate(buffer_[FM], nSamples, intrSize);
			}

			// Set SSG buffer
			if (internalRate_[SSG] == rate_) {
				intf_->stream_update_ay(id_, buffer_[SSG], nSamples);
				bufSSG = buffer_[SSG];
			}
			else {
				size_t intrSize = resampler_[SSG]->calculateInternalSampleSize(nSamples);
				intf_->stream_update_ay(id_, buffer_[SSG], intrSize);
				bufSSG = resampler_[SSG]->interpolate(buffer_[SSG], nSamples, intrSize);
			}
			int16_t* p = stream;
			for (size_t i = 0; i < nSamples; ++i) {
				for (int pan = LEFT; pan <= RIGHT; ++pan) {
					double s = volumeRatio_[FM] * bufFM[pan][i] + volumeRatio_[SSG] * bufSSG[pan][i];
					*p++ = static_cast<int16_t>(clamp(s * masterVolumeRatio_, -32768.0, 32767.0));
				}
			}
		}

		if (exCntr_) exCntr_->recordStream(stream, nSamples);
	}

	void OPNA::useSCCI(scci::SoundInterfaceManager* manager)
	{
		if (manager) {
			scciManager_ = manager;
			scciManager_->initializeInstance();
			scciManager_->reset();
			scciChip_ = scciManager_->getSoundChip(scci::SC_TYPE_YM2608, scci::SC_CLOCK_7987200);
			if (!scciChip_) {
				scciManager_->releaseInstance();
				scciManager_ = nullptr;
			}
		}
		else {
			if (!scciChip_) return;
			scciManager_->releaseSoundChip(scciChip_);
			scciChip_ = nullptr;

			scciManager_->releaseInstance();
			scciManager_ = nullptr;
		}
	}

	bool OPNA::isUsedSCCI() const
	{
		return (scciManager_ != nullptr);
	}

	void OPNA::useC86CTL(C86ctlBase* base)
	{
		if (!base || base->isEmpty()) {
			if (!c86ctlBase_) return;
			c86ctlRC_->resetChip();
			c86ctlGm_.reset();
			c86ctlRC_.reset();
			c86ctlBase_->deinitialize();
		}
		else {
			c86ctlBase_.reset(base);
			c86ctlBase_->initialize();
			int nChip = c86ctlBase_->getNumberOfChip();
			for (int i = 0; i < nChip; ++i) {
				C86ctlRealChip* rc = c86ctlBase_->getChipInterface(i);
				if (rc) {
					c86ctlRC_.reset(rc);
					c86ctlRC_->resetChip();
					if (C86ctlGimic* gm = c86ctlRC_->queryInterface()) {
						c86ctlGm_.reset(gm);
						return;
					}
					c86ctlRC_.reset();
				}
			}
			base->deinitialize();
		}
		c86ctlBase_.reset();
	}

	bool OPNA::isUsedC86CTL() const
	{
		return (c86ctlBase_ != nullptr);
	}
}
