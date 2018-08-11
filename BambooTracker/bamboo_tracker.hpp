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
	int raiseOctave();
	int lowerOctave();

	// Current track
	void setCurrentTrack(int num);
	TrackAttribute getCurrentTrackAttribute() const;

	// Current instrument
	void setCurrentInstrument(int n);

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
	void insertNewOrder(int prevOdrNum);

	// Pattern edit
	int getCurrentStepNumber() const;
	void setCurrentStepNumber(int num);

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
	int getStepInstrumentNumber(int songNum, int trackNum, int orderNum, int stepNum) const;
	int getStepVolume(int songNum, int trackNum, int orderNum, int stepNum) const;
	std::string getStepEffectString(int songNum, int trackNum, int orderNum, int stepNum) const;
	size_t getPatternSizeFromOrderNumber(int songNum, int orderNum) const;

private:
	CommandManager comMan_;
	InstrumentsManager instMan_;
	JamManager jamMan_;
	OPNAController opnaCtrl_;

    TickCounter tickCounter_;

	std::unique_ptr<Module> mod_;

	// Current status
	int octave_;	// 0-7
	int curSongNum_;
	ModuleStyle modStyle_;
	int curTrackNum_;
	int curOrderNum_;
	int curStepNum_;
	int curInstNum_;

	int streamIntrRate_;
};
