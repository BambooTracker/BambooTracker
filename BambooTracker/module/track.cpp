#include "track.hpp"
#include <utility>

Track::Track(int number, SoundSource source, int channelInSource, int defPattenSize)
	: attrib_(std::make_unique<TrackAttribute>()),
	  effetDisplayWidth_(0)

{	
	attrib_->number = number;
	attrib_->source = source;
	attrib_->channelInSource = channelInSource;

	for (int i = 0; i < 256; ++i) {
		patterns_.emplace_back(i, defPattenSize);
	}

	patterns_[0].usedCountUp();
	order_.push_back(0);	// Set first order
}

Track::Track(const Track& other)
	: attrib_(std::make_unique<TrackAttribute>())
{
	attrib_->number = other.attrib_->number;
	attrib_->source = other.attrib_->source;
	attrib_->channelInSource = other.attrib_->channelInSource;

	order_ = other.order_;
	patterns_ = other.patterns_;

	effetDisplayWidth_ = other.effetDisplayWidth_;
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
	res.patten = order_.at(static_cast<size_t>(order));
	return res;
}

size_t Track::getOrderSize() const
{
	return order_.size();
}

Pattern& Track::getPattern(int num)
{
	return patterns_.at(static_cast<size_t>(num));
}

Pattern& Track::getPatternFromOrderNumber(int num)
{
	return getPattern(order_.at(static_cast<size_t>(num)));
}

int Track::searchFirstUneditedUnusedPattern() const
{
	for (size_t i = 0; i < patterns_.size(); ++i) {
		if (!patterns_[i].existCommand() && !patterns_[i].getUsedCount())
			return static_cast<int>(i);
	}
	return -1;
}

int Track::clonePattern(int num)
{
	int n = searchFirstUneditedUnusedPattern();
	if (n == -1) return num;
	else {
		patterns_.at(static_cast<size_t>(n)) = patterns_.at(static_cast<size_t>(num)).clone(n);
		return n;
	}
}

std::vector<int> Track::getEditedPatternIndices() const
{
	std::vector<int> list;
	for (size_t i = 0; i < 256; ++i) {
		if (patterns_[i].existCommand()) list.push_back(static_cast<int>(i));
	}
	return list;
}

std::unordered_set<int> Track::getRegisteredInstruments() const
{
	std::unordered_set<int> set;
	for (auto& pattern : patterns_) {
		for (auto& n : pattern.getRegisteredInstruments()) {
			set.insert(n);
		}
	}
	return set;
}

void Track::registerPatternToOrder(int order, int pattern)
{
	patterns_.at(static_cast<size_t>(pattern)).usedCountUp();
	patterns_.at(static_cast<size_t>(order_.at(static_cast<size_t>(order)))).usedCountDown();
	order_.at(static_cast<size_t>(order)) = pattern;
}

void Track::insertOrderBelow(int order)
{
	int n = searchFirstUneditedUnusedPattern();
	if (n == -1) n = 255;

	if (order == static_cast<int>(order_.size()) - 1) order_.push_back(n);
	else order_.insert(order_.begin() + order + 1, n);
	patterns_[static_cast<size_t>(n)].usedCountUp();
}

void Track::deleteOrder(int order)
{
	patterns_.at(static_cast<size_t>(order_.at(static_cast<size_t>(order)))).usedCountDown();
	order_.erase(order_.begin() + order);
}

void Track::swapOrder(int a, int b)
{
	std::swap(order_.at(static_cast<size_t>(a)), order_.at((static_cast<size_t>(b))));
}

void Track::changeDefaultPatternSize(size_t size)
{
	for (auto& ptn : patterns_) {
		ptn.changeSize(size);
	}
}

void Track::setEffectDisplayWidth(size_t w)
{
	effetDisplayWidth_ = w;
}

size_t Track::getEffectDisplayWidth() const
{
	return effetDisplayWidth_;
}

void Track::clearUnusedPatterns()
{
	for (size_t i = 0; i < 256; ++i) {
		if (!patterns_[i].getUsedCount() && patterns_[i].existCommand())
			patterns_[i].clear();
	}
}

void Track::replaceDuplicateInstrumentsInPatterns(std::unordered_map<int, int> map)
{
	for (size_t i = 0; i < 256; ++i) {
		Pattern& pattern = patterns_[i];
		if (pattern.existCommand()) {
			for (size_t i = 0; i < pattern.getSize(); ++i) {
				Step& step = pattern.getStep(static_cast<int>(i));
				int inst = step.getInstrumentNumber();
				if (map.count(inst)) step.setInstrumentNumber(map[inst]);
			}
		}

	}
}
