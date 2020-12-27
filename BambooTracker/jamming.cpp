/*
 * Copyright (C) 2018-2020 Rerrah
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#include "jamming.hpp"
#include <algorithm>
#include <functional>
#include <stdexcept>

namespace jam_utils
{
Note jamKeyToNote(JamKey &key)
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
	case JamKey::HighE:		return Note::E;
	case JamKey::LowF:
	case JamKey::HighF:		return Note::F;
	case JamKey::LowFS:
	case JamKey::HighFS:	return Note::FS;
	case JamKey::LowG:
	case JamKey::HighG:		return Note::G;
	case JamKey::LowGS:
	case JamKey::HighGS:	return Note::GS;
	case JamKey::LowA:
	case JamKey::HighA:		return Note::A;
	case JamKey::LowAS:
	case JamKey::HighAS:	return Note::AS;
	case JamKey::LowB:
	case JamKey::HighB:		return Note::B;
	default:	throw std::invalid_argument("Unexpected JamKey.");
	}
}

JamKey noteToJamKey(Note& note)
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

int calculateJamKeyOctave(int baseOctave, JamKey &key)
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
	case JamKey::HighB:		return (baseOctave + 1);
	case JamKey::HighC2:
	case JamKey::HighCS2:
	case JamKey::HighD2:	return (baseOctave + 2);
	default:	throw std::invalid_argument("Unexpected JamKey.");
	}
}
}

JamManager::JamManager()
	: isJamMode_(true),
	  isPoly_(true)
{
	reset();
}

bool JamManager::toggleJamMode()
{
	isJamMode_ = !isJamMode_;
	return isJamMode_;
}

bool JamManager::isJamMode() const noexcept
{
	return isJamMode_;
}

void JamManager::polyphonic(bool flag)
{
	isPoly_ = flag;
	reset();
}

std::vector<JamKeyInfo> JamManager::keyOn(JamKey key, int channel, SoundSource source, int keyNum)
{
	std::vector<JamKeyInfo> keyDataList;
	JamKeyInfo onData{ key, channel, source, keyNum };
	std::deque<int>& unusedCh = unusedCh_.at(source);

	if (!unusedCh.empty()) {
		if (key == JamKey::MidiKey) {
			if (isPoly_) onData.channelInSource = unusedCh.front();
			unusedCh.pop_front();
			keyOnTable_.push_back(onData);
			keyDataList.push_back(onData);
		}
		else {
			auto&& it = std::find_if(keyOnTable_.begin(),
									 keyOnTable_.end(),
									 [&](JamKeyInfo x) { return (x.source == source && x.key == key); });
			if (it == keyOnTable_.end()) {
				if (isPoly_) onData.channelInSource = unusedCh.front();
				unusedCh.pop_front();
				keyOnTable_.push_back(onData);
				keyDataList.push_back(onData);
			}
			else {
				JamKeyInfo del = *it;
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
								 [&](JamKeyInfo x) { return (x.source == source); });
		JamKeyInfo del = *it;
		if (isPoly_) onData.channelInSource = del.channelInSource;
		keyDataList.push_back(onData);
		keyDataList.push_back(del);
		keyOnTable_.erase(it);
		keyOnTable_.push_back(onData);
	}

	return keyDataList;
}

JamKeyInfo JamManager::keyOff(JamKey key, int keyNum)
{
	JamKeyInfo keyData;

	auto cond = (key == JamKey::MidiKey)
				? std::function<bool(JamKeyInfo)>([&](JamKeyInfo x) -> bool { return (x.key == JamKey::MidiKey && x.keyNum == keyNum); })
			: std::function<bool(JamKeyInfo)>([&](JamKeyInfo x) -> bool { return (x.key == key); });
	auto&& it = std::find_if(keyOnTable_.begin(), keyOnTable_.end(), cond);
	if (it == keyOnTable_.end()) {
		keyData.channelInSource = -1;	// Already released
	}
	else {
		JamKeyInfo data = *it;
		keyData.channelInSource = data.channelInSource;
		keyData.key = key;
		keyData.source = data.source;
		unusedCh_.at(keyData.source).push_front(keyData.channelInSource);
		keyOnTable_.erase(it);
	}

	return keyData;
}

void JamManager::reset()
{
	if (isPoly_) {
		unusedCh_[SoundSource::FM] = std::deque<int>(6);
		unusedCh_[SoundSource::SSG] = std::deque<int>(3);
		unusedCh_[SoundSource::ADPCM] = std::deque<int>(1);

		for (auto& pair : unusedCh_) {
			std::generate(pair.second.begin(), pair.second.end(), [i = 0]() mutable -> int { return i++; });
		}
	}
	else {
		for (auto& pair : unusedCh_) pair.second.resize(1);
	}
}
