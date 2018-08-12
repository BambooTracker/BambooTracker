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
	std::vector<JamKeyData> keyOn(JamKey key, int channel, SoundSource source);
	JamKeyData keyOff(JamKey key);

	static Note jamKeyToNote(JamKey& key);
	static int calcOctave(int baseOctave, JamKey& key);

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
};


enum class JamKey
{
	LOW_C, LOW_CS, LOW_D, LOW_DS, LOW_E, LOW_F, LOW_FS, LOW_G,
	LOW_GS, LOW_A, LOW_AS, LOW_B, LOW_C_H, LOW_CS_H, LOW_D_H,
	HIGH_C, HIGH_CS, HIGH_D, HIGH_DS, HIGH_E, HIGH_F, HIGH_FS, HIGH_G,
	HIGH_GS, HIGH_A, HIGH_AS, HIGH_B, HIGH_C_H, HIGH_CS_H, HIGH_D_H
};
