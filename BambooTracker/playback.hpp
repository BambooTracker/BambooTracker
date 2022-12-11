/*
 * Copyright (C) 2019-2022 Rerrah
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

#include <vector>
#include <memory>
#include <mutex>
#include <unordered_map>
#include "module.hpp"
#include "effect.hpp"
#include "enum_hash.hpp"
#include "bamboo_tracker_defs.hpp"

class OPNAController;
class InstrumentsManager;
class TickCounter;

class EffectMemory
{
public:
	EffectMemory();
	void enqueue(const Effect& eff);
	void clear();

	using container_type = std::vector<Effect>;
	using reference = container_type::reference;
	using const_reference = container_type::const_reference;
	using iterator = container_type::iterator;
	using const_iterator = container_type::const_iterator;
	using value_type = container_type::value_type;

	iterator begin() noexcept { return mem_.begin(); }
	const_iterator begin() const noexcept { return mem_.begin(); }
	const_iterator cbegin() const noexcept { return mem_.cbegin(); }
	iterator end() noexcept { return mem_.end(); }
	const_iterator end() const noexcept { return mem_.end(); }
	const_iterator cend() const noexcept { return mem_.cend(); }

	iterator erase(iterator position) { return mem_.erase(position); }
	iterator erase(const_iterator position) { return mem_.erase(position); }

private:
	std::vector<Effect> mem_;
};

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
	bool isPlaySong() const noexcept;
	bool isPlayingStep() const noexcept;
	int getPlayingOrderNumber() const noexcept;
	int getPlayingStepNumber() const noexcept;

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

	struct Position
	{
		int order, step;
		static constexpr int INVALID = -1;
		Position(int o, int s) : order(o), step(s) {}
		void set(int o, int s)	// Faster than making and copying a new instance
		{
			order = o;
			step = s;
		}
		void invalidate() noexcept { set(INVALID, INVALID); }
		bool isValid() const noexcept { return (order > INVALID && step > INVALID); }
	} playingPos_, nextReadPos_;

	uint8_t playStateFlags_;

	// Play song
	bool isFindNextStep_;
	void startPlay();
	void stopPlay();
	bool stepDown();
	void findNextStep();
	void checkValidPosition();

	void stepProcess();

	void executeFMStepEvents(const Step& step, int ch, bool calledByNoteDelay = false);
	void executeSSGStepEvents(const Step& step, int ch, bool calledByNoteDelay = false);
	void executeRhythmStepEvents(const Step& step, int ch, bool calledByNoteDelay = false);
	void executeADPCMStepEvents(const Step& step, bool calledByNoteDelay = false);

	EffectMemory playbackSpeedEffMem_, posChangeEffMem_;
	std::unordered_map<SoundSource, std::vector<EffectMemory>> effOnKeyOnMem_, effOnStepBeginMem_;

	struct RegisterUnit
	{
		int address, value;
		bool hasCompleted;
	};
	using DirectRegisterSetQueue = std::vector<RegisterUnit>;
	using DirectRegisterSetSource = std::vector<DirectRegisterSetQueue>;
	std::unordered_map<SoundSource, DirectRegisterSetSource> directRegisterSets_;

	bool executeStoredEffectsGlobal();
	void storeEffectToMapFM(int ch, const Effect& eff);
	void executeStoredEffectsFM(int ch);
	void storeEffectToMapSSG(int ch, const Effect& eff);
	void executeStoredEffectsSSG(int ch);
	void storeEffectToMapRhythm(int ch, const Effect& eff);
	void executeStoredEffectsRhythm(int ch);
	void storeEffectToMapADPCM(int ch, const Effect& eff);
	void executeStoredEffectsADPCM();
	void storeDirectRegisterSetEffectToQueue(SoundSource src, int ch, const Effect& eff);
	void executeDirectRegisterSetEffect(DirectRegisterSetQueue& queue);

	bool effPositionJump(int nextOrder);
	void effSongEnd();
	bool effPatternBreak(int nextStep);
	void effSpeedChange(int speed);
	void effTempoChange(int tempo);
	void effGrooveChange(int num);

	void tickProcess(int rest);

	/**
	 * @brief checkFMDelayEventsInTick
	 * @param step
	 * @param ch
	 * @return \c true \c if note delay is occurred.
	 */
	bool checkFMDelayEventsInTick(const Step& step, int ch);
	/**
	 * @brief checkFMNoteDelayAndEnvelopeReset
	 * @param step
	 * @param ch
	 * @return \c true \c if note delay is occurred.
	 */
	bool checkFMNoteDelayAndEnvelopeReset(const Step& step, int ch);
	void envelopeResetEffectFM(const Step& step, int ch);
	/**
	 * @brief checkSSGDelayEventsInTick
	 * @param step
	 * @param ch
	 * @return \c true \c if note delay is occurred.
	 */
	bool checkSSGDelayEventsInTick(const Step& step, int ch);
	/**
	 * @brief checkRhythmDelayEventsInTick
	 * @param step
	 * @param ch
	 * @return \c true \c if note delay is occurred.
	 */
	bool checkRhythmDelayEventsInTick(const Step& step, int ch);
	/**
	 * @brief checkADPCMDelayEventsInTick
	 * @param step
	 * @return \c true \c if note delay is occurred.
	 */
	bool checkADPCMDelayEventsInTick(const Step& step);

	std::vector<int> ntDlyCntFM_, ntReleaseDlyCntFM_, volDlyCntFM_, ntCutDlyCntFM_, rtrgCntFM_;
	std::vector<int> ntDlyCntSSG_, ntReleaseDlyCntSSG_, volDlyCntSSG_, ntCutDlyCntSSG_, rtrgCntSSG_;
	std::vector<int> ntDlyCntRhythm_, ntReleaseDlyCntRhythm_, volDlyCntRhythm_, ntCutDlyCntRhythm_, rtrgCntRhythm_;
	int ntDlyCntADPCM_, ntReleaseDlyCntADPCM_, volDlyCntADPCM_, ntCutDlyCntADPCM_, rtrgCntADPCM_;
	std::vector<int> volDlyValueFM_, volDlyValueSSG_, volDlyValueRhythm_;
	int volDlyValueADPCM_;
	std::vector<int> tposeDlyCntFM_, tposeDlyCntSSG_;
	int tposeDlyCntADPCM_;
	std::vector<int> tposeDlyValueFM_, tposeDlyValueSSG_;
	int tposeDlyValueADPCM_;
	std::vector<int> rtrgCntValueFM_, rtrgCntValueSSG_, rtrgCntValueRhythm_;
	int rtrgCntValueADPCM_;
	std::vector<int> rtrgVolValueFM_, rtrgVolValueSSG_, rtrgVolValueRhythm_;
	int rtrgVolValueADPCM_;

	void clearEffectMaps();
	void clearDelayWithinStepCounts();
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
