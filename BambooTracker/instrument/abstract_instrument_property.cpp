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
