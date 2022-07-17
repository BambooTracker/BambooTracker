/*
 * Copyright (C) 2018-2022 Rerrah
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
#include <set>
#include <array>
#include "jamming.hpp"
#include "instrument.hpp"
#include "module.hpp"
#include "command/command_manager.hpp"
#include "chip/real_chip_interface.hpp"
#include "io/binary_container.hpp"
#include "io/export_io.hpp"
#include "io/wav_container.hpp"
#include "bamboo_tracker_defs.hpp"
#include "enum_hash.hpp"

class Configuration;
enum class EffectDisplayControl;
class AbstractBank;
class OPNAController;
class PlaybackManager;
class TickCounter;

class BambooTracker
{
public:
	explicit BambooTracker(std::weak_ptr<Configuration> config);
	~BambooTracker();

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
	void addInstrument(int num, InstrumentType type, const std::string& name);
	void removeInstrument(int num);
	std::unique_ptr<AbstractInstrument> getInstrument(int num);
	void cloneInstrument(int num, int refNum);
	void deepCloneInstrument(int num, int refNum);
	void swapInstruments(int a, int b, bool patternChange);
	void loadInstrument(io::BinaryContainer& container, const std::string& path, int instNum);
	void saveInstrument(io::BinaryContainer& container, int instNum);
	void importInstrument(const AbstractBank &bank, size_t index, int instNum);
	void exportInstruments(io::BinaryContainer& container, const std::vector<int>& instNums);
	int findFirstFreeInstrumentNumber() const;
	void setInstrumentName(int num, const std::string& name);
	void clearAllInstrument();
	std::vector<int> getInstrumentIndices() const;
	std::vector<int> getUnusedInstrumentIndices() const;
	void clearUnusedInstrumentProperties();
	std::vector<std::string> getInstrumentNames() const;

	//--- FM
	void setEnvelopeFMParameter(int envNum, FMEnvelopeParameter param, int value);
	void setEnvelopeFMOperatorEnable(int envNum, int opNum, bool enable);
	void setInstrumentFMEnvelope(int instNum, int envNum);
	std::multiset<int> getEnvelopeFMUsers(int envNum) const;

	void setLFOFMParameter(int lfoNum, FMLFOParameter param, int value);
	void setInstrumentFMLFOEnabled(int instNum, bool enabled);
	void setInstrumentFMLFO(int instNum, int lfoNum);
	std::multiset<int> getLFOFMUsers(int lfoNum) const;

	void addOperatorSequenceFMSequenceData(FMEnvelopeParameter param, int opSeqNum, int data);
	void removeOperatorSequenceFMSequenceData(FMEnvelopeParameter param, int opSeqNum);
	void setOperatorSequenceFMSequenceData(FMEnvelopeParameter param, int opSeqNum, int cnt, int data);
	void addOperatorSequenceFMLoop(FMEnvelopeParameter param, int opSeqNum, const InstrumentSequenceLoop& loop);
	void removeOperatorSequenceFMLoop(FMEnvelopeParameter param, int opSeqNum, int begin, int end);
	void changeOperatorSequenceFMLoop(FMEnvelopeParameter param, int opSeqNum, int prevBegin, int prevEnd, const InstrumentSequenceLoop& loop);
	void clearOperatorSequenceFMLoops(FMEnvelopeParameter param, int opSeqNum);
	void setOperatorSequenceFMRelease(FMEnvelopeParameter param, int opSeqNum, const InstrumentSequenceRelease& release);
	void setInstrumentFMOperatorSequence(int instNum, FMEnvelopeParameter param, int opSeqNum);
	void setInstrumentFMOperatorSequenceEnabled(int instNum, FMEnvelopeParameter param, bool enabled);
	std::multiset<int> getOperatorSequenceFMUsers(FMEnvelopeParameter param, int opSeqNum) const;

	void setArpeggioFMType(int arpNum, SequenceType type);
	void addArpeggioFMSequenceData(int arpNum, int data);
	void removeArpeggioFMSequenceData(int arpNum);
	void setArpeggioFMSequenceData(int arpNum, int cnt, int data);
	void addArpeggioFMLoop(int arpNum, const InstrumentSequenceLoop& loop);
	void removeArpeggioFMLoop(int arpNum, int begin, int end);
	void changeArpeggioFMLoop(int arpNum, int prevBegin, int prevEnd, const InstrumentSequenceLoop& loop);
	void clearArpeggioFMLoops(int arpNum);
	void setArpeggioFMRelease(int arpNum, const InstrumentSequenceRelease& release);
	void setInstrumentFMArpeggio(int instNum, FMOperatorType op, int arpNum);
	void setInstrumentFMArpeggioEnabled(int instNum, FMOperatorType op, bool enabled);
	std::multiset<int> getArpeggioFMUsers(int arpNum) const;

	void setPitchFMType(int ptNum, SequenceType type);
	void addPitchFMSequenceData(int ptNum, int data);
	void removePitchFMSequenceData(int ptNum);
	void setPitchFMSequenceData(int ptNum, int cnt, int data);
	void addPitchFMLoop(int ptNum, const InstrumentSequenceLoop& loop);
	void removePitchFMLoop(int ptNum, int begin, int end);
	void changePitchFMLoop(int ptNum, int prevBegin, int prevEnd, const InstrumentSequenceLoop& loop);
	void clearPitchFMLoops(int ptNum);
	void setPitchFMRelease(int ptNum, const InstrumentSequenceRelease& release);
	void setInstrumentFMPitch(int instNum, FMOperatorType op, int ptNum);
	void setInstrumentFMPitchEnabled(int instNum, FMOperatorType op, bool enabled);
	std::multiset<int> getPitchFMUsers(int ptNum) const;

	void addPanFMSequenceData(int panNum, int data);
	void removePanFMSequenceData(int panNum);
	void setPanFMSequenceData(int panNum, int cnt, int data);
	void addPanFMLoop(int panNum, const InstrumentSequenceLoop& loop);
	void removePanFMLoop(int panNum, int begin, int end);
	void changePanFMLoop(int panNum, int prevBegin, int prevEnd, const InstrumentSequenceLoop& loop);
	void clearPanFMLoops(int panNum);
	void setPanFMRelease(int panNum, const InstrumentSequenceRelease& release);
	void setInstrumentFMPan(int instNum, int panNum);
	void setInstrumentFMPanEnabled(int instNum, bool enabled);
	std::multiset<int> getPanFMUsers(int panNum) const;

	void setInstrumentFMEnvelopeResetEnabled(int instNum, FMOperatorType op, bool enabled);

	//--- SSG
	void addWaveformSSGSequenceData(int wfNum, const SSGWaveformUnit& data);
	void removeWaveformSSGSequenceData(int wfNum);
	void setWaveformSSGSequenceData(int wfNum, int cnt, const SSGWaveformUnit& data);
	void addWaveformSSGLoop(int wfNum, const InstrumentSequenceLoop& loop);
	void removeWaveformSSGLoop(int wfNum, int begin, int end);
	void changeWaveformSSGLoop(int wfNum, int prevBegin, int prevEnd, const InstrumentSequenceLoop& loop);
	void clearWaveformSSGLoops(int wfNum);
	void setWaveformSSGRelease(int wfNum, const InstrumentSequenceRelease& release);
	void setInstrumentSSGWaveform(int instNum, int wfNum);
	void setInstrumentSSGWaveformEnabled(int instNum, bool enabled);
	std::multiset<int> getWaveformSSGUsers(int wfNum) const;

	void addToneNoiseSSGSequenceData(int tnNum, int data);
	void removeToneNoiseSSGSequenceData(int tnNum);
	void setToneNoiseSSGSequenceData(int tnNum, int cnt, int data);
	void addToneNoiseSSGLoop(int tnNum, const InstrumentSequenceLoop& loop);
	void removeToneNoiseSSGLoop(int tnNum, int begin, int end);
	void changeToneNoiseSSGLoop(int tnNum, int prevBegin, int prevEnd, const InstrumentSequenceLoop& loop);
	void clearToneNoiseSSGLoops(int tnNum);
	void setToneNoiseSSGRelease(int tnNum, const InstrumentSequenceRelease& release);
	void setInstrumentSSGToneNoise(int instNum, int tnNum);
	void setInstrumentSSGToneNoiseEnabled(int instNum, bool enabled);
	std::multiset<int> getToneNoiseSSGUsers(int tnNum) const;

	void addEnvelopeSSGSequenceData(int envNum, const SSGEnvelopeUnit& data);
	void removeEnvelopeSSGSequenceData(int envNum);
	void setEnvelopeSSGSequenceData(int envNum, int cnt, const SSGEnvelopeUnit& data);
	void addEnvelopeSSGLoop(int envNum, const InstrumentSequenceLoop& loop);
	void removeEnvelopeSSGLoop(int envNum, int begin, int end);
	void changeEnvelopeSSGLoop(int envNum, int prevBegin, int prevEnd, const InstrumentSequenceLoop& loop);
	void clearEnvelopeSSGLoops(int envNum);
	void setEnvelopeSSGRelease(int envNum, const InstrumentSequenceRelease& release);
	void setInstrumentSSGEnvelope(int instNum, int envNum);
	void setInstrumentSSGEnvelopeEnabled(int instNum, bool enabled);
	std::multiset<int> getEnvelopeSSGUsers(int envNum) const;

	void setArpeggioSSGType(int arpNum, SequenceType type);
	void addArpeggioSSGSequenceData(int arpNum, int data);
	void removeArpeggioSSGSequenceData(int arpNum);
	void setArpeggioSSGSequenceData(int arpNum, int cnt, int data);
	void addArpeggioSSGLoop(int arpNum, const InstrumentSequenceLoop& loop);
	void removeArpeggioSSGLoop(int arpNum, int begin, int end);
	void changeArpeggioSSGLoop(int arpNum, int prevBegin, int prevEnd, const InstrumentSequenceLoop& loop);
	void clearArpeggioSSGLoops(int arpNum);
	void setArpeggioSSGRelease(int arpNum, const InstrumentSequenceRelease& release);
	void setInstrumentSSGArpeggio(int instNum, int arpNum);
	void setInstrumentSSGArpeggioEnabled(int instNum, bool enabled);
	std::multiset<int> getArpeggioSSGUsers(int arpNum) const;

	void setPitchSSGType(int ptNum, SequenceType type);
	void addPitchSSGSequenceData(int ptNum, int data);
	void removePitchSSGSequenceData(int ptNum);
	void setPitchSSGSequenceData(int ptNum, int cnt, int data);
	void addPitchSSGLoop(int ptNum, const InstrumentSequenceLoop& loop);
	void removePitchSSGLoop(int ptNum, int begin, int end);
	void changePitchSSGLoop(int ptNum, int prevBegin, int prevEnd, const InstrumentSequenceLoop& loop);
	void clearPitchSSGLoops(int ptNum);
	void setPitchSSGRelease(int ptNum, const InstrumentSequenceRelease& release);
	void setInstrumentSSGPitch(int instNum, int ptNum);
	void setInstrumentSSGPitchEnabled(int instNum, bool enabled);
	std::multiset<int> getPitchSSGUsers(int ptNum) const;

	//--- ADPCM
	size_t getADPCMLimit() const;
	size_t getADPCMStoredSize() const;

	void setSampleADPCMRootKeyNumber(int sampNum, int n);
	int getSampleADPCMRootKeyNumber(int sampNum) const;
	void setSampleADPCMRootDeltaN(int sampNum, int dn);
	int getSampleADPCMRootDeltaN(int sampNum) const;
	void setSampleADPCMRepeatEnabled(int sampNum, bool enabled);
	bool getSampleADPCMRepeatEnabled(int sampNum) const;
	void storeSampleADPCMRawSample(int sampNum, const std::vector<uint8_t>& sample);
	void storeSampleADPCMRawSample(int sampNum, std::vector<uint8_t>&& sample);
	std::vector<uint8_t> getSampleADPCMRawSample(int sampNum) const;
	void clearSampleADPCMRawSample(int sampNum);
	bool assignSampleADPCMRawSamples();
	size_t getSampleADPCMStartAddress(int sampNum) const;
	size_t getSampleADPCMStopAddress(int sampNum) const;
	void setInstrumentADPCMSample(int instNum, int sampNum);
	std::multiset<int> getSampleADPCMUsers(int sampNum) const;

	void addEnvelopeADPCMSequenceData(int envNum, int data);
	void removeEnvelopeADPCMSequenceData(int envNum);
	void setEnvelopeADPCMSequenceData(int envNum, int cnt, int data);
	void addEnvelopeADPCMLoop(int arpNum, const InstrumentSequenceLoop& loop);
	void removeEnvelopeADPCMLoop(int envNum, int begin, int end);
	void changeEnvelopeADPCMLoop(int envNum, int prevBegin, int prevEnd, const InstrumentSequenceLoop& loop);
	void clearEnvelopeADPCMLoops(int envNum);
	void setEnvelopeADPCMRelease(int arpNum, const InstrumentSequenceRelease& release);
	void setInstrumentADPCMEnvelope(int instNum, int envNum);
	void setInstrumentADPCMEnvelopeEnabled(int instNum, bool enabled);
	std::multiset<int> getEnvelopeADPCMUsers(int envNum) const;

	void setArpeggioADPCMType(int arpNum, SequenceType type);
	void addArpeggioADPCMSequenceData(int arpNum, int data);
	void removeArpeggioADPCMSequenceData(int arpNum);
	void setArpeggioADPCMSequenceData(int arpNum, int cnt, int data);
	void addArpeggioADPCMLoop(int arpNum, const InstrumentSequenceLoop& loop);
	void removeArpeggioADPCMLoop(int arpNum, int begin, int end);
	void changeArpeggioADPCMLoop(int arpNum, int prevBegin, int prevEnd, const InstrumentSequenceLoop& loop);
	void clearArpeggioADPCMLoops(int arpNum);
	void setArpeggioADPCMRelease(int arpNum, const InstrumentSequenceRelease& release);
	void setInstrumentADPCMArpeggio(int instNum, int arpNum);
	void setInstrumentADPCMArpeggioEnabled(int instNum, bool enabled);
	std::multiset<int> getArpeggioADPCMUsers(int arpNum) const;

	void setPitchADPCMType(int ptNum, SequenceType type);
	void addPitchADPCMSequenceData(int ptNum, int data);
	void removePitchADPCMSequenceData(int ptNum);
	void setPitchADPCMSequenceData(int ptNum, int cnt, int data);
	void addPitchADPCMLoop(int ptNum, const InstrumentSequenceLoop& loop);
	void removePitchADPCMLoop(int ptNum, int begin, int end);
	void changePitchADPCMLoop(int ptNum, int prevBegin, int prevEnd, const InstrumentSequenceLoop& loop);
	void clearPitchADPCMLoops(int ptNum);
	void setPitchADPCMRelease(int ptNum, const InstrumentSequenceRelease& release);
	void setInstrumentADPCMPitch(int instNum, int ptNum);
	void setInstrumentADPCMPitchEnabled(int instNum, bool enabled);
	std::multiset<int> getPitchADPCMUsers(int ptNum) const;

	void addPanADPCMSequenceData(int panNum, int data);
	void removePanADPCMSequenceData(int panNum);
	void setPanADPCMSequenceData(int panNum, int cnt, int data);
	void addPanADPCMLoop(int panNum, const InstrumentSequenceLoop& loop);
	void removePanADPCMLoop(int panNum, int begin, int end);
	void changePanADPCMLoop(int panNum, int prevBegin, int prevEnd, const InstrumentSequenceLoop& loop);
	void clearPanADPCMLoops(int panNum);
	void setPanADPCMRelease(int panNum, const InstrumentSequenceRelease& release);
	void setInstrumentADPCMPan(int instNum, int panNum);
	void setInstrumentADPCMPanEnabled(int instNum, bool enabled);
	std::multiset<int> getPanADPCMUsers(int panNum) const;

	//--- Drumkit
	void setInstrumentDrumkitSample(int instNum, int key, int sampNum);
	void setInstrumentDrumkitSampleEnabled(int instNum, int key, bool enabled);
	void setInstrumentDrumkitPitch(int instNum, int key, int pitch);
	void setInstrumentDrumkitPan(int instNum, int key, int pan);

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
	bool assignADPCMBeforeForcedJamKeyOn(std::shared_ptr<AbstractInstrument> inst,
										 std::unordered_map<int, std::array<size_t, 2>>& sampAddrs);

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
	using ExportCancellCallback = std::function<bool()>;
	bool exportToWav(io::WavContainer& container, int loopCnt, ExportCancellCallback checkFunc);
	bool exportToVgm(io::BinaryContainer& container, int target, bool gd3TagEnabled,
					 const io::GD3Tag& tag, ExportCancellCallback checkFunc);
	bool exportToS98(io::BinaryContainer& container, int target, bool tagEnabled,
					 const io::S98Tag& tag, int rate, ExportCancellCallback checkFunc);

	// Real chip interface
	void connectToRealChip(RealChipInterfaceType type, RealChipInterfaceGeneratorFunc* f = nullptr);
	RealChipInterfaceType getRealChipInterfaceType() const;
	bool hasConnectedToRealChip() const;

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
	void setModulePath(const std::string& path);
	std::string getModulePath() const;
	void setModuleTitle(const std::string& title);
	std::string getModuleTitle() const;
	void setModuleAuthor(const std::string& author);
	std::string getModuleAuthor() const;
	void setModuleCopyright(const std::string& copyright);
	std::string getModuleCopyright() const;
	void setModuleComment(const std::string& comment);
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
	void setGroove(int num, const std::vector<int>& seq);
	void setGrooves(const std::vector<std::vector<int>>& seqs);
	std::vector<int> getGroove(int num) const;
	void clearUnusedPatterns();
	std::unordered_map<int, int> replaceDuplicateInstrumentsInPatterns();
	void clearUnusedADPCMSamples();
	/*----- Song -----*/
	void setSongTitle(int songNum, const std::string& title);
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
	void addSong(SongType songType, const std::string& title);
	void sortSongs(const std::vector<int>& numbers);
	void transposeSong(int songNum, int seminotes, const std::vector<int>& excludeInsts);
	void swapTracks(int songNum, int track1, int track2);
	double estimateSongLength(int songNum) const;
	size_t getTotalStepCount(int songNum, size_t loopCnt) const;
	/*----- Bookmark -----*/
	void addBookmark(int songNum, const std::string& name, int order, int step);
	void changeBookmark(int songNum, int i, const std::string& name, int order, int step);
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
	/*----- Key signature -----*/
	void addKeySignature(int songNum, KeySignature::Type key, int order, int step);
	void changeKeySignature(int songNum, int i, KeySignature::Type key, int order, int step);
	void removeKeySignature(int songNum, int i);
	void clearKeySignature(int songNum);
	KeySignature getKeySignature(int songNum, int i) const;
	size_t getKeySignatureSize(int songNum) const;
	KeySignature::Type searchKeySignatureAt(int songNum, int order, int step) const;
	/*----- Track -----*/
	void setEffectDisplayWidth(int songNum, int trackNum, size_t w);
	size_t getEffectDisplayWidth(int songNum, int trackNum) const;
	void setTrackVisibility(int songNum, int trackNum, bool visible);
	bool isVisibleTrack(int songNum, int trackNum);
	/*----- Order -----*/
	std::vector<OrderInfo> getOrderData(int songNum, int orderNum) const;
	void setOrderPatternDigit(int songNum, int trackNum, int orderNum, int patternNum, bool secondEntry);
	void insertOrderBelow(int songNum, int orderNum);
	void deleteOrder(int songNum, int orderNum);
	void pasteOrderCells(int songNum, int beginTrack, int beginOrder,
						 const std::vector<std::vector<std::string>>& cells);
	void duplicateOrder(int songNum, int orderNum);
	void MoveOrder(int songNum, int orderNum, bool isUp);
	void clonePatterns(int songNum, int beginOrder, int beginTrack, int endOrder, int endTrack);
	void cloneOrder(int songNum, int orderNum);
	size_t getOrderSize(int songNum) const;
	bool canAddNewOrder(int songNum) const;
	/*----- Pattern -----*/
	int getStepNoteNumber(int songNum, int trackNum, int orderNum, int stepNum) const;
	void setStepNote(int songNum, int trackNum, int orderNum, int stepNum, const Note& note, bool instMask, bool volMask);
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
	void setStepEffectIDCharacter(int songNum, int trackNum, int orderNum, int stepNum, int n, const std::string& id, bool fillValue00, bool secondEntry);
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
	///		3: effect ID
	///		4: effect value
	void pastePatternCells(int songNum, int beginTrack, int beginColmn, int beginOrder, int beginStep,
						   const std::vector<std::vector<std::string>>& cells);
	void pasteMixPatternCells(int songNum, int beginTrack, int beginColmn, int beginOrder, int beginStep,
							  const std::vector<std::vector<std::string>>& cells);
	void pasteOverwritePatternCells(int songNum, int beginTrack, int beginColmn, int beginOrder,
									int beginStep, const std::vector<std::vector<std::string>>& cells);
	void pasteInsertPatternCells(int songNum, int beginTrack, int beginColmn, int beginOrder,
								 int beginStep, const std::vector<std::vector<std::string>>& cells);
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

	// Module details
	void makeNewModule(bool withInstrument);

	// Jam mode
	void funcJamKeyOn(JamKey key, int keyNum, const TrackAttribute& attrib, bool volumeSet,
					  std::shared_ptr<AbstractInstrument> inst = nullptr);
	void funcJamKeyOff(JamKey key, int keyNum, const TrackAttribute& attrib);

	// Play song
	void startPlay();
};
