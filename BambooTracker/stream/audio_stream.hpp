#pragma once

#include <QObject>
#include <QSemaphore>
#include <QString>
#include <thread>
#include <mutex>
#include <memory>
#include <atomic>
#include <cstdint>

class AudioStream : public QObject
{
	Q_OBJECT

public:
	explicit AudioStream(QObject* parent = nullptr);
	virtual ~AudioStream();

	using Callback = void (int16_t*, size_t, void*);
	void setCallback(Callback* cb, void* cbPtr);

	// duration: miliseconds
	virtual void initialize(uint32_t rate, uint32_t duration, uint32_t intrRate, const QString& device);
	virtual void shutdown() = 0;

	virtual std::vector<std::string> getAvailableDevices() = 0;

	void setInterruption(uint32_t inrtRate);

	void start();
	void stop();

signals:
	void streamInterrupted();

protected:
	static const std::string AUDIO_OUT_CLIENT_NAME;
	void generate(int16_t* container, uint32_t nSamples);

private:
	uint32_t rate_;
	uint32_t intrRate_;
	uint32_t intrCount_;
	uint32_t intrCountRest_;

	std::mutex mutex_;
	Callback* cb_;
	void* cbPtr_;
	bool started_;

	std::atomic_bool quitNotify_;
	QSemaphore tickNotifierSem_;
	std::thread tickNotifier_;

	void generateTick();

	void tickNotifierRun();
};
