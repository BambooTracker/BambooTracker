#include "song.hpp"

Song::Song(int number, ModuleType modType, std::string title)
	: num_(number), modType_(modType), title_(title)
{
	switch (modType) {
	case ModuleType::STD:
		for (int i = 0; i < 6; ++i) {
			tracks_.emplace_back(i, SoundSource::FM, i);
		}
		for (int i = 0; i < 3; ++i) {
			tracks_.emplace_back(i + 6, SoundSource::PSG, i);
		}
		break;
	case ModuleType::FMEX:
		// UNDONE: FM extend mode
		break;
	}
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
