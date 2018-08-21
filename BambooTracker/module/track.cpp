#include "track.hpp"

Track::Track(int number, SoundSource source, int channelInSource)
	: attrib_(std::make_unique<TrackAttribute>())
{	
	attrib_->number = number;
	attrib_->source = source;
	attrib_->channelInSource = channelInSource;

	order_.push_back(0);	// Set first order

	for (int i = 0; i < 128; ++i) {
		patterns_.emplace_back(i);
	}
}

TrackAttribute Track::getAttribute() const
{
	return *attrib_;
}

OrderData Track::getOrderData(int order)
{
	OrderData res;
	res.trackAttribute = getAttribute();
	res.order = order;
	res.patten = order_.at(order);
	return res;
}

size_t Track::getOrderSize() const
{
	return order_.size();
}

Pattern& Track::getPattern(int num)
{
	return patterns_.at(num);
}

Pattern& Track::getPatternFromOrderNumber(int num)
{
	return getPattern(order_.at(num));
}

void Track::registerPatternToOrder(int order, int pattern)
{
	order_.at(order) = pattern;
}

void Track::insertOrderBelow(int order)
{
	if (order == order_.size() - 1) order_.push_back(0);
	else order_.insert(order_.begin() + order + 1, 0);
}

void Track::deleteOrder(int order)
{
	order_.erase(order_.begin() + order);
}
