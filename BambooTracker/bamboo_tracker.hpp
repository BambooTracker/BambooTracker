#pragma once

#include <cstdint>
#include <string>
#include <memory>
#include "opna_controller.hpp"

#include "opna.hpp"

#include "jam_manager.hpp"
#include "channel_attribute.hpp"
#include "command_manager.hpp"
#include "instruments_manager.hpp"
#include "instrument.hpp"
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

	// Current instrument
	void setCurrentInstrument(int n);

	// Instrument edit
	void addInstrument(int num, std::string name);
	void removeInstrument(int num);
	std::unique_ptr<AbstructInstrument> getInstrument(int num);
	void setInstrumentName(int num, std::string name);
	void setInstrumentFMParameter(int num, FMParameter param, int value);
	void setInstrumentFMOperatorEnable(int instNum, int opNum, bool enable);

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
	CommandManager comMan_;
	InstrumentsManager instMan_;
	JamManager jamMan_;
	OPNAController opnaCtrl_;

	// Current status
	int octave_;	// 0-7
	ChannelAttribute curChannel_;
	int curInstNum_;
	bool isPlaySong_;
};
