#pragma once

#include <memory>
#include <vector>
#include <mutex>
#include <QObject>
#include <QAudioOutput>
#include <QIODevice>
#include <QAudioDeviceInfo>
#include <QAudioFormat>
#include <QString>

class AudioStream : public QObject
{
	Q_OBJECT
public:
	AudioStream(uint32_t rate, uint32_t bufferDuration, uint32_t intrRate, QString device);
	~AudioStream();

	void start();
	void stop();
	bool hasRun() const;

	void setRate(uint32_t rate);
	void setDuration(uint32_t duration);
	void setInturuption(uint32_t rate);
	void setDevice(QString device);

	int16_t* getTransferBuffer();
	size_t getInterruptionSampleSize() const;
	void flushSamples(int16_t* buf, size_t nSamples);

	static constexpr size_t TRANS_BUFFER_SIZE = 0x10000;

private:
	std::mutex mutex_;
	QAudioDeviceInfo info_;
	QAudioFormat format_;
	std::unique_ptr<QAudioOutput> audio_;
	QIODevice* device_;
	uint32_t rate_, duration_, intrRate_;
	size_t bufferSampleSize_, bufferSize_, bufferByteSize_, bufferIdx_;
	size_t intrSampSize_;
	std::vector<int16_t> buf_;
	int16_t transBuf_[TRANS_BUFFER_SIZE];

	void setDeviceFromString(QString device);
	void updateBufferSize();

	inline void updateIntrruptionSampleSize()
	{
		intrSampSize_ = rate_ / intrRate_;
	}
};
