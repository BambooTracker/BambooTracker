#pragma once

#include <vector>
#include <set>
#include <string>
#include "song.hpp"
#include "groove.hpp"

class Module
{
public:
	Module(std::string filePath = "", std::string title = u8"", std::string author = u8"",
		   std::string copyright = u8"", std::string comment = u8"", unsigned int tickFreq = 60);

	void setFilePath(std::string path);
	std::string getFilePath() const;
	void setTitle(std::string title);
	std::string getTitle() const;
	void setAuthor(std::string author);
	std::string getAuthor() const;
	void setCopyright(std::string copyright);
	std::string getCopyright() const;
	void setComment(std::string comment);
	std::string getComment() const;
	void setTickFrequency(unsigned int freq);
	unsigned int getTickFrequency() const;
	void setStepHighlight1Distance(size_t dist);
	size_t getStepHighlight1Distance() const;
	void setStepHighlight2Distance(size_t dist);
	size_t getStepHighlight2Distance() const;
	size_t getSongCount() const;
	size_t getGrooveCount() const;

	void addSong(SongType songType, std::string title);
	void addSong(int n, SongType songType, std::string title, bool isUsedTempo,
				 int tempo, int groove, int speed, size_t defaultPatternSize);
	void sortSongs(std::vector<int> numbers);
	Song& getSong(int num);

	void addGroove();
	void removeGroove(int num);
	void setGroove(int num, std::vector<int> seq);
	void setGrooves(std::vector<std::vector<int>> seqs);
	Groove& getGroove(int num);

	std::set<int> getRegisterdInstruments() const;

	void clearUnusedPatterns();

private:
	std::string filePath_;
	std::string title_;
	std::string author_;
	std::string copyright_;
	std::string comment_;
	unsigned int tickFreq_;
	size_t stepHl1Dist_, stepHl2Dist_;
	std::vector<Song> songs_;
	std::vector<Groove> grooves_;
};
