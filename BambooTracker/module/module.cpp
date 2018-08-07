#include "module.hpp"
#include <utility>

Module::Module(ModuleType type)
	: type_(type)
{
	songs_.emplace_back(0, type);
}

ModuleStyle Module::getStyle() const
{
	ModuleStyle style;
	style.type = type_;
	style.trackAttribs = songs_.at(0).getTrackAttributes();
	return std::move(style);
}

std::vector<int> Module::getOrderList(int songNum, int trackNum) const
{
	return songs_.at(songNum).getOrderList(trackNum);
}
