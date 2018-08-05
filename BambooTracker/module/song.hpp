#pragma once

#include <vector>
#include <memory>
#include <string>
#include "track.hpp"
#include "misc.hpp"

class Song
{
public:
	Song(int number, ModuleType modType, std::string title = u8"Song");

private:
	int num_;
	ModuleType modType_;
	std::string title_;

	std::vector<std::unique_ptr<Track>> tracks_;
};
