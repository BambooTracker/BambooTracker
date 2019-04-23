#include "audio_stream.hpp"
#include <algorithm>
#include <QSysInfo>
#include <QAudio>

AudioStream::AudioStream(uint32_t rate, uint32_t bufferDuration, uint32_t intrRate, QString device)
	: rate_(rate), duration_(bufferDuration), intrRate_(intrRate)
{
	setDeviceFromString(device);
	updateBufferSize();
	updateIntrruptionSampleSize();

	format_.setByteOrder(QAudioFormat::Endian(QSysInfo::ByteOrder));
	format_.setChannelCount(2); // Stereo
	format_.setCodec("audio/pcm");
	format_.setSampleRate(static_cast<int>(rate));
	format_.setSampleSize(16);   // int16
	format_.setSampleType(QAudioFormat::SignedInt);

	audio_ = std::make_unique<QAudioOutput>(info_, format_);

	start();
}

AudioStream::~AudioStream()
{
	stop();
}

void AudioStream::start()
{
	if (audio_->state() != QAudio::ActiveState) device_ = audio_->start();
}

void AudioStream::stop()
{
	if (device_ && device_->isOpen()) {
		device_->close();
		device_ = nullptr;
	}
	if (audio_->state() != QAudio::StoppedState) audio_->stop();
}

void AudioStream::setRate(uint32_t rate)
{
	bool isOpen = device_ ? device_->isOpen() : false;
	if (isOpen) stop();
	rate_ = rate;
	format_.setSampleRate(static_cast<int>(rate));
	updateBufferSize();
	updateIntrruptionSampleSize();
	audio_ = std::make_unique<QAudioOutput>(info_, format_);
	if (isOpen) start();
}

void AudioStream::setDuration(uint32_t duration)
{
	bool isOpen = device_ ? device_->isOpen() : false;
	if (isOpen) stop();
	duration_ = duration;
	updateBufferSize();
	if (isOpen) start();
}

void AudioStream::setInturuption(uint32_t rate)
{
	intrRate_ = rate;
	updateIntrruptionSampleSize();
}

void AudioStream::setDevice(QString device)
{
	bool isOpen = device_ ? device_->isOpen() : false;
	if (isOpen) stop();
	setDeviceFromString(device);
	audio_ = std::make_unique<QAudioOutput>(info_, format_);
	if (isOpen) start();
}

int16_t* AudioStream::getTransferBuffer()
{
	return transBuf_;
}

size_t AudioStream::getInterruptionSampleSize() const
{
	return intrSampSize_;
}

void AudioStream::flushSamples(int16_t* buf, size_t nSamples)
{
	std::lock_guard<std::mutex> lg(mutex_);
	if (buf_.empty() || audio_->state() == QAudio::StoppedState || !device_->isOpen()) return;
	size_t idx = 0;
	size_t argBufSize = nSamples << 1;	// Stereo

	while (idx < argBufSize) {
		buf_[bufferIdx_++] = buf[idx++];	// Left
		buf_[bufferIdx_++] = buf[idx++];	// Right
		if (bufferIdx_ >= bufferSize_) {
			device_->write(reinterpret_cast<const char*>(&buf_[0]), bufferByteSize_);
			bufferIdx_ = 0;
		}
	}
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

void AudioStream::updateBufferSize()
{
	bufferSampleSize_ = rate_ * duration_ / 1000;
	bufferSize_ = bufferSampleSize_ << 1;	// Stereo
	bufferByteSize_ = bufferSize_ << 1;		// int16
	buf_.resize(bufferSize_);
	buf_.shrink_to_fit();
	bufferIdx_ = 0;
}
