#include "module.hpp"

Module::Module(ModuleType modType) :
	modType_(modType)
{
	songs_.push_back(std::make_unique<Song>(0, modType));
}
