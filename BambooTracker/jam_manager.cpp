#include "jam_manager.hpp"
#include <algorithm>
#include <functional>
#include <stdexcept>

JamManager::JamManager()
	: isJamMode_(true),
	  isPoly_(true)
{
	clear();
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

void JamManager::polyphonic(bool flag)
{
	isPoly_ = flag;
	clear();
}

std::vector<JamKeyData> JamManager::keyOn(JamKey key, int channel, SoundSource source, int keyNum)
{
	std::vector<JamKeyData> keyDataList;
	JamKeyData onData{ key, channel, source, keyNum };

	std::deque<int>* unusedCh = nullptr;
	switch (source) {
	case SoundSource::FM:	unusedCh = &unusedChFM_;	break;
	case SoundSource::SSG:	unusedCh = &unusedChSSG_;	break;
	default:	break;
	}

	if (!unusedCh->empty()) {
		if (key == JamKey::MidiKey) {
			if (isPoly_) onData.channelInSource = unusedCh->front();
			unusedCh->pop_front();
			keyOnTable_.push_back(onData);
			keyDataList.push_back(onData);
		}
		else {
			auto&& it = std::find_if(keyOnTable_.begin(),
									 keyOnTable_.end(),
									 [&](JamKeyData x) { return (x.source == source && x.key == key); });
			if (it == keyOnTable_.end()) {
				if (isPoly_) onData.channelInSource = unusedCh->front();
				unusedCh->pop_front();
				keyOnTable_.push_back(onData);
				keyDataList.push_back(onData);
			}
			else {
				JamKeyData del = *it;
				if (isPoly_) onData.channelInSource = del.channelInSource;
				keyDataList.push_back(onData);
				keyDataList.push_back(del);
				keyOnTable_.erase(it);
				keyOnTable_.push_back(onData);
			}
		}
	}
	else {
		auto&& it = std::find_if(keyOnTable_.begin(),
								 keyOnTable_.end(),
								 [&](JamKeyData x) { return (x.source == source); });
		JamKeyData del = *it;
		if (isPoly_) onData.channelInSource = del.channelInSource;
		keyDataList.push_back(onData);
		keyDataList.push_back(del);
		keyOnTable_.erase(it);
		keyOnTable_.push_back(onData);
	}

	return keyDataList;
}

JamKeyData JamManager::keyOff(JamKey key, int keyNum)
{
	JamKeyData keyData;

	auto cond = (key == JamKey::MidiKey)
			  ? std::function<bool(JamKeyData)>([&](JamKeyData x) -> bool { return (x.key == JamKey::MidiKey && x.keyNum == keyNum); })
			  : std::function<bool(JamKeyData)>([&](JamKeyData x) -> bool { return (x.key == key); });
	auto&& it = std::find_if(keyOnTable_.begin(), keyOnTable_.end(), cond);
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
	case JamKey::LowC:
	case JamKey::LowC2:
	case JamKey::HighC:
	case JamKey::HighC2:	return Note::C;
	case JamKey::LowCS:
	case JamKey::LowCS2:
	case JamKey::HighCS:
	case JamKey::HighCS2:	return Note::CS;
	case JamKey::LowD:
	case JamKey::LowD2:
	case JamKey::HighD:
	case JamKey::HighD2:	return Note::D;
	case JamKey::LowDS:
	case JamKey::HighDS:	return Note::DS;
	case JamKey::LowE:
	case JamKey::HighE:	return Note::E;
	case JamKey::LowF:
	case JamKey::HighF:	return Note::F;
	case JamKey::LowFS:
	case JamKey::HighFS:	return Note::FS;
	case JamKey::LowG:
	case JamKey::HighG:	return Note::G;
	case JamKey::LowGS:
	case JamKey::HighGS:	return Note::GS;
	case JamKey::LowA:
	case JamKey::HighA:	return Note::A;
	case JamKey::LowAS:
	case JamKey::HighAS:	return Note::AS;
	case JamKey::LowB:
	case JamKey::HighB:	return Note::B;
	default:	throw std::invalid_argument("Unexpected JamKey.");
	}
}

JamKey JamManager::noteToJamKey(Note& note)
{
	switch (note) {
	case Note::C:	return JamKey::LowC;
	case Note::CS:	return JamKey::LowCS;
	case Note::D:	return JamKey::LowD;
	case Note::DS:	return JamKey::LowDS;
	case Note::E:	return JamKey::LowE;
	case Note::F:	return JamKey::LowF;
	case Note::FS:	return JamKey::LowFS;
	case Note::G:	return JamKey::LowG;
	case Note::GS:	return JamKey::LowGS;
	case Note::A:	return JamKey::LowA;
	case Note::AS:	return JamKey::LowAS;
	case Note::B:	return JamKey::LowB;
	default:	throw std::invalid_argument("Unexpected Note.");
	}
}

int JamManager::calcOctave(int baseOctave, JamKey &key)
{
	switch (key) {
	case JamKey::LowC:
	case JamKey::LowCS:
	case JamKey::LowD:
	case JamKey::LowDS:
	case JamKey::LowE:
	case JamKey::LowF:
	case JamKey::LowFS:
	case JamKey::LowG:
	case JamKey::LowGS:
	case JamKey::LowA:
	case JamKey::LowAS:
	case JamKey::LowB:		return baseOctave;
	case JamKey::LowC2:
	case JamKey::LowCS2:
	case JamKey::LowD2:
	case JamKey::HighC:
	case JamKey::HighCS:
	case JamKey::HighD:
	case JamKey::HighDS:
	case JamKey::HighE:
	case JamKey::HighF:
	case JamKey::HighFS:
	case JamKey::HighG:
	case JamKey::HighGS:
	case JamKey::HighA:
	case JamKey::HighAS:
	case JamKey::HighB:	return (baseOctave + 1);
	case JamKey::HighC2:
	case JamKey::HighCS2:
	case JamKey::HighD2:	return (baseOctave + 2);
	default:	throw std::invalid_argument("Unexpected JamKey.");
	}
}

void JamManager::clear()
{
	if (isPoly_) {
		unusedChFM_ = std::deque<int>(6);
		unusedChSSG_= std::deque<int>(3);

		auto gennums = [i = 0]() mutable -> int { return i++; };
		std::generate(unusedChFM_.begin(), unusedChFM_.end(), gennums);
		std::generate(unusedChSSG_.begin(), unusedChSSG_.end(), gennums);
	}
	else {
		unusedChFM_.resize(1);
		unusedChSSG_.resize(1);
	}
}
