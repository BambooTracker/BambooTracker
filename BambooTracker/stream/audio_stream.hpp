/*
 * Copyright (C) 2018-2020 Rerrah
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

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
	virtual bool initialize(uint32_t rate, uint32_t duration, uint32_t intrRate,
							const QString& backend, const QString& device,
							QString* errDetail = nullptr);
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
