/*
 * Copyright (C) 2018-2021 Rerrah
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

#include "track.hpp"
#include <utility>
#include <algorithm>
#include "utils.hpp"

namespace
{
constexpr int PATTERN_SIZE = 256;
constexpr int MAX_ORDER_SIZE = 256;
}

Track::Track(int number, SoundSource source, int channelInSource, int defPattenSize)
	: effetDisplayWidth_(0), visibility_(true)

{	
	setAttribute(number, source, channelInSource);

	patterns_.reserve(PATTERN_SIZE);
	for (int i = 0; i < PATTERN_SIZE; ++i) {
		patterns_.emplace_back(i, defPattenSize);
	}

	patterns_[0].increaseUsedCount();
	order_.push_back(0);	// Set first order
}

void Track::setAttribute(int number, SoundSource source, int channelInSource) noexcept
{
	attrib_.number = number;
	attrib_.source = source;
	attrib_.channelInSource = channelInSource;
}

OrderInfo Track::getOrderInfo(int order) const
{
	OrderInfo info;
	info.trackAttribute = attrib_;
	info.order = order;
	info.patten = order_.at(static_cast<size_t>(order));
	return info;
}

size_t Track::getOrderSize() const
{
	return order_.size();
}

bool Track::canAddNewOrder() const
{
	return order_.size() < MAX_ORDER_SIZE;
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
	auto it = utils::findIf(patterns_, [](const Pattern& pattern) {
		return (!pattern.hasEvent() && !pattern.getUsedCount());
	});
	return (it == patterns_.cend() ? -1 : std::distance(patterns_.cbegin(), it));
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
	return utils::findIndicesIf(patterns_, [](const Pattern& pattern) { return pattern.hasEvent(); });
}

std::set<int> Track::getRegisteredInstruments() const
{
	std::set<int> set;
	for (const Pattern& pattern : patterns_) {
		auto&& insts = pattern.getRegisteredInstruments();
		std::copy(insts.cbegin(), insts.cend(), std::inserter(set, set.end()));
	}
	return set;
}

void Track::registerPatternToOrder(int order, int pattern)
{
	patterns_.at(static_cast<size_t>(pattern)).increaseUsedCount();
	patterns_.at(static_cast<size_t>(order_.at(static_cast<size_t>(order)))).decreaseUsedCount();
	order_.at(static_cast<size_t>(order)) = pattern;
}

void Track::insertOrderBelow(int order)
{
	int n = searchFirstUneditedUnusedPattern();
	if (n == -1) n = PATTERN_SIZE - 1;

	if (order == static_cast<int>(order_.size()) - 1) order_.push_back(n);
	else order_.insert(order_.begin() + order + 1, n);
	patterns_[static_cast<size_t>(n)].increaseUsedCount();
}

void Track::deleteOrder(int order)
{
	patterns_.at(static_cast<size_t>(order_.at(static_cast<size_t>(order)))).decreaseUsedCount();
	order_.erase(order_.begin() + order);
}

void Track::swapOrder(int a, int b)
{
	std::swap(order_.at(static_cast<size_t>(a)), order_.at((static_cast<size_t>(b))));
}

void Track::changeDefaultPatternSize(size_t size)
{
	for (auto& ptn : patterns_) ptn.changeSize(size);
}

void Track::clearUnusedPatterns()
{
	for (Pattern& pattern : patterns_) {
		if (!pattern.getUsedCount() && pattern.hasEvent())
			pattern.clear();
	}
}

void Track::replaceDuplicateInstrumentsInPatterns(const std::unordered_map<int, int>& map)
{
	for (Pattern& pattern : patterns_) {
		if (pattern.hasEvent()) {
			for (size_t i = 0; i < pattern.getSize(); ++i) {
				Step& step = pattern.getStep(static_cast<int>(i));
				int inst = step.getInstrumentNumber();
				if (map.count(inst)) step.setInstrumentNumber(map.at(inst));
			}
		}
	}
}

void Track::transpose(int semitones, const std::vector<int>& excludeInsts)
{
	for (Pattern& pattern : patterns_) pattern.transpose(semitones, excludeInsts);
}
