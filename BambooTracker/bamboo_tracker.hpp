/*
 * Copyright (C) 2018-2020 Rerrah
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#pragma once

#include <cstdint>
#include <string>
#include <memory>
#include <vector>
#include <functional>
#include <unordered_map>
#include "configuration.hpp"
#include "opna_controller.hpp"
#include "jam_manager.hpp"
#include "instruments_manager.hpp"
#include "instrument.hpp"
#include "tick_counter.hpp"
#include "module.hpp"
#include "song.hpp"
#include "command/command_manager.hpp"
#include "chips/scci/scci.hpp"
#include "chips/c86ctl/c86ctl_wrapper.hpp"
#include "effect.hpp"
#include "playback.hpp"
#include "io/binary_container.hpp"
#include "io/wav_container.hpp"
#include "io/export_io.hpp"
#include "enum_hash.hpp"
#include "misc.hpp"

class AbstractBank;

class BambooTracker
{
public:
	explicit BambooTracker(std::weak_ptr<Configuration> config);

	// Change confuguration
	void changeConfiguration(std::weak_ptr<Configuration> config);

	// Current octave
	void setCurrentOctave(int octave);
	int getCurrentOctave() const;

	// Current volume
	void setCurrentVolume(int volume);
	int getCurrentVolume() const;

	// Current track
	void setCurrentTrack(int num);
	TrackAttribute getCurrentTrackAttribute() const;

	// Current instrument
	void setCurrentInstrument(int n);
	int getCurrentInstrumentNumber() const;

	// Instrument edit
	void addInstrument(int num, InstrumentType type, std::string name);
	void removeInstrument(int num);
	std::unique_ptr<AbstractInstrument> getInstrument(int num);
	void cloneInstrument(int num, int refNum);
	void deepCloneInstrument(int num, int refNum);
	void swapInstruments(int a, int b, bool patternChange);
	void loadInstrument(io::BinaryContainer& container, std::string path, int instNum);
	void saveInstrument(io::BinaryContainer& container, int instNum);
	void importInstrument(const AbstractBank &bank, size_t index, int instNum);
	void exportInstruments(io::BinaryContainer& container, std::vector<int> instNums);
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
	void addWaveformSSGSequenceCommand(int wfNum, int type, int data);
	void removeWaveformSSGSequenceCommand(int wfNum);
	void setWaveformSSGSequenceCommand(int wfNum, int cnt, int type, int data);
	void setWaveformSSGLoops(int wfNum, std::vector<int> begins, std::vector<int> ends, std::vector<int> times);
	void setWaveformSSGRelease(int wfNum, ReleaseType type, int begin);
	void setInstrumentSSGWaveform(int instNum, int wfNum);
	void setInstrumentSSGWaveformEnabled(int instNum, bool enabled);
	std::vector<int> getWaveformSSGUsers(int wfNum) const;

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

	//--- ADPCM
	size_t getADPCMLimit() const;
	size_t getADPCMStoredSize() const;

	void setSampleADPCMRootKeyNumber(int sampNum, int n);
	int getSampleADPCMRootKeyNumber(int sampNum) const;
	void setSampleADPCMRootDeltaN(int sampNum, int dn);
	int getSampleADPCMRootDeltaN(int sampNum) const;
	void setSampleADPCMRepeatEnabled(int sampNum, bool enabled);
	bool getSampleADPCMRepeatEnabled(int sampNum) const;
	void storeSampleADPCMRawSample(int sampNum, std::vector<uint8_t> sample);
	std::vector<uint8_t> getSampleADPCMRawSample(int sampNum) const;
	void clearSampleADPCMRawSample(int sampNum);
	void assignSampleADPCMRawSamples();
	size_t getSampleADPCMStartAddress(int sampNum) const;
	size_t getSampleADPCMStopAddress(int sampNum) const;
	void setInstrumentADPCMSample(int instNum, int sampNum);
	std::vector<int> getSampleADPCMUsers(int sampNum) const;

	void addEnvelopeADPCMSequenceCommand(int envNum, int type, int data);
	void removeEnvelopeADPCMSequenceCommand(int envNum);
	void setEnvelopeADPCMSequenceCommand(int envNum, int cnt, int type, int data);
	void setEnvelopeADPCMLoops(int envNum, std::vector<int> begins, std::vector<int> ends, std::vector<int> times);
	void setEnvelopeADPCMRelease(int envNum, ReleaseType type, int begin);
	void setInstrumentADPCMEnvelope(int instNum, int envNum);
	void setInstrumentADPCMEnvelopeEnabled(int instNum, bool enabled);
	std::vector<int> getEnvelopeADPCMUsers(int envNum) const;

	void setArpeggioADPCMType(int arpNum, SequenceType type);
	void addArpeggioADPCMSequenceCommand(int arpNum, int type, int data);
	void removeArpeggioADPCMSequenceCommand(int arpNum);
	void setArpeggioADPCMSequenceCommand(int arpNum, int cnt, int type, int data);
	void setArpeggioADPCMLoops(int arpNum, std::vector<int> begins, std::vector<int> ends, std::vector<int> times);
	void setArpeggioADPCMRelease(int arpNum, ReleaseType type, int begin);
	void setInstrumentADPCMArpeggio(int instNum, int arpNum);
	void setInstrumentADPCMArpeggioEnabled(int instNum, bool enabled);
	std::vector<int> getArpeggioADPCMUsers(int arpNum) const;

	void setPitchADPCMType(int ptNum, SequenceType type);
	void addPitchADPCMSequenceCommand(int ptNum, int type, int data);
	void removePitchADPCMSequenceCommand(int ptNum);
	void setPitchADPCMSequenceCommand(int ptNum, int cnt, int type, int data);
	void setPitchADPCMLoops(int ptNum, std::vector<int> begins, std::vector<int> ends, std::vector<int> times);
	void setPitchADPCMRelease(int ptNum, ReleaseType type, int begin);
	void setInstrumentADPCMPitch(int instNum, int ptNum);
	void setInstrumentADPCMPitchEnabled(int instNum, bool enabled);
	std::vector<int> getPitchADPCMUsers(int ptNum) const;

	//--- Drumkit
	void setInstrumentDrumkitSample(int instNum, int key, int sampNum);
	void setInstrumentDrumkitSampleEnabled(int instNum, int key, bool enabled);
	void setInstrumentDrumkitPitch(int instNum, int key, int pitch);

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
	void jamKeyOn(JamKey key, bool volumeSet);
	void jamKeyOn(int keyNum, bool volumeSet);
	void jamKeyOff(JamKey key);
	void jamKeyOff(int keyNum);
	void jamKeyOnForced(JamKey key, SoundSource src, bool volumeSet,  std::shared_ptr<AbstractInstrument> inst = nullptr);
	void jamKeyOnForced(int keyNum, SoundSource src, bool volumeSet, std::shared_ptr<AbstractInstrument> inst = nullptr);
	void jamKeyOffForced(JamKey key, SoundSource src);
	void jamKeyOffForced(int keyNum, SoundSource src);
	std::vector<std::vector<size_t>> assignADPCMBeforeForcedJamKeyOn(std::shared_ptr<AbstractInstrument> inst);

	// Play song
	void startPlaySong();
	void startPlayFromStart();
	void startPlayPattern();
	void startPlayFromCurrentStep();
	bool startPlayFromMarker();
	void playStep();
	void stopPlaySong();
	bool isPlaySong() const;
	void setTrackMuteState(int trackNum, bool isMute);
	bool isMute(int trackNum);
	void setFollowPlay(bool isFollowed);
	bool isFollowPlay() const;
	int getPlayingOrderNumber() const;
	int getPlayingStepNumber() const;
	void setMarker(int order, int step);
	int getMarkerOrder() const;
	int getMarkerStep() const;

	// Export
	bool exportToWav(io::WavContainer& container, int loopCnt, std::function<bool()> bar);
	bool exportToVgm(io::BinaryContainer& container, int target, bool gd3TagEnabled,
					 io::GD3Tag tag, std::function<bool()> bar);
	bool exportToS98(io::BinaryContainer& container, int target, bool tagEnabled, io::S98Tag tag,
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
	void loadModule(io::BinaryContainer& container);
	void saveModule(io::BinaryContainer& container);
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
	void clearUnusedADPCMSamples();
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
	void changeSongType(int songNum, SongType type);
	void setSongSpeed(int songNum, int speed);
	int getSongSpeed(int songNum) const;
	size_t getSongCount() const;
	void addSong(SongType songType, std::string title);
	void sortSongs(std::vector<int> numbers);
	size_t getAllStepCount(int songNum, size_t loopCnt) const;
	void transposeSong(int songNum, int seminotes, std::vector<int> excludeInsts);
	void swapTracks(int songNum, int track1, int track2);
	double calculateSongLength(int songNum) const;
	/*----- Bookmark -----*/
	void addBookmark(int songNum, std::string name, int order, int step);
	void changeBookmark(int songNum, int i, std::string name, int order, int step);
	void removeBookmark(int songNum, int i);
	void clearBookmark(int songNum);
	void swapBookmarks(int songNum, int a, int b);
	void sortBookmarkByPosition(int songNum);
	void sortBookmarkByName(int songNum);
	Bookmark getBookmark(int songNum, int i) const;
	std::vector<int> findBookmarks(int songNum, int order, int step);
	Bookmark getPreviousBookmark(int songNum, int order, int step);
	Bookmark getNextBookmark(int songNum, int order, int step);
	size_t getBookmarkSize(int songNum) const;
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
	void setStepNote(int songNum, int trackNum, int orderNum, int stepNum, int octave, Note note, bool instMask, bool volMask);
	void setStepKeyOff(int songNum, int trackNum, int orderNum, int stepNum);
	void setEchoBufferAccess(int songNum, int trackNum, int orderNum, int stepNum, int bufNum);
	void eraseStepNote(int songNum, int trackNum, int orderNum, int stepNum);
	int getStepInstrument(int songNum, int trackNum, int orderNum, int stepNum) const;
	void setStepInstrumentDigit(int songNum, int trackNum, int orderNum, int stepNum, int instNum, bool secondEntry);
	void eraseStepInstrument(int songNum, int trackNum, int orderNum, int stepNum);
	int getStepVolume(int songNum, int trackNum, int orderNum, int stepNum) const;
	int setStepVolumeDigit(int songNum, int trackNum, int orderNum, int stepNum, int volume, bool secondEntry);
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
	void pasteInsertPatternCells(int songNum, int beginTrack, int beginColmn, int beginOrder,
								 int beginStep, std::vector<std::vector<std::string>> cells);
	std::vector<std::vector<std::string>> arrangePatternDataCells(int songNum, int beginTrack, int beginColmn, int beginOrder, int beginStep,
																  std::vector<std::vector<std::string>> cells);
	void erasePatternCells(int songNum, int beginTrack, int beginColmn, int beginOrder, int beginStep,
						   int endTrack, int endColmn, int endStep);
	void transposeNoteInPattern(int songNum, int beginTrack, int beginOrder, int beginStep,
								int endTrack, int endStep, int seminote);
	void changeValuesInPattern(int songNum, int beginTrack, int beginColumn, int beginOrder,
							   int beginStep, int endTrack, int endColumn, int endStep, int value);
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
	int curOctave_;	// 0-7
	int curSongNum_;
	SongStyle songStyle_;
	int curTrackNum_;
	int curOrderNum_, curStepNum_;
	///	-1: not set
	int curInstNum_;
	int curVolume_;
	bool volFMReversed_;
	std::unordered_map<SoundSource, std::vector<bool>> muteState_;
	int mkOrder_, mkStep_;

	bool isFollowPlay_;
	bool storeOnlyUsedSamples_;

	static const uint32_t CHIP_CLOCK;

	// Jam mode
	void funcJamKeyOn(JamKey key, int keyNum, const TrackAttribute& attrib, bool volumeSet,
					  std::shared_ptr<AbstractInstrument> inst = nullptr);
	void funcJamKeyOff(JamKey key, int keyNum, const TrackAttribute& attrib);

	// Play song
	void startPlay();

	void checkNextPositionOfLastStepAndStepSize(
			int songNum, int& endOrder, int& endStep, size_t& nIntroStep, size_t& nLoopStep) const;
};
