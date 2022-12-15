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

#include "playback.hpp"
#include <algorithm>
#include "opna_controller.hpp"
#include "instruments_manager.hpp"
#include "tick_counter.hpp"
#include "note.hpp"
#include "utils.hpp"

EffectMemory::EffectMemory()
{
	mem_.reserve(Step::N_EFFECT);
}

void EffectMemory::enqueue(const Effect& eff)
{
	auto itr = utils::findIf(mem_, [type = eff.type](const Effect& a) { return a.type == type; });
	if (itr != mem_.end()) mem_.erase(itr);
	mem_.push_back(eff);
}

void EffectMemory::clear()
{
	mem_.clear();
}

//-----------------------------------------------

namespace
{
namespace PlayStateFlag
{
enum : uint8_t
{
	Clear			= 0,
	// Read state
	Playing			= 1 << 0,
	ReadFirstStep	= 1 << 1,
	// Play type
	LoopPattern		= 1 << 2,
	PlayStep		= 1 << 3
};
}
}

PlaybackManager::PlaybackManager(std::shared_ptr<OPNAController> opnaCtrl,
								 std::weak_ptr<InstrumentsManager> instMan, std::weak_ptr<TickCounter> tickCounter,
								 std::weak_ptr<Module> mod,
								 bool isRetrieveChannel)
	: opnaCtrl_(opnaCtrl),
	  instMan_(instMan),
	  tickCounter_(tickCounter),
	  mod_(mod),
	  curSongNum_(0),
	  playingPos_(Position::INVALID, Position::INVALID),
	  nextReadPos_(Position::INVALID, Position::INVALID),
	  isFindNextStep_(false),
	  isRetrieveChannel_(isRetrieveChannel)
{
	songStyle_ = mod.lock()->getSong(curSongNum_).getStyle();

	clearEffectMaps();
	clearDelayWithinStepCounts();
	clearDelayBeyondStepCounts();
}

void PlaybackManager::setSong(std::weak_ptr<Module> mod, int songNum)
{
	mod_ = mod;
	curSongNum_ = songNum;
	songStyle_ = mod_.lock()->getSong(curSongNum_).getStyle();

	/* opna mode is changed in BambooTracker class */

	size_t fmch = Song::getFMChannelCount(songStyle_.type);
	effOnKeyOnMem_[SoundSource::FM] = std::vector<EffectMemory>(fmch);
	effOnStepBeginMem_[SoundSource::FM] = std::vector<EffectMemory>(fmch);
	directRegisterSets_[SoundSource::FM] = DirectRegisterSetSource(fmch);
	ntDlyCntFM_ = std::vector<int>(fmch);
	ntReleaseDlyCntFM_ = std::vector<int>(fmch);
	volDlyCntFM_ = std::vector<int>(fmch);
	volDlyValueFM_ = std::vector<int>(fmch, -1);
	tposeDlyCntFM_ = std::vector<int>(fmch);
	tposeDlyValueFM_ = std::vector<int>(fmch);
	ntCutDlyCntFM_ = std::vector<int>(fmch);
	rtrgCntFM_ = std::vector<int>(fmch);
	rtrgCntValueFM_ = std::vector<int>(fmch);
	rtrgVolValueFM_ = std::vector<int>(fmch);

	effOnKeyOnMem_[SoundSource::SSG] = std::vector<EffectMemory>(3);
	effOnStepBeginMem_[SoundSource::SSG] = std::vector<EffectMemory>(3);
	directRegisterSets_[SoundSource::SSG] = DirectRegisterSetSource(3);
	ntDlyCntSSG_ = std::vector<int>(3);
	ntReleaseDlyCntSSG_ = std::vector<int>(3);
	volDlyCntSSG_ = std::vector<int>(3);
	volDlyValueSSG_ = std::vector<int>(3, -1);
	tposeDlyCntSSG_ = std::vector<int>(3);
	tposeDlyValueSSG_ = std::vector<int>(3);
	ntCutDlyCntSSG_ = std::vector<int>(3);
	rtrgCntSSG_ = std::vector<int>(3);
	rtrgCntValueSSG_ = std::vector<int>(3);
	rtrgVolValueSSG_ = std::vector<int>(3);

	effOnKeyOnMem_[SoundSource::RHYTHM] = std::vector<EffectMemory>(6);
	effOnStepBeginMem_[SoundSource::RHYTHM] = std::vector<EffectMemory>(6);
	directRegisterSets_[SoundSource::RHYTHM] = DirectRegisterSetSource(6);
	ntDlyCntRhythm_ = std::vector<int>(6);
	ntReleaseDlyCntRhythm_ = std::vector<int>(6);
	volDlyCntRhythm_ = std::vector<int>(6);
	volDlyValueRhythm_ = std::vector<int>(6, -1);
	ntCutDlyCntRhythm_ = std::vector<int>(6);
	rtrgCntRhythm_ = std::vector<int>(6);
	rtrgCntValueRhythm_ = std::vector<int>(6);
	rtrgVolValueRhythm_ = std::vector<int>(6);

	effOnKeyOnMem_[SoundSource::ADPCM] = std::vector<EffectMemory>(1);
	effOnStepBeginMem_[SoundSource::ADPCM] = std::vector<EffectMemory>(1);
	directRegisterSets_[SoundSource::ADPCM] = DirectRegisterSetSource(1);
	ntDlyCntADPCM_ = 0;
	ntReleaseDlyCntADPCM_ = 0;
	volDlyCntADPCM_ = 0;
	volDlyValueADPCM_ = -1;
	ntCutDlyCntADPCM_ = 0;
	rtrgCntADPCM_ = 0;
	rtrgCntValueADPCM_ = 0;
	rtrgVolValueADPCM_ = 0;
}

/********** Play song **********/
void PlaybackManager::startPlaySong(int order)
{
	std::lock_guard<std::mutex> lock(mutex_);

	startPlay();
	playStateFlags_ = PlayStateFlag::Playing;
	playingPos_.set(order, 0);
	findNextStep();
	if (isRetrieveChannel_) retrieveChannelStates();
}

void PlaybackManager::startPlayFromStart()
{
	std::lock_guard<std::mutex> lock(mutex_);

	startPlay();
	playStateFlags_ = PlayStateFlag::Playing;
	playingPos_.set(0, 0);
	findNextStep();
}

void PlaybackManager::startPlayPattern(int order)
{
	std::lock_guard<std::mutex> lock(mutex_);

	startPlay();
	playStateFlags_ = PlayStateFlag::Playing | PlayStateFlag::LoopPattern;
	playingPos_.set(order, 0);
	findNextStep();
	if (isRetrieveChannel_) retrieveChannelStates();
}

void PlaybackManager::startPlayFromPosition(int order, int step)
{
	std::lock_guard<std::mutex> lock(mutex_);

	startPlay();
	playStateFlags_ = PlayStateFlag::Playing;
	playingPos_.set(order, step);
	findNextStep();
	if (isRetrieveChannel_) retrieveChannelStates();
}

void PlaybackManager::playStep(int order, int step)
{
	std::lock_guard<std::mutex> lock(mutex_);

	bool isPlaying = isPlayingStep();
	if (!isPlaying) {
		opnaCtrl_->reset();

		Song& song = mod_.lock()->getSong(curSongNum_);
		tickCounter_.lock()->setTempo(song.getTempo());
		tickCounter_.lock()->setSpeed(song.getSpeed());
		tickCounter_.lock()->setGroove(mod_.lock()->getGroove(song.getGroove()));
		tickCounter_.lock()->setGrooveState(song.isUsedTempo() ? GrooveState::Invalid
															   : GrooveState::ValidByGlobal);
	}
	tickCounter_.lock()->resetCount();
	tickCounter_.lock()->setPlayState(true);

	clearEffectMaps();
	clearDelayWithinStepCounts();
	clearDelayBeyondStepCounts();

	playStateFlags_ = PlayStateFlag::PlayStep;
	playingPos_.set(order, step);

	if (!isPlaying && isRetrieveChannel_) retrieveChannelStates();
}

void PlaybackManager::startPlay()
{
	opnaCtrl_->reset();

	Song& song = mod_.lock()->getSong(curSongNum_);
	tickCounter_.lock()->setTempo(song.getTempo());
	tickCounter_.lock()->setSpeed(song.getSpeed());
	tickCounter_.lock()->setGroove(mod_.lock()->getGroove(song.getGroove()));
	tickCounter_.lock()->setGrooveState(song.isUsedTempo() ? GrooveState::Invalid
														   : GrooveState::ValidByGlobal);
	tickCounter_.lock()->resetCount();
	tickCounter_.lock()->setPlayState(true);

	clearEffectMaps();
	clearDelayWithinStepCounts();
	clearDelayBeyondStepCounts();
}

void PlaybackManager::stopPlaySong()
{
	std::lock_guard<std::mutex> lock(mutex_);
	stopPlay();
}

void PlaybackManager::stopPlay()
{
	// No mutex to call from PlaybackManager::streamCountUp
	opnaCtrl_->reset();

	tickCounter_.lock()->setPlayState(false);
	playStateFlags_ = PlayStateFlag::Clear;
	playingPos_.invalidate();
}

bool PlaybackManager::isPlaySong() const noexcept
{
	return playStateFlags_ & PlayStateFlag::Playing;
}

bool PlaybackManager::isPlayingStep() const noexcept
{
	return playStateFlags_ & PlayStateFlag::PlayStep;
}

int PlaybackManager::getPlayingOrderNumber() const noexcept
{
	return playingPos_.order;
}

int PlaybackManager::getPlayingStepNumber() const noexcept
{
	return playingPos_.step;
}

/********** Stream events **********/
int PlaybackManager::streamCountUp()
{
	std::lock_guard<std::mutex> lock(mutex_);

	int state = tickCounter_.lock()->countUp();

	if (state > 0) {	// Tick process in playback
		checkValidPosition();
		tickProcess(state);
	}
	else if (!state) {	// Step process in playback
		checkValidPosition();
		if (stepDown()) {
			stepProcess();
			if (!isFindNextStep_) findNextStep();
		}
		else if (!isPlayingStep()) {
			stopPlay();
		}
	}
	else {	// Stop playback
		for (auto& attrib : songStyle_.trackAttribs) {
			opnaCtrl_->tickEvent(attrib.source, attrib.channelInSource);
		}
	}

	return state;
}

bool PlaybackManager::stepDown()
{
	if (playStateFlags_ & PlayStateFlag::ReadFirstStep) {	// Foward current step
		if (isPlayingStep()) {
			return false;
		}
		else {
			if (!nextReadPos_.isValid()) {
				isFindNextStep_ = false;
				return false;
			}
			else {
				playingPos_ = nextReadPos_;
			}
		}
	}
	else {	// First read
		playStateFlags_ |= PlayStateFlag::ReadFirstStep;
	}

	return true;
}

void PlaybackManager::findNextStep()
{
	// Init
	nextReadPos_ = playingPos_;

	// Search
	int ptnSize = static_cast<int>(getPatternSizeFromOrderNumber(curSongNum_, nextReadPos_.order));
	if (!ptnSize || nextReadPos_.step >= ptnSize - 1) {
		if (!(playStateFlags_ & PlayStateFlag::LoopPattern)) {	// Loop pattern
			if (nextReadPos_.order >= static_cast<int>(getOrderSize(curSongNum_)) - 1) {
				nextReadPos_.order = 0;
			}
			else {
				++nextReadPos_.order;
			}
		}
		nextReadPos_.step = 0;
	}
	else {
		++nextReadPos_.step;
	}

	isFindNextStep_ = true;
}

void PlaybackManager::checkValidPosition()
{
	auto& song = mod_.lock()->getSong(curSongNum_);
	int orderSize = static_cast<int>(song.getOrderSize());
	if (playingPos_.order >= orderSize) {
		playingPos_.set(0, 0);
		nextReadPos_ = playingPos_;
	}
	else if (playingPos_.step >= static_cast<int>(song.getPatternSizeFromOrderNumber(playingPos_.order))) {
		if (playingPos_.order == orderSize - 1) {
			playingPos_.set(0, 0);
			nextReadPos_ = playingPos_;
		}
		else {
			++playingPos_.order;
			playingPos_.step = 0;
			nextReadPos_ = playingPos_;
		}
	}
}

/// Register update order: volume -> instrument -> effect -> key on
void PlaybackManager::stepProcess()
{
	clearDelayWithinStepCounts();
	updateDelayEventCounts();

	auto& song = mod_.lock()->getSong(curSongNum_);

	// Store effects from the step to map
	for (auto& attrib : songStyle_.trackAttribs) {
		auto& step = song.getTrack(attrib.number)
					 .getPatternFromOrderNumber(playingPos_.order).getStep(playingPos_.step);
		size_t uch = static_cast<size_t>(attrib.channelInSource);
		effOnKeyOnMem_[attrib.source].at(uch).clear();
		directRegisterSets_[attrib.source].at(uch).clear();
		using storeFunc = void (PlaybackManager::*)(int, const Effect&);
		static const std::unordered_map<SoundSource, storeFunc> storeEffectToMap = {
			{ SoundSource::FM, &PlaybackManager::storeEffectToMapFM },
			{ SoundSource::SSG, &PlaybackManager::storeEffectToMapSSG },
			{ SoundSource::RHYTHM, &PlaybackManager::storeEffectToMapRhythm },
			{ SoundSource::ADPCM, &PlaybackManager::storeEffectToMapADPCM }
		};
		for (int i = 0; i < Step::N_EFFECT; ++i) {
			Effect&& eff = effect_utils::validateEffect(attrib.source, step.getEffect(i));
			(this->*storeEffectToMap.at(attrib.source))(attrib.channelInSource, eff);
		}
	}

	// Execute step events
	bool isNextSet = executeStoredEffectsGlobal();
	const int countsInStep = tickCounter_.lock()->getCountsInCurrentStep();
	for (auto& attrib : songStyle_.trackAttribs) {
		// Check whether it has been set note delay effect
		size_t uch = static_cast<size_t>(attrib.channelInSource);
		bool hasSetNoteDelay = false;
		auto& mem = effOnStepBeginMem_[attrib.source].at(uch);
		for (auto itr = mem.begin(); itr != mem.end(); ) {
			if (itr->type == EffectType::NoteDelay) {
				if (itr->value < countsInStep) {
					hasSetNoteDelay = true;
				}
				else {
					itr = mem.erase(itr);
					continue;
				}
			}
			++itr;
		}

		//
		auto& step = song.getTrack(attrib.number)
					 .getPatternFromOrderNumber(playingPos_.order).getStep(playingPos_.step);
		switch (attrib.source) {
		case SoundSource::FM:
			if (hasSetNoteDelay) {
				// Set effect
				executeStoredEffectsFM(attrib.channelInSource);
				checkFMNoteDelayAndEnvelopeReset(step, attrib.channelInSource);
				opnaCtrl_->tickEvent(SoundSource::FM, attrib.channelInSource);
			}
			else {
				executeFMStepEvents(step, attrib.channelInSource);
			}
			break;
		case SoundSource::SSG:
			if (hasSetNoteDelay) {
				// Set effect
				executeStoredEffectsSSG(attrib.channelInSource);
				opnaCtrl_->tickEvent(SoundSource::SSG, attrib.channelInSource);
			}
			else {
				executeSSGStepEvents(step, attrib.channelInSource);
			}
			break;
		case SoundSource::RHYTHM:
			if (hasSetNoteDelay) {
				// Set effect
				executeStoredEffectsRhythm(attrib.channelInSource);
				opnaCtrl_->tickEvent(SoundSource::RHYTHM, attrib.channelInSource);
			}
			else {
				executeRhythmStepEvents(step, attrib.channelInSource);
			}
			break;
		case SoundSource::ADPCM:
			if (hasSetNoteDelay) {
				// Set effect
				executeStoredEffectsADPCM();
				opnaCtrl_->tickEvent(SoundSource::ADPCM, attrib.channelInSource);
			}
			else {
				executeADPCMStepEvents(step);
			}
			break;
		}
	}
	opnaCtrl_->updateRegisterStates();	// Update for other changes

	isFindNextStep_ = isNextSet;
}

void PlaybackManager::executeFMStepEvents(const Step& step, int ch, bool calledByNoteDelay)
{
	if (!calledByNoteDelay && !step.isEmptyNote()) clearFMDelayBeyondStepCounts(ch);	// Except no key event

	// Set volume
	int vol = step.getVolume();
	if (step.hasVolume() && vol < bt_defs::NSTEP_FM_VOLUME) {
		opnaCtrl_->setVolumeFM(ch, vol);
	}

	// Set instrument
	if (step.hasInstrument()) {
		if (auto inst = std::dynamic_pointer_cast<InstrumentFM>(
					instMan_.lock()->getInstrumentSharedPtr(step.getInstrumentNumber())))
			opnaCtrl_->setInstrumentFM(ch, inst);
	}
	else {
		opnaCtrl_->restoreFMEnvelopeFromReset(ch);
	}

	// Set effect
	executeStoredEffectsFM(ch);

	// Set key
	int noteNum = step.getNoteNumber();
	switch (noteNum) {
	case Step::NOTE_NONE:
		if (!calledByNoteDelay) {
			// When this is called by note delay, skip delay check because it has already checked.
			checkFMDelayEventsInTick(step, ch);
		}
		opnaCtrl_->tickEvent(SoundSource::FM, ch);
		break;
	case Step::NOTE_KEY_OFF:
		opnaCtrl_->keyOffFM(ch);
		break;
	case Step::NOTE_KEY_CUT:
		opnaCtrl_->resetFMChannelEnvelope(ch);
		break;
	case Step::NOTE_ECHO0:
		opnaCtrl_->keyOnFM(ch, 0);
		break;
	case Step::NOTE_ECHO1:
		opnaCtrl_->keyOnFM(ch, 1);
		break;
	case Step::NOTE_ECHO2:
		opnaCtrl_->keyOnFM(ch, 2);
		break;
	case Step::NOTE_ECHO3:
		opnaCtrl_->keyOnFM(ch, 3);
		break;
	default:	// Key on
		opnaCtrl_->keyOnFM(ch, Note(noteNum));
		break;
	}
}

void PlaybackManager::executeSSGStepEvents(const Step& step, int ch, bool calledByNoteDelay)
{
	if (!calledByNoteDelay && !step.isEmptyNote()) clearSSGDelayBeyondStepCounts(ch);	// Except no key event

	// Set volume
	int vol = step.getVolume();
	if (step.hasVolume() && vol < bt_defs::NSTEP_SSG_VOLUME) {
		opnaCtrl_->setVolumeSSG(ch, vol);
	}

	// Set instrument
	if (step.hasInstrument()) {
		if (auto inst = std::dynamic_pointer_cast<InstrumentSSG>(
					instMan_.lock()->getInstrumentSharedPtr(step.getInstrumentNumber())))
			opnaCtrl_->setInstrumentSSG(ch, inst);
	}

	// Set effect
	executeStoredEffectsSSG(ch);

	// Set key
	int noteNum = step.getNoteNumber();
	switch (noteNum) {
	case Step::NOTE_NONE:
		if (!calledByNoteDelay) {
			// When this is called by note delay, skip delay check because it has already checked.
			checkSSGDelayEventsInTick(step, ch);
		}
		opnaCtrl_->tickEvent(SoundSource::SSG, ch);
		break;
	case Step::NOTE_KEY_OFF:
		opnaCtrl_->keyOffSSG(ch);
		break;
	case Step::NOTE_KEY_CUT:
		opnaCtrl_->setNoteCutSSG(ch);
		break;
	case Step::NOTE_ECHO0:
		opnaCtrl_->keyOnSSG(ch, 0);
		break;
	case Step::NOTE_ECHO1:
		opnaCtrl_->keyOnSSG(ch, 1);
		break;
	case Step::NOTE_ECHO2:
		opnaCtrl_->keyOnSSG(ch, 2);
		break;
	case Step::NOTE_ECHO3:
		opnaCtrl_->keyOnSSG(ch, 3);
		break;
	default:	// Key on
		opnaCtrl_->keyOnSSG(ch, Note(noteNum));
		break;
	}
}

void PlaybackManager::executeRhythmStepEvents(const Step& step, int ch, bool calledByNoteDelay)
{
	if (!calledByNoteDelay && !step.isEmptyNote()) clearRhythmDelayBeyondStepCounts(ch);	// Except no key event

	// Set volume
	int vol = step.getVolume();
	if (step.hasVolume() && vol < bt_defs::NSTEP_RHYTHM_VOLUME) {
		opnaCtrl_->setVolumeRhythm(ch, vol);
	}

	// Set effect
	executeStoredEffectsRhythm(ch);

	// Set key
	switch (step.getNoteNumber()) {
	case Step::NOTE_NONE:
		if (!calledByNoteDelay) {
			// When this is called by note delay, skip delay check because it has already checked.
			checkRhythmDelayEventsInTick(step, ch);
		}
		opnaCtrl_->tickEvent(SoundSource::RHYTHM, ch);
		break;
	case Step::NOTE_KEY_OFF:
	case Step::NOTE_KEY_CUT:
		opnaCtrl_->setKeyOffFlagRhythm(ch);
		break;
	default:	// Key on & Echo
		opnaCtrl_->setKeyOnFlagRhythm(ch);
		break;
	}
}

void PlaybackManager::executeADPCMStepEvents(const Step& step, bool calledByNoteDelay)
{
	if (!calledByNoteDelay && !step.isEmptyNote()) clearADPCMDelayBeyondStepCounts();	// Except no key event

	// Set volume
	int vol = step.getVolume();
	if (step.hasVolume() && vol < bt_defs::NSTEP_ADPCM_VOLUME) {
		opnaCtrl_->setVolumeADPCM(vol);
	}

	// Set instrument
	if (step.hasInstrument()) {
		if (auto inst = instMan_.lock()->getInstrumentSharedPtr(step.getInstrumentNumber())) {
			if (inst->getType() == InstrumentType::ADPCM)
				opnaCtrl_->setInstrumentADPCM(std::dynamic_pointer_cast<InstrumentADPCM>(inst));
			else if (inst->getType() == InstrumentType::Drumkit)
				opnaCtrl_->setInstrumentDrumkit(std::dynamic_pointer_cast<InstrumentDrumkit>(inst));
		}
	}

	// Set effect
	executeStoredEffectsADPCM();

	// Set key
	int noteNum = step.getNoteNumber();
	switch (noteNum) {
	case Step::NOTE_NONE:
		if (!calledByNoteDelay) {
			// When this is called by note delay, skip delay check because it has already checked.
			checkADPCMDelayEventsInTick(step);
		}
		opnaCtrl_->tickEvent(SoundSource::ADPCM, 0);
		break;
	case Step::NOTE_KEY_OFF:
		opnaCtrl_->keyOffADPCM();
		break;
	case Step::NOTE_KEY_CUT:
		opnaCtrl_->setNoteCutADPCM();
		break;
	case Step::NOTE_ECHO0:
		opnaCtrl_->keyOnADPCM(0);
		break;
	case Step::NOTE_ECHO1:
		opnaCtrl_->keyOnADPCM(1);
		break;
	case Step::NOTE_ECHO2:
		opnaCtrl_->keyOnADPCM(2);
		break;
	case Step::NOTE_ECHO3:
		opnaCtrl_->keyOnADPCM(3);
		break;
	default:	// Key on
		opnaCtrl_->keyOnADPCM(Note(noteNum));
		break;
	}
}

bool PlaybackManager::executeStoredEffectsGlobal()
{
	bool changedNextPos = false;

	// Read step end based effects
	for (const Effect& eff : posChangeEffMem_) {
		switch (eff.type) {
		case EffectType::PositionJump:
			if (!(playStateFlags_ & PlayStateFlag::LoopPattern)) {	// Skip when loop pattern
				changedNextPos |= effPositionJump(eff.value);
			}
			break;
		case EffectType::SongEnd:
			if (!(playStateFlags_ & PlayStateFlag::LoopPattern)) {	// Skip when loop pattern
				effSongEnd();
				changedNextPos = true;
			}
			break;
		case EffectType::PatternBreak:
			if (!(playStateFlags_ & PlayStateFlag::LoopPattern)) {	// Skip when loop pattern
				changedNextPos |= effPatternBreak(eff.value);
			}
			break;
		default:
			break;
		}
	}
	posChangeEffMem_.clear();

	// Read step beginning based effects
	for (const Effect& eff : playbackSpeedEffMem_) {
		switch (eff.type) {
		case EffectType::SpeedTempoChange:
			if (eff.value < 0x20) effSpeedChange(eff.value);
			else effTempoChange(eff.value);
			break;
		case EffectType::Groove:
			if (eff.value < static_cast<int>(mod_.lock()->getGrooveCount()))
				effGrooveChange(eff.value);
			break;
		default:
			break;
		}
	}
	playbackSpeedEffMem_.clear();

	return changedNextPos;
}

void PlaybackManager::storeEffectToMapFM(int ch, const Effect& eff)
{
	switch (eff.type) {
	case EffectType::Arpeggio:
	case EffectType::PortamentoUp:
	case EffectType::PortamentoDown:
	case EffectType::TonePortamento:
	case EffectType::Vibrato:
	case EffectType::Tremolo:
	case EffectType::Pan:
	case EffectType::VolumeSlide:
	case EffectType::Detune:
	case EffectType::FineDetune:
	case EffectType::NoteSlideUp:
	case EffectType::NoteSlideDown:
	case EffectType::NoteRelease:
	case EffectType::TransposeDelay:
	case EffectType::VolumeDelay:
	case EffectType::FBControl:
	case EffectType::TLControl:
	case EffectType::MLControl:
	case EffectType::ARControl:
	case EffectType::DRControl:
	case EffectType::RRControl:
	case EffectType::Brightness:
	case EffectType::NoteCut:
	case EffectType::Retrigger:
	case EffectType::XVolumeSlide:
		effOnKeyOnMem_[SoundSource::FM].at(static_cast<size_t>(ch)).enqueue(eff);
		break;
	case EffectType::SpeedTempoChange:
	case EffectType::Groove:
		playbackSpeedEffMem_.enqueue(eff);
		break;
	case EffectType::NoteDelay:
		effOnStepBeginMem_[SoundSource::FM].at(static_cast<size_t>(ch)).enqueue(eff);
		break;
	case EffectType::PositionJump:
	case EffectType::SongEnd:
	case EffectType::PatternBreak:
		posChangeEffMem_.enqueue(eff);
		break;
	default:
		storeDirectRegisterSetEffectToQueue(SoundSource::FM, ch, eff);
		break;
	}
}

void PlaybackManager::executeStoredEffectsFM(int ch)
{
	size_t uch = static_cast<size_t>(ch);
	bool isNoteDelay = false;

	// Read step beginning based effects
	auto& stepBeginBasedEffs = effOnStepBeginMem_[SoundSource::FM].at(uch);
	for (const auto& eff : stepBeginBasedEffs) {
		switch (eff.type) {
		case EffectType::NoteDelay:
			ntDlyCntFM_.at(uch) = eff.value;
			isNoteDelay = true;
			break;
		default:
			break;
		}
	}
	stepBeginBasedEffs.clear();

	// Read note on and step beginning based effects
	if (!isNoteDelay) {
		auto& keyOnBasedEffs = effOnKeyOnMem_[SoundSource::FM].at(uch);
		for (auto& eff : keyOnBasedEffs) {
			switch (eff.type) {
			case EffectType::Arpeggio:
				opnaCtrl_->setArpeggioEffectFM(ch, eff.value >> 4, eff.value & 0x0f);
				break;
			case EffectType::PortamentoUp:
				opnaCtrl_->setPortamentoEffectFM(ch, eff.value);
				break;
			case EffectType::PortamentoDown:
				opnaCtrl_->setPortamentoEffectFM(ch, -eff.value);
				break;
			case EffectType::TonePortamento:
				opnaCtrl_->setPortamentoEffectFM(ch, eff.value, true);
				break;
			case EffectType::Vibrato:
				opnaCtrl_->setVibratoEffectFM(ch, eff.value >> 4, eff.value & 0x0f);
				break;
			case EffectType::Tremolo:
				opnaCtrl_->setTremoloEffectFM(ch, eff.value >> 4, eff.value & 0x0f);
				break;
			case EffectType::Pan:
				if (-1 < eff.value && eff.value < 4) opnaCtrl_->setPanFM(ch, eff.value);
				break;
			case EffectType::VolumeSlide:
			{
				int hi = eff.value >> 4;
				int low = eff.value & 0x0f;
				if (hi && !low) opnaCtrl_->setVolumeSlideFM(ch, hi, true);	// Slide up
				else if (!hi) opnaCtrl_->setVolumeSlideFM(ch, low, false);	// Slide down
				break;
			}
			case EffectType::Detune:
				opnaCtrl_->setDetuneFM(ch, eff.value - 0x80);
				break;
			case EffectType::FineDetune:
				opnaCtrl_->setFineDetuneFM(ch, eff.value - 0x80);
				break;
			case EffectType::NoteSlideUp:
				opnaCtrl_->setNoteSlideFM(ch, eff.value >> 4, eff.value & 0x0f);
				break;
			case EffectType::NoteSlideDown:
				opnaCtrl_->setNoteSlideFM(ch, eff.value >> 4, -(eff.value & 0x0f));
				break;
			case EffectType::NoteRelease:
				ntReleaseDlyCntFM_.at(uch) = eff.value;
				break;
			case EffectType::TransposeDelay:
				tposeDlyCntFM_.at(uch) = (eff.value & 0x70) >> 4;
				tposeDlyValueFM_.at(uch) = ((eff.value & 0x80) ? -1 : 1) * (eff.value & 0x0f);
				break;
			case EffectType::VolumeDelay:
				volDlyCntFM_.at(uch) = eff.value >> 8;
				volDlyValueFM_.at(uch) = eff.value & 0x00ff;
				break;
			case EffectType::FBControl:
				if (-1 < eff.value && eff.value < 8) opnaCtrl_->setFBControlFM(ch, eff.value);
				break;
			case EffectType::TLControl:
			{
				int op = eff.value >> 8;
				int val = eff.value & 0x00ff;
				if (0 < op && op < 5 && -1 < val && val < 128) opnaCtrl_->setTLControlFM(ch, op - 1, val);
				break;
			}
			case EffectType::MLControl:
			{
				int op = eff.value >> 4;
				int val = eff.value & 0x0f;
				if (0 < op && op < 5 && -1 < val && val < 16) opnaCtrl_->setMLControlFM(ch, op - 1, val);
				break;
			}
			case EffectType::ARControl:
			{
				int op = eff.value >> 8;
				int val = eff.value & 0x00ff;
				if (0 < op && op < 5 && -1 < val && val < 32) opnaCtrl_->setARControlFM(ch, op - 1, val);
				break;
			}
			case EffectType::DRControl:
			{
				int op = eff.value >> 8;
				int val = eff.value & 0x00ff;
				if (0 < op && op < 5 && -1 < val && val < 32) opnaCtrl_->setDRControlFM(ch, op - 1, val);
				break;
			}
			case EffectType::RRControl:
			{
				int op = eff.value >> 4;
				int val = eff.value & 0x0f;
				if (0 < op && op < 5 && -1 < val && val < 16) opnaCtrl_->setRRControlFM(ch, op - 1, val);
				break;
			}
			case EffectType::Brightness:
			{
				if (0 < eff.value) opnaCtrl_->setBrightnessFM(ch, eff.value - 0x80);
				break;
			}
			case EffectType::NoteCut:
				ntCutDlyCntFM_.at(uch) = eff.value;
				break;
			case EffectType::Retrigger:
			{
				int cnt = eff.value & 0x0f;
				if (cnt) {
					rtrgCntFM_.at(uch) = 0;
					rtrgCntValueFM_.at(uch) = cnt;
					rtrgVolValueFM_.at(uch) = ((eff.value & 0x80) ? 1 : -1) * ((eff.value & 0x70) >> 4);	// Reverse
				}
				break;
			}
			case EffectType::XVolumeSlide:
			{
				int factor = (eff.value >> 4) - (eff.value & 0x0f);
				opnaCtrl_->setXVolumeSlideFM(ch, factor);
				break;
			}
			default:
				break;
			}
		}
		keyOnBasedEffs.clear();

		executeDirectRegisterSetEffect(directRegisterSets_[SoundSource::FM].at(uch));
	}
}

void PlaybackManager::storeEffectToMapSSG(int ch, const Effect& eff)
{
	switch (eff.type) {
	case EffectType::Arpeggio:
	case EffectType::PortamentoUp:
	case EffectType::PortamentoDown:
	case EffectType::TonePortamento:
	case EffectType::Vibrato:
	case EffectType::Tremolo:
	case EffectType::VolumeSlide:
	case EffectType::Detune:
	case EffectType::FineDetune:
	case EffectType::NoteSlideUp:
	case EffectType::NoteSlideDown:
	case EffectType::NoteRelease:
	case EffectType::TransposeDelay:
	case EffectType::ToneNoiseMix:
	case EffectType::NoisePitch:
	case EffectType::VolumeDelay:
	case EffectType::HardEnvHighPeriod:
	case EffectType::HardEnvLowPeriod:
	case EffectType::AutoEnvelope:
	case EffectType::NoteCut:
	case EffectType::Retrigger:
	case EffectType::XVolumeSlide:
		effOnKeyOnMem_[SoundSource::SSG].at(static_cast<size_t>(ch)).enqueue(eff);
		break;
	case EffectType::SpeedTempoChange:
	case EffectType::Groove:
		playbackSpeedEffMem_.enqueue(eff);
		break;
	case EffectType::NoteDelay:
		effOnStepBeginMem_[SoundSource::SSG].at(static_cast<size_t>(ch)).enqueue(eff);
		break;
	case EffectType::PositionJump:
	case EffectType::SongEnd:
	case EffectType::PatternBreak:
		posChangeEffMem_.enqueue(eff);
		break;
	default:
		storeDirectRegisterSetEffectToQueue(SoundSource::SSG, ch, eff);
		break;
	}
}

void PlaybackManager::executeStoredEffectsSSG(int ch)
{
	size_t uch = static_cast<size_t>(ch);
	bool isNoteDelay = false;

	// Read step beginning based effects
	auto& stepBeginBasedEffs = effOnStepBeginMem_[SoundSource::SSG].at(uch);
	for (const auto& eff : stepBeginBasedEffs) {
		switch (eff.type) {
		case EffectType::NoteDelay:
			ntDlyCntSSG_.at(uch) = eff.value;
			isNoteDelay = true;
			break;
		default:
			break;
		}
	}
	stepBeginBasedEffs.clear();

	// Read note on and step beginning based effects
	if (!isNoteDelay) {
		auto& keyOnBasedEffs = effOnKeyOnMem_[SoundSource::SSG].at(uch);
		for (const auto& eff : keyOnBasedEffs) {
			switch (eff.type) {
			case EffectType::Arpeggio:
				opnaCtrl_->setArpeggioEffectSSG(ch, eff.value >> 4, eff.value & 0x0f);
				break;
			case EffectType::PortamentoUp:
				opnaCtrl_->setPortamentoEffectSSG(ch, eff.value);
				break;
			case EffectType::PortamentoDown:
				opnaCtrl_->setPortamentoEffectSSG(ch, -eff.value);
				break;
			case EffectType::TonePortamento:
				opnaCtrl_->setPortamentoEffectSSG(ch, eff.value, true);
				break;
			case EffectType::Vibrato:
				opnaCtrl_->setVibratoEffectSSG(ch, eff.value >> 4, eff.value & 0x0f);
				break;
			case EffectType::Tremolo:
				opnaCtrl_->setTremoloEffectSSG(ch, eff.value >> 4, eff.value & 0x0f);
				break;
			case EffectType::VolumeSlide:
			{
				int hi = eff.value >> 4;
				int low = eff.value & 0x0f;
				if (hi && !low) opnaCtrl_->setVolumeSlideSSG(ch, hi, true);	// Slide up
				else if (!hi) opnaCtrl_->setVolumeSlideSSG(ch, low, false);	// Slide down
				break;
			}
			case EffectType::Detune:
				opnaCtrl_->setDetuneSSG(ch, eff.value - 0x80);
				break;
			case EffectType::FineDetune:
				opnaCtrl_->setFineDetuneSSG(ch, eff.value - 0x80);
				break;
			case EffectType::NoteSlideUp:
				opnaCtrl_->setNoteSlideSSG(ch, eff.value >> 4, eff.value & 0x0f);
				break;
			case EffectType::NoteSlideDown:
				opnaCtrl_->setNoteSlideSSG(ch, eff.value >> 4, -(eff.value & 0x0f));
				break;
			case EffectType::NoteRelease:
				ntReleaseDlyCntSSG_.at(uch) = eff.value;
				break;
			case EffectType::TransposeDelay:
				tposeDlyCntSSG_.at(uch) = (eff.value & 0x70) >> 4;
				tposeDlyValueSSG_.at(uch) = ((eff.value & 0x80) ? -1 : 1) * (eff.value & 0x0f);
				break;
			case EffectType::ToneNoiseMix:
				if (-1 < eff.value && eff.value < 4) opnaCtrl_->setToneNoiseMixSSG(ch, eff.value);
				break;
			case EffectType::NoisePitch:
				if (-1 < eff.value && eff.value < 32) opnaCtrl_->setNoisePitchSSG(ch, eff.value);
				break;
			case EffectType::HardEnvHighPeriod:
				opnaCtrl_->setHardEnvelopePeriod(ch, true, eff.value);
				break;
			case EffectType::HardEnvLowPeriod:
				opnaCtrl_->setHardEnvelopePeriod(ch, false, eff.value);
				break;
			case EffectType::VolumeDelay:
				volDlyCntSSG_.at(uch) = eff.value >> 8;
				volDlyValueSSG_.at(uch) = eff.value & 0x00ff;
				break;
			case EffectType::AutoEnvelope:
				opnaCtrl_->setAutoEnvelopeSSG(ch, (eff.value >> 4) - 8, eff.value & 0x0f);
				break;
			case EffectType::NoteCut:
				ntCutDlyCntSSG_.at(uch) = eff.value;
				break;
			case EffectType::Retrigger:
			{
				int cnt = eff.value & 0x0f;
				if (cnt) {
					rtrgCntSSG_.at(uch) = 0;
					rtrgCntValueSSG_.at(uch) = cnt;
					rtrgVolValueSSG_.at(uch) = ((eff.value & 0x80) ? -1 : 1) * ((eff.value & 0x70) >> 4);
				}
				break;
			}
			case EffectType::XVolumeSlide:
			{
				int factor = (eff.value >> 4) - (eff.value & 0x0f);
				opnaCtrl_->setXVolumeSlideSSG(ch, factor);
				break;
			}
			default:
				break;
			}
		}
		keyOnBasedEffs.clear();

		executeDirectRegisterSetEffect(directRegisterSets_[SoundSource::SSG].at(uch));
	}
}

void PlaybackManager::storeEffectToMapRhythm(int ch, const Effect& eff)
{
	switch (eff.type) {
	case EffectType::Pan:
	case EffectType::NoteRelease:
	case EffectType::MasterVolume:
	case EffectType::VolumeDelay:
	case EffectType::NoteCut:
	case EffectType::Retrigger:
		effOnKeyOnMem_[SoundSource::RHYTHM].at(static_cast<size_t>(ch)).enqueue(eff);
		break;
	case EffectType::SpeedTempoChange:
	case EffectType::Groove:
		playbackSpeedEffMem_.enqueue(eff);
		break;
	case EffectType::NoteDelay:
		effOnStepBeginMem_[SoundSource::RHYTHM].at(static_cast<size_t>(ch)).enqueue(eff);
		break;
	case EffectType::PositionJump:
	case EffectType::SongEnd:
	case EffectType::PatternBreak:
		posChangeEffMem_.enqueue(eff);
		break;
	default:
		storeDirectRegisterSetEffectToQueue(SoundSource::RHYTHM, ch, eff);
		break;
	}
}

void PlaybackManager::executeStoredEffectsRhythm(int ch)
{
	size_t uch = static_cast<size_t>(ch);
	bool isNoteDelay = false;

	// Read step beginning based effects
	auto& stepBeginBasedEffs = effOnStepBeginMem_[SoundSource::RHYTHM].at(uch);
	for (const auto& eff : stepBeginBasedEffs) {
		switch (eff.type) {
		case EffectType::NoteDelay:
			ntDlyCntRhythm_.at(uch) = eff.value;
			isNoteDelay = true;
			break;
		default:
			break;
		}
	}
	stepBeginBasedEffs.clear();

	// Read key on and step beginning based effects
	if (!isNoteDelay) {
		auto& keyOnBasedEffs = effOnKeyOnMem_[SoundSource::RHYTHM].at(uch);
		for (const auto& eff : keyOnBasedEffs) {
			switch (eff.type) {
			case EffectType::Pan:
				if (-1 < eff.value && eff.value < 4) opnaCtrl_->setPanRhythm(ch, eff.value);
				break;
			case EffectType::NoteRelease:
				ntReleaseDlyCntRhythm_.at(uch) = eff.value;
				break;
			case EffectType::MasterVolume:
				if (-1 < eff.value && eff.value < 64) opnaCtrl_->setMasterVolumeRhythm(eff.value);
				break;
			case EffectType::VolumeDelay:
			{
				int count = eff.value >> 8;
				if (count > 0) {
					volDlyCntRhythm_.at(uch) = count;
					volDlyValueRhythm_.at(uch) = eff.value & 0x00ff;
				}
				break;
			}
			case EffectType::NoteCut:
				ntCutDlyCntRhythm_.at(uch) = eff.value;
				break;
			case EffectType::Retrigger:
			{
				int cnt = eff.value & 0x0f;
				if (cnt) {
					rtrgCntRhythm_.at(uch) = 0;
					rtrgCntValueRhythm_.at(uch) = cnt;
					rtrgVolValueRhythm_.at(uch) = ((eff.value & 0x80) ? -1 : 1) * ((eff.value & 0x70) >> 4);
				}
				break;
			}
			default:
				break;
			}
		}
		keyOnBasedEffs.clear();

		executeDirectRegisterSetEffect(directRegisterSets_[SoundSource::RHYTHM].at(uch));
	}
}

void PlaybackManager::storeEffectToMapADPCM(int ch, const Effect& eff)
{
	switch (eff.type) {
	case EffectType::Arpeggio:
	case EffectType::PortamentoUp:
	case EffectType::PortamentoDown:
	case EffectType::TonePortamento:
	case EffectType::Vibrato:
	case EffectType::Tremolo:
	case EffectType::Pan:
	case EffectType::VolumeSlide:
	case EffectType::Detune:
	case EffectType::FineDetune:
	case EffectType::NoteSlideUp:
	case EffectType::NoteSlideDown:
	case EffectType::NoteRelease:
	case EffectType::TransposeDelay:
	case EffectType::VolumeDelay:
	case EffectType::NoteCut:
	case EffectType::Retrigger:
	case EffectType::XVolumeSlide:
		effOnKeyOnMem_[SoundSource::ADPCM].front().enqueue(eff);
		break;
	case EffectType::SpeedTempoChange:
	case EffectType::Groove:
		playbackSpeedEffMem_.enqueue(eff);
		break;
	case EffectType::NoteDelay:
		effOnStepBeginMem_[SoundSource::ADPCM].front().enqueue(eff);
		break;
	case EffectType::PositionJump:
	case EffectType::SongEnd:
	case EffectType::PatternBreak:
		posChangeEffMem_.enqueue(eff);
		break;
	default:
		storeDirectRegisterSetEffectToQueue(SoundSource::ADPCM, ch, eff);
		break;
	}
}

void PlaybackManager::executeStoredEffectsADPCM()
{
	bool isNoteDelay = false;

	// Read step beginning based effects
	auto& stepBeginBasedEffs = effOnStepBeginMem_[SoundSource::ADPCM].front();
	for (const auto& eff : stepBeginBasedEffs) {
		switch (eff.type) {
		case EffectType::NoteDelay:
			ntDlyCntADPCM_ = eff.value;
			isNoteDelay = true;
			break;
		default:
			break;
		}
	}
	stepBeginBasedEffs.clear();

	// Read note on and step beginning based effects
	if (!isNoteDelay) {
		auto& keyOnBasedEffs = effOnKeyOnMem_[SoundSource::ADPCM].front();
		for (const auto& eff : keyOnBasedEffs) {
			switch (eff.type) {
			case EffectType::Arpeggio:
				opnaCtrl_->setArpeggioEffectADPCM(eff.value >> 4, eff.value & 0x0f);
				break;
			case EffectType::PortamentoUp:
				opnaCtrl_->setPortamentoEffectADPCM(eff.value);
				break;
			case EffectType::PortamentoDown:
				opnaCtrl_->setPortamentoEffectADPCM(-eff.value);
				break;
			case EffectType::TonePortamento:
				opnaCtrl_->setPortamentoEffectADPCM(eff.value, true);
				break;
			case EffectType::Vibrato:
				opnaCtrl_->setVibratoEffectADPCM(eff.value >> 4, eff.value & 0x0f);
				break;
			case EffectType::Tremolo:
				opnaCtrl_->setTremoloEffectADPCM(eff.value >> 4, eff.value & 0x0f);
				break;
			case EffectType::Pan:
				if (-1 < eff.value && eff.value < 4) opnaCtrl_->setPanADPCM(eff.value);
				break;
			case EffectType::VolumeSlide:
			{
				int hi = eff.value >> 4;
				int low = eff.value & 0x0f;
				if (hi && !low) opnaCtrl_->setVolumeSlideADPCM(hi, true);	// Slide up
				else if (!hi) opnaCtrl_->setVolumeSlideADPCM(low, false);	// Slide down
				break;
			}
			case EffectType::Detune:
				opnaCtrl_->setDetuneADPCM(eff.value - 0x80);
				break;
			case EffectType::FineDetune:
				opnaCtrl_->setFineDetuneADPCM(eff.value - 0x80);
				break;
			case EffectType::NoteSlideUp:
				opnaCtrl_->setNoteSlideADPCM(eff.value >> 4, eff.value & 0x0f);
				break;
			case EffectType::NoteSlideDown:
				opnaCtrl_->setNoteSlideADPCM(eff.value >> 4, -(eff.value & 0x0f));
				break;
			case EffectType::NoteRelease:
				ntReleaseDlyCntADPCM_ = eff.value;
				break;
			case EffectType::TransposeDelay:
				tposeDlyCntADPCM_ = (eff.value & 0x70) >> 4;
				tposeDlyValueADPCM_ = ((eff.value & 0x80) ? -1 : 1) * ((eff.value & 0x70) >> 4);
				break;
			case EffectType::VolumeDelay:
				volDlyCntADPCM_ = eff.value >> 8;
				volDlyValueADPCM_ = eff.value & 0x00ff;
				break;
			case EffectType::NoteCut:
				ntCutDlyCntADPCM_ = eff.value;
				break;
			case EffectType::Retrigger:
			{
				int cnt = eff.value & 0x0f;
				if (cnt) {
					rtrgCntADPCM_ = 0;
					rtrgCntValueADPCM_ = cnt;
					rtrgVolValueADPCM_ = ((eff.value & 0x80) ? -1 : 1) * (eff.value & 0x0f);
				}
				break;
			}
			case EffectType::XVolumeSlide:
			{
				int factor = (eff.value >> 4) - (eff.value & 0x0f);
				opnaCtrl_->setXVolumeSlideADPCM(factor);
				break;
			}
			default:
				break;
			}
		}
		keyOnBasedEffs.clear();

		executeDirectRegisterSetEffect(directRegisterSets_[SoundSource::ADPCM].front());
	}
}

void PlaybackManager::storeDirectRegisterSetEffectToQueue(SoundSource src, int ch, const Effect& eff)
{
	size_t uch = static_cast<size_t>(ch);
	switch (eff.type) {
	case EffectType::RegisterAddress0:
		if (-1 < eff.value && eff.value < 0x6c) {
			directRegisterSets_.at(src).at(uch).push_back({ eff.value, 0, false });
		}
		break;
	case EffectType::RegisterAddress1:
		if (-1 < eff.value && eff.value < 0x6c) {
			directRegisterSets_.at(src).at(uch).push_back({ 0x100 | eff.value, 0, false });
		}
		break;
	case EffectType::RegisterValue:
	{
		DirectRegisterSetQueue& queue = directRegisterSets_.at(src).at(uch);
		if (!queue.empty() && -1 < eff.value) {
			RegisterUnit& unit = queue.back();
			unit.value = eff.value;
			unit.hasCompleted = true;
		}
		break;
	}
	default:
		break;
	}
}

void PlaybackManager::executeDirectRegisterSetEffect(DirectRegisterSetQueue& queue)
{
	for (const RegisterUnit& unit : queue) {
		if (unit.hasCompleted) opnaCtrl_->sendRegister(unit.address, unit.value);
	}
	queue.clear();
}

bool PlaybackManager::effPositionJump(int nextOrder)
{
	if (nextOrder < static_cast<int>(getOrderSize(curSongNum_))) {
		nextReadPos_.set(nextOrder, 0);
		return true;
	}
	return false;
}

void PlaybackManager::effSongEnd()
{
	nextReadPos_.invalidate();
}

bool PlaybackManager::effPatternBreak(int nextStep)
{
	if (playingPos_.order == static_cast<int>(getOrderSize(curSongNum_)) - 1
			&& nextStep < static_cast<int>(getPatternSizeFromOrderNumber(curSongNum_, 0))) {
		nextReadPos_.set(0, nextStep);
		return true;
	}
	else if (nextStep < static_cast<int>(getPatternSizeFromOrderNumber(curSongNum_, playingPos_.order + 1))) {
		nextReadPos_.set(playingPos_.order + 1, nextStep);
		return true;
	}
	return false;
}

void PlaybackManager::effSpeedChange(int speed)
{
	tickCounter_.lock()->setSpeed(speed ? speed : 1);
	tickCounter_.lock()->setGrooveState(GrooveState::Invalid);
}

void PlaybackManager::effTempoChange(int tempo)
{
	auto tc = tickCounter_.lock();
	if (tc->getGrooveEnabled() || tc->getTempo() != tempo) {
		tc->setTempo(tempo);
		tc->setGrooveState(GrooveState::Invalid);
	}
}

void PlaybackManager::effGrooveChange(int num)
{
	tickCounter_.lock()->setGroove(mod_.lock()->getGroove(num));
	tickCounter_.lock()->setGrooveState(GrooveState::ValidByLocal);
}

void PlaybackManager::tickProcess(int rest)
{
	if (!(playStateFlags_ & PlayStateFlag::ReadFirstStep)) return;	// When it has not read first step

	updateDelayEventCounts();

	auto& song = mod_.lock()->getSong(curSongNum_);
	for (auto& attrib : songStyle_.trackAttribs) {
		auto& curStep = song.getTrack(attrib.number)
						.getPatternFromOrderNumber(playingPos_.order).getStep(playingPos_.step);
		int ch = attrib.channelInSource;
		bool hasDoneNoteDelay = false;
		switch (attrib.source) {
		case SoundSource::FM:		hasDoneNoteDelay = checkFMDelayEventsInTick(curStep, ch);		break;
		case SoundSource::SSG:		hasDoneNoteDelay = checkSSGDelayEventsInTick(curStep, ch);		break;
		case SoundSource::RHYTHM:	hasDoneNoteDelay = checkRhythmDelayEventsInTick(curStep, ch);	break;
		case SoundSource::ADPCM:	hasDoneNoteDelay = checkADPCMDelayEventsInTick(curStep);		break;
		}

		if (rest == 1 && nextReadPos_.isValid() && attrib.source == SoundSource::FM && !isPlayingStep()) {
			// Channel envelope reset before next key on
			auto& step = song.getTrack(attrib.number)
						 .getPatternFromOrderNumber(nextReadPos_.order).getStep(nextReadPos_.step);
			bool hasNoteDelay = false;
			for (int i = 0; i < Step::N_EFFECT; ++i) {
				auto&& eff = effect_utils::validateEffect(attrib.source, step.getEffect(i));
				if (eff.type == EffectType::NoteDelay && eff.value > 0) {	// Note delay check
					opnaCtrl_->tickEvent(attrib.source, ch);
					hasNoteDelay = true;
					break;
				}
			}
			// Skip the statement below if envelope reset effect has cexecuted
			if (!hasNoteDelay && ntCutDlyCntFM_.at(static_cast<size_t>(ch))) envelopeResetEffectFM(step, ch);
		}
		else if (!hasDoneNoteDelay) {
			// Skip tick process when step process was called by note delay event.
			opnaCtrl_->tickEvent(attrib.source, ch);
		}
	}
	opnaCtrl_->updateRegisterStates();
}

bool PlaybackManager::checkFMDelayEventsInTick(const Step& step, int ch)
{
	size_t uch = static_cast<size_t>(ch);
	// Check volume delay
	if (!volDlyCntFM_.at(uch))
		opnaCtrl_->setOneshotVolumeFM(ch, volDlyValueFM_.at(uch));
	// Check note release
	if (!ntReleaseDlyCntFM_.at(uch))
		opnaCtrl_->keyOffFM(ch);
	// Check transpose delay
	if (!tposeDlyCntFM_.at(uch))
		opnaCtrl_->setTransposeEffectFM(ch, tposeDlyValueFM_.at(uch));
	// Check envelope reset delay
	if (!ntCutDlyCntFM_.at(uch))
		opnaCtrl_->resetFMChannelEnvelope(ch);
	// Check retrigger
	if (!rtrgCntFM_.at(uch))
		opnaCtrl_->retriggerKeyOnFM(ch, rtrgVolValueFM_.at(uch));
	// Check note delay and envelope reset
	return checkFMNoteDelayAndEnvelopeReset(step, ch);
}

bool PlaybackManager::checkFMNoteDelayAndEnvelopeReset(const Step& step, int ch)
{
	int cnt = ntDlyCntFM_.at(static_cast<size_t>(ch));
	if (!cnt) {
		executeFMStepEvents(step, ch, true);
		return true;
	}

	// Skip the statement below if envelope reset effect has cexecuted
	if (cnt == 1 && ntCutDlyCntFM_.at(static_cast<size_t>(ch))) {
		// Channel envelope reset before next key on
		envelopeResetEffectFM(step, ch);
	}

	return false;
}

void PlaybackManager::envelopeResetEffectFM(const Step& step, int ch)
{
	if (!(step.isEmptyNote() || step.hasKeyOff())
			&& opnaCtrl_->enableFMEnvelopeReset(ch)) {	// Key on or echo buffer access
		for (int i = 0; i < Step::N_EFFECT; ++i) {
			auto&& eff = effect_utils::validateEffect(SoundSource::FM, step.getEffect(i)); // "SoundSource::FM" is dummy
			if (eff.type == EffectType::TonePortamento) {
				if (eff.value) opnaCtrl_->tickEvent(SoundSource::FM, ch);
				else opnaCtrl_->resetFMChannelEnvelope(ch);
				return;
			}
		}
		if (opnaCtrl_->isTonePortamentoFM(ch)) opnaCtrl_->tickEvent(SoundSource::FM, ch);
		else opnaCtrl_->resetFMChannelEnvelope(ch);
	}
	else {
		opnaCtrl_->tickEvent(SoundSource::FM, ch);
	}
}

bool PlaybackManager::checkSSGDelayEventsInTick(const Step& step, int ch)
{
	size_t uch = static_cast<size_t>(ch);
	// Check volume delay
	if (!volDlyCntSSG_.at(uch))
		opnaCtrl_->setOneshotVolumeSSG(ch, volDlyValueSSG_.at(uch));
	// Check note release
	if (!ntReleaseDlyCntSSG_.at(uch))
		opnaCtrl_->keyOffSSG(ch);
	// Check note cut
	if (!ntCutDlyCntSSG_.at(uch))
		opnaCtrl_->setNoteCutSSG(ch);
	// Check transpose delay
	if (!tposeDlyCntSSG_.at(uch))
		opnaCtrl_->setTransposeEffectSSG(ch, tposeDlyValueSSG_.at(uch));
	// Check retrigger
	if (!rtrgCntSSG_.at(uch))
		opnaCtrl_->retriggerKeyOnSSG(ch, rtrgVolValueSSG_.at(uch));
	// Check note delay
	if (!ntDlyCntSSG_.at(uch)) {
		executeSSGStepEvents(step, ch, true);
		return true;
	}
	return false;
}

bool PlaybackManager::checkRhythmDelayEventsInTick(const Step& step, int ch)
{
	size_t uch = static_cast<size_t>(ch);
	// Check volume delay
	if (!volDlyCntRhythm_.at(uch))
		opnaCtrl_->setOneshotVolumeRhythm(ch, volDlyValueRhythm_.at(uch));
	// Check note release/cut
	if (!ntReleaseDlyCntRhythm_.at(uch) || !ntCutDlyCntRhythm_.at(uch))
		opnaCtrl_->setKeyOffFlagRhythm(ch);
	// Check retrigger
	if (!rtrgCntRhythm_.at(uch))
		opnaCtrl_->retriggerKeyOnFlagRhythm(ch, rtrgVolValueRhythm_.at(uch));
	// Check note delay
	if (!ntDlyCntRhythm_.at(uch)) {
		executeRhythmStepEvents(step, ch, true);
		return true;
	}
	return false;
}

bool PlaybackManager::checkADPCMDelayEventsInTick(const Step& step)
{
	// Check volume delay
	if (!volDlyCntADPCM_)
		opnaCtrl_->setOneshotVolumeADPCM(volDlyValueADPCM_);
	// Check note release
	if (!ntReleaseDlyCntADPCM_)
		opnaCtrl_->keyOffADPCM();
	// Check note cut
	if (!ntCutDlyCntADPCM_)
		opnaCtrl_->setNoteCutADPCM();
	// Check transpose delay
	if (!tposeDlyCntADPCM_)
		opnaCtrl_->setTransposeEffectADPCM(tposeDlyValueADPCM_);
	// Check retrigger
	if (!rtrgCntADPCM_)
		opnaCtrl_->retriggerKeyOnADPCM(rtrgVolValueADPCM_);
	// Check note delay
	if (!ntDlyCntADPCM_) {
		executeADPCMStepEvents(step, true);
		return true;
	}
	return false;
}

void PlaybackManager::clearEffectMaps()
{
	playbackSpeedEffMem_.clear();
	posChangeEffMem_.clear();

	for (auto& maps: effOnKeyOnMem_) {
		for (EffectMemory& mem : maps.second) mem.clear();
	}
	for (auto& maps: effOnStepBeginMem_) {
		for (EffectMemory& mem : maps.second) mem.clear();
	}
	for (auto& maps: directRegisterSets_) {
		for (DirectRegisterSetQueue& queue : maps.second) queue.clear();
	}
}

void PlaybackManager::clearDelayWithinStepCounts()
{
	std::fill(ntDlyCntFM_.begin(), ntDlyCntFM_.end(), -1);
	std::fill(ntDlyCntSSG_.begin(), ntDlyCntSSG_.end(), -1);
	std::fill(ntDlyCntRhythm_.begin(), ntDlyCntRhythm_.end(), -1);
	ntDlyCntADPCM_ = -1;

	std::fill(rtrgCntFM_.begin(), rtrgCntFM_.end(), -1);
	std::fill(rtrgCntValueFM_.begin(), rtrgCntValueFM_.end(), -1);
	std::fill(rtrgCntSSG_.begin(), rtrgCntSSG_.end(), -1);
	std::fill(rtrgCntValueSSG_.begin(), rtrgCntValueSSG_.end(), -1);
	std::fill(rtrgCntRhythm_.begin(), rtrgCntRhythm_.end(), -1);
	std::fill(rtrgCntValueRhythm_.begin(), rtrgCntValueRhythm_.end(), -1);
	rtrgCntADPCM_ = -1;
	rtrgCntValueADPCM_ = -1;
}

void PlaybackManager::clearDelayBeyondStepCounts()
{
	std::fill(ntReleaseDlyCntFM_.begin(), ntReleaseDlyCntFM_.end(), -1);
	std::fill(ntCutDlyCntFM_.begin(), ntCutDlyCntFM_.end(), -1);
	std::fill(volDlyCntFM_.begin(), volDlyCntFM_.end(), -1);
	std::fill(volDlyValueFM_.begin(), volDlyValueFM_.end(), -1);
	std::fill(tposeDlyCntFM_.begin(), tposeDlyCntFM_.end(), -1);
	std::fill(tposeDlyValueFM_.begin(), tposeDlyValueFM_.end(), 0);

	std::fill(ntReleaseDlyCntSSG_.begin(), ntReleaseDlyCntSSG_.end(), -1);
	std::fill(ntCutDlyCntSSG_.begin(), ntCutDlyCntSSG_.end(), -1);
	std::fill(volDlyCntSSG_.begin(), volDlyCntSSG_.end(), -1);
	std::fill(volDlyValueSSG_.begin(), volDlyValueSSG_.end(), -1);
	std::fill(tposeDlyCntSSG_.begin(), tposeDlyCntSSG_.end(), -1);
	std::fill(tposeDlyValueSSG_.begin(), tposeDlyValueSSG_.end(), 0);

	std::fill(ntReleaseDlyCntRhythm_.begin(), ntReleaseDlyCntRhythm_.end(), -1);
	std::fill(ntCutDlyCntRhythm_.begin(), ntCutDlyCntRhythm_.end(), -1);
	std::fill(volDlyCntRhythm_.begin(), volDlyCntRhythm_.end(), -1);
	std::fill(volDlyValueRhythm_.begin(), volDlyValueRhythm_.end(), -1);

	ntReleaseDlyCntADPCM_ = -1;
	ntCutDlyCntADPCM_ = -1;
	volDlyCntADPCM_ = -1;
	volDlyValueADPCM_ = -1;
	tposeDlyCntADPCM_ = -1;
	tposeDlyValueADPCM_ = 0;
}

void PlaybackManager::clearFMDelayBeyondStepCounts(int ch)
{
	size_t uch = static_cast<size_t>(ch);
	ntReleaseDlyCntFM_.at(uch) = -1;
	ntCutDlyCntFM_.at(uch) = -1;
	volDlyCntFM_.at(uch) = -1;
	volDlyValueFM_.at(uch) = -1;
	tposeDlyCntFM_.at(uch) = -1;
	tposeDlyValueFM_.at(uch) = 0;
}

void PlaybackManager::clearSSGDelayBeyondStepCounts(int ch)
{
	size_t uch = static_cast<size_t>(ch);
	ntReleaseDlyCntSSG_.at(uch) = -1;
	ntCutDlyCntSSG_.at(uch) = -1;
	volDlyCntSSG_.at(uch) = -1;
	volDlyValueSSG_.at(uch) = -1;
	tposeDlyCntSSG_.at(uch) = -1;
	tposeDlyValueSSG_.at(uch) = 0;
}

void PlaybackManager::clearRhythmDelayBeyondStepCounts(int ch)
{
	size_t uch = static_cast<size_t>(ch);
	ntReleaseDlyCntRhythm_.at(uch) = -1;
	ntCutDlyCntRhythm_.at(uch) = -1;
	volDlyCntRhythm_.at(uch) = -1;
	volDlyValueRhythm_.at(uch) = -1;
}

void PlaybackManager::clearADPCMDelayBeyondStepCounts()
{
	ntReleaseDlyCntADPCM_ = -1;
	ntCutDlyCntADPCM_ = -1;
	volDlyCntADPCM_ = -1;
	volDlyValueADPCM_ = -1;
	tposeDlyCntADPCM_ = -1;
	tposeDlyValueADPCM_ = 0;
}

void PlaybackManager::updateDelayEventCounts()
{
	static auto cd1 = [](int x) { return (x == -1) ? -1 : --x; };
	std::transform(ntDlyCntFM_.begin(), ntDlyCntFM_.end(), ntDlyCntFM_.begin(), cd1);
	std::transform(ntDlyCntSSG_.begin(), ntDlyCntSSG_.end(), ntDlyCntSSG_.begin(), cd1);
	std::transform(ntDlyCntRhythm_.begin(), ntDlyCntRhythm_.end(), ntDlyCntRhythm_.begin(), cd1);
	--ntDlyCntADPCM_;
	std::transform(ntReleaseDlyCntFM_.begin(), ntReleaseDlyCntFM_.end(), ntReleaseDlyCntFM_.begin(), cd1);
	std::transform(ntReleaseDlyCntSSG_.begin(), ntReleaseDlyCntSSG_.end(), ntReleaseDlyCntSSG_.begin(), cd1);
	std::transform(ntReleaseDlyCntRhythm_.begin(), ntReleaseDlyCntRhythm_.end(), ntReleaseDlyCntRhythm_.begin(), cd1);
	--ntReleaseDlyCntADPCM_;
	std::transform(volDlyCntFM_.begin(), volDlyCntFM_.end(), volDlyCntFM_.begin(), cd1);
	std::transform(volDlyCntSSG_.begin(), volDlyCntSSG_.end(), volDlyCntSSG_.begin(), cd1);
	std::transform(volDlyCntRhythm_.begin(), volDlyCntRhythm_.end(), volDlyCntRhythm_.begin(), cd1);
	--volDlyCntADPCM_;
	std::transform(tposeDlyCntFM_.begin(), tposeDlyCntFM_.end(), tposeDlyCntFM_.begin(), cd1);
	std::transform(tposeDlyCntSSG_.begin(), tposeDlyCntSSG_.end(), tposeDlyCntSSG_.begin(), cd1);
	--tposeDlyCntADPCM_;
	std::transform(ntCutDlyCntFM_.begin(), ntCutDlyCntFM_.end(), ntCutDlyCntFM_.begin(), cd1);
	std::transform(ntCutDlyCntSSG_.begin(), ntCutDlyCntSSG_.end(), ntCutDlyCntSSG_.begin(), cd1);
	std::transform(ntCutDlyCntRhythm_.begin(), ntCutDlyCntRhythm_.end(), ntCutDlyCntRhythm_.begin(), cd1);
	--ntCutDlyCntADPCM_;

	static auto cd2 = [](int& cnt, int max) {
		if (cnt != -1) {
			if (cnt) --cnt;
			else cnt = max - 1;
		}
	};
	for (size_t uch = 0; uch < rtrgCntFM_.size(); ++uch) cd2(rtrgCntFM_.at(uch), rtrgCntValueFM_.at(uch));
	for (size_t uch = 0; uch < rtrgCntSSG_.size(); ++uch) cd2(rtrgCntSSG_.at(uch), rtrgCntValueSSG_.at(uch));
	for (size_t uch = 0; uch < rtrgCntRhythm_.size(); ++uch) cd2(rtrgCntRhythm_.at(uch), rtrgCntValueRhythm_.at(uch));
	cd2(rtrgCntADPCM_, rtrgCntValueADPCM_);
}

void PlaybackManager::checkPlayPosition(int maxStepSize)
{
	if (isPlaySong() && playingPos_.step >= maxStepSize) {
		playingPos_.step = maxStepSize - 1;
		findNextStep();
	}
}

void PlaybackManager::setChannelRetrieving(bool enabled)
{
	isRetrieveChannel_ = enabled;
}

void PlaybackManager::retrieveChannelStates()
{
	size_t fmch = Song::getFMChannelCount(songStyle_.type);

	// Skip echo buffer due to takes time to search
	std::vector<bool> isSetInstFM(fmch, false), isSetVolFM(fmch, false), isSetArpFM(fmch, false);
	std::vector<bool> isSetPrtFM(fmch, false), isSetVibFM(fmch, false), isSetTreFM(fmch, false);
	std::vector<bool> isSetPanFM(fmch, false), isSetVolSldFM(fmch, false), isSetDtnFM(fmch, false);
	std::vector<bool> isSetFBCtrlFM(fmch, false), isSetTLCtrlFM(fmch, false), isSetMLCtrlFM(fmch, false);
	std::vector<bool> isSetARCtrlFM(fmch, false), isSetDRCtrlFM(fmch, false), isSetRRCtrlFM(fmch, false);
	std::vector<bool> isSetBrightFM(fmch, false), isSetFiDtnFM(fmch, false), isSetXVolSldFM(fmch, false);
	std::vector<bool> isSetInstSSG(3, false), isSetVolSSG(3, false), isSetArpSSG(3, false), isSetPrtSSG(3, false);
	std::vector<bool> isSetVibSSG(3, false), isSetTreSSG(3, false), isSetVolSldSSG(3, false), isSetDtnSSG(3, false);
	std::vector<bool> isSetTNMixSSG(3, false), isSetFiDtnSSG(3, false), isSetXVolSldSSG(3, false);
	std::vector<bool> isSetVolRhythm(6, false), isSetPanRhythm(6, false);
	bool isSetInstADPCM(false), isSetVolADPCM(false), isSetArpADPCM(false), isSetPrtADPCM(false);
	bool isSetVibADPCM(false), isSetTreADPCM(false), isSetPanADPCM(false), isSetVolSldADPCM(false);
	bool isSetDtnADPCM(false), isSetFiDtnADPCM(false), isSetXVolSldADPCM(false);
	bool isSetMVolRhythm = false;
	bool isSetNoisePitchSSG = false;
	bool isSetHardEnvPeriodHighSSG = false;
	bool isSetHardEnvPeriodLowSSG = false;
	bool isSetAutoEnvSSG = false;
	/// bit0: step
	/// bit1: tempo
	/// bit2: groove
	uint8_t speedStates = 0;

	Position pos = playingPos_;
	bool isPrevPos = false;
	Song& song = mod_.lock()->getSong(curSongNum_);

	while (true) {
		for (auto it = songStyle_.trackAttribs.rbegin(), e = songStyle_.trackAttribs.rend(); it != e; ++it) {
			Step& step = song.getTrack(it->number).getPatternFromOrderNumber(pos.order).getStep(pos.step);
			int ch = it->channelInSource;
			size_t uch = static_cast<size_t>(ch);

			switch (it->source) {
			case SoundSource::FM:
			{
				// Volume
				int vol = step.getVolume();
				if (!isSetVolFM[uch] && step.hasVolume() && vol < bt_defs::NSTEP_FM_VOLUME) {
					isSetVolFM[uch] = true;
					if (isPrevPos)
						opnaCtrl_->setVolumeFM(ch, vol);
				}
				// Instrument
				if (!isSetInstFM[uch] && step.hasInstrument()) {
					if (auto inst = std::dynamic_pointer_cast<InstrumentFM>(
								instMan_.lock()->getInstrumentSharedPtr(step.getInstrumentNumber()))) {
						isSetInstFM[uch] = true;
						if (isPrevPos)
							opnaCtrl_->setInstrumentFM(ch, inst);
					}
				}
				// Effects
				for (int i = Step::N_EFFECT - 1; i > -1; --i) {
					Effect eff = effect_utils::validateEffect(SoundSource::FM, step.getEffect(i));
					switch (eff.type) {
					case EffectType::Arpeggio:
						if (!isSetArpFM[uch]) {
							isSetArpFM[uch] = true;
							if (isPrevPos) opnaCtrl_->setArpeggioEffectFM(ch, eff.value >> 4, eff.value & 0x0f);
						}
						break;
					case EffectType::PortamentoUp:
						if (!isSetPrtFM[uch]) {
							isSetPrtFM[uch] = true;
							if (isPrevPos) opnaCtrl_->setPortamentoEffectFM(ch, eff.value);
						}
						break;
					case EffectType::PortamentoDown:
						if (!isSetPrtFM[uch]) {
							isSetPrtFM[uch] = true;
							if (isPrevPos) opnaCtrl_->setPortamentoEffectFM(ch, -eff.value);
						}
						break;
					case EffectType::TonePortamento:
						if (!isSetPrtFM[uch]) {
							isSetPrtFM[uch] = true;
							if (isPrevPos) opnaCtrl_->setPortamentoEffectFM(ch, eff.value, true);
						}
						break;
					case EffectType::Vibrato:
						if (!isSetVibFM[uch]) {
							isSetVibFM[uch] = true;
							if (isPrevPos) opnaCtrl_->setVibratoEffectFM(ch, eff.value >> 4, eff.value & 0x0f);
						}
						break;
					case EffectType::Tremolo:
						if (!isSetTreFM[uch]) {
							isSetTreFM[uch] = true;
							if (isPrevPos) opnaCtrl_->setTremoloEffectFM(ch, eff.value >> 4, eff.value & 0x0f);
						}
						break;
					case EffectType::Pan:
						if (-1 < eff.value && eff.value < 4 && !isSetPanFM[uch]) {
							isSetPanFM[uch] = true;
							if (isPrevPos) opnaCtrl_->setPanFM(ch, eff.value);
						}
						break;
					case EffectType::VolumeSlide:
						if (!isSetVolSldFM[uch]) {
							isSetVolSldFM[uch] = true;
							if (isPrevPos) {
								int hi = eff.value >> 4;
								int low = eff.value & 0x0f;
								if (hi && !low) opnaCtrl_->setVolumeSlideFM(ch, hi, true);	// Slide up
								else if (!hi) opnaCtrl_->setVolumeSlideFM(ch, low, false);	// Slide down
							}
						}
						break;
					case EffectType::SpeedTempoChange:
						if (!(speedStates & 0x4)) {
							if (eff.value < 0x20) {				// Speed change
								if (!(speedStates & 0x1)) {
									speedStates |= 0x1;
									if (isPrevPos) effSpeedChange(eff.value);
								}
							}
							else if (!(speedStates & 0x2)) {	// Tempo change
								speedStates |= 0x2;
								if (isPrevPos) effTempoChange(eff.value);
							}
						}
						break;
					case EffectType::Groove:
						if (eff.value < static_cast<int>(mod_.lock()->getGrooveCount()) && !speedStates) {
							speedStates |= 0x4;
							if (isPrevPos) effGrooveChange(eff.value);
						}
						break;
					case EffectType::Detune:
						if (!isSetDtnFM[uch]) {
							isSetDtnFM[uch] = true;
							if (isPrevPos) opnaCtrl_->setDetuneFM(ch, eff.value - 0x80);
						}
						break;
					case EffectType::FineDetune:
						if (!isSetFiDtnFM[uch]) {
							isSetFiDtnFM[uch] = true;
							if (isPrevPos) opnaCtrl_->setFineDetuneFM(ch, eff.value - 0x80);
						}
						break;
					case EffectType::FBControl:
						if (!isSetFBCtrlFM[uch]) {
							isSetFBCtrlFM[uch] = true;
							if (isPrevPos) {
								if (-1 < eff.value && eff.value < 8) opnaCtrl_->setFBControlFM(ch, eff.value);
							}
						}
						break;
					case EffectType::TLControl:
						if (!isSetTLCtrlFM[uch]) {
							isSetTLCtrlFM[uch] = true;
							if (isPrevPos) {
								int op = eff.value >> 8;
								int val = eff.value & 0x00ff;
								if (0 < op && op < 5 && -1 < val && val < 128)
									opnaCtrl_->setTLControlFM(ch, op - 1, val);
							}
						}
						break;
					case EffectType::MLControl:
						if (!isSetMLCtrlFM[uch]) {
							isSetMLCtrlFM[uch] = true;
							if (isPrevPos) {
								int op = eff.value >> 4;
								int val = eff.value & 0x0f;
								if (0 < op && op < 5 && -1 < val && val < 16)
									opnaCtrl_->setMLControlFM(ch, op - 1, val);
							}
						}
						break;
					case EffectType::ARControl:
						if (!isSetARCtrlFM[uch]) {
							isSetARCtrlFM[uch] = true;
							if (isPrevPos) {
								int op = eff.value >> 8;
								int val = eff.value & 0x00ff;
								if (0 < op && op < 5 && -1 < val && val < 32)
									opnaCtrl_->setARControlFM(ch, op - 1, val);
							}
						}
						break;
					case EffectType::DRControl:
						if (!isSetDRCtrlFM[uch]) {
							isSetDRCtrlFM[uch] = true;
							if (isPrevPos) {
								int op = eff.value >> 8;
								int val = eff.value & 0x00ff;
								if (0 < op && op < 5 && -1 < val && val < 32)
									opnaCtrl_->setDRControlFM(ch, op - 1, val);
							}
						}
						break;
					case EffectType::RRControl:
						if (!isSetRRCtrlFM[uch]) {
							isSetRRCtrlFM[uch] = true;
							if (isPrevPos) {
								int op = eff.value >> 4;
								int val = eff.value & 0x0f;
								if (0 < op && op < 5 && -1 < val && val < 16)
									opnaCtrl_->setRRControlFM(ch, op - 1, val);
							}
						}
						break;
					case EffectType::Brightness:
						if (!isSetBrightFM[uch]) {
							isSetBrightFM[uch] = true;
							if (isPrevPos) {
								if (0 < eff.value) opnaCtrl_->setBrightnessFM(ch, eff.value - 0x80);
							}
						}
						break;
					case EffectType::XVolumeSlide:
						if (!isSetXVolSldFM[uch]) {
							isSetXVolSldFM[uch] = true;
							if (isPrevPos) {
								int factor = (eff.value >> 4) - (eff.value & 0x0f);
								opnaCtrl_->setXVolumeSlideFM(ch, factor);
							}
						}
						break;
					default:
						break;
					}
				}
				break;
			}
			case SoundSource::SSG:
			{
				// Volume
				int vol = step.getVolume();
				if (!isSetVolSSG[uch] && step.hasVolume() && vol < bt_defs::NSTEP_SSG_VOLUME) {
					isSetVolSSG[uch] = true;
					if (isPrevPos)
						opnaCtrl_->setVolumeSSG(ch, vol);
				}
				// Instrument
				if (!isSetInstSSG[uch] && step.hasInstrument()) {
					if (auto inst = std::dynamic_pointer_cast<InstrumentSSG>(
								instMan_.lock()->getInstrumentSharedPtr(step.getInstrumentNumber()))) {
						isSetInstSSG[uch] = true;
						if (isPrevPos)
							opnaCtrl_->setInstrumentSSG(ch, inst);
					}
				}
				// Effects
				for (int i = Step::N_EFFECT - 1; i > -1; --i) {
					Effect eff = effect_utils::validateEffect(SoundSource::SSG, step.getEffect(i));
					switch (eff.type) {
					case EffectType::Arpeggio:
						if (!isSetArpSSG[uch]) {
							isSetArpSSG[uch] = true;
							if (isPrevPos) opnaCtrl_->setArpeggioEffectSSG(ch, eff.value >> 4, eff.value & 0x0f);
						}
						break;
					case EffectType::PortamentoUp:
						if (!isSetPrtSSG[uch]) {
							isSetPrtSSG[uch] = true;
							if (isPrevPos) opnaCtrl_->setPortamentoEffectSSG(ch, eff.value);
						}
						break;
					case EffectType::PortamentoDown:
						if (!isSetPrtSSG[uch]) {
							isSetPrtSSG[uch] = true;
							if (isPrevPos) opnaCtrl_->setPortamentoEffectSSG(ch, -eff.value);
						}
						break;
					case EffectType::TonePortamento:
						if (!isSetPrtSSG[uch]) {
							isSetPrtSSG[uch] = true;
							if (isPrevPos) opnaCtrl_->setPortamentoEffectSSG(ch, eff.value, true);
						}
						break;
					case EffectType::Vibrato:
						if (!isSetVibSSG[uch]) {
							isSetVibSSG[uch] = true;
							if (isPrevPos) opnaCtrl_->setVibratoEffectSSG(ch, eff.value >> 4, eff.value & 0x0f);
						}
						break;
					case EffectType::Tremolo:
						if (!isSetTreSSG[uch]) {
							isSetTreSSG[uch] = true;
							if (isPrevPos) opnaCtrl_->setTremoloEffectSSG(ch, eff.value >> 4, eff.value & 0x0f);
						}
						break;
					case EffectType::VolumeSlide:
						if (!isSetVolSldSSG[uch]) {
							isSetVolSldSSG[uch] = true;
							if (isPrevPos) {
								int hi = eff.value >> 4;
								int low = eff.value & 0x0f;
								if (hi && !low) opnaCtrl_->setVolumeSlideSSG(ch, hi, true);	// Slide up
								else if (!hi) opnaCtrl_->setVolumeSlideSSG(ch, low, false);	// Slide down
							}
						}
						break;
					case EffectType::SpeedTempoChange:
						if (!(speedStates & 0x4)) {
							if (eff.value < 0x20) {				// Speed change
								if (!(speedStates & 0x1)) {
									speedStates |= 0x1;
									if (isPrevPos) effSpeedChange(eff.value);
								}
							}
							else if (!(speedStates & 0x2)) {	// Tempo change
								speedStates |= 0x2;
								if (isPrevPos) effTempoChange(eff.value);
							}
						}
						break;
					case EffectType::Groove:
						if (eff.value < static_cast<int>(mod_.lock()->getGrooveCount()) && !speedStates) {
							speedStates |= 0x4;
							if (isPrevPos) effGrooveChange(eff.value);
						}
						break;
					case EffectType::Detune:
						if (!isSetDtnSSG[uch]) {
							isSetDtnSSG[uch] = true;
							if (isPrevPos) opnaCtrl_->setDetuneSSG(ch, eff.value - 0x80);
						}
						break;
					case EffectType::FineDetune:
						if (!isSetFiDtnSSG[uch]) {
							isSetFiDtnSSG[uch] = true;
							if (isPrevPos) opnaCtrl_->setFineDetuneSSG(ch, eff.value - 0x80);
						}
						break;
					case EffectType::ToneNoiseMix:
						if (-1 < eff.value && eff.value < 4 && !isSetTNMixSSG[uch]) {
							isSetTNMixSSG[uch] = true;
							if (isPrevPos) opnaCtrl_->setToneNoiseMixSSG(ch, eff.value);
						}
						break;
					case EffectType::NoisePitch:
						if (-1 < eff.value && eff.value < 32 && !isSetNoisePitchSSG) {
							isSetNoisePitchSSG = true;
							if (isPrevPos) opnaCtrl_->setNoisePitchSSG(ch, eff.value);
						}
						break;
					case EffectType::HardEnvHighPeriod:
						if (!isSetHardEnvPeriodHighSSG) {
							isSetHardEnvPeriodHighSSG = true;
							if (isPrevPos) opnaCtrl_->setHardEnvelopePeriod(ch, true, eff.value);
						}
						break;
					case EffectType::HardEnvLowPeriod:
						if (!isSetHardEnvPeriodLowSSG) {
							isSetHardEnvPeriodLowSSG = true;
							if (isPrevPos) opnaCtrl_->setHardEnvelopePeriod(ch, false, eff.value);
						}
						break;
					case EffectType::AutoEnvelope:
						if (!isSetAutoEnvSSG) {
							isSetAutoEnvSSG = true;
							if (isPrevPos) opnaCtrl_->setAutoEnvelopeSSG(ch, (eff.value >> 4) - 8, eff.value & 0x0f);
						}
						break;
					case EffectType::XVolumeSlide:
						if (!isSetXVolSldSSG[uch]) {
							isSetXVolSldSSG[uch] = true;
							if (isPrevPos) {
								int factor = (eff.value >> 4) - (eff.value & 0x0f);
								opnaCtrl_->setXVolumeSlideSSG(ch, factor);
							}
						}
						break;
					default:
						break;
					}
				}
				break;
			}
			case SoundSource::RHYTHM:
			{
				// Volume
				int vol = step.getVolume();
				if (!isSetVolRhythm[uch] && step.hasVolume() && vol < bt_defs::NSTEP_RHYTHM_VOLUME) {
					isSetVolRhythm[uch] = true;
					if (isPrevPos)
						opnaCtrl_->setVolumeRhythm(ch, vol);
				}
				// Effects
				for (int i = Step::N_EFFECT - 1; i > -1; --i) {
					Effect eff = effect_utils::validateEffect(SoundSource::RHYTHM, step.getEffect(i));
					switch (eff.type) {
					case EffectType::Pan:
						if (-1 < eff.value && eff.value < 4 && !isSetPanRhythm[uch]) {
							isSetPanRhythm[uch] = true;
							if (isPrevPos) opnaCtrl_->setPanRhythm(ch, eff.value);
						}
						break;
					case EffectType::SpeedTempoChange:
						if (!(speedStates & 0x4)) {
							if (eff.value < 0x20) {				// Speed change
								if (!(speedStates & 0x1)) {
									speedStates |= 0x1;
									if (isPrevPos) effSpeedChange(eff.value);
								}
							}
							else if (!(speedStates & 0x2)) {	// Tempo change
								speedStates |= 0x2;
								if (isPrevPos) effTempoChange(eff.value);
							}
						}
						break;
					case EffectType::Groove:
						if (eff.value < static_cast<int>(mod_.lock()->getGrooveCount()) && !speedStates) {
							speedStates |= 0x4;
							if (isPrevPos) effGrooveChange(eff.value);
						}
						break;
					case EffectType::MasterVolume:
						if (-1 < eff.value && eff.value < 64 && !isSetMVolRhythm) {
							isSetMVolRhythm = true;
							if (isPrevPos) opnaCtrl_->setMasterVolumeRhythm(eff.value);
						}
						break;
					default:
						break;
					}
				}
				break;
			}
			case SoundSource::ADPCM:
			{
				// Volume
				int vol = step.getVolume();
				if (!isSetVolADPCM && step.hasVolume() && vol < bt_defs::NSTEP_ADPCM_VOLUME) {
					isSetVolADPCM = true;
					if (isPrevPos)
						opnaCtrl_->setVolumeADPCM(vol);
				}
				// Instrument
				if (!isSetInstADPCM && step.hasInstrument()) {
					if (auto inst = std::dynamic_pointer_cast<InstrumentADPCM>(
								instMan_.lock()->getInstrumentSharedPtr(step.getInstrumentNumber()))) {
						isSetInstADPCM = true;
						if (isPrevPos)
							opnaCtrl_->setInstrumentADPCM(inst);
					}
				}
				// Effects
				for (int i = Step::N_EFFECT - 1; i > -1; --i) {
					Effect eff = effect_utils::validateEffect(SoundSource::ADPCM, step.getEffect(i));
					switch (eff.type) {
					case EffectType::Arpeggio:
						if (!isSetArpADPCM) {
							isSetArpADPCM = true;
							if (isPrevPos) opnaCtrl_->setArpeggioEffectADPCM(eff.value >> 4, eff.value & 0x0f);
						}
						break;
					case EffectType::PortamentoUp:
						if (!isSetPrtADPCM) {
							isSetPrtADPCM = true;
							if (isPrevPos) opnaCtrl_->setPortamentoEffectADPCM(eff.value);
						}
						break;
					case EffectType::PortamentoDown:
						if (!isSetPrtADPCM) {
							isSetPrtADPCM = true;
							if (isPrevPos) opnaCtrl_->setPortamentoEffectADPCM(-eff.value);
						}
						break;
					case EffectType::TonePortamento:
						if (!isSetPrtADPCM) {
							isSetPrtADPCM = true;
							if (isPrevPos) opnaCtrl_->setPortamentoEffectADPCM(eff.value, true);
						}
						break;
					case EffectType::Vibrato:
						if (!isSetVibADPCM) {
							isSetVibADPCM = true;
							if (isPrevPos) opnaCtrl_->setVibratoEffectADPCM(eff.value >> 4, eff.value & 0x0f);
						}
						break;
					case EffectType::Tremolo:
						if (!isSetTreADPCM) {
							isSetTreADPCM = true;
							if (isPrevPos) opnaCtrl_->setTremoloEffectADPCM(eff.value >> 4, eff.value & 0x0f);
						}
						break;
					case EffectType::Pan:
						if (-1 < eff.value && eff.value < 4 && !isSetPanADPCM) {
							isSetPanADPCM = true;
							if (isPrevPos) opnaCtrl_->setPanADPCM(eff.value);
						}
						break;
					case EffectType::VolumeSlide:
						if (!isSetVolSldADPCM) {
							isSetVolSldADPCM = true;
							if (isPrevPos) {
								int hi = eff.value >> 4;
								int low = eff.value & 0x0f;
								if (hi && !low) opnaCtrl_->setVolumeSlideADPCM(hi, true);	// Slide up
								else if (!hi) opnaCtrl_->setVolumeSlideADPCM(low, false);	// Slide down
							}
						}
						break;
					case EffectType::SpeedTempoChange:
						if (!(speedStates & 0x4)) {
							if (eff.value < 0x20) {				// Speed change
								if (!(speedStates & 0x1)) {
									speedStates |= 0x1;
									if (isPrevPos) effSpeedChange(eff.value);
								}
							}
							else if (!(speedStates & 0x2)) {	// Tempo change
								speedStates |= 0x2;
								if (isPrevPos) effTempoChange(eff.value);
							}
						}
						break;
					case EffectType::Groove:
						if (eff.value < static_cast<int>(mod_.lock()->getGrooveCount()) && !speedStates) {
							speedStates |= 0x4;
							if (isPrevPos) effGrooveChange(eff.value);
						}
						break;
					case EffectType::Detune:
						if (!isSetDtnADPCM) {
							isSetDtnADPCM = true;
							if (isPrevPos) opnaCtrl_->setDetuneADPCM(eff.value - 0x80);
						}
						break;
					case EffectType::FineDetune:
						if (!isSetFiDtnADPCM) {
							isSetFiDtnADPCM = true;
							if (isPrevPos) opnaCtrl_->setFineDetuneADPCM(eff.value - 0x80);
						}
						break;
					case EffectType::XVolumeSlide:
						if (!isSetXVolSldADPCM) {
							isSetXVolSldADPCM = true;
							if (isPrevPos) {
								int factor = (eff.value >> 4) - (eff.value & 0x0f);
								opnaCtrl_->setXVolumeSlideADPCM(factor);
							}
						}
						break;
					default:
						break;
					}
				}
				break;
			}
			}
		}

		// Move position
		isPrevPos = true;
		if (--pos.step < 0) {
			if (--pos.order < 0) break;
			pos.step = static_cast<int>(getPatternSizeFromOrderNumber(curSongNum_, pos.order)) - 1;
		}
	}

	// Sequence reset
	for (size_t ch = 0; ch < fmch; ++ch) {
		opnaCtrl_->haltSequencesFM(static_cast<int>(ch));
	}
	for (size_t ch = 0; ch < 3; ++ch) {
		opnaCtrl_->haltSequencesSSG(static_cast<int>(ch));
	}
	opnaCtrl_->haltSequencesADPCM();
}

size_t PlaybackManager::getOrderSize(int songNum) const
{
	return mod_.lock()->getSong(songNum).getOrderSize();
}

size_t PlaybackManager::getPatternSizeFromOrderNumber(int songNum, int orderNum) const
{
	return mod_.lock()->getSong(songNum).getPatternSizeFromOrderNumber(orderNum);
}
