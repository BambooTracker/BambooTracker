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
#include "song.hpp"
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
	void cloneInstrument(int num, int refNum);
	void deepCloneInstrument(int num, int refNum);
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
	void clearCommandHistory();

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
	void setTrackMuteState(int trackNum, bool isMute);
	bool isMute(int trackNum);

	// Stream events
	int streamCountUp();
	void getStreamSamples(int16_t *container, size_t nSamples);

	// Stream details
	int getStreamRate() const;
	int getStreamDuration() const;
	int getStreamInterruptRate() const;

	// Module details
	/*----- Module -----*/
	void setModuleTitle(std::string title);
	std::string getModuleTitle() const;
	void setModuleAuthor(std::string author);
	std::string getModuleAuthor() const;
	void setModuleCopyright(std::string copyright);
	std::string getModuleCopyright() const;
	/*----- Song -----*/
	void setSongTitle(int songNum, std::string title);
	std::string getSongTitle(int songNum) const;
	void setSongTickFrequency(int songNum, unsigned int freq);
	unsigned int getSongTickFrequency(int songNum) const;
	void setSongTempo(int songNum, int tempo);
	int getSongtempo(int songNum) const;
	SongStyle getSongStyle(int songNum) const;
	void setSongStepSize(int songNum, size_t size);
	size_t getSongStepSize(int songNum) const;
	size_t getSongCount() const;
	void addSong(SongType songType, std::string title);
	void sortSongs(std::vector<int> numbers);
	/*----- Order -----*/
	std::vector<OrderData> getOrderData(int songNum, int orderNum) const;
	void setOrderPattern(int songNum, int trackNum, int orderNum, int patternNum);
	void insertOrderBelow(int songNum, int orderNum);
	void deleteOrder(int songNum, int orderNum);
	void pasteOrderCells(int songNum, int beginTrack, int beginOrder,
						   std::vector<std::vector<std::string>> cells);
	size_t getOrderSize(int songNum) const;
	/*----- Pattern -----*/
	int getStepNoteNumber(int songNum, int trackNum, int orderNum, int stepNum) const;
	void setStepNote(int songNum, int trackNum, int orderNum, int stepNum, int octave, Note note);
	void setStepKeyOff(int songNum, int trackNum, int orderNum, int stepNum);
	void eraseStepNote(int songNum, int trackNum, int orderNum, int stepNum);
	int getStepInstrument(int songNum, int trackNum, int orderNum, int stepNum) const;
	void setStepInstrument(int songNum, int trackNum, int orderNum, int stepNum, int instNum);
	void eraseStepInstrument(int songNum, int trackNum, int orderNum, int stepNum);
	int getStepVolume(int songNum, int trackNum, int orderNum, int stepNum) const;
	void setStepVolume(int songNum, int trackNum, int orderNum, int stepNum, int volume);
	void eraseStepVolume(int songNum, int trackNum, int orderNum, int stepNum);
	std::string getStepEffectID(int songNum, int trackNum, int orderNum, int stepNum) const;
	void setStepEffectID(int songNum, int trackNum, int orderNum, int stepNum, std::string id);
	int getStepEffectValue(int songNum, int trackNum, int orderNum, int stepNum) const;
	void setStepEffectValue(int songNum, int trackNum, int orderNum, int stepNum, int value);
	void eraseStepEffect(int songNum, int trackNum, int orderNum, int stepNum);
	void eraseStepEffectValue(int songNum, int trackNum, int orderNum, int stepNum);
	void deletePreviousStep(int songNum, int trackNum, int orderNum, int stepNum);
	void insertStep(int songNum, int trackNum, int orderNum, int stepNum);
	/// beginColumn
	///		0: note
	///		1: instrument
	///		2: volume
	///		3: effect id
	///		4: effect value
	void pastePatternCells(int songNum, int beginTrack, int beginColmn, int beginOrder, int beginStep,
						   std::vector<std::vector<std::string>> cells);
	void erasePatternCells(int songNum, int beginTrack, int beginColmn, int beginOrder, int beginStep,
						   int endTrack, int endColmn, int endStep);
	size_t getPatternSizeFromOrderNumber(int songNum, int orderNum) const;
	void setDefaultPatternSize(int songNum, size_t size);
	size_t getDefaultPatternSize(int songNum) const;

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
	SongStyle songStyle_;
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

	int nextReadStepOrder_, nextReadStepStep_;

	int streamIntrRate_;

	// Play song
	void startPlay();
	void stepDown();
	void findNextStep();
	void readStep();
	void readTick(int rest);
};
