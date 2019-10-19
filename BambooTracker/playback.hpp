#pragma once

#include <vector>
#include <memory>
#include <mutex>
#include "opna_controller.hpp"
#include "instruments_manager.hpp"
#include "module.hpp"
#include "tick_counter.hpp"
#include "effect.hpp"

enum class PlaybackState
{
	PlaySong, PlayFromStart, PlayPattern, PlayFromCurrentStep, Stop
};

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
	void startPlayFromCurrentStep(int order, int step);
	void stopPlaySong();
	bool isPlaySong() const;
	PlaybackState getPlaybackState() const;
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
	/// Low nibble - read state
	///		bit 0: playing
	///		bit 1: have read first step data
	unsigned int playState_;
	PlaybackState managerState_;

	// Play song
	bool isFindNextStep_;
	void startPlay();
	void stopPlay();
	bool stepDown();
	void findNextStep();
	void checkValidPosition();

	void readStep();

	bool readFMStep(Step& step, int ch);
	bool readFMEventsInStep(Step& step, int ch, bool calledByNoteDelay = false);
	bool readSSGStep(Step& step, int ch);
	bool readSSGEventsInStep(Step& step, int ch, bool calledByNoteDelay = false);
	bool readDrumStep(Step& step, int ch);
	bool readDrumEventsInStep(Step& step, int ch, bool calledByNoteDelay = false);

	std::vector<std::vector<Effect>> keyOnBasedEffsFM_, stepBeginBasedEffsFM_, stepEndBasedEffsFM_;
	std::vector<std::vector<Effect>> keyOnBasedEffsSSG_, stepBeginBasedEffsSSG_, stepEndBasedEffsSSG_;
	std::vector<std::vector<Effect>> keyOnBasedEffsDrum_, stepBeginBasedEffsDrum_, stepEndBasedEffsDrum_;
	bool setEffectToQueueFM(int ch, Effect eff);
	bool readFMEffectFromQueue(int ch);
	bool setEffectToQueueSSG(int ch, Effect eff);
	bool readSSGEffectFromQueue(int ch);
	bool setEffectToQueueDrum(int ch, Effect eff);
	bool readDrumEffectFromQueue(int ch);

	bool effPositionJump(int nextOrder);
	void effTrackEnd();
	bool effPatternBreak(int nextStep);
	void effSpeedChange(int speed);
	void effTempoChange(int tempo);
	void effGrooveChange(int num);

	void readTick(int rest);

	void checkFMDelayEventsInTick(Step& step, int ch);
	void readTickFMForNoteDelay(Step& step, int ch);
	void envelopeResetEffectFM(Step& step, int ch);
	void checkSSGDelayEventsInTick(Step& step, int ch);
	void checkDrumDelayEventsInTick(Step& step, int ch);


	std::vector<int> ntDlyCntFM_, ntCutDlyCntFM_, volDlyCntFM_;
	std::vector<int> ntDlyCntSSG_, ntCutDlyCntSSG_, volDlyCntSSG_;
	std::vector<int> ntDlyCntDrum_, ntCutDlyCntDrum_, volDlyCntDrum_;
	std::vector<int> volDlyValueFM_, volDlyValueSSG_, volDlyValueDrum_;
	std::vector<int> tposeDlyCntFM_, tposeDlyCntSSG_;
	std::vector<int> tposeDlyValueFM_, tposeDlyValueSSG_;

	void clearEffectQueues();
	void clearNoteDelayCounts();
	void clearDelayBeyondStepCounts();
	void clearFMDelayBeyondStepCounts(int ch);
	void clearSSGDelayBeyondStepCounts(int ch);
	void clearDrumDelayBeyondStepCounts(int ch);
	void updateDelayEventCounts();

	bool isRetrieveChannel_;
	void retrieveChannelStates();

	size_t getOrderSize(int songNum) const;
	size_t getPatternSizeFromOrderNumber(int songNum, int orderNum) const;
};
