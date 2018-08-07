#pragma once

#include <vector>
#include "song.hpp"

struct ModuleStyle;

class Module
{
public:
	Module(ModuleType type);
	ModuleStyle getStyle() const;
	std::vector<int> getOrderList(int songNum, int trackNum) const;

private:
	ModuleType type_;
	std::vector<Song> songs_;
};


struct ModuleStyle
{
	ModuleType type;
	std::vector<TrackAttribute> trackAttribs;
};
