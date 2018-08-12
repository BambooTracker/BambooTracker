#include "module.hpp"

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
	return  style;
}

Song& Module::getSong(int num)
{
	return songs_.at(num);
}
