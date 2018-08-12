#pragma once

#include <QObject>
#include <QIODevice>
#include <cstdint>

class AudioStreamMixier : public QIODevice
{
	Q_OBJECT

public:
	AudioStreamMixier(uint32_t rate, uint32_t duration, uint32_t intrRate, QObject* parent = nullptr);
	~AudioStreamMixier() override;

	void start();
	void stop();
	bool hasRun();

	void setRate(uint32_t rate);
	void setDuration(uint32_t duration);
	void setInterruption(uint32_t rate);

	qint64 readData(char *data, qint64 maxlen) override;
	qint64 writeData(const char *data, qint64 len) override;

signals:	
	void streamInterrupted();
	void bufferPrepared(int16_t *container, size_t nSamples);

private:
	size_t rate_;
	size_t duration_;
	qint64 bufferSampleSize_;

	size_t intrRate_;
	size_t intrCount_;
	size_t intrCountRest_;

	bool isFirstRead_;

	void updateBufferSampleSize();
	void updateIntrruptCount();
};
