#pragma once

#include <vector>
#include <array>
#include <memory>
#include "pattern.hpp"
#include "misc.hpp"

class Track
{
public:
	Track(int number, SoundSource source, int channelInSource);

private:
	int num_;
	SoundSource src_;
	int chInSrc_;

	std::vector<int> order_;
	std::array<std::unique_ptr<Pattern>, 128> patterns_;
};
