#pragma once

#include <vector>
#include <string>
#include "song.hpp"
#include "groove.hpp"

class Module
{
public:
	Module(std::string title = u8"", std::string author = u8"",
		   std::string copyright = u8"", unsigned int tickFreq = 60);

	void setTitle(std::string title);
	std::string getTitle() const;
	void setAuthor(std::string author);
	std::string getAuthor() const;
	void setCopyright(std::string copyright);
	std::string getCopyright() const;
	void setTickFrequency(unsigned int freq);
	unsigned int getTickFrequency() const;
	size_t getSongCount() const;
	size_t getGrooveCount() const;

	void addSong(SongType songType, std::string title);
	void sortSongs(std::vector<int> numbers);
	Song& getSong(int num);

	void addGroove();
	void removeGroove(int num);
	void setGroove(int num, std::vector<int> seq);
	void setGrooves(std::vector<std::vector<int>> seqs);
	Groove& getGroove(int num);

private:
	std::string title_;
	std::string author_;
	std::string copyright_;
	unsigned int tickFreq_;
	std::vector<Song> songs_;
	std::vector<Groove> grooves_;
};
