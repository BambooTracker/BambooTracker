#pragma once

#include <cstdint>
#include <QObject>
#include "opna.hpp"
#include "audio_stream.hpp"
#include "jam_manager.hpp"
#include "channel_attribute.hpp"
#include "misc.hpp"

class BambooTracker : public QObject
{
	Q_OBJECT

public:
	BambooTracker();

	// Change octave
	int raiseOctave();
	int lowerOctave();

	// Current channel
	void selectChannel(int channel);
	ChannelAttribute getCurrentChannel() const;

	// Jam mode
	bool toggleJamMode();
	bool isJamMode() const;
	void jamKeyOn(JamKey key);
	void jamKeyOff(JamKey key);

	// Play song
	void startPlaySong();
	void stopPlaySong();

private:
	chip::OPNA chip_;
	AudioStream stream_;

	// Current status
	int octave_;	// 0-7
	ChannelAttribute curChannel_;
	bool isPlaySong_;

	// Chip parameters
	uint8_t mixerPSG_;

	JamManager jm_;

	void initChip();

	// Key on-off
	void keyOnFM(int id, Note note, int octave, int fine);
	void keyOnPSG(int id, Note note, int octave, int fine);
	void keyOffFM(int id);
	void keyOffPSG(int id);

	// Volume change
	void changeVolumePSG(int id, int level);

private slots:
	void onNextStepArrived();
	void onNextTickArrived();
};
