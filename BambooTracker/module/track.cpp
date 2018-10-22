#include "track.hpp"
#include <utility>

Track::Track(int number, SoundSource source, int channelInSource)
	: attrib_(std::make_unique<TrackAttribute>())
{	
	attrib_->number = number;
	attrib_->source = source;
	attrib_->channelInSource = channelInSource;

	for (int i = 0; i < 128; ++i) {
		patterns_.emplace_back(i);
	}

	patterns_[0].usedCountUp();
	order_.push_back(0);	// Set first order
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

int Track::searchFirstUneditedUnusedPattern() const
{
	for (size_t i = 0; i < patterns_.size(); ++i) {
		if (!patterns_[i].existCommand() && !patterns_[i].getUsedCount())
			return i;
	}
	return -1;
}

int Track::clonePattern(int num)
{
	int n = searchFirstUneditedUnusedPattern();
	if (n == -1) return num;
	else {
		patterns_.at(n) = patterns_.at(num).clone(n);
		return n;
	}
}

std::vector<int> Track::getEditedPatternIndices() const
{
	std::vector<int> list;
	for (size_t i = 0; i < 128; ++i) {
		if (patterns_[i].existCommand()) list.push_back(i);
	}
	return list;
}

void Track::registerPatternToOrder(int order, int pattern)
{
	patterns_.at(pattern).usedCountUp();
	patterns_.at(order_.at(order)).usedCountDown();
	order_.at(order) = pattern;
}

void Track::insertOrderBelow(int order)
{
	if (order == order_.size() - 1) order_.push_back(0);
	else order_.insert(order_.begin() + order + 1, 0);
	patterns_[0].usedCountUp();
}

void Track::deleteOrder(int order)
{
	patterns_.at(order_.at(order)).usedCountDown();
	order_.erase(order_.begin() + order);
}

void Track::swapOrder(int a, int b)
{
	std::swap(order_.at(a), order_.at((b)));
}

void Track::changeDefaultPatternSize(size_t size)
{
	for (auto& ptn : patterns_) {
		ptn.changeSize(size);
	}
}
