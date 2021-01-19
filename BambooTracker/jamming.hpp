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

#pragma once

#include <vector>
#include <deque>
#include <unordered_map>
#include "enum_hash.hpp"
#include "bamboo_tracker_defs.hpp"

class Note;

enum class JamKey
{
	LowC, LowCS, LowD, LowDS, LowE, LowF, LowFS, LowG,
	LowGS, LowA, LowAS, LowB, LowC2, LowCS2, LowD2,
	HighC, HighCS, HighD, HighDS, HighE, HighF, HighFS, HighG,
	HighGS, HighA, HighAS, HighB, HighC2, HighCS2, HighD2, MidiKey
};

struct JamKeyInfo
{
	JamKey key;
	int channelInSource;
	SoundSource source;
	int keyNum;
};

namespace jam_utils
{
Note makeNote(const JamKeyInfo& info, int baseOctave);
Note makeNote(int baseOctave, JamKey key);
}

class JamManager
{
public:
	JamManager();
	bool toggleJamMode();
	bool isJamMode() const noexcept { return isJamMode_; }
	void polyphonic(bool flag);
	std::vector<JamKeyInfo> keyOn(JamKey key, int channel, SoundSource source, int keyNum);
	JamKeyInfo keyOff(JamKey key, int keyNum);
	void reset();

private:
	bool isJamMode_;
	bool isPoly_;
	std::vector<JamKeyInfo> keyOnTable_;
	std::unordered_map<SoundSource, std::deque<int>> unusedCh_;
};

//===============================================
inline bool JamManager::toggleJamMode()
{
	isJamMode_ = !isJamMode_;
	return isJamMode_;
}

inline void JamManager::polyphonic(bool flag)
{
	isPoly_ = flag;
	reset();
}
