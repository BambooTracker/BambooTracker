#pragma once

#include <vector>
#include <string>
#include "track.hpp"
#include "misc.hpp"

class Song
{
public:
	Song(int number, ModuleType modType, std::string title = u8"Song");
	std::vector<TrackAttribute> getTrackAttributes() const;
	Track& getTrack(int num);

	std::vector<OrderData> getOrderData(int order);
	void insertOrderBelow(int order);
	void deleteOrder(int order);

private:
	int num_;
	ModuleType modType_;
	std::string title_;

	std::vector<Track> tracks_;
};
