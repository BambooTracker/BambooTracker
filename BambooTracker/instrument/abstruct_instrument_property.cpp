#include "abstruct_instrument_property.hpp"
#include <algorithm>

AbstructInstrumentProperty::AbstructInstrumentProperty(int num)
	: num_(num)
{
}

AbstructInstrumentProperty::AbstructInstrumentProperty(const AbstructInstrumentProperty& other)
{
	num_ = other.num_;
	users_ = other.users_;
}

void AbstructInstrumentProperty::setNumber(int num)
{
	num_ = num;
}

int AbstructInstrumentProperty::getNumber() const
{
	return num_;
}

void AbstructInstrumentProperty::registerUserInstrument(int instNum)
{
	users_.push_back(instNum);
	std::sort(users_.begin(), users_.end());
}

void AbstructInstrumentProperty::deregisterUserInstrument(int instNum)
{
	users_.erase(std::find(users_.begin(), users_.end(), instNum));
}

bool AbstructInstrumentProperty::isUserInstrument() const
{
	return !users_.empty();
}

std::vector<int> AbstructInstrumentProperty::getUserInstruments() const
{
	return users_;
}
