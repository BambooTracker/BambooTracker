#include "jam_manager.hpp"
#include <algorithm>

JamManager::JamManager(SongType type)
	: isJamMode_(true),
	  isPoly_(true)
{
	clear(type);
}

bool JamManager::toggleJamMode()
{
	isJamMode_ = !isJamMode_;
	return isJamMode_;
}

bool JamManager::isJamMode() const
{
	return isJamMode_;
}

void JamManager::polyphonic(bool flag, SongType type)
{
	isPoly_ = flag;
	clear(type);
}

std::vector<JamKeyData> JamManager::keyOn(JamKey key, int channel, SoundSource source)
{
	std::vector<JamKeyData> keyDataList;
	JamKeyData onData{key, channel, source};

	std::deque<int>* unusedCh = nullptr;
	switch (source) {
	case SoundSource::FM:	unusedCh = &unusedChFM_;	break;
	case SoundSource::SSG:	unusedCh = &unusedChSSG_;	break;
	default:	break;
	}

	if (!unusedCh->empty()) {
		if (isPoly_) onData.channelInSource = unusedCh->front();
		unusedCh->pop_front();
		keyOnTable_.push_back(onData);
		keyDataList.push_back(onData);
	}
	else {
		auto&& it = std::find_if(keyOnTable_.begin(),
								 keyOnTable_.end(),
								 [&](JamKeyData x) {return (x.source == source);});
		JamKeyData del = *it;
		if (isPoly_) onData.channelInSource = del.channelInSource;
		keyDataList.push_back(onData);
		keyDataList.push_back(del);
		keyOnTable_.erase(it);
		keyOnTable_.push_back(onData);
	}

	return keyDataList;
}

JamKeyData JamManager::keyOff(JamKey key)
{
	JamKeyData keyData;

	auto&& it = std::find_if(keyOnTable_.begin(),
							 keyOnTable_.end(),
							 [&](JamKeyData x) {return (x.key == key);});
	if (it == keyOnTable_.end()) {
		keyData.channelInSource = -1;	// Already released
	}
	else {
		JamKeyData data = *it;
		keyData.channelInSource = data.channelInSource;
		keyData.key = key;
		keyData.source = data.source;
		switch (keyData.source) {
		case SoundSource::FM:	unusedChFM_.push_front(keyData.channelInSource);	break;
		case SoundSource::SSG:	unusedChSSG_.push_front(keyData.channelInSource);	break;
		default:	break;
		}
		keyOnTable_.erase(it);
	}

	return keyData;
}

Note JamManager::jamKeyToNote(JamKey &key)
{
	switch (key) {
	case JamKey::LOW_C:
	case JamKey::LOW_C_H:
	case JamKey::HIGH_C:
	case JamKey::HIGH_C_H:	return Note::C;
	case JamKey::LOW_CS:
	case JamKey::LOW_CS_H:
	case JamKey::HIGH_CS:
	case JamKey::HIGH_CS_H:	return Note::CS;
	case JamKey::LOW_D:
	case JamKey::LOW_D_H:
	case JamKey::HIGH_D:
	case JamKey::HIGH_D_H:	return Note::D;
	case JamKey::LOW_DS:
	case JamKey::HIGH_DS:	return Note::DS;
	case JamKey::LOW_E:
	case JamKey::HIGH_E:	return Note::E;
	case JamKey::LOW_F:
	case JamKey::HIGH_F:	return Note::F;
	case JamKey::LOW_FS:
	case JamKey::HIGH_FS:	return Note::FS;
	case JamKey::LOW_G:
	case JamKey::HIGH_G:	return Note::G;
	case JamKey::LOW_GS:
	case JamKey::HIGH_GS:	return Note::GS;
	case JamKey::LOW_A:
	case JamKey::HIGH_A:	return Note::A;
	case JamKey::LOW_AS:
	case JamKey::HIGH_AS:	return Note::AS;
	case JamKey::LOW_B:
	case JamKey::HIGH_B:	return Note::B;
	}
}

int JamManager::calcOctave(int baseOctave, JamKey &key)
{
	switch (key) {
	case JamKey::LOW_C:
	case JamKey::LOW_CS:
	case JamKey::LOW_D:
	case JamKey::LOW_DS:
	case JamKey::LOW_E:
	case JamKey::LOW_F:
	case JamKey::LOW_FS:
	case JamKey::LOW_G:
	case JamKey::LOW_GS:
	case JamKey::LOW_A:
	case JamKey::LOW_AS:
	case JamKey::LOW_B:		return baseOctave;
	case JamKey::LOW_C_H:
	case JamKey::LOW_CS_H:
	case JamKey::LOW_D_H:
	case JamKey::HIGH_C:
	case JamKey::HIGH_CS:
	case JamKey::HIGH_D:
	case JamKey::HIGH_DS:
	case JamKey::HIGH_E:
	case JamKey::HIGH_F:
	case JamKey::HIGH_FS:
	case JamKey::HIGH_G:
	case JamKey::HIGH_GS:
	case JamKey::HIGH_A:
	case JamKey::HIGH_AS:
	case JamKey::HIGH_B:	return (baseOctave + 1);
	case JamKey::HIGH_C_H:
	case JamKey::HIGH_CS_H:
	case JamKey::HIGH_D_H:	return (baseOctave + 2);
	}
}

void JamManager::clear(SongType type)
{
	if (isPoly_) {
		switch (type) {
		case SongType::STD:
			unusedChFM_ = std::deque<int>(6);
			unusedChSSG_= std::deque<int>(3);
			break;
		case SongType::FMEX:
			// UNDONE: change channel size by Effect mode
			break;
		}

		for (int i = 0; i < unusedChFM_.size(); ++i) {
			unusedChFM_[i] = i;
		}
		for (int i = 0; i < unusedChSSG_.size(); ++i) {
			unusedChSSG_[i] = i;
		}
	}
	else {
		unusedChFM_.resize(1);
		unusedChSSG_.resize(1);
	}
}
