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

#include "song.hpp"
#include <algorithm>
#include <utility>
#include <stdexcept>

Song::Song(int number, SongType songType, std::string title, bool isUsedTempo,
		   int tempo, int groove, int speed, size_t defaultPatternSize)
	: num_(number),
	  type_(songType),
	  title_(title),
	  isUsedTempo_(isUsedTempo),
	  tempo_(tempo),
	  groove_(groove),
	  speed_(speed),
	  defPtnSize_(defaultPatternSize)
{
	switch (songType) {
	case SongType::Standard:
		tracks_.reserve(15);
		for (int i = 0; i < 6; ++i) {
			tracks_.emplace_back(i, SoundSource::FM, i, defaultPatternSize);
		}
		for (int i = 0; i < 3; ++i) {
			tracks_.emplace_back(i + 6, SoundSource::SSG, i, defaultPatternSize);
		}
		for (int i = 0; i < 6; ++i) {
			tracks_.emplace_back(i + 9, SoundSource::RHYTHM, i, defaultPatternSize);
		}
		tracks_.emplace_back(15, SoundSource::ADPCM, 0, defaultPatternSize);
		break;
	case SongType::FM3chExpanded:
		tracks_.reserve(18);
		for (int i = 0; i < 9; ++i) {
			int ch = (i < 3) ? i
							 : (i < 6) ? (i + 3) : (i - 3);
			tracks_.emplace_back(i, SoundSource::FM, ch, defaultPatternSize);
		}
		for (int i = 0; i < 3; ++i) {
			tracks_.emplace_back(i + 9, SoundSource::SSG, i, defaultPatternSize);
		}
		for (int i = 0; i < 6; ++i) {
			tracks_.emplace_back(i + 12, SoundSource::RHYTHM, i, defaultPatternSize);
		}
		tracks_.emplace_back(18, SoundSource::ADPCM, 0, defaultPatternSize);
		break;
	}
}

void Song::setNumber(int n)
{
	num_ = n;
}

int Song::getNumber() const
{
	return num_;
}

void Song::setTitle(std::string title)
{
	title_ = title;
}

std::string Song::getTitle() const
{
	return title_;
}

void Song::setTempo(int tempo)
{
	tempo_ = tempo;
}

int Song::getTempo() const
{
	return tempo_;
}

void Song::setGroove(int groove)
{
	groove_ = groove;
}

int Song::getGroove() const
{
	return groove_;
}

void Song::toggleTempoOrGroove(bool isUsedTempo)
{
	isUsedTempo_ = isUsedTempo;
}

bool Song::isUsedTempo() const
{
	return isUsedTempo_;
}

void Song::setSpeed(int speed)
{
	speed_ = speed;
}

int Song::getSpeed() const
{
	return speed_;
}

void Song::setDefaultPatternSize(size_t size)
{
	defPtnSize_ = size;
	for (auto& t : tracks_) {
		t.changeDefaultPatternSize(size);
	}
}

size_t Song::getDefaultPatternSize() const
{
	return defPtnSize_;
}

size_t Song::getPatternSizeFromOrderNumber(int order)
{
	if (static_cast<int>(getOrderSize()) <= order) return 0;	// Ilegal value
	size_t size = 0;
	for (auto& t : tracks_) {
		size_t ptnSize = t.getPatternFromOrderNumber(order).getSize();
		size = !size ? ptnSize : std::min(size, ptnSize);
	}
	return size;
}

SongStyle Song::getStyle() const
{
	SongStyle style;
	style.type = type_;
	style.trackAttribs = getTrackAttributes();
	return style;
}


std::vector<TrackAttribute> Song::getTrackAttributes() const
{
	std::vector<TrackAttribute> ret;
	std::transform(tracks_.begin(), tracks_.end(), std::back_inserter(ret),
				   [](const Track& track) { return track.getAttribute(); });
	return ret;
}

Track& Song::getTrack(int num)
{
	return tracks_.at(static_cast<size_t>(num));
}

void Song::changeType(SongType type)
{
	if (std::exchange(type_, type) == type_) return;

	switch (type_) {
	case SongType::Standard:	// Previous type: FM3chExpanded
		// Remove FM3-OP2,3,4 (track 3,4,5)
		tracks_.erase(tracks_.begin() + 3, tracks_.begin() + 6);
		for (size_t i = 3; i < tracks_.size(); ++i) {
			const auto attrib = tracks_[i].getAttribute();
			tracks_[i].setAttribute(static_cast<int>(i), attrib.source, attrib.channelInSource);
		}
		break;
	case SongType::FM3chExpanded:	// Previous type: Standard
		// Expand FM3 track
		for (int i = 3; i < 6; ++i) {
			tracks_.insert(tracks_.begin() + 3, tracks_[static_cast<size_t>(i) - 1]);
			tracks_[static_cast<size_t>(i)].setAttribute(i, SoundSource::FM, i + 3);
		}
		for (size_t i = 6; i < tracks_.size(); ++i) {
			const auto attrib = tracks_[i].getAttribute();
			tracks_[i].setAttribute(static_cast<int>(i), attrib.source, attrib.channelInSource);
		}
		break;
	}
}

std::vector<OrderData> Song::getOrderData(int order)
{
	std::vector<OrderData> ret;
	for (auto& track : tracks_) {
		ret.push_back(track.getOrderData(order));
	}
	return ret;
}

size_t Song::getOrderSize() const
{
	return tracks_[0].getOrderSize();
}

bool Song::canAddNewOrder() const
{
	return getOrderSize() < 256;
}

void Song::insertOrderBelow(int order)
{
	if (!canAddNewOrder()) return;
	for (auto& track : tracks_) {
		track.insertOrderBelow(order);
	}
}

void Song::deleteOrder(int order)
{
	for (auto& track : tracks_) {
		track.deleteOrder(order);
	}
}

void Song::swapOrder(int a, int b)
{
	for (auto& track : tracks_) {
		track.swapOrder(a, b);
	}
}

std::unordered_set<int> Song::getRegisteredInstruments() const
{
	std::unordered_set<int> set;
	for (auto& track : tracks_) {
		for (auto& n : track.getRegisteredInstruments()) {
			set.insert(n);
		}
	}
	return set;
}

void Song::clearUnusedPatterns()
{
	for (auto& track : tracks_) track.clearUnusedPatterns();
}

void Song::replaceDuplicateInstrumentsInPatterns(std::unordered_map<int, int> map)
{
	for (auto& track : tracks_) track.replaceDuplicateInstrumentsInPatterns(map);
}

int Song::addBookmark(std::string name, int order, int step)
{
	bms_.push_back(Bookmark(name, order, step));
	return static_cast<int>(bms_.size() - 1);
}

void Song::changeBookmark(int i, std::string name, int order, int step)
{
	Bookmark& bm = bms_.at(static_cast<size_t>(i));
	bm.name = name;
	bm.order = order;
	bm.step  = step;
}

void Song::removeBookmark(int i)
{
	bms_.erase(bms_.begin() + i);
}

void Song::clearBookmark()
{
	bms_.clear();
}

void Song::swapBookmarks(int a, int b)
{
	std::swap(bms_.at(static_cast<size_t>(a)), bms_.at((static_cast<size_t>(b))));
}

void Song::sortBookmarkByPosition()
{
	std::stable_sort(bms_.begin(), bms_.end(), [](Bookmark a, Bookmark b) {
		return ((a.order == b.order) ? (a.step < b.step) : (a.order < b.order));
	});
}

void Song::sortBookmarkByName()
{
	std::stable_sort(bms_.begin(), bms_.end(), [](Bookmark a, Bookmark b) {
		return (a.name < b.name);
	});
}

Bookmark Song::getBookmark(int i) const
{
	return bms_.at(static_cast<size_t>(i));
}

std::vector<int> Song::findBookmarks(int order, int step) const
{
	std::vector<int> idcs;
	for (size_t i = 0; i < bms_.size(); ++i) {
		const Bookmark& bm = bms_[i];
		if (bm.order == order && bm.step == step)
			idcs.push_back(static_cast<int>(i));
	}
	return idcs;
}

std::vector<Bookmark> Song::getSortedBookmarkList() const
{
	std::vector<Bookmark> tmp(bms_);
	std::stable_sort(tmp.begin(), tmp.end(), [](Bookmark a, Bookmark b) {
		return ((a.order == b.order) ? (a.step < b.step) : (a.order < b.order));
	});
	return tmp;
}

Bookmark Song::getPreviousBookmark(int order, int step)
{
	std::vector<Bookmark> list = getSortedBookmarkList();
	size_t i = 0;
	for (; i < list.size(); ++i) {
		Bookmark& bm = list.at(i);
		if (order < bm.order || (order == bm.order && step <= bm.step)) {
			break;
		}
	}
	return list.at((list.size() + i - 1) % list.size());
}

Bookmark Song::getNextBookmark(int order, int step)
{
	std::vector<Bookmark> list = getSortedBookmarkList();
	size_t i = 0;
	for (; i < list.size(); ++i) {
		Bookmark& bm = list.at(i);
		if (order < bm.order || (order == bm.order && step < bm.step)) {
			break;
		}
	}
	return list.at(i % list.size());
}

size_t Song::getBookmarkSize() const
{
	return bms_.size();
}

void Song::transpose(int seminotes, std::vector<int> excludeInsts)
{
	for (auto& track : tracks_) track.transpose(seminotes, excludeInsts);
}

void Song::swapTracks(int track1, int track2)
{
	auto it1 = std::find_if(tracks_.begin(), tracks_.end(), [&](const Track& t) {
		return t.getAttribute().number == track1;
	});
	if (it1 == tracks_.end()) throw std::invalid_argument("Invalid track number");
	auto it2 = std::find_if(tracks_.begin(), tracks_.end(), [&](const Track& t) {
		return t.getAttribute().number == track2;
	});
	if (it2 == tracks_.end()) throw std::invalid_argument("Invalid track number");

	TrackAttribute attrib1 = it1->getAttribute();
	TrackAttribute attrib2 = it2->getAttribute();
	it1->setAttribute(attrib2.number, attrib2.source, attrib2.channelInSource);
	it2->setAttribute(attrib1.number, attrib1.source, attrib1.channelInSource);

	std::iter_swap(it1, it2);
}

Bookmark::Bookmark(std::string argname, int argorder, int argstep)
	: name(argname), order(argorder), step(argstep)
{
}
