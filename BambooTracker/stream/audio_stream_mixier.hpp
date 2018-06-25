#pragma once

#include <QObject>
#include <QIODevice>
#include <cstdint>
#include "chips/chip.hpp"

class AudioStreamMixier : public QIODevice
{
	Q_OBJECT

public:
	AudioStreamMixier(chip::Chip& chip, uint32_t rate, uint32_t duration, QObject* parent = nullptr);
	~AudioStreamMixier();

	void start();
	void stop();
	bool hasRun();

	void setRate(uint32_t rate);
	void setDuration(uint32_t duration);

	bool startPlaySong();
	bool stopPlaySong();

	qint64 readData(char *data, qint64 maxlen) override;
	qint64 writeData(const char *data, qint64 len) override;

signals:
	void nextStepArrived();
	void nextTickArrived();

private:
	chip::Chip& chip_;
	size_t rate_;
	size_t duration_;
	qint64 bufferSampleSize_;

	size_t tickRate_;
	size_t tickIntrCount_;
	size_t tickIntrCountRest_;

	bool isPlaySong_;
	size_t specificTicksPerStep_;
	size_t executingTicksPerStep_;
	size_t tickCount_;
	size_t tempo_;
	float strictTicksPerStepByBpm_;
	float tickDifSum_;

	bool isFirstRead_;

	static const size_t NTSC_;
	static const size_t PAL_;

	void setBufferSampleSize(uint32_t rate, uint32_t duration);
};
