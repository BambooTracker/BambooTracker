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
#include <string>
#include <unordered_map>
#include "track.hpp"

enum class SongType
{
	Standard,
	FM3chExpanded
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

	Bookmark(const std::string& argname, int argorder, int argstep);
};

struct KeySignature
{
	enum Type : int
	{
		C	= 0,
		CS	= 1,
		DF	= 2,
		D	= 3,
		DS	= 4,
		EF	= 5,
		E	= 6,
		FF	= 7,
		ES	= 8,
		F	= 9,
		FS	= 10,
		GF	= 11,
		G	= 12,
		GS	= 13,
		AF	= 14,
		A	= 15,
		AS	= 16,
		BF	= 17,
		B	= 18,
		CF	= 19,
		BS	= 20,
		FISRT = C,
		LAST = BS
	} type;
	int order, step;

	KeySignature(Type type, int scorder, int scstep);
};

class Song
{
public:
	Song(int number, SongType songType = SongType::Standard, const std::string& title = u8"",
		 bool isUsedTempo = true, int tempo = 150, int groove = 0, int speed = 6,
		 size_t defaultPatternSize = 64);

	void setNumber(int n) noexcept { num_ = n; }
	int getNumber() const noexcept { return num_; }
	void setTitle(const std::string& title) { title_ = title; }
	std::string getTitle() const noexcept { return title_; }
	void setTempo(int tempo) noexcept { tempo_ = tempo; }
	int getTempo() const noexcept { return tempo_; }
	void setGroove(int groove) noexcept { groove_ = groove; }
	int getGroove() const noexcept { return groove_; }
	void toggleTempoOrGroove(bool isUsedTempo) noexcept { isUsedTempo_ = isUsedTempo; }
	bool isUsedTempo() const noexcept { return isUsedTempo_; }
	void setSpeed(int speed) noexcept { speed_ = speed; }
	int getSpeed() const noexcept { return speed_; }
	void setDefaultPatternSize(size_t size);
	size_t getDefaultPatternSize() const noexcept { return defPtnSize_; }
	size_t getPatternSizeFromOrderNumber(int order);

	SongStyle getStyle() const;
	std::vector<TrackAttribute> getTrackAttributes() const;
	Track& getTrack(int num);
	void changeType(SongType type);

	std::vector<OrderInfo> getOrderData(int order) const;
	size_t getOrderSize() const;
	bool canAddNewOrder() const;
	void insertOrderBelow(int order);
	void deleteOrder(int order);
	void swapOrder(int a, int b);

	std::set<int> getRegisteredInstruments() const;

	void clearUnusedPatterns();
	void replaceDuplicateInstrumentsInPatterns(const std::unordered_map<int, int>& map);

	void transpose(int seminotes, const std::vector<int>& excludeInsts);
	void swapTracks(int track1, int track2);

	// Bookmark
	int addBookmark(const std::string& name, int order, int step);
	void changeBookmark(int i, const std::string& name, int order, int step);
	void removeBookmark(int i);
	void clearBookmark();
	void swapBookmarks(int a, int b);
	void sortBookmarkByPosition();
	void sortBookmarkByName();
	Bookmark getBookmark(int i) const;
	std::vector<int> findBookmarks(int order, int step) const;
	Bookmark getPreviousBookmark(int order, int step) const;
	Bookmark getNextBookmark(int order, int step) const;
	size_t getBookmarkSize() const;

	// Key signature
	void addKeySignature(KeySignature::Type key, int order, int step);
	void changeKeySignature(int i, KeySignature::Type key, int order, int step);
	void removeKeySignature(int i);
	void clearKeySignature();
	KeySignature getKeySignature(int i) const;
	size_t getKeySignatureSize() const;
	KeySignature::Type searchKeySignatureAt(int order, int step) const;

	static size_t getFMChannelCount(SongType type)
	{
		switch (type) {
		case SongType::Standard:		return 6;
		case SongType::FM3chExpanded:	return 9;
		default:	throw std::invalid_argument("Invalid SongType.");
		}
	}

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
	std::vector<KeySignature> sigs_;

	std::vector<Bookmark> getSortedBookmarkList() const;
};
