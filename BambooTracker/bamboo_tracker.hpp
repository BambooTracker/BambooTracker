#pragma once

#include <cstdint>
#include <string>
#include <memory>
#include <vector>
#include <functional>
#include "configuration.hpp"
#include "opna_controller.hpp"
#include "jam_manager.hpp"
#include "command_manager.hpp"
#include "instruments_manager.hpp"
#include "instrument.hpp"
#include "tick_counter.hpp"
#include "module.hpp"
#include "song.hpp"
#include "gd3_tag.hpp"
#include "s98_tag.hpp"
#include "chips/scci/scci.h"
#include "chips/c86ctl/c86ctl_wrapper.hpp"
#include "effect.hpp"
#include "playback.hpp"
#include "binary_container.hpp"
#include "misc.hpp"

class AbstractBank;

class BambooTracker
{
public:
	explicit BambooTracker(std::weak_ptr<Configuration> config);

	// Change confuguration
	void changeConfiguration(std::weak_ptr<Configuration> config);

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
	std::unique_ptr<AbstractInstrument> getInstrument(int num);
	void cloneInstrument(int num, int refNum);
	void deepCloneInstrument(int num, int refNum);
	void loadInstrument(BinaryContainer& container, std::string path, int instNum);
	void saveInstrument(BinaryContainer& container, int instNum);
	void importInstrument(const AbstractBank &bank, size_t index, int instNum);
	void exportInstruments(BinaryContainer& container, std::vector<int> instNums);
	int findFirstFreeInstrumentNumber() const;
	void setInstrumentName(int num, std::string name);
	void clearAllInstrument();
	std::vector<int> getInstrumentIndices() const;
	std::vector<int> getUnusedInstrumentIndices() const;
	void clearUnusedInstrumentProperties();
	std::vector<std::string> getInstrumentNames() const;
	std::vector<std::vector<int>> checkDuplicateInstruments() const;

	//--- FM
	void setEnvelopeFMParameter(int envNum, FMEnvelopeParameter param, int value);
	void setEnvelopeFMOperatorEnable(int envNum, int opNum, bool enable);
	void setInstrumentFMEnvelope(int instNum, int envNum);
	std::vector<int> getEnvelopeFMUsers(int envNum) const;

	void setLFOFMParameter(int lfoNum, FMLFOParameter param, int value);
	void setInstrumentFMLFOEnabled(int instNum, bool enabled);
	void setInstrumentFMLFO(int instNum, int lfoNum);
	std::vector<int> getLFOFMUsers(int lfoNum) const;

	void addOperatorSequenceFMSequenceCommand(FMEnvelopeParameter param, int opSeqNum, int type, int data);
	void removeOperatorSequenceFMSequenceCommand(FMEnvelopeParameter param, int opSeqNum);
	void setOperatorSequenceFMSequenceCommand(FMEnvelopeParameter param, int opSeqNum, int cnt, int type, int data);
	void setOperatorSequenceFMLoops(FMEnvelopeParameter param, int opSeqNum, std::vector<int> begins, std::vector<int> ends, std::vector<int> times);
	void setOperatorSequenceFMRelease(FMEnvelopeParameter param, int opSeqNum, ReleaseType type, int begin);
	void setInstrumentFMOperatorSequence(int instNum, FMEnvelopeParameter param, int opSeqNum);
	void setInstrumentFMOperatorSequenceEnabled(int instNum, FMEnvelopeParameter param, bool enabled);
	std::vector<int> getOperatorSequenceFMUsers(FMEnvelopeParameter param, int opSeqNum) const;

	void setArpeggioFMType(int arpNum, SequenceType type);
	void addArpeggioFMSequenceCommand(int arpNum, int type, int data);
	void removeArpeggioFMSequenceCommand(int arpNum);
	void setArpeggioFMSequenceCommand(int arpNum, int cnt, int type, int data);
	void setArpeggioFMLoops(int arpNum, std::vector<int> begins, std::vector<int> ends, std::vector<int> times);
	void setArpeggioFMRelease(int arpNum, ReleaseType type, int begin);
	void setInstrumentFMArpeggio(int instNum, FMOperatorType op, int arpNum);
	void setInstrumentFMArpeggioEnabled(int instNum, FMOperatorType op, bool enabled);
	std::vector<int> getArpeggioFMUsers(int arpNum) const;

	void setPitchFMType(int ptNum, SequenceType type);
	void addPitchFMSequenceCommand(int ptNum, int type, int data);
	void removePitchFMSequenceCommand(int ptNum);
	void setPitchFMSequenceCommand(int ptNum, int cnt, int type, int data);
	void setPitchFMLoops(int ptNum, std::vector<int> begins, std::vector<int> ends, std::vector<int> times);
	void setPitchFMRelease(int ptNum, ReleaseType type, int begin);
	void setInstrumentFMPitch(int instNum, FMOperatorType op, int ptNum);
	void setInstrumentFMPitchEnabled(int instNum, FMOperatorType op, bool enabled);
	std::vector<int> getPitchFMUsers(int ptNum) const;

	void setInstrumentFMEnvelopeResetEnabled(int instNum, FMOperatorType op, bool enabled);

	//--- SSG
	void addWaveFormSSGSequenceCommand(int wfNum, int type, int data);
	void removeWaveFormSSGSequenceCommand(int wfNum);
	void setWaveFormSSGSequenceCommand(int wfNum, int cnt, int type, int data);
	void setWaveFormSSGLoops(int wfNum, std::vector<int> begins, std::vector<int> ends, std::vector<int> times);
	void setWaveFormSSGRelease(int wfNum, ReleaseType type, int begin);
	void setInstrumentSSGWaveForm(int instNum, int wfNum);
	void setInstrumentSSGWaveFormEnabled(int instNum, bool enabled);
	std::vector<int> getWaveFormSSGUsers(int wfNum) const;

	void addToneNoiseSSGSequenceCommand(int tnNum, int type, int data);
	void removeToneNoiseSSGSequenceCommand(int tnNum);
	void setToneNoiseSSGSequenceCommand(int tnNum, int cnt, int type, int data);
	void setToneNoiseSSGLoops(int tnNum, std::vector<int> begins, std::vector<int> ends, std::vector<int> times);
	void setToneNoiseSSGRelease(int tnNum, ReleaseType type, int begin);
	void setInstrumentSSGToneNoise(int instNum, int tnNum);
	void setInstrumentSSGToneNoiseEnabled(int instNum, bool enabled);
	std::vector<int> getToneNoiseSSGUsers(int tnNum) const;

	void addEnvelopeSSGSequenceCommand(int envNum, int type, int data);
	void removeEnvelopeSSGSequenceCommand(int envNum);
	void setEnvelopeSSGSequenceCommand(int envNum, int cnt, int type, int data);
	void setEnvelopeSSGLoops(int envNum, std::vector<int> begins, std::vector<int> ends, std::vector<int> times);
	void setEnvelopeSSGRelease(int envNum, ReleaseType type, int begin);
	void setInstrumentSSGEnvelope(int instNum, int envNum);
	void setInstrumentSSGEnvelopeEnabled(int instNum, bool enabled);
	std::vector<int> getEnvelopeSSGUsers(int envNum) const;

	void setArpeggioSSGType(int arpNum, SequenceType type);
	void addArpeggioSSGSequenceCommand(int arpNum, int type, int data);
	void removeArpeggioSSGSequenceCommand(int arpNum);
	void setArpeggioSSGSequenceCommand(int arpNum, int cnt, int type, int data);
	void setArpeggioSSGLoops(int arpNum, std::vector<int> begins, std::vector<int> ends, std::vector<int> times);
	void setArpeggioSSGRelease(int arpNum, ReleaseType type, int begin);
	void setInstrumentSSGArpeggio(int instNum, int arpNum);
	void setInstrumentSSGArpeggioEnabled(int instNum, bool enabled);
	std::vector<int> getArpeggioSSGUsers(int arpNum) const;

	void setPitchSSGType(int ptNum, SequenceType type);
	void addPitchSSGSequenceCommand(int ptNum, int type, int data);
	void removePitchSSGSequenceCommand(int ptNum);
	void setPitchSSGSequenceCommand(int ptNum, int cnt, int type, int data);
	void setPitchSSGLoops(int ptNum, std::vector<int> begins, std::vector<int> ends, std::vector<int> times);
	void setPitchSSGRelease(int ptNum, ReleaseType type, int begin);
	void setInstrumentSSGPitch(int instNum, int ptNum);
	void setInstrumentSSGPitchEnabled(int instNum, bool enabled);
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
	void jamKeyOn(int keyNum);
	void jamKeyOff(JamKey key);
	void jamKeyOff(int keyNum);
	void jamKeyOnForced(JamKey key, SoundSource src);
	void jamKeyOnForced(int keyNum, SoundSource src);
	void jamKeyOffForced(JamKey key, SoundSource src);
	void jamKeyOffForced(int keyNum, SoundSource src);

	// Play song
	void startPlaySong();
	void startPlayFromStart();
	void startPlayPattern();
	void startPlayFromCurrentStep();
	void stopPlaySong();
	bool isPlaySong() const;
	PlaybackState getPlaybackState() const;
	void setTrackMuteState(int trackNum, bool isMute);
	bool isMute(int trackNum);
	void setFollowPlay(bool isFollowed);
	bool isFollowPlay() const;
	int getPlayingOrderNumber() const;
	int getPlayingStepNumber() const;

	// Export
	bool exportToWav(BinaryContainer& container, int rate, int loopCnt, std::function<bool()> bar);
	bool exportToVgm(BinaryContainer& container, int target, bool gd3TagEnabled,
					 GD3Tag tag, std::function<bool()> bar);
	bool exportToS98(BinaryContainer& container, int target, bool tagEnabled, S98Tag tag,
					 int rate, std::function<bool()> bar);

	// Real chip interface
	void useSCCI(scci::SoundInterfaceManager* manager);
	void useC86CTL(C86ctlBase* base);
	RealChipInterface getRealChipinterface() const;

	// Stream events
	/// 0<: Tick
	///  0: Step
	/// -1: Stop
	int streamCountUp();
	void getStreamSamples(int16_t *container, size_t nSamples);
	void killSound();

	// Stream details
	int getStreamRate() const;
	void setStreamRate(int rate);
	int getStreamDuration() const;
	void setStreamDuration(int duration);
	int getStreamTempo() const;
	int getStreamSpeed() const;
	bool getStreamGrooveEnabled() const;
	void setMasterVolume(int percentage);
	void setMasterVolumeFM(double dB);
	void setMasterVolumeSSG(double dB);

	// Module details
	/*----- Module -----*/
	void makeNewModule();
	void loadModule(BinaryContainer& container);
	void saveModule(BinaryContainer& container);
	void setModulePath(std::string path);
	std::string getModulePath() const;
	void setModuleTitle(std::string title);
	std::string getModuleTitle() const;
	void setModuleAuthor(std::string author);
	std::string getModuleAuthor() const;
	void setModuleCopyright(std::string copyright);
	std::string getModuleCopyright() const;
	void setModuleComment(std::string comment);
	std::string getModuleComment() const;
	void setModuleTickFrequency(unsigned int freq);
	unsigned int getModuleTickFrequency() const;
	void setModuleStepHighlight1Distance(size_t dist);
	size_t getModuleStepHighlight1Distance() const;
	void setModuleStepHighlight2Distance(size_t dist);
	size_t getModuleStepHighlight2Distance() const;
	void setModuleMixerType(MixerType type);
	MixerType getModuleMixerType() const;
	void setModuleCustomMixerFMLevel(double level);
	double getModuleCustomMixerFMLevel() const;
	void setModuleCustomMixerSSGLevel(double level);
	double getModuleCustomMixerSSGLevel() const;
	size_t getGrooveCount() const;
	void setGroove(int num, std::vector<int> seq);
	void setGrooves(std::vector<std::vector<int>> seqs);
	std::vector<int> getGroove(int num) const;
	void clearUnusedPatterns();
	void replaceDuplicateInstrumentsInPatterns(std::vector<std::vector<int>> list);
	/*----- Song -----*/
	void setSongTitle(int songNum, std::string title);
	std::string getSongTitle(int songNum) const;
	void setSongTempo(int songNum, int tempo);
	int getSongTempo(int songNum) const;
	void setSongGroove(int songNum, int groove);
	int getSongGroove(int songNum) const;
	void toggleTempoOrGrooveInSong(int songNum, bool isTempo);
	bool isUsedTempoInSong(int songNum) const;
	SongStyle getSongStyle(int songNum) const;
	void setSongSpeed(int songNum, int speed);
	int getSongSpeed(int songNum) const;
	size_t getSongCount() const;
	void addSong(SongType songType, std::string title);
	void sortSongs(std::vector<int> numbers);
	size_t getAllStepCount(int songNum, int loopCnt) const;
	/*----- Track -----*/
	void setEffectDisplayWidth(int songNum, int trackNum, size_t w);
	size_t getEffectDisplayWidth(int songNum, int trackNum) const;
	/*----- Order -----*/
	std::vector<OrderData> getOrderData(int songNum, int orderNum) const;
	void setOrderPatternDigit(int songNum, int trackNum, int orderNum, int patternNum, bool secondEntry);
	void insertOrderBelow(int songNum, int orderNum);
	void deleteOrder(int songNum, int orderNum);
	void pasteOrderCells(int songNum, int beginTrack, int beginOrder,
						   std::vector<std::vector<std::string>> cells);
	void duplicateOrder(int songNum, int orderNum);
	void MoveOrder(int songNum, int orderNum, bool isUp);
	void clonePatterns(int songNum, int beginOrder, int beginTrack, int endOrder, int endTrack);
	void cloneOrder(int songNum, int orderNum);
	size_t getOrderSize(int songNum) const;
	bool canAddNewOrder(int songNum) const;
	/*----- Pattern -----*/
	int getStepNoteNumber(int songNum, int trackNum, int orderNum, int stepNum) const;
	void setStepNote(int songNum, int trackNum, int orderNum, int stepNum, int octave, Note note, bool autosetInst);
	void setStepKeyOff(int songNum, int trackNum, int orderNum, int stepNum);
	void setEchoBufferAccess(int songNum, int trackNum, int orderNum, int stepNum, int bufNum);
	void eraseStepNote(int songNum, int trackNum, int orderNum, int stepNum);
	int getStepInstrument(int songNum, int trackNum, int orderNum, int stepNum) const;
	void setStepInstrumentDigit(int songNum, int trackNum, int orderNum, int stepNum, int instNum, bool secondEntry);
	void eraseStepInstrument(int songNum, int trackNum, int orderNum, int stepNum);
	int getStepVolume(int songNum, int trackNum, int orderNum, int stepNum) const;
	void setStepVolumeDigit(int songNum, int trackNum, int orderNum, int stepNum, int volume, bool isFMReversed, bool secondEntry);
	void eraseStepVolume(int songNum, int trackNum, int orderNum, int stepNum);
	std::string getStepEffectID(int songNum, int trackNum, int orderNum, int stepNum, int n) const;
	void setStepEffectIDCharacter(int songNum, int trackNum, int orderNum, int stepNum, int n, std::string id, bool fillValue00, bool secondEntry);
	int getStepEffectValue(int songNum, int trackNum, int orderNum, int stepNum, int n) const;
	void setStepEffectValueDigit(int songNum, int trackNum, int orderNum, int stepNum, int n, int value, EffectDisplayControl ctrl, bool secondEntry);
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
	void pasteMixPatternCells(int songNum, int beginTrack, int beginColmn, int beginOrder, int beginStep,
							  std::vector<std::vector<std::string>> cells);
	void pasteOverwritePatternCells(int songNum, int beginTrack, int beginColmn, int beginOrder,
									int beginStep, std::vector<std::vector<std::string>> cells);
	std::vector<std::vector<std::string>> arrangePatternDataCells(int songNum, int beginTrack, int beginColmn, int beginOrder, int beginStep,
																  std::vector<std::vector<std::string>> cells);
	void erasePatternCells(int songNum, int beginTrack, int beginColmn, int beginOrder, int beginStep,
						   int endTrack, int endColmn, int endStep);
	void increaseNoteKeyInPattern(int songNum, int beginTrack, int beginOrder, int beginStep,
								  int endTrack, int endStep);
	void decreaseNoteKeyInPattern(int songNum, int beginTrack, int beginOrder, int beginStep,
								  int endTrack, int endStep);
	void increaseNoteOctaveInPattern(int songNum, int beginTrack, int beginOrder, int beginStep,
									 int endTrack, int endStep);
	void decreaseNoteOctaveInPattern(int songNum, int beginTrack, int beginOrder, int beginStep,
									 int endTrack, int endStep);
	void expandPattern(int songNum, int beginTrack, int beginColmn, int beginOrder, int beginStep,
					   int endTrack, int endColmn, int endStep);
	void shrinkPattern(int songNum, int beginTrack, int beginColmn, int beginOrder, int beginStep,
					   int endTrack, int endColmn, int endStep);
	void interpolatePattern(int songNum, int beginTrack, int beginColmn, int beginOrder, int beginStep,
							int endTrack, int endColmn, int endStep);
	void reversePattern(int songNum, int beginTrack, int beginColmn, int beginOrder, int beginStep,
						int endTrack, int endColmn, int endStep);
	void replaceInstrumentInPattern(int songNum, int beginTrack, int beginOrder, int beginStep,
									int endTrack, int endStep, int newInstNum);
	size_t getPatternSizeFromOrderNumber(int songNum, int orderNum) const;
	void setDefaultPatternSize(int songNum, size_t size);
	size_t getDefaultPatternSize(int songNum) const;
	/*----- Visual -----*/
	void getOutputHistory(int16_t* container);

private:
	CommandManager comMan_;
	std::shared_ptr<InstrumentsManager> instMan_;
	std::unique_ptr<JamManager> jamMan_;
	std::shared_ptr<OPNAController> opnaCtrl_;
	std::shared_ptr<TickCounter> tickCounter_;
	std::unique_ptr<PlaybackManager> playback_;
	std::shared_ptr<Module> mod_;

	// Current status
	int octave_;	// 0-7
	int curSongNum_;
	SongStyle songStyle_;
	int curTrackNum_;
	int curOrderNum_, curStepNum_;
	///	-1: not set
	int curInstNum_;
	std::vector<bool> muteStateFM_, muteStateSSG_, muteStateDrum_;

	bool isFollowPlay_;

	static const uint32_t CHIP_CLOCK;

	// Jam mode
	void funcJamKeyOn(JamKey key, int keyNum, const TrackAttribute& attrib);
	void funcJamKeyOff(JamKey key, int keyNum, const TrackAttribute& attrib);

	// Play song
	void startPlay();

	void checkNextPositionOfLastStep(int& endOrder, int& endStep) const;
};
