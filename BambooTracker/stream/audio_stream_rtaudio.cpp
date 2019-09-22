#include "audio_stream_rtaudio.hpp"
#include "RtAudio/RtAudio.h"

AudioStreamRtAudio::AudioStreamRtAudio(QObject* parent)
	: AudioStream(parent)
{
	audio_.reset(new RtAudio);
}

AudioStreamRtAudio::~AudioStreamRtAudio()
{
}

bool AudioStreamRtAudio::initialize(uint32_t rate, uint32_t duration, uint32_t intrRate, const QString& device)
{
	RtAudio* audio = audio_.get();
	const std::string deviceUtf8 = device.toStdString();

	shutdown();

	AudioStream::initialize(rate, duration, intrRate, device);

	RtAudio::StreamParameters param;
	param.nChannels = 2;
	param.deviceId = ~0u;
	for (unsigned int i = 0, n = audio->getDeviceCount(); i < n && param.deviceId == ~0u; ++i) {
		RtAudio::DeviceInfo info = audio->getDeviceInfo(i);
		if (info.outputChannels >= 2 && info.name == deviceUtf8)
			param.deviceId = i;
	}
	if (param.deviceId == ~0u)
		param.deviceId = audio->getDefaultOutputDevice();

	RtAudio::StreamOptions opts;
	opts.flags = RTAUDIO_SCHEDULE_REALTIME;
	opts.streamName = AUDIO_OUT_CLIENT_NAME;

	auto callback =
			+[](void* outputBuffer, void*, unsigned int nFrames,
			double, RtAudioStreamStatus, void* userData) -> int {
		auto stream = reinterpret_cast<AudioStreamRtAudio*>(userData);
		stream->generate(static_cast<int16_t*>(outputBuffer), nFrames);
		return 0;
	};

	unsigned int bufferSize = rate * duration / 1000;
	try {
		audio->openStream(&param, nullptr, RTAUDIO_SINT16, rate, &bufferSize, callback, this, &opts);
		return true;
	}
	catch (...) {
		return false;
	}
}

void AudioStreamRtAudio::shutdown()
{
	if (audio_->isStreamOpen())	audio_->closeStream();
}

std::vector<std::string> AudioStreamRtAudio::getAvailableDevices()
{
	RtAudio* audio = audio_.get();
	std::vector<std::string> devices;

	for (unsigned int i = 0, n = audio->getDeviceCount(); i < n; ++i) {
		RtAudio::DeviceInfo info = audio->getDeviceInfo(i);
		if (info.outputChannels >= 2)
			devices.push_back(info.name);
	}

	return devices;
}

void AudioStreamRtAudio::start()
{
	AudioStream::start();
	if (audio_->isStreamOpen()) audio_->startStream();
}

void AudioStreamRtAudio::stop()
{
	AudioStream::stop();
	if (audio_->isStreamOpen()) audio_->stopStream();
}
