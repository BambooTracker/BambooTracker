/*
 * Copyright (C) 2018-2021 Rerrah
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

#include "module.hpp"
#include <algorithm>
#include <iterator>
#include "utils.hpp"

Module::Module(const std::string& filePath, const std::string& title,
			   const std::string& author, const std::string& copyright,
			   const std::string& comment, unsigned int tickFreq)
	: filePath_(filePath),
	  title_(title),
	  author_(author),
	  copyright_(copyright),
	  comment_(comment),
	  tickFreq_(tickFreq),
	  stepHl1Dist_(4),
	  stepHl2Dist_(16),
	  mixType_(MixerType::PC_9821_PC_9801_86),
	  customLevelFM_(0),
	  customLevelSSG_(0)
{
	songs_.emplace_back(0);
	addGroove();
}

size_t Module::getSongCount() const
{
	return songs_.size();
}

size_t Module::getGrooveCount() const
{
	return grooves_.size();
}

void Module::addSong(SongType songType, const std::string& title)
{
	songs_.emplace_back(static_cast<int>(songs_.size()), songType, title);
}

void Module::addSong(int n, SongType songType, const std::string& title, bool isUsedTempo,
					 int tempo, int groove, int speed, size_t defaultPatternSize)
{
	if (n < static_cast<int>(songs_.size()))
		songs_.at(static_cast<size_t>(n))
				= Song(n, songType, title, isUsedTempo, tempo, groove, speed, defaultPatternSize);
	else
		songs_.emplace_back(
					n, songType, title, isUsedTempo, tempo, groove, speed, defaultPatternSize);
}

void Module::sortSongs(const std::vector<int>& numbers)
{
	std::vector<Song> newSongs;
	newSongs.reserve(songs_.size());

	for (auto& n : numbers) {
		auto it = std::make_move_iterator(songs_.begin() + n);
		it->setNumber(static_cast<int>(newSongs.size()));
		newSongs.push_back(*it);
	}

	songs_ = std::move(newSongs);
}

Song& Module::getSong(int num)
{
	return *utils::findIf(songs_, [num](Song& s) { return s.getNumber() == num; });;
}

void Module::addGroove()
{
	// Default groove is "6 6"
	grooves_.push_back({ 6, 6 });
}

void Module::removeGroove(int num)
{
	grooves_.erase(grooves_.begin() + num);
}

void Module::setGroove(int num, const std::vector<int>& seq)
{
	grooves_.at(static_cast<size_t>(num)) = seq;
}

void Module::setGrooves(const std::vector<std::vector<int>>& seqs)
{
	grooves_ = seqs;
}

Groove Module::getGroove(int num) const
{
	return grooves_.at(static_cast<size_t>(num));
}

std::set<int> Module::getRegisterdInstruments() const
{
	std::set<int> set;
	for (const Song& song : songs_) {
		auto&& subset = song.getRegisteredInstruments();
		std::copy(subset.begin(), subset.end(), std::inserter(set, set.end()));
	}
	return set;
}

void Module::clearUnusedPatterns()
{
	for (Song& song : songs_) song.clearUnusedPatterns();
}

void Module::replaceDuplicateInstrumentsInPatterns(const std::unordered_map<int, int>& map)
{
	for (Song& song : songs_) song.replaceDuplicateInstrumentsInPatterns(map);
}
