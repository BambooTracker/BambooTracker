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
	  stepHlDist_(8)
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

void Module::setStepHighlightDistance(size_t dist)
{
	stepHlDist_ = dist;
}

size_t Module::getStepHighlightDistance() const
{
	return stepHlDist_;
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
	int n = songs_.size();
	songs_.emplace_back(n, songType, title);
}

void Module::sortSongs(std::vector<int> numbers)
{
	std::vector<Song> newSongs;

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
