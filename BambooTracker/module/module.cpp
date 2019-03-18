#include "module.hpp"
#include <algorithm>
#include <iterator>

Module::Module(std::string filePath, std::string title, std::string author, std::string copyright,
			   std::string comment, unsigned int tickFreq)
	: filePath_(filePath),
	  title_(title),
	  author_(author),
	  copyright_(copyright),
	  comment_(comment),
	  tickFreq_(tickFreq),
	  stepHl1Dist_(4),
	  stepHl2Dist_(16)
{
	songs_.emplace_back(0);
	grooves_.emplace_back();
}

void Module::setFilePath(std::string path)
{
	filePath_ = path;
}

std::string Module::getFilePath() const
{
	return filePath_;
}

void Module::setTitle(std::string title)
{
	title_ = title;
}

std::string Module::getTitle() const
{
	return title_;
}

void Module::setAuthor(std::string author)
{
	author_ = author;
}

std::string Module::getAuthor() const
{
	return author_;
}

void Module::setCopyright(std::string copyright)
{
	copyright_ = copyright;
}

std::string Module::getCopyright() const
{
	return copyright_;
}

void Module::setComment(std::string comment)
{
	comment_ = comment;
}

std::string Module::getComment() const
{
	return comment_;
}

void Module::setTickFrequency(unsigned int freq)
{
	tickFreq_ = freq;
}

unsigned int Module::getTickFrequency() const
{
	return tickFreq_;
}

void Module::setStepHighlight1Distance(size_t dist)
{
	stepHl1Dist_ = dist;
}

size_t Module::getStepHighlight1Distance() const
{
	return stepHl1Dist_;
}

void Module::setStepHighlight2Distance(size_t dist)
{
	stepHl2Dist_ = dist;
}

size_t Module::getStepHighlight2Distance() const
{
	return stepHl2Dist_;
}

size_t Module::getSongCount() const
{
	return songs_.size();
}

size_t Module::getGrooveCount() const
{
	return grooves_.size();
}

void Module::addSong(SongType songType, std::string title)
{
	int n = static_cast<int>(songs_.size());
	songs_.emplace_back(n, songType, title);
}

void Module::addSong(int n, SongType songType, std::string title, bool isUsedTempo,
					 int tempo, int groove, int speed, size_t defaultPatternSize)
{
	if (n < static_cast<int>(songs_.size()))
		songs_.at(n) = Song(n, songType, title, isUsedTempo, tempo, groove, speed, defaultPatternSize);
	else
		songs_.emplace_back(
					n, songType, title, isUsedTempo, tempo, groove, speed, defaultPatternSize);
}

void Module::sortSongs(std::vector<int> numbers)
{
	std::vector<Song> newSongs;
	newSongs.reserve(songs_.size());

	for (auto& n : numbers) {
		auto it = std::make_move_iterator(songs_.begin() + n);
		it->setNumber(newSongs.size());
		newSongs.push_back(*it);
	}

	songs_.assign(std::make_move_iterator(newSongs.begin()),
				  std::make_move_iterator(newSongs.end()));
	songs_.shrink_to_fit();
}

Song& Module::getSong(int num)
{
	auto it = std::find_if(songs_.begin(), songs_.end(),
						   [num](Song& s) { return s.getNumber() == num; });
	return *it;
}

void Module::addGroove()
{
	grooves_.emplace_back();
}

void Module::removeGroove(int num)
{
	grooves_.erase(grooves_.begin() + num);
}

void Module::setGroove(int num, std::vector<int> seq)
{
	grooves_.at(num).setSequrnce(seq);
}

void Module::setGrooves(std::vector<std::vector<int>> seqs)
{
	grooves_.clear();
	for (auto& seq : seqs) {
		grooves_.emplace_back(seq);
	}
}

Groove& Module::getGroove(int num)
{
	return grooves_.at(num);
}

std::set<int> Module::getRegisterdInstruments() const
{
	std::set<int> set;
	for (auto& song : songs_) {
		for (auto& n : song.getRegisteredInstruments()) {
			set.insert(n);
		}
	}
	return set;
}

void Module::clearUnusedPatterns()
{
	for (auto& song : songs_) song.clearUnusedPatterns();
}
