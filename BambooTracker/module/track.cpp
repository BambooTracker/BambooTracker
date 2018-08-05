#include "track.hpp"
#include <algorithm>

Track::Track(int number, SoundSource source, int channelInSource) :
	num_(number), src_(source), chInSrc_(channelInSource)
{
	order_.push_back(0);
	int n = 0;
	std::generate(patterns_.begin(), patterns_.end(), [&]() { return std::make_unique<Pattern>(n++); });
}
