#pragma once

#include <vector>
#include <memory>
#include "pattern.hpp"
#include "misc.hpp"

struct TrackAttribute;

class Track
{
public:
	Track(int number, SoundSource source, int channelInSource);
	TrackAttribute getAttribute() const;
	std::vector<int> getOrderList() const;
	Pattern& getPattern(int num);
	Pattern& getPatternFromOrderNumber(int num);

private:
	std::unique_ptr<TrackAttribute> attrib_;

	std::vector<int> order_;
	std::vector<Pattern> patterns_;
};

struct TrackAttribute
{
	int number;
	SoundSource source;
	int channelInSource;
};
