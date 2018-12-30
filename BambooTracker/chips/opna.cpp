#include "opna.hpp"
#include "chip_misc.h"

#ifdef  __cplusplus
extern "C"
{
#endif //  __cplusplus

#include "mame/2608intf.h"

#ifdef __cplusplus
}
#endif // __cplusplus
#include <QDebug>
namespace chip
{
	size_t OPNA::count_ = 0;
	
	/*const int OPNA::DEF_AMP_FM_ = 11722;*/
	/*const int OPNA::DEF_AMP_SSG_ = 7250;*/

	OPNA::OPNA(int clock, int rate, size_t maxDuration,
			   std::unique_ptr<AbstractResampler> fmResampler, std::unique_ptr<AbstractResampler> ssgResampler,
			   std::shared_ptr<ExportContainerInterface> exportContainer)
		: Chip(count_++, clock, rate, 110933, maxDuration,
			   std::move(fmResampler), std::move(ssgResampler),	// autoRate = 110933: FM internal rate
			   exportContainer),
		  scciManager_(nullptr),
		  scciChip_(nullptr)
	{
		funcSetRate(rate);

		UINT8 EmuCore = 0;
		ym2608_set_ay_emu_core(EmuCore);

		UINT8 AYDisable = 0;	// Enable
		UINT8 AYFlags = 0;		// None
		internalRate_[FM] = device_start_ym2608(id_, clock, AYDisable, AYFlags, reinterpret_cast<int*>(&internalRate_[SSG]));

		initResampler();

		setVolume(0, 0);

		reset();
	}

	OPNA::~OPNA()
	{
		device_stop_ym2608(id_);

		--count_;

		useSCCI(nullptr);
	}

	void OPNA::reset()
	{
		std::lock_guard<std::mutex> lg(mutex_);

		device_reset_ym2608(id_);

		if (scciChip_) scciChip_->init();
	}

	void OPNA::setRegister(uint32_t offset, uint8_t value)
	{
		std::lock_guard<std::mutex> lg(mutex_);

		if (offset & 0x100) {
			ym2608_control_port_b_w(id_, 2, offset & 0xff);
			ym2608_data_port_b_w(id_, 3, value & 0xff);
		}
		else
		{
			ym2608_control_port_a_w(id_, 0, offset & 0xff);
			ym2608_data_port_a_w(id_, 1, value & 0xff);
		}

		if (scciChip_) scciChip_->setRegister(offset, value);

		if (exCntr_) exCntr_->recordRegisterChange(offset, value);
	}

	uint8_t OPNA::getRegister(uint32_t offset) const
	{
		if (offset & 0x100) {
			ym2608_control_port_b_w(id_, 2, offset & 0xff);
		}
		else
		{
			ym2608_control_port_a_w(id_, 0, offset & 0xff);
		}
		return ym2608_read_port_r(id_, 1);
	}

	// TODO: Volume settings
	void OPNA::setVolume(float dBFM, float dBSSG)
	{
		std::lock_guard<std::mutex> lg(mutex_);

		/*dB_[FM] = dBFM;*/
		/*dB_[SSG] = dBFM;*/

		/*VolumeRatio_[FM] = maxAmplitude_ / defaultFMAmplitude_ * std::pow(10, fmdB / 20);
		VolumeRatio_[SSG] = maxAmplitude_ / defaultSSGAmplitude_ * std::pow(10, ssgdB / 20);*/
		volumeRatio_[FM] = 0.25;
		volumeRatio_[SSG] = 0.25;
	}

	void OPNA::mix(int16_t* stream, size_t nSamples)
	{
		std::lock_guard<std::mutex> lg(mutex_);
		sample **bufFM, **bufSSG;

		// Set FM buffer
		if (internalRate_[FM] == rate_) {
			ym2608_stream_update(id_, buffer_[FM], nSamples);
			bufFM = buffer_[FM];
		}
		else {
			size_t intrSize = resampler_[FM]->calculateInternalSampleSize(nSamples);
			ym2608_stream_update(id_, buffer_[FM], intrSize);
			bufFM = resampler_[FM]->interpolate(buffer_[FM], nSamples, intrSize);
		}

		// Set SSG buffer
		if (internalRate_[SSG] == rate_) {
			ym2608_stream_update_ay(id_, buffer_[SSG], nSamples);
			bufSSG = buffer_[SSG];
		}
		else {
			size_t intrSize = resampler_[SSG]->calculateInternalSampleSize(nSamples);
			ym2608_stream_update_ay(id_, buffer_[SSG], intrSize);
			bufSSG = resampler_[SSG]->interpolate(buffer_[SSG], nSamples, intrSize);
		}
		int16_t* p = stream;
		for (size_t i = 0; i < nSamples; ++i) {
			for (int pan = LEFT; pan <= RIGHT; ++pan) {
				float s = volumeRatio_[FM] * bufFM[pan][i] + volumeRatio_[SSG] * bufSSG[pan][i];
				*p++ = static_cast<int16_t>(clamp(s, -32768.0f, 32767.0f));
			}
		}

		if (exCntr_) exCntr_->recordStream(stream, nSamples);
	}

	void OPNA::useSCCI(SoundInterfaceManager* manager)
	{
		if (manager) {
			scciManager_ = manager;
			scciManager_->initializeInstance();
			scciManager_->reset();

			scciChip_ = scciManager_->getSoundChip(SC_TYPE_YM2608, SC_CLOCK_7987200);
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
}
