#pragma once

#include <cstdint>
#include <string>
#include <memory>
#include "opna.hpp"
#include "jam_manager.hpp"
#include "channel_attribute.hpp"
#include "command_manager.hpp"
#include "instruments_manager.hpp"
#include "abstruct_instrument.hpp"
#include "misc.hpp"

class BambooTracker
{
public:
	BambooTracker();

	// Change octave
	int raiseOctave();
	int lowerOctave();

	// Current channel
	void selectChannel(int channel);
	ChannelAttribute getCurrentChannel() const;

	// Instrument edit
	void addInstrument(int num, std::string name);
	void removeInstrument(int num);
	std::unique_ptr<AbstructInstrument> getInstrument(int num);

	// Undo-Redo
	void undo();
	void redo();

	// Jam mode
	bool toggleJamMode();
	bool isJamMode() const;
	void jamKeyOn(JamKey key);
	void jamKeyOff(JamKey key);

	// Play song
	void startPlaySong();
	void stopPlaySong();
	void readStep();
	void readTick();

	// Stream samples
	void getStreamSamples(int16_t *container, size_t nSamples);

	// Stream details
	int getStreamRate() const;
	int getStreamDuration() const;

private:
	chip::OPNA chip_;
	CommandManager comMan_;
	InstrumentsManager instMan_;

	// Current status
	int octave_;	// 0-7
	ChannelAttribute curChannel_;
	bool isPlaySong_;

	// Chip parameters
	uint8_t mixerPSG_;

	JamManager jamMan_;

	void initChip();

	// Key on-off
	void keyOnFM(int id, Note note, int octave, int fine);
	void keyOnPSG(int id, Note note, int octave, int fine);
	void keyOffFM(int id);
	void keyOffPSG(int id);

	// Volume change
	void changeVolumePSG(int id, int level);
};
