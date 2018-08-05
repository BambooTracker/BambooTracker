#pragma once

#include <vector>
#include <memory>
#include "song.hpp"

class Module
{
public:
	Module(ModuleType modType);

private:
	ModuleType modType_;
	std::vector<std::unique_ptr<Song>> songs_;
};
