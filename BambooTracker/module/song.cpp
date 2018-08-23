#include "song.hpp"

Song::Song(int number, ModuleType modType, std::string title, unsigned int tickFreq, unsigned int tempo, unsigned int stepSize)
	: num_(number),
	  modType_(modType),
	  title_(title),
	  tickFreq_(tickFreq),
	  tempo_(tempo),
	  stepSize_(stepSize)
{
	switch (modType) {
	case ModuleType::STD:
		for (int i = 0; i < 6; ++i) {
			tracks_.emplace_back(i, SoundSource::FM, i);
		}
		for (int i = 0; i < 3; ++i) {
			tracks_.emplace_back(i + 6, SoundSource::SSG, i);
		}
		break;
	case ModuleType::FMEX:
		// UNDONE: FM extend mode
		break;
	}
}

int Song::getNumber() const
{
	return num_;
}

void Song::setTitle(std::string title)
{
	title_ = title;
}

std::string Song::getTitle() const
{
	return title_;
}

void Song::setTickFrequency(unsigned int freq)
{
	tickFreq_ = freq;
}

unsigned int Song::getTickFrequency() const
{
	return tickFreq_;
}

void Song::setTempo(int tempo)
{
	tempo_ = tempo;
}

int Song::getTempo() const
{
	return tempo_;
}

void Song::setStepSize(size_t size)
{
	stepSize_ = size;
}

size_t Song::getStepSize() const
{
	return stepSize_;
}

std::vector<TrackAttribute> Song::getTrackAttributes() const
{
	std::vector<TrackAttribute> ret;
	for (auto& track : tracks_) {
		ret.push_back(track.getAttribute());
	}
	return ret;
}

Track& Song::getTrack(int num)
{
	return tracks_.at(num);
}

std::vector<OrderData> Song::getOrderData(int order)
{
	std::vector<OrderData> ret;
	for (auto& track : tracks_) {
		ret.push_back(track.getOrderData(order));
	}
	return ret;
}

size_t Song::getOrderSize() const
{
	return tracks_[0].getOrderSize();
}

void Song::insertOrderBelow(int order)
{
	for (auto& track : tracks_) {
		track.insertOrderBelow(order);
	}
}

void Song::deleteOrder(int order)
{
	for (auto& track : tracks_) {
		track.deleteOrder(order);
	}
}
