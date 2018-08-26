#include "module.hpp"
#include <algorithm>
#include <iterator>

Module::Module(std::string title, std::string author, std::string copyright)
	: title_(title),
	  author_(author),
	  copyright_(copyright)
{
	songs_.emplace_back(0);
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

size_t Module::getSongCount() const
{
	return songs_.size();
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
