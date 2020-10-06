/*
 * Copyright (C) 2018 Rerrah
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

#include "abstract_instrument_property.hpp"
#include <algorithm>

AbstractInstrumentProperty::AbstractInstrumentProperty(int num)
	: num_(num)
{
}

AbstractInstrumentProperty::AbstractInstrumentProperty(const AbstractInstrumentProperty& other)
{
	num_ = other.num_;
	users_ = other.users_;
}

void AbstractInstrumentProperty::setNumber(int num)
{
	num_ = num;
}

int AbstractInstrumentProperty::getNumber() const
{
	return num_;
}

void AbstractInstrumentProperty::registerUserInstrument(int instNum)
{
	users_.push_back(instNum);
	std::sort(users_.begin(), users_.end());
}

void AbstractInstrumentProperty::deregisterUserInstrument(int instNum)
{
	users_.erase(std::find(users_.begin(), users_.end(), instNum));
}

bool AbstractInstrumentProperty::isUserInstrument() const
{
	return !users_.empty();
}

std::vector<int> AbstractInstrumentProperty::getUserInstruments() const
{
	return users_;
}

void AbstractInstrumentProperty::clearUserInstruments()
{
	users_.clear();
}
