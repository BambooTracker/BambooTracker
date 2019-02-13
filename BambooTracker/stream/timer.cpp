#include "timer.hpp"

Timer::Timer()
	: checkTime_(100)
{
}

Timer::~Timer()
{
	stop();
}

void Timer::setFunction(std::function<void()> func)
{
	func_ = func;
}

void Timer::setInterval(const int milisec)
{
	time_.store(milisec);
}

void Timer::start()
{
	isContinue_.store(true);
	thread_ = std::thread([&] {
		while (isContinue_.load()) {
			std::this_thread::sleep_for(std::chrono::milliseconds(time_.load()));
			func_();
		}
	});
}

void Timer::stop()
{
	if (isContinue_.load()) {
		isContinue_.store(false);
		thread_.join();
	}
}
