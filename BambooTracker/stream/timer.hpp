#pragma once

#include <functional>
#include <chrono>
#include <atomic>
#include <thread>

class Timer
{
public:
	Timer();
	~Timer();

	void setFunction(std::function<void()> func);
	void setInterval(const int microsec);

	void start();
	void stop();

private:
	std::atomic<int> time_;
	const int checkTime_;
	std::function<void()> func_;
	std::thread thread_;
	std::atomic_bool isContinue_;
};
