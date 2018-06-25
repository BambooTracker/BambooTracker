#pragma once

#include <QObject>
#include <QAudioOutput>
#include <QAudioFormat>
#include <memory>
#include "chip.hpp"
#include "audio_stream_mixier.hpp"

class AudioStream : public QObject
{
	Q_OBJECT

public:
	// duration: miliseconds
	AudioStream(chip::Chip& chip, uint32_t rate, uint32_t duration);
	~AudioStream();

	void setRate(uint32_t rate);
	void setDuration(uint32_t duration);

	bool startPlaySong();
	bool stopPlaySong();

signals:
	void nextStepArrived();
	void nextTickArrived();

private:
	chip::Chip& chip_;
	QAudioFormat format_;
	std::unique_ptr<QAudioOutput> audio_;
	std::unique_ptr<AudioStreamMixier> mixer_;

	void start();
	void stop();

private slots:
	void onNextStepArrived();
	void onNextTickArrived();
};
