/*
 * Copyright (C) 2018-2023 Rerrah
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

#include "audio_stream.hpp"
#include <algorithm>

const std::string AudioStream::AUDIO_OUT_CLIENT_NAME = "BambooTracker";

AudioStream::AudioStream(QObject *parent)
	: QObject(parent),
	  rate_(0),
	  intrRate_(0),
	  intrCount_(0),
	  intrCountRest_(0),
	  gcb_(nullptr),
	  gcbPtr_(nullptr),
	  tuState_(-1),
	  started_(false),
	  quitNotify_(false),
	  tickNotifier_([this]() { tickNotifierRun(); })
{
}

AudioStream::~AudioStream()
{
	quitNotify_.store(true);
	tickNotifierSem_.release();
	tickNotifier_.join();
}

void AudioStream::setGenerateCallback(GenerateCallback* cb, void* cbPtr)
{
	std::lock_guard<std::mutex> lock(mutex_);
	gcb_ = cb;
	gcbPtr_ = cbPtr;
}

void AudioStream::setTickUpdateCallback(TickUpdateCallback* cb, void* cbPtr)
{
	std::lock_guard<std::mutex> lock(mutex_);
	tucb_ = cb;
	tucbPtr_ = cbPtr;
}

bool AudioStream::initialize(uint32_t rate, uint32_t duration, uint32_t intrRate,
							 const QString& backend, const QString& device, QString* errDetail)
{
	Q_UNUSED(duration)
	Q_UNUSED(backend)
	Q_UNUSED(device)
	Q_UNUSED(errDetail)

	started_ = false;
	rate_ = rate;
	setInterruption(intrRate);
	return true;
}

void AudioStream::setInterruption(uint32_t intrRate)
{
	std::lock_guard<std::mutex> lock(mutex_);
	intrRate_ = intrRate;
	intrCount_ = rate_ / intrRate_;
}

uint32_t AudioStream::getStreamRate() const noexcept
{
	return rate_;
}

void AudioStream::start()
{
	std::lock_guard<std::mutex> lock(mutex_);
	started_ = true;
}

void AudioStream::stop()
{
	std::lock_guard<std::mutex> lock(mutex_);
	started_ = false;
}

bool AudioStream::generate(int16_t* container, uint32_t nSamples)
{
	GenerateCallback* gcb = nullptr;
	void* gcbPtr = nullptr;
	TickUpdateCallback* tucb = nullptr;
	bool started = false;

	std::unique_lock<std::mutex> lock(mutex_, std::try_to_lock);
	if (lock.owns_lock()) {
		gcb = gcb_;
		gcbPtr = gcbPtr_;
		tucb = tucb_;
		started = started_;
	}

	if (!gcb || !tucb || !started) {
		std::fill(container, container + (nSamples << 1), 0);
		return true;
	}

	int16_t* destPtr = container;
	while (nSamples) {
		if (!intrCountRest_) {	// Interruption
			intrCountRest_ = intrCount_;    // Set counts to next interruption
			generateTick();
		}

		size_t count = std::min(intrCountRest_, nSamples);
		nSamples -= count;
		intrCountRest_ -= count;

		bool result = gcb(destPtr, count, gcbPtr);
		if (!result) {
			// Something went wrong in sample generation callback
			emit streamErrorInCallback(QVariant());
			return false;
		}

		destPtr += (count << 1);	// Move head
	}

	return true;
}

void AudioStream::generateTick()
{
	tuState_.store(tucb_(tucbPtr_));
	tickNotifierSem_.release();
}

void AudioStream::tickNotifierRun()
{
	while (true) {
		tickNotifierSem_.acquire();

		if (quitNotify_.load()) return;

		emit streamInterrupted(tuState_.load());
	}
}
