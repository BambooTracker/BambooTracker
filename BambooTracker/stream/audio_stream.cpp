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

		gcb(destPtr, count, gcbPtr);

		destPtr += (count << 1);	// Move head
	}
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
