#pragma once

#include <vector>
#include <string>
#include "track.hpp"
#include "misc.hpp"

struct SongStyle;

class Song
{
public:
	Song(int number, SongType songType = SongType::STD, std::string title = u8"",
		 int tempo = 150, size_t stepSize = 6, size_t defaultPatternSize = 64);

	void setNumber(int n);
	int getNumber() const;
	void setTitle(std::string title);
	std::string getTitle() const;
	void setTickFrequency(unsigned int freq);
	unsigned int getTickFrequency() const;
	void setTempo(int tempo);
	int getTempo() const;
	void setStepSize(size_t size);
	size_t getStepSize() const;
	void setDefaultPatternSize(size_t size);
	size_t getDefaultPatternSize() const;

	SongStyle getStyle() const;
	std::vector<TrackAttribute> getTrackAttributes() const;
	Track& getTrack(int num);

	std::vector<OrderData> getOrderData(int order);
	size_t getOrderSize() const;
	void insertOrderBelow(int order);
	void deleteOrder(int order);

private:
	int num_;
	SongType type_;
	std::string title_;
	int tempo_;
	size_t stepSize_;
	size_t defPtnSize_;

	std::vector<Track> tracks_;
};

struct SongStyle
{
	SongType type;
	std::vector<TrackAttribute> trackAttribs;
};
