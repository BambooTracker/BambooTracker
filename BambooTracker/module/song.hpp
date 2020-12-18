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

#pragma once

#include <vector>
#include <unordered_set>
#include <string>
#include <unordered_map>
#include "track.hpp"
#include "misc.hpp"

struct SongStyle;
struct Bookmark;

class Song
{
public:
	Song(int number, SongType songType = SongType::Standard, std::string title = u8"", bool isUsedTempo = true,
		 int tempo = 150, int groove = 0, int speed = 6, size_t defaultPatternSize = 64);

	void setNumber(int n);
	int getNumber() const;
	void setTitle(std::string title);
	std::string getTitle() const;
	void setTempo(int tempo);
	int getTempo() const;
	void setGroove(int groove);
	int getGroove() const;
	void toggleTempoOrGroove(bool isUsedTempo);
	bool isUsedTempo() const;
	void setSpeed(int speed);
	int getSpeed() const;
	void setDefaultPatternSize(size_t size);
	size_t getDefaultPatternSize() const;
	size_t getPatternSizeFromOrderNumber(int order);

	SongStyle getStyle() const;
	std::vector<TrackAttribute> getTrackAttributes() const;
	Track& getTrack(int num);
	void changeType(SongType type);

	std::vector<OrderInfo> getOrderData(int order);
	size_t getOrderSize() const;
	bool canAddNewOrder() const;
	void insertOrderBelow(int order);
	void deleteOrder(int order);
	void swapOrder(int a, int b);

	std::unordered_set<int> getRegisteredInstruments() const;

	void clearUnusedPatterns();
	void replaceDuplicateInstrumentsInPatterns(std::unordered_map<int, int> map);

	int addBookmark(std::string name, int order, int step);
	void changeBookmark(int i, std::string name, int order, int step);
	void removeBookmark(int i);
	void clearBookmark();
	void swapBookmarks(int a, int b);
	void sortBookmarkByPosition();
	void sortBookmarkByName();
	Bookmark getBookmark(int i) const;
	std::vector<int> findBookmarks(int order, int step) const;
	Bookmark getPreviousBookmark(int order, int step);
	Bookmark getNextBookmark(int order, int step);
	size_t getBookmarkSize() const;

	void transpose(int seminotes, std::vector<int> excludeInsts);
	void swapTracks(int track1, int track2);

private:
	int num_;
	SongType type_;
	std::string title_;
	bool isUsedTempo_;
	int tempo_;
	int groove_;
	int speed_;
	size_t defPtnSize_;

	std::vector<Track> tracks_;
	std::vector<Bookmark> bms_;

	std::vector<Bookmark> getSortedBookmarkList() const;
};

struct SongStyle
{
	SongType type;
	std::vector<TrackAttribute> trackAttribs;	// Always sorted by number
};

struct Bookmark
{
	std::string name = u8"";
	int order, step;

	Bookmark(std::string argname, int argorder, int argstep);
};
