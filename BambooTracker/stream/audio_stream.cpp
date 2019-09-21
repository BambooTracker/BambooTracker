#include "audio_stream.hpp"
#include <algorithm>

const std::string AudioStream::AUDIO_OUT_CLIENT_NAME = "BambooTracker";

AudioStream::AudioStream(QObject *parent)
	: QObject(parent),
	  rate_(0),
	  intrRate_(0),
	  intrCount_(0),
	  intrCountRest_(0),
	  cb_(nullptr),
	  cbPtr_(nullptr),
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

void AudioStream::setCallback(Callback* cb, void* cbPtr)
{
	std::lock_guard<std::mutex> lock(mutex_);
	cb_ = cb;
	cbPtr_ = cbPtr;
}

void AudioStream::initialize(uint32_t rate, uint32_t duration, uint32_t intrRate, const QString& device)
{
	Q_UNUSED(duration)
	Q_UNUSED(device)

	started_ = false;
	rate_ = rate;
	setInterruption(intrRate);
}

void AudioStream::setInterruption(uint32_t intrRate)
{
	std::lock_guard<std::mutex> lock(mutex_);
	intrRate_ = intrRate;
	intrCount_ = rate_ / intrRate_;
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

void AudioStream::generate(int16_t* container, uint32_t nSamples)
{
	Callback* cb = nullptr;
	void* cbPtr = nullptr;
	bool started = false;

	std::unique_lock<std::mutex> lock(mutex_, std::try_to_lock);
	if (lock.owns_lock()) {
		cb = cb_;
		cbPtr = cbPtr_;
		started = started_;
	}

	if (!cb || !started) {
		std::fill(container, container + (nSamples << 1), 0);
		return;
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

		cb(destPtr, count, cbPtr);

		destPtr += (count << 1);	// Move head
	}
}

void AudioStream::generateTick()
{
	tickNotifierSem_.release();
}

void AudioStream::tickNotifierRun()
{
	while (true) {
		tickNotifierSem_.acquire();

		if (quitNotify_.load()) return;

		emit streamInterrupted();
	}
}
