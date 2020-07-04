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

	using GenerateCallback = void (int16_t*, size_t, void*);
	void setGenerateCallback(GenerateCallback* cb, void* cbPtr);

	using TickUpdateCallback = int (void*);
	void setTickUpdateCallback(TickUpdateCallback* cb, void* cbPtr);

	// duration: miliseconds
	virtual bool initialize(uint32_t rate, uint32_t duration, uint32_t intrRate, const QString& backend, const QString& device);
	virtual void shutdown() = 0;

	virtual std::vector<QString> getAvailableBackends() const = 0;
	virtual QString getCurrentBackend() const = 0;
	virtual std::vector<QString> getAvailableDevices() const = 0;
	virtual std::vector<QString> getAvailableDevices(const QString& backend) const = 0;
	virtual QString getDefaultOutputDevice() const = 0;
	virtual QString getDefaultOutputDevice(const QString& backend) const = 0;

	void setInterruption(uint32_t inrtRate);

	virtual void start();
	virtual void stop();

signals:
	void streamInterrupted(int state);

protected:
	static const std::string AUDIO_OUT_CLIENT_NAME;
	void generate(int16_t* container, uint32_t nSamples);

private:
	uint32_t rate_;
	uint32_t intrRate_;
	uint32_t intrCount_;
	uint32_t intrCountRest_;

	std::mutex mutex_;
	GenerateCallback* gcb_;
	void* gcbPtr_;
	TickUpdateCallback* tucb_;
	void* tucbPtr_;
	std::atomic_int tuState_;
	bool started_;

	std::atomic_bool quitNotify_;
	QSemaphore tickNotifierSem_;
	std::thread tickNotifier_;

	void generateTick();

	void tickNotifierRun();
};
