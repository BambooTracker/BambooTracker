#include "audio_stream_rtaudio.hpp"
#include <string>
#include <vector>
#include "RtAudio/RtAudio.hpp"

AudioStreamRtAudio::AudioStreamRtAudio(QObject* parent)
	: AudioStream(parent)
{
	audio_.reset(new RtAudio);
}

AudioStreamRtAudio::~AudioStreamRtAudio()
{
}

bool AudioStreamRtAudio::initialize(uint32_t rate, uint32_t duration, uint32_t intrRate, const QString& backend, const QString& device)
{
	shutdown();

	setBackend(backend);

	AudioStream::initialize(rate, duration, intrRate, backend, device);

	RtAudio* audio = audio_.get();
	const std::string deviceUtf8 = device.toStdString();

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
	catch (RtAudioError& error) {
		error.printMessage();
		return false;
	}
}

void AudioStreamRtAudio::shutdown()
{
	if (audio_->isStreamOpen())	audio_->closeStream();
}

void AudioStreamRtAudio::setBackend(const QString& backend)
{
	std::vector<RtAudio::Api> apis;
	RtAudio::getCompiledApi(apis);
	size_t i = 0;
	for (const auto& api : apis) {
		if (backend == QString::fromStdString(RtAudio::getApiDisplayName(api))) {
			audio_.reset(new RtAudio(apis[i]));
			return;
		}
		++i;
	}
	audio_.reset(new RtAudio);
}

std::vector<QString> AudioStreamRtAudio::getAvailableBackends() const
{
	std::vector<RtAudio::Api> apis;
	std::vector<QString> names;
	RtAudio::getCompiledApi(apis);
	for (const auto& api : apis)
		names.push_back(QString::fromStdString(RtAudio::getApiDisplayName(api)));
	return names;
}

QString AudioStreamRtAudio::getCurrentBackend() const
{
	return QString::fromStdString(RtAudio::getApiDisplayName(audio_->getCurrentApi()));
}

std::vector<QString> AudioStreamRtAudio::getAvailableDevices() const
{
	RtAudio* audio = audio_.get();
	std::vector<QString> devices;

	for (unsigned int i = 0, n = audio->getDeviceCount(); i < n; ++i) {
		RtAudio::DeviceInfo info = audio->getDeviceInfo(i);
		if (info.outputChannels >= 2)
			devices.push_back(QString::fromStdString(info.name));
	}

	return devices;
}

std::vector<QString> AudioStreamRtAudio::getAvailableDevices(const QString& backend) const
{
	std::vector<RtAudio::Api> apis;
	RtAudio::getCompiledApi(apis);
	RtAudio::Api api = RtAudio::RTAUDIO_DUMMY;
	for (const auto& apiAvailable : apis) {
		if (backend == QString::fromStdString(RtAudio::getApiDisplayName(apiAvailable))) {
			api = apiAvailable;
			break;
		}
	}

	std::vector<QString> list;
	auto a = std::make_unique<RtAudio>(api);
	for (unsigned int i = 0; i < a->getDeviceCount(); ++i) {
		RtAudio::DeviceInfo info = a->getDeviceInfo(i);
		if (info.outputChannels >= 2)
			list.push_back(QString::fromStdString(info.name));
	}
	return list;
}

QString AudioStreamRtAudio::getDefaultOutputDevice() const
{
	return QString::fromStdString(audio_->getDeviceInfo(audio_->getDefaultOutputDevice()).name);
}

QString AudioStreamRtAudio::getDefaultOutputDevice(const QString& backend) const
{
	std::vector<RtAudio::Api> apis;
	RtAudio::getCompiledApi(apis);
	RtAudio::Api api = RtAudio::RTAUDIO_DUMMY;
	for (const auto& apiAvailable : apis) {
		if (backend == QString::fromStdString(RtAudio::getApiDisplayName(apiAvailable))) {
			api = apiAvailable;
			break;
		}
	}

	std::vector<QString> list;
	auto a = std::make_unique<RtAudio>(api);
	return QString::fromStdString(a->getDeviceInfo(a->getDefaultOutputDevice()).name);
}

void AudioStreamRtAudio::start()
{
	AudioStream::start();
	if (audio_->isStreamOpen() && !audio_->isStreamRunning()) audio_->startStream();
}

void AudioStreamRtAudio::stop()
{
	AudioStream::stop();
	if (audio_->isStreamOpen() && audio_->isStreamRunning()) audio_->stopStream();
}
