#include "module.hpp"

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

Song& Module::getSong(int num)
{
	return songs_.at(num);
}
