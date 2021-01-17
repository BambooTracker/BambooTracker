/*
 * Copyright (C) 2018-2020 Rerrah
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#pragma once

#include <vector>
#include <set>
#include <string>
#include <unordered_map>
#include "song.hpp"

using Groove = std::vector<int>;

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
	Module(const std::string& filePath = "", const std::string& title = u8"",
		   const std::string& author = u8"", const std::string& copyright = u8"",
		   const std::string& comment = u8"", unsigned int tickFreq = 60);

	inline void setFilePath(const std::string& path) { filePath_ = path; }
	std::string getFilePath() const noexcept { return filePath_; };
	inline void setTitle(const std::string& title) { title_ = title; }
	inline std::string getTitle() const noexcept { return title_; }
	inline void setAuthor(const std::string& author) { author_ = author; }
	inline std::string getAuthor() const noexcept { return author_; }
	inline void setCopyright(const std::string& copyright) { copyright_ = copyright; }
	inline std::string getCopyright() const noexcept { return copyright_; }
	inline void setComment(const std::string& comment) { comment_ = comment; }
	inline std::string getComment() const noexcept { return comment_; }
	inline void setTickFrequency(unsigned int freq) { tickFreq_ = freq; }
	inline unsigned int getTickFrequency() const noexcept { return tickFreq_; }
	inline void setStepHighlight1Distance(size_t dist) { stepHl1Dist_ = dist; }
	inline size_t getStepHighlight1Distance() const noexcept { return stepHl1Dist_; }
	inline void setStepHighlight2Distance(size_t dist) { stepHl2Dist_ = dist; }
	inline size_t getStepHighlight2Distance() const noexcept { return stepHl2Dist_; }
	size_t getSongCount() const;
	size_t getGrooveCount() const;
	inline void setMixerType(MixerType type) { mixType_ = type; }
	inline MixerType getMixerType() const noexcept { return mixType_; }
	inline void setCustomMixerFMLevel(double level) { customLevelFM_ = level; }
	inline double getCustomMixerFMLevel() const noexcept { return customLevelFM_; }
	inline void setCustomMixerSSGLevel(double level) { customLevelSSG_ = level; }
	inline double getCustomMixerSSGLevel() const noexcept { return customLevelSSG_; }

	void addSong(SongType songType, const std::string& title);
	void addSong(int n, SongType songType, const std::string& title, bool isUsedTempo,
				 int tempo, int groove, int speed, size_t defaultPatternSize);
	void sortSongs(const std::vector<int>& numbers);
	Song& getSong(int num);

	void addGroove();
	void removeGroove(int num);
	void setGroove(int num, const std::vector<int>& seq);
	void setGrooves(const std::vector<std::vector<int>>& seqs);
	Groove getGroove(int num) const;

	std::set<int> getRegisterdInstruments() const;

	void clearUnusedPatterns();
	void replaceDuplicateInstrumentsInPatterns(const std::unordered_map<int, int>& map);

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
