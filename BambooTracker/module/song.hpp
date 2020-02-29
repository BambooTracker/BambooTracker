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

	std::vector<OrderData> getOrderData(int order);
	size_t getOrderSize() const;
	bool canAddNewOrder() const;
	void insertOrderBelow(int order);
	void deleteOrder(int order);
	void swapOrder(int a, int b);

	std::unordered_set<int> getRegisteredInstruments() const;

	void clearUnusedPatterns();
	void replaceDuplicateInstrumentsInPatterns(std::unordered_map<int, int> map);

	int addBookmark(std::string name, int order, int step);
	int changeBookmark(int i, std::string name, int order, int step);
	void removeBookmark(int i);
	void clearBookmark();
	void swapBookmarks(int a, int b);
	void sortBookmarkByPosition();
	void sortBookmarkByName();
	Bookmark getBookmark(int i) const;
	std::vector<int> findBookmarks(int order, int step) const;
	size_t getBookmarkSize() const;

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
};

struct SongStyle
{
	SongType type;
	std::vector<TrackAttribute> trackAttribs;
};

struct Bookmark
{
	std::string name = u8"";
	int order, step;

	Bookmark(std::string argname, int argorder, int argstep);
};
