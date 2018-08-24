#pragma once

#include <vector>
#include <string>
#include "song.hpp"

class Module
{
public:
	Module(std::string title = u8"", std::string author = u8"", std::string copyright = u8"");

	void setTitle(std::string title);
	std::string getTitle() const;
	void setAuthor(std::string author);
	std::string getAuthor() const;
	void setCopyright(std::string copyright);
	std::string getCopyright() const;

	Song& getSong(int num);

private:
	std::string title_;
	std::string author_;
	std::string copyright_;
	std::vector<Song> songs_;
};
