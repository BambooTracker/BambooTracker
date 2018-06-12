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

namespace chip
{
	size_t OPNA::count_ = 0;
	
	/*const int OPNA::DEF_AMP_FM_ = 11722;*/
	/*const int OPNA::DEF_AMP_PSG_ = 7250;*/

	#ifdef SINC_INTERPOLATION
	OPNA::OPNA(int clock, int rate, size_t maxDuration) :
	#else
	OPNA::OPNA(int clock, int rate) :
	#endif
		Chip(count_++, clock, rate, 110933)	// autoRate = 110933: FM internal rate
	{
		funcSetRate(rate);

		UINT8 EmuCore = 0;
		ym2608_set_ay_emu_core(EmuCore);

		UINT8 AYDisable = 0;	// Enable
		UINT8 AYFlags = 0;		// None
		internalRate_[FM] = device_start_ym2608(id_, clock, AYDisable, AYFlags, reinterpret_cast<int*>(&internalRate_[PSG]));

		// Init resampler
		#ifdef SINC_INTERPOLATION
		initResampler(maxDuration);
		#else
		initResampler();
		#endif

		setVolume(0, 0);

		reset();
	}

	OPNA::~OPNA()
	{
		device_stop_ym2608(id_);

		--count_;
	}

	void OPNA::reset()
	{
		std::lock_guard<std::mutex> lg(mutex_);

		device_reset_ym2608(id_);
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
	void OPNA::setVolume(float dBFM, float dBPSG)
	{
		std::lock_guard<std::mutex> lg(mutex_);

		/*dB_[FM] = dBFM;*/
		/*dB_[PSG] = dBFM;*/

		/*VolumeRatio_[FM] = maxAmplitude_ / defaultFMAmplitude_ * std::pow(10, fmdB / 20);
		VolumeRatio_[PSG] = maxAmplitude_ / defaultPSGAmplitude_ * std::pow(10, psgdB / 20);*/
		volumeRatio_[FM] = 0.25;
		volumeRatio_[PSG] = 0.25;
	}

	void OPNA::mix(int16_t* stream, size_t nSamples)
	{
		std::lock_guard<std::mutex> lg(mutex_);
		sample **bufFM, **bufPSG;

		// Set FM buffer
		if (internalRate_[FM] == rate_) {
			ym2608_stream_update(id_, buffer_[FM], nSamples);
			bufFM = buffer_[FM];
		}
		else {
			size_t intrSize = resampler_[FM].calculateInternalSampleSize(nSamples);
			ym2608_stream_update(id_, buffer_[FM], intrSize);
			bufFM = resampler_[FM].interpolate(buffer_[FM], nSamples, intrSize);
		}

		// Set PSG buffer
		if (internalRate_[PSG] == rate_) {
			ym2608_stream_update_ay(id_, buffer_[PSG], nSamples);
			bufPSG = buffer_[PSG];
		}
		else {
			size_t intrSize = resampler_[PSG].calculateInternalSampleSize(nSamples);
			ym2608_stream_update_ay(id_, buffer_[PSG], intrSize);
			bufPSG = resampler_[PSG].interpolate(buffer_[PSG], nSamples, intrSize);
		}
		for (size_t i = 0; i < nSamples; ++i) {
			for (int pan = LEFT; pan <= RIGHT; ++pan) {
				float s = volumeRatio_[FM] * bufFM[pan][i] + volumeRatio_[PSG] * bufPSG[pan][i];
				*stream++ = static_cast<int16_t>(clamp(s, -32768.0f, 32767.0f));
			}
		}
	}
}
