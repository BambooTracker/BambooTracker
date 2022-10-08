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

#pragma once

#include <vector>
#include <set>
#include <unordered_map>
#include "pattern.hpp"
#include "bamboo_tracker_defs.hpp"

struct TrackAttribute
{
	int number;
	SoundSource source;
	int channelInSource;
};

struct OrderInfo
{
	TrackAttribute trackAttribute;
	int order;
	int patten;
};

class Track
{
public:
	Track(int number, SoundSource source, int channelInSource, int defPattenSize);

	void setAttribute(int number, SoundSource source, int channelInSource) noexcept;
	TrackAttribute getAttribute() const noexcept { return attrib_; }
	OrderInfo getOrderInfo(int order) const;
	size_t getOrderSize() const;
	bool canAddNewOrder() const;
	Pattern& getPattern(int num);
	Pattern& getPatternFromOrderNumber(int num);
	int searchFirstUneditedUnusedPattern() const;
	int clonePattern(int num);
	std::vector<int> getEditedPatternIndices() const;
	std::set<int> getRegisteredInstruments() const;

	void registerPatternToOrder(int order, int pattern);
	void insertOrderBelow(int order);
	void deleteOrder(int order);
	void swapOrder(int a, int b);

	void changeDefaultPatternSize(size_t size);

	void setEffectDisplayWidth(size_t w) noexcept { effetDisplayWidth_ = w; }
	size_t getEffectDisplayWidth() const noexcept { return effetDisplayWidth_; }

	void setVisibility(bool visible) noexcept { visibility_ = visible; }
	bool isVisible() const noexcept { return visibility_; }

	void clearUnusedPatterns();
	void replaceDuplicateInstrumentsInPatterns(const std::unordered_map<int, int>& map);

	void transpose(int semitones, const std::vector<int>& excludeInsts);

private:
	TrackAttribute attrib_;

	std::vector<int> order_;
	std::vector<Pattern> patterns_;
	size_t effetDisplayWidth_;
	bool visibility_;
};
