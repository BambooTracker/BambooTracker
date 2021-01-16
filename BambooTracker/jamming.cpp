/*
 * Copyright (C) 2018-2021 Rerrah
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
#include <numeric>
#include <functional>
#include <stdexcept>
#include "note.hpp"
#include "misc.hpp"

namespace jam_utils
{
Note makeNote(const JamKeyInfo& info, int baseOctave)
{
	return (info.key == JamKey::MidiKey) ? Note(info.keyNum) : makeNote(baseOctave, info.key);
}

Note makeNote(int baseOctave, JamKey key)
{
	switch (key) {
	case JamKey::LowC:
		return Note(baseOctave, Note::C);
	case JamKey::LowCS:
		return Note(baseOctave, Note::CS);
	case JamKey::LowD:
		return Note(baseOctave, Note::D);
	case JamKey::LowDS:
		return Note(baseOctave, Note::DS);
	case JamKey::LowE:
		return Note(baseOctave, Note::E);
	case JamKey::LowF:
		return Note(baseOctave, Note::F);
	case JamKey::LowFS:
		return Note(baseOctave, Note::FS);
	case JamKey::LowG:
		return Note(baseOctave, Note::G);
	case JamKey::LowGS:
		return Note(baseOctave, Note::GS);
	case JamKey::LowA:
		return Note(baseOctave, Note::A);
	case JamKey::LowAS:
		return Note(baseOctave, Note::AS);
	case JamKey::LowB:
		return Note(baseOctave, Note::B);
	case JamKey::LowC2:
	case JamKey::HighC:
		return Note(baseOctave + 1, Note::C);
	case JamKey::LowCS2:
	case JamKey::HighCS:
		return Note(baseOctave + 1, Note::CS);
	case JamKey::LowD2:
	case JamKey::HighD:
		return Note(baseOctave + 1, Note::D);
	case JamKey::HighDS:
		return Note(baseOctave + 1, Note::DS);
	case JamKey::HighE:
		return Note(baseOctave + 1, Note::E);
	case JamKey::HighF:
		return Note(baseOctave + 1, Note::F);
	case JamKey::HighFS:
		return Note(baseOctave + 1, Note::FS);
	case JamKey::HighG:
		return Note(baseOctave + 1, Note::G);
	case JamKey::HighGS:
		return Note(baseOctave + 1, Note::GS);
	case JamKey::HighA:
		return Note(baseOctave + 1, Note::A);
	case JamKey::HighAS:
		return Note(baseOctave + 1, Note::AS);
	case JamKey::HighB:
		return Note(baseOctave + 1, Note::B);
	case JamKey::HighC2:
		return Note(baseOctave + 2, Note::C);
	case JamKey::HighCS2:
		return Note(baseOctave + 2, Note::CS);
	case JamKey::HighD2:
		return Note(baseOctave + 2, Note::D);
	default:
		throw std::invalid_argument("invalid jam key");
	}
}
}

JamManager::JamManager()
	: isJamMode_(true), isPoly_(true)
{
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
			auto&& it = utils::findIf(keyOnTable_,
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
		auto&& it = utils::findIf(keyOnTable_, [&](JamKeyInfo x) { return (x.source == source); });
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
	auto&& it = utils::findIf(keyOnTable_, cond);
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
		for (auto& pair : unusedCh_) std::iota(pair.second.begin(), pair.second.end(), 0);
	}
	else {
		for (auto& pair : unusedCh_) pair.second.resize(1);
	}
}
