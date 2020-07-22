#pragma once

#include <vector>
#include <memory>
#include <mutex>
#include <unordered_map>
#include "opna_controller.hpp"
#include "instruments_manager.hpp"
#include "module.hpp"
#include "tick_counter.hpp"
#include "effect.hpp"
#include "enum_hash.hpp"

/// Divede playback routine from main class BambooTracker
class PlaybackManager
{
public:
	PlaybackManager(std::shared_ptr<OPNAController> opnaCtrl,
					std::weak_ptr<InstrumentsManager> instMan,
					std::weak_ptr<TickCounter> tickCounter,
					std::weak_ptr<Module> mod, bool isRetrieveChannel);

	void setSong(std::weak_ptr<Module> mod, int songNum);

	// Play song
	void startPlaySong(int order);
	void startPlayFromStart();
	void startPlayPattern(int order);
	void startPlayFromPosition(int order, int step);
	void playStep(int order, int step);
	void stopPlaySong();
	bool isPlaySong() const;
	bool isPlayingStep() const;
	int getPlayingOrderNumber() const;
	int getPlayingStepNumber() const;

	// Stream events
	/// 0<: Tick
	///  0: Step
	/// -1: Stop
	int streamCountUp();

	void checkPlayPosition(int maxStepSize);

	void setChannelRetrieving(bool enabled);

private:
	std::shared_ptr<OPNAController> opnaCtrl_;
	std::weak_ptr<InstrumentsManager> instMan_;
	std::weak_ptr<TickCounter> tickCounter_;
	std::weak_ptr<Module> mod_;

	std::mutex mutex_;

	int curSongNum_;
	SongStyle songStyle_;
	int playOrderNum_, playStepNum_;
	int nextReadOrder_, nextReadStep_;
	/// High nibble - play type
	///		bit 4: If high, loop pattern
	///		bit 5: Play step
	/// Low nibble - read state
	///		bit 0: playing
	///		bit 1: have read first step data
	unsigned int playState_;

	// Play song
	bool isFindNextStep_;
	void startPlay();
	void stopPlay();
	bool stepDown();
	void findNextStep();
	void checkValidPosition();

	void stepProcess();
	std::unordered_map<SoundSource, std::vector<bool>> isNoteDelay_;

	void executeFMStepEvents(Step& step, int ch, bool calledByNoteDelay = false);
	void executeSSGStepEvents(Step& step, int ch, bool calledByNoteDelay = false);
	void executeRhythmStepEvents(Step& step, int ch, bool calledByNoteDelay = false);
	void executeADPCMStepEvents(Step& step, bool calledByNoteDelay = false);

	using EffectMemory = std::unordered_map<EffectType, int>;
	EffectMemory stepBeginBasedEffsGlobal_, stepEndBasedEffsGlobal_;
	using EffectMemorySource = std::vector<std::unordered_map<EffectType, int>>;
	std::unordered_map<SoundSource, EffectMemorySource> keyOnBasedEffs_, stepBeginBasedEffs_;
	bool executeStoredEffectsGlobal();
	bool storeEffectToMapFM(int ch, Effect eff);
	void executeStoredEffectsFM(int ch);
	bool storeEffectToMapSSG(int ch, Effect eff);
	void executeStoredEffectsSSG(int ch);
	bool storeEffectToMapRhythm(int ch, Effect eff);
	void executeStoredEffectsRhythm(int ch);
	bool storeEffectToMapADPCM(int ch, Effect eff);
	void executeStoredEffectsADPCM();

	bool effPositionJump(int nextOrder);
	void effSongEnd();
	bool effPatternBreak(int nextStep);
	void effSpeedChange(int speed);
	void effTempoChange(int tempo);
	void effGrooveChange(int num);

	void tickProcess(int rest);

	void checkFMDelayEventsInTick(Step& step, int ch);
	void checkFMNoteDelayAndEnvelopeReset(Step& step, int ch);
	void envelopeResetEffectFM(Step& step, int ch);
	void checkSSGDelayEventsInTick(Step& step, int ch);
	void checkRhythmDelayEventsInTick(Step& step, int ch);
	void checkADPCMDelayEventsInTick(Step& step);


	std::vector<int> ntDlyCntFM_, ntCutDlyCntFM_, volDlyCntFM_;
	std::vector<int> ntDlyCntSSG_, ntCutDlyCntSSG_, volDlyCntSSG_;
	std::vector<int> ntDlyCntRhythm_, ntCutDlyCntRhythm_, volDlyCntRhythm_;
	int ntDlyCntADPCM_, ntCutDlyCntADPCM_, volDlyCntADPCM_;
	std::vector<int> volDlyValueFM_, volDlyValueSSG_, volDlyValueRhythm_;
	int volDlyValueADPCM_;
	std::vector<int> tposeDlyCntFM_, tposeDlyCntSSG_;
	int tposeDlyCntADPCM_;
	std::vector<int> tposeDlyValueFM_, tposeDlyValueSSG_;
	int tposeDlyValueADPCM_;

	void clearEffectMaps();
	void clearNoteDelayCounts();
	void clearDelayBeyondStepCounts();
	void clearFMDelayBeyondStepCounts(int ch);
	void clearSSGDelayBeyondStepCounts(int ch);
	void clearRhythmDelayBeyondStepCounts(int ch);
	void clearADPCMDelayBeyondStepCounts();
	void updateDelayEventCounts();

	bool isRetrieveChannel_;
	void retrieveChannelStates();

	size_t getOrderSize(int songNum) const;
	size_t getPatternSizeFromOrderNumber(int songNum, int orderNum) const;
};
