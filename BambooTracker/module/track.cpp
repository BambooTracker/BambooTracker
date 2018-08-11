#include "track.hpp"
#include <utility>

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
	TrackAttribute ret = *attrib_;
	return std::move(ret);
}

std::vector<int> Track::getOrderList() const
{
	std::vector<int> ret = order_;
	return std::move(ret);
}

Pattern& Track::getPattern(int num)
{
	return patterns_.at(num);
}

Pattern& Track::getPatternFromOrderNumber(int num)
{
	return getPattern(order_.at(num));
}
