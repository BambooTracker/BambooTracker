#include "song.hpp"

Song::Song(int number, SongType songType, std::string title, bool isUsedTempo,
		   int tempo, int groove, int speed, size_t defaultPatternSize)
	: num_(number),
	  type_(songType),
	  title_(title),
	  isUsedTempo_(isUsedTempo),
	  tempo_(tempo),
	  groove_(groove),
	  speed_(speed),
	  defPtnSize_(defaultPatternSize)
{
	switch (songType) {
	case SongType::STD:
		for (int i = 0; i < 6; ++i) {
			tracks_.emplace_back(i, SoundSource::FM, i);
		}
		for (int i = 0; i < 3; ++i) {
			tracks_.emplace_back(i + 6, SoundSource::SSG, i);
		}
		for (int i = 0; i < 6; ++i) {
			tracks_.emplace_back(i + 9, SoundSource::DRUM, i);
		}
		break;
	case SongType::FMEX:
		// UNDONE: FM extend mode
		break;
	}
}

void Song::setNumber(int n)
{
	num_ = n;
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

void Song::setTempo(int tempo)
{
	tempo_ = tempo;
}

int Song::getTempo() const
{
	return tempo_;
}

void Song::setGroove(int groove)
{
	groove_ = groove;
}

int Song::getGroove() const
{
	return groove_;
}

void Song::toggleTempoOrGroove(bool isUsedTempo)
{
	isUsedTempo_ = isUsedTempo;
}

bool Song::isUsedTempo() const
{
	return isUsedTempo_;
}

void Song::setSpeed(int speed)
{
	speed_ = speed;
}

int Song::getSpeed() const
{
	return speed_;
}

void Song::setDefaultPatternSize(size_t size)
{
	defPtnSize_ = size;
	for (auto& t : tracks_) {
		t.changeDefaultPatternSize(size);
	}
}

size_t Song::getDefaultPatternSize() const
{
	return defPtnSize_;
}


SongStyle Song::getStyle() const
{
	SongStyle style;
	style.type = type_;
	style.trackAttribs = getTrackAttributes();
	return style;
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

bool Song::canAddNewOrder() const
{
	return getOrderSize() < 256;
}

void Song::insertOrderBelow(int order)
{
	if (!canAddNewOrder()) return;
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

void Song::swapOrder(int a, int b)
{
	for (auto& track : tracks_) {
		track.swapOrder(a, b);
	}
}

std::set<int> Song::getRegisteredInstruments() const
{
	std::set<int> set;
	for (auto& track : tracks_) {
		for (auto& n : track.getRegisteredInstruments()) {
			set.insert(n);
		}
	}
	return set;
}

void Song::clearUnusedPatterns()
{
	for (auto& track : tracks_) track.clearUnusedPatterns();
}
