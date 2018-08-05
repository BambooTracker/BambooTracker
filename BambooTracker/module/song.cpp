#include "song.hpp"

Song::Song(int number, ModuleType modType, std::string title) :
	num_(number), modType_(modType), title_(title)

{
	switch (modType) {
	case ModuleType::STD:
		for (int i = 0; i < 6; ++i) {
			tracks_.push_back(std::make_unique<Track>(i, SoundSource::FM, i));
		}
		for (int i = 0; i < 3; ++i) {
			tracks_.push_back(std::make_unique<Track>(i + 6, SoundSource::PSG, i));
		}
		break;
	case ModuleType::FMEX:
		// UNDONE: FM extend mode
		break;
	}
}
