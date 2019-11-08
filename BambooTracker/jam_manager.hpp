#pragma once

#include <vector>
#include <deque>
#include "misc.hpp"


struct JamKeyData;
enum class JamKey;


class JamManager
{
public:
	JamManager();
	bool toggleJamMode();
	bool isJamMode() const;
	void polyphonic(bool flag);
	std::vector<JamKeyData> keyOn(JamKey key, int channel, SoundSource source, int keyNum);
	JamKeyData keyOff(JamKey key, int keyNum);

	static Note jamKeyToNote(JamKey& key);
	static JamKey noteToJamKey(Note& note);
	static int calcOctave(int baseOctave, JamKey& key);

	void clear();

private:
	bool isJamMode_;
	bool isPoly_;
	std::vector<JamKeyData> keyOnTable_;
	std::deque<int> unusedChFM_, unusedChSSG_;
};


struct JamKeyData
{
	JamKey key;
	int channelInSource;
	SoundSource source;
	int keyNum;
};


enum class JamKey
{
	LowC, LowCS, LowD, LowDS, LowE, LowF, LowFS, LowG,
	LowGS, LowA, LowAS, LowB, LowC2, LowCS2, LowD2,
	HighC, HighCS, HighD, HighDS, HighE, HighF, HighFS, HighG,
	HighGS, HighA, HighAS, HighB, HighC2, HighCS2, HighD2, MidiKey
};
