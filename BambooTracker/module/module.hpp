#pragma once

#include <vector>
#include <unordered_set>
#include <string>
#include <unordered_map>
#include "song.hpp"
#include "groove.hpp"

enum class MixerType : int
{
	UNSPECIFIED = 0,
	CUSTOM = 1,
	PC_9821_PC_9801_86 = 2,
	PC_9821_SPEAK_BOARD = 3,
	PC_8801_VA2 = 4,
	PC_8801_MKII_SR = 5
};

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
	void setMixerType(MixerType type);
	MixerType getMixerType() const;
	void setCustomMixerFMLevel(double level);
	double getCustomMixerFMLevel() const;
	void setCustomMixerSSGLevel(double level);
	double getCustomMixerSSGLevel() const;

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

	std::unordered_set<int> getRegisterdInstruments() const;

	void clearUnusedPatterns();
	void replaceDuplicateInstrumentsInPatterns(std::unordered_map<int, int> map);

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
	MixerType mixType_;
	double customLevelFM_, customLevelSSG_;
};
