#pragma once

#include <vector>
#include <set>
#include <string>
#include "track.hpp"
#include "misc.hpp"

struct SongStyle;

class Song
{
public:
	Song(int number, SongType songType = SongType::STD, std::string title = u8"", bool isUsedTempo = true,
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

	SongStyle getStyle() const;
	std::vector<TrackAttribute> getTrackAttributes() const;
	Track& getTrack(int num);

	std::vector<OrderData> getOrderData(int order);
	size_t getOrderSize() const;
	void insertOrderBelow(int order);
	void deleteOrder(int order);
	void swapOrder(int a, int b);

	std::set<int> getRegisteredInstruments() const;

	void clearUnusedPatterns();

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
};

struct SongStyle
{
	SongType type;
	std::vector<TrackAttribute> trackAttribs;
};
