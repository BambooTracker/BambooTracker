#pragma once

#include <vector>
#include <string>
#include "song.hpp"

struct ModuleStyle;

class Module
{
public:
	Module(ModuleType type,
		   std::string title = u8"", std::string author = u8"", std::string copyright = u8"");

	void setTitle(std::string title);
	std::string getTitle() const;
	void setAuthor(std::string author);
	std::string getAuthor() const;
	void setCopyright(std::string copyright);
	std::string getCopyright() const;

	ModuleStyle getStyle() const;
	Song& getSong(int num);

private:
	ModuleType type_;
	std::string title_;
	std::string author_;
	std::string copyright_;
	std::vector<Song> songs_;
};


struct ModuleStyle
{
	ModuleType type;
	std::vector<TrackAttribute> trackAttribs;
};
