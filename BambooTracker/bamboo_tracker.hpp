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

	//--- FM
	void setEnvelopeFMParameter(int envNum, FMEnvelopeParameter param, int value);
	void setEnvelopeFMOperatorEnable(int envNum, int opNum, bool enable);
	void setInstrumentFMEnvelope(int instNum, int envNum);
	std::vector<int> getEnvelopeFMUsers(int envNum) const;

	void setLFOFMParameter(int lfoNum, FMLFOParameter param, int value);
	void setInstrumentFMLFO(int instNum, int lfoNum);
	std::vector<int> getLFOFMUsers(int lfoNum) const;

	void addOperatorSequenceFMSequenceCommand(FMEnvelopeParameter param, int opSeqNum, int type, int data);
	void removeOperatorSequenceFMSequenceCommand(FMEnvelopeParameter param, int opSeqNum);
	void setOperatorSequenceFMSequenceCommand(FMEnvelopeParameter param, int opSeqNum, int cnt, int type, int data);
	void setOperatorSequenceFMLoops(FMEnvelopeParameter param, int opSeqNum, std::vector<int> begins, std::vector<int> ends, std::vector<int> times);
	void setOperatorSequenceFMRelease(FMEnvelopeParameter param, int opSeqNum, ReleaseType type, int begin);
	void setInstrumentFMOperatorSequence(int instNum, FMEnvelopeParameter param, int opSeqNum);
	std::vector<int> getOperatorSequenceFMUsers(FMEnvelopeParameter param, int opSeqNum) const;

	void setArpeggioFMType(int arpNum, int type);
	void addArpeggioFMSequenceCommand(int arpNum, int type, int data);
	void removeArpeggioFMSequenceCommand(int arpNum);
	void setArpeggioFMSequenceCommand(int arpNum, int cnt, int type, int data);
	void setArpeggioFMLoops(int arpNum, std::vector<int> begins, std::vector<int> ends, std::vector<int> times);
	void setArpeggioFMRelease(int arpNum, ReleaseType type, int begin);
	void setInstrumentFMArpeggio(int instNum, int arpNum);
	std::vector<int> getArpeggioFMUsers(int arpNum) const;

	void setPitchFMType(int ptNum, int type);
	void addPitchFMSequenceCommand(int ptNum, int type, int data);
	void removePitchFMSequenceCommand(int ptNum);
	void setPitchFMSequenceCommand(int ptNum, int cnt, int type, int data);
	void setPitchFMLoops(int ptNum, std::vector<int> begins, std::vector<int> ends, std::vector<int> times);
	void setPitchFMRelease(int ptNum, ReleaseType type, int begin);
	void setInstrumentFMPitch(int instNum, int ptNum);
	std::vector<int> getPitchFMUsers(int ptNum) const;

	void setInstrumentFMEnvelopeResetEnabled(int instNum, bool enabled);

	//--- SSG
	void addWaveFormSSGSequenceCommand(int wfNum, int type, int data);
	void removeWaveFormSSGSequenceCommand(int wfNum);
	void setWaveFormSSGSequenceCommand(int wfNum, int cnt, int type, int data);
	void setWaveFormSSGLoops(int wfNum, std::vector<int> begins, std::vector<int> ends, std::vector<int> times);
	void setWaveFormSSGRelease(int wfNum, ReleaseType type, int begin);
	void setInstrumentSSGWaveForm(int instNum, int wfNum);
	std::vector<int> getWaveFormSSGUsers(int wfNum) const;

	void addToneNoiseSSGSequenceCommand(int tnNum, int type, int data);
	void removeToneNoiseSSGSequenceCommand(int tnNum);
	void setToneNoiseSSGSequenceCommand(int tnNum, int cnt, int type, int data);
	void setToneNoiseSSGLoops(int tnNum, std::vector<int> begins, std::vector<int> ends, std::vector<int> times);
	void setToneNoiseSSGRelease(int tnNum, ReleaseType type, int begin);
	void setInstrumentSSGToneNoise(int instNum, int tnNum);
	std::vector<int> getToneNoiseSSGUsers(int tnNum) const;

	void addEnvelopeSSGSequenceCommand(int envNum, int type, int data);
	void removeEnvelopeSSGSequenceCommand(int envNum);
	void setEnvelopeSSGSequenceCommand(int envNum, int cnt, int type, int data);
	void setEnvelopeSSGLoops(int envNum, std::vector<int> begins, std::vector<int> ends, std::vector<int> times);
	void setEnvelopeSSGRelease(int envNum, ReleaseType type, int begin);
	void setInstrumentSSGEnvelope(int instNum, int envNum);
	std::vector<int> getEnvelopeSSGUsers(int envNum) const;

	void setArpeggioSSGType(int arpNum, int type);
	void addArpeggioSSGSequenceCommand(int arpNum, int type, int data);
	void removeArpeggioSSGSequenceCommand(int arpNum);
	void setArpeggioSSGSequenceCommand(int arpNum, int cnt, int type, int data);
	void setArpeggioSSGLoops(int arpNum, std::vector<int> begins, std::vector<int> ends, std::vector<int> times);
	void setArpeggioSSGRelease(int arpNum, ReleaseType type, int begin);
	void setInstrumentSSGArpeggio(int instNum, int arpNum);
	std::vector<int> getArpeggioSSGUsers(int arpNum) const;

	void setPitchSSGType(int ptNum, int type);
	void addPitchSSGSequenceCommand(int ptNum, int type, int data);
	void removePitchSSGSequenceCommand(int ptNum);
	void setPitchSSGSequenceCommand(int ptNum, int cnt, int type, int data);
	void setPitchSSGLoops(int ptNum, std::vector<int> begins, std::vector<int> ends, std::vector<int> times);
	void setPitchSSGRelease(int ptNum, ReleaseType type, int begin);
	void setInstrumentSSGPitch(int instNum, int ptNum);
	std::vector<int> getPitchSSGUsers(int ptNum) const;

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
	void setModuleTickFrequency(unsigned int freq);
	unsigned int getModuleTickFrequency() const;
	/*----- Song -----*/
	void setSongTitle(int songNum, std::string title);
	std::string getSongTitle(int songNum) const;
	void setSongTempo(int songNum, int tempo);
	int getSongtempo(int songNum) const;
	SongStyle getSongStyle(int songNum) const;
	void setSongSpeed(int songNum, int speed);
	size_t getSongSpeed(int songNum) const;
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
	std::string getStepEffectID(int songNum, int trackNum, int orderNum, int stepNum, int n) const;
	void setStepEffectID(int songNum, int trackNum, int orderNum, int stepNum, int n, std::string id);
	int getStepEffectValue(int songNum, int trackNum, int orderNum, int stepNum, int n) const;
	void setStepEffectValue(int songNum, int trackNum, int orderNum, int stepNum, int n, int value);
	void eraseStepEffect(int songNum, int trackNum, int orderNum, int stepNum, int n);
	void eraseStepEffectValue(int songNum, int trackNum, int orderNum, int stepNum, int n);
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
	void setPatternStepHighlightCount(int count);
	int getPatternStepHighlightCount() const;

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

	int ptnHlCnt_;

	// Play song
	bool isFindNextStep_;
	void startPlay();
	bool stepDown();
	void findNextStep();
	void readStep();
	void readTick(int rest);

	void readTickFMForNoteDelay(Step& step, int ch);
	void envelopeResetEffectFM(Step& step, int ch);

	void clearDelayCounts();

	bool readFMStep(Step& step, int ch, bool isSkippedSpecial = false);
	bool readSSGStep(Step& step, int ch, bool isSkippedSpecial = false);
	bool readDrumStep(Step& step, int ch, bool isSkippedSpecial = false);

	bool readFMEffect(int ch, std::string id, int value, bool isSkippedSpecial = false);
	bool readSSGEffect(int ch, std::string id, int value, bool isSkippedSpecial = false);
	bool readDrumEffect(int ch, std::string id, int value, bool isSkippedSpecial = false);
	bool readFMSpecialEffect(int ch, std::string id, int value);
	bool readSSGSpecialEffect(int ch, std::string id, int value);
	bool readDrumSpecialEffect(int ch, std::string id, int value);

	bool effPositionJump(int nextOrder);
	void effTrackEnd();
	bool effPatternBreak(int nextStep);
	void effSpeedChange(int speed);
	void effTempoChange(int tempo);
	std::vector<int> ntDlyCntFM_, ntCutDlyCntFM_, volDlyCntFM_;
	std::vector<int> ntDlyCntSSG_, ntCutDlyCntSSG_, volDlyCntSSG_;
	std::vector<int> ntDlyCntDrum_, ntCutDlyCntDrum_, volDlyCntDrum_;
	std::vector<int> volDlyValueFM_, volDlyValueSSG_, volDlyValueDrum_;
	std::vector<int> tposeDlyCntFM_, tposeDlyCntSSG_;
	std::vector<int> tposeDlyValueFM_, tposeDlyValueSSG_;

	int ctohex(const char c) const;
};
