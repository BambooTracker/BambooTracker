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

#include "precise_timer.hpp"
#include <chrono>

PreciseTimer::~PreciseTimer()
{
	stop();
}

void PreciseTimer::setFunction(std::function<void()> func)
{
	func_ = func;
}

void PreciseTimer::setInterval(const int microsec)
{
	time_.store(microsec);
}

void PreciseTimer::start()
{
	isContinue_.store(true);
	thread_ = std::thread([&] {
		while (isContinue_.load()) {
			std::this_thread::sleep_for(std::chrono::microseconds(time_.load()));
			func_();
		}
	});
}

void PreciseTimer::stop()
{
	if (isContinue_.load()) {
		isContinue_.store(false);
		thread_.join();
	}
}
