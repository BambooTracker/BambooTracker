#include "audio_stream.hpp"
#include <QSysInfo>
#include <QAudio>

AudioStream::AudioStream(uint32_t rate, uint32_t duration, uint32_t intrRate, QString device)
{
	setDeviceFromString(device);

	format_.setByteOrder(QAudioFormat::Endian(QSysInfo::ByteOrder));
	format_.setChannelCount(2); // Stereo
	format_.setCodec("audio/pcm");
	format_.setSampleRate(static_cast<int>(rate));
	format_.setSampleSize(16);   // int16
	format_.setSampleType(QAudioFormat::SignedInt);

	audio_ = std::make_unique<QAudioOutput>(info_, format_);
	mixer_ = std::make_unique<AudioStreamMixier>(rate, duration, intrRate);
	QObject::connect(mixer_.get(), &AudioStreamMixier::streamInterrupted,
					 this, [&]() { emit streamInterrupted(); }, Qt::DirectConnection);
	QObject::connect(mixer_.get(), &AudioStreamMixier::bufferPrepared,
					 this, [&](int16_t *container, size_t nSamples) {
		emit bufferPrepared(container, nSamples);
	}, Qt::DirectConnection);

	start();
}

AudioStream::~AudioStream()
{
	stop();
}

void AudioStream::start()
{
	if (!mixer_->hasRun()) mixer_->start();
	if (audio_->state() != QAudio::ActiveState) audio_->start(mixer_.get());
}

void AudioStream::stop()
{
	if (mixer_->hasRun()) mixer_->stop();
	if (audio_->state() != QAudio::StoppedState) audio_->stop();
}

void AudioStream::setRate(uint32_t rate)
{
	bool hasRun = mixer_->hasRun();
	if (hasRun) stop();
	format_.setSampleRate(static_cast<int>(rate));
	audio_ = std::make_unique<QAudioOutput>(info_, format_);
	mixer_->setRate(rate);
	if (hasRun) start();
}

void AudioStream::setDuration(uint32_t duration)
{
	bool hasRun = mixer_->hasRun();
	if (hasRun) stop();
	mixer_->setDuration(duration);
	if (hasRun) start();
}

void AudioStream::setInturuption(uint32_t rate)
{
	bool hasRun = mixer_->hasRun();
	if (hasRun) stop();
	mixer_->setInterruption(rate);
	if (hasRun) start();
}

void AudioStream::setDevice(QString device)
{
	bool hasRun = mixer_->hasRun();
	if (hasRun) stop();
	setDeviceFromString(device);
	audio_ = std::make_unique<QAudioOutput>(info_, format_);
	if (hasRun) start();
}

void AudioStream::setDeviceFromString(QString device)
{
	if (device == QAudioDeviceInfo::defaultOutputDevice().deviceName()) {
		info_ = QAudioDeviceInfo::defaultOutputDevice();
		return;
	}
	for (auto& i : QAudioDeviceInfo::availableDevices(QAudio::AudioOutput)) {
		if (device == i.deviceName()) {
			info_ = i;
			break;
		}
	}
}
