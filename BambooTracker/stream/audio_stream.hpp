#pragma once

#include <QObject>
#include <QAudioOutput>
#include <QAudioDeviceInfo>
#include <QAudioFormat>
#include <QString>
#include <memory>
#include "audio_stream_mixier.hpp"

class AudioStream : public QObject
{
	Q_OBJECT

public:
	// duration: miliseconds
	AudioStream(uint32_t rate, uint32_t duration, uint32_t intrRate, QString device);
	~AudioStream();

	void start();
	void stop();

	void setRate(uint32_t rate);
	void setDuration(uint32_t duration);
	void setInturuption(uint32_t rate);
	void setDevice(QString device);

signals:
	void streamInterrupted();
	void bufferPrepared(int16_t *container, size_t nSamples);

private:
	QAudioDeviceInfo info_;
	QAudioFormat format_;
	std::unique_ptr<QAudioOutput> audio_;
	std::unique_ptr<AudioStreamMixier> mixer_;

	void setDeviceFromString(QString device);
};
