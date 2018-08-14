#pragma once

#include <cstdint>
#include <string>
#include <memory>
#include <vector>
#include "opna_controller.hpp"
#include "jam_manager.hpp"
#include "command_manager.hpp"
#include "instruments_manager.hpp"
#include "instrument.hpp"
#include "tick_counter.hpp"
#include "module.hpp"
#include "misc.hpp"

class BambooTracker
{
public:
	BambooTracker();

	// Change octave
	void setCurrentOctave(int octave);
	int getCurrentOctave() const;

	// Current track
	void setCurrentTrack(int num);
	TrackAttribute getCurrentTrackAttribute() const;

	// Current instrument
	void setCurrentInstrument(int n);
	int getCurrentInstrumentNumber() const;

	// Instrument edit
	void addInstrument(int num, std::string name);
	void removeInstrument(int num);
	std::unique_ptr<AbstructInstrument> getInstrument(int num);
	void pasteInstrument(int num, int refNum);
	void cloneInstrument(int num, int refNum);
	int findFirstFreeInstrumentNumber() const;
	void setInstrumentName(int num, std::string name);
	void setEnvelopeFMParameter(int envNum, FMParameter param, int value);
	void setEnvelopeFMOperatorEnable(int envNum, int opNum, bool enable);
	void setInstrumentFMEnvelope(int instNum, int envNum);

	// Song edit
	void setCurrentSongNumber(int num);
	int getCurrentSongNumber() const;

	// Order edit
	int getCurrentOrderNumber() const;
	void setCurrentOrderNumber(int num);

	// Pattern edit
	int getCurrentStepNumber() const;
	void setCurrentStepNumber(int num);

	// Undo-Redo
	void undo();
	void redo();

	// Jam mode
	void toggleJamMode();
	bool isJamMode() const;
	void jamKeyOn(JamKey key);
	void jamKeyOff(JamKey key);

	// Play song
	void startPlaySong();
	void startPlayPattern();
	void startPlayFromCurrentStep();
	void stopPlaySong();
	bool isPlaySong() const;
	void readStep();
	void readTick();

	// Stream events
	int streamCountUp();
	void getStreamSamples(int16_t *container, size_t nSamples);

	// Stream details
	int getStreamRate() const;
	int getStreamDuration() const;
	int getStreamInterruptRate() const;

	// Module details
	ModuleStyle getModuleStyle() const;
	std::vector<int> getOrderList(int songNum, int trackNum) const;
	int getStepNoteNumber(int songNum, int trackNum, int orderNum, int stepNum) const;
	void setStepNote(int songNum, int trackNum, int orderNum, int stepNum, int octave, Note note);
	void setStepKeyOn(int songNum, int trackNum, int orderNum, int stepNum);
	void setStepKeyOff(int songNum, int trackNum, int orderNum, int stepNum);
	void setStepKeyRelease(int songNum, int trackNum, int orderNum, int stepNum);
	void eraseStepNote(int songNum, int trackNum, int orderNum, int stepNum);
	int getStepInstrument(int songNum, int trackNum, int orderNum, int stepNum) const;
	void setStepInstrument(int songNum, int trackNum, int orderNum, int stepNum, int instNum);
	void eraseStepInstrument(int songNum, int trackNum, int orderNum, int stepNum);
	int getStepVolume(int songNum, int trackNum, int orderNum, int stepNum) const;
	void setStepVolume(int songNum, int trackNum, int orderNum, int stepNum, int volume);
	void eraseStepVolume(int songNum, int trackNum, int orderNum, int stepNum);
	std::string getStepEffectString(int songNum, int trackNum, int orderNum, int stepNum) const;
	void setStepEffectString(int songNum, int trackNum, int orderNum, int stepNum, std::string str);
	void eraseEffectString(int songNum, int trackNum, int orderNum, int stepNum);
	size_t getPatternSizeFromOrderNumber(int songNum, int orderNum) const;

private:
	CommandManager comMan_;
	InstrumentsManager instMan_;
	JamManager jamMan_;
	OPNAController opnaCtrl_;

    TickCounter tickCounter_;

	std::shared_ptr<Module> mod_;

	// Current status
	int octave_;	// 0-7
	int curSongNum_;
	ModuleStyle modStyle_;
	int curTrackNum_;
	int curOrderNum_;
	int curStepNum_;
	///	-1: not set
	int curInstNum_;
	/// High nibble - play type
	///		bit 4: play song
	///		bit 5: play pattern
	///		bit 6: play from current step
	/// Low nibble - read state
	///		bit 0: playing
	///		bit 1: have read first step data
	unsigned int playState_;

	int streamIntrRate_;

	void startPlay();
};
