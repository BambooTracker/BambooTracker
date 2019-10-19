#include "playback.hpp"
#include <algorithm>
#include "song.hpp"
#include "misc.hpp"

PlaybackManager::PlaybackManager(std::shared_ptr<OPNAController> opnaCtrl,
								 std::weak_ptr<InstrumentsManager> instMan, std::weak_ptr<TickCounter> tickCounter,
								 std::weak_ptr<Module> mod,
								 bool isRetrieveChannel)
	: opnaCtrl_(opnaCtrl),
	  instMan_(instMan),
	  tickCounter_(tickCounter),
	  mod_(mod),
	  curSongNum_(0),
	  playOrderNum_(-1),
	  playStepNum_(-1),
	  playState_(0),
	  managerState_(PlaybackState::Stop),
	  isFindNextStep_(false),
	  isRetrieveChannel_(isRetrieveChannel)
{
	songStyle_ = mod.lock()->getSong(curSongNum_).getStyle();

	clearEffectQueues();
	clearNoteDelayCounts();
	clearDelayBeyondStepCounts();
}

void PlaybackManager::setSong(std::weak_ptr<Module> mod, int songNum)
{
	mod_ = mod;
	curSongNum_ = songNum;
	songStyle_ = mod_.lock()->getSong(curSongNum_).getStyle();

	/* opna mode is changed in BambooTracker class */

	size_t fmch = getFMChannelCount(songStyle_.type);
	keyOnBasedEffsFM_ = std::vector<std::vector<Effect>>(fmch);
	stepBeginBasedEffsFM_ = std::vector<std::vector<Effect>>(fmch);
	stepEndBasedEffsFM_ = std::vector<std::vector<Effect>>(fmch);
	ntDlyCntFM_ = std::vector<int>(fmch);
	ntCutDlyCntFM_ = std::vector<int>(fmch);
	volDlyCntFM_ = std::vector<int>(fmch);
	volDlyValueFM_ = std::vector<int>(fmch, -1);
	tposeDlyCntFM_ = std::vector<int>(fmch);
	tposeDlyValueFM_ = std::vector<int>(fmch);

	keyOnBasedEffsSSG_ = std::vector<std::vector<Effect>>(3);
	stepBeginBasedEffsSSG_ = std::vector<std::vector<Effect>>(3);
	stepEndBasedEffsSSG_ = std::vector<std::vector<Effect>>(3);
	ntDlyCntSSG_ = std::vector<int>(3);
	ntCutDlyCntSSG_ = std::vector<int>(3);
	volDlyCntSSG_ = std::vector<int>(3);
	volDlyValueSSG_ = std::vector<int>(3, -1);
	tposeDlyCntSSG_ = std::vector<int>(3);
	tposeDlyValueSSG_ = std::vector<int>(3);

	keyOnBasedEffsDrum_ = std::vector<std::vector<Effect>>(6);
	stepBeginBasedEffsDrum_ = std::vector<std::vector<Effect>>(6);
	stepEndBasedEffsDrum_ = std::vector<std::vector<Effect>>(6);
	ntDlyCntDrum_ = std::vector<int>(6);
	ntCutDlyCntDrum_ = std::vector<int>(6);
	volDlyCntDrum_ = std::vector<int>(6);
	volDlyValueDrum_ = std::vector<int>(6, -1);
}

/********** Play song **********/
void PlaybackManager::startPlaySong(int order)
{
	std::lock_guard<std::mutex> lock(mutex_);

	startPlay();
	playState_ = 0x01;
	managerState_ = PlaybackState::PlaySong;
	playStepNum_ = 0;
	playOrderNum_ = order;
	findNextStep();
	if (isRetrieveChannel_) retrieveChannelStates();
}

void PlaybackManager::startPlayFromStart()
{
	std::lock_guard<std::mutex> lock(mutex_);

	startPlay();
	playState_ = 0x01;
	managerState_ = PlaybackState::PlayFromStart;
	playOrderNum_ = 0;
	playStepNum_ = 0;
	findNextStep();
}

void PlaybackManager::startPlayPattern(int order)
{
	std::lock_guard<std::mutex> lock(mutex_);

	startPlay();
	playState_ = 0x11;
	managerState_ = PlaybackState::PlayPattern;
	playStepNum_ = 0;
	playOrderNum_ = order;
	findNextStep();
	if (isRetrieveChannel_) retrieveChannelStates();
}

void PlaybackManager::startPlayFromCurrentStep(int order, int step)
{
	std::lock_guard<std::mutex> lock(mutex_);

	startPlay();
	playState_ = 0x01;
	managerState_ = PlaybackState::PlayFromCurrentStep;
	playOrderNum_ = order;
	playStepNum_ = step;
	findNextStep();
	if (isRetrieveChannel_) retrieveChannelStates();
}

void PlaybackManager::startPlay()
{
	opnaCtrl_->reset();

	Song& song = mod_.lock()->getSong(curSongNum_);
	tickCounter_.lock()->setTempo(song.getTempo());
	tickCounter_.lock()->setSpeed(song.getSpeed());
	tickCounter_.lock()->setGroove(mod_.lock()->getGroove(song.getGroove()).getSequence());
	tickCounter_.lock()->setGrooveEnebled(!song.isUsedTempo());
	tickCounter_.lock()->resetCount();
	tickCounter_.lock()->setPlayState(true);

	clearEffectQueues();
	clearNoteDelayCounts();
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
	playState_ = 0;
	managerState_ = PlaybackState::Stop;
	playOrderNum_ = -1;
	playStepNum_ = -1;
}

bool PlaybackManager::isPlaySong() const
{
	return ((playState_ & 0x01) > 0);
}

PlaybackState PlaybackManager::getPlaybackState() const
{
	return managerState_;
}

int PlaybackManager::getPlayingOrderNumber() const
{
	return playOrderNum_;
}

int PlaybackManager::getPlayingStepNumber() const
{
	return playStepNum_;
}

/********** Stream events **********/
int PlaybackManager::streamCountUp()
{
	std::lock_guard<std::mutex> lock(mutex_);

	int state = tickCounter_.lock()->countUp();

	if (state > 0) {
		readTick(state);
	}
	else if (!state) {
		if (stepDown()) {
			readStep();
			if (!isFindNextStep_) findNextStep();
		}
		else {
			stopPlay();
		}
	}
	else {
		for (auto& attrib : songStyle_.trackAttribs) {
			opnaCtrl_->tickEvent(attrib.source, attrib.channelInSource);
		}
	}

	return state;
}

bool PlaybackManager::stepDown()
{
	if (playState_ & 0x02) {	// Foward current step
		if (nextReadOrder_ == -1) {
			isFindNextStep_ = false;
			return false;
		}
		else {
			playOrderNum_ = nextReadOrder_;
			playStepNum_ = nextReadStep_;
		}
	}
	else {	// First read
		playState_ |= 0x02;
	}

	return true;
}

void PlaybackManager::findNextStep()
{
	// Init
	nextReadOrder_ = playOrderNum_;
	nextReadStep_ = playStepNum_;

	// Search
	if (nextReadStep_ >= static_cast<int>(getPatternSizeFromOrderNumber(curSongNum_, nextReadOrder_)) - 1) {
		if (!(playState_ & 0x10)) {	// Not play pattern
			if (nextReadOrder_ == static_cast<int>(getOrderSize(curSongNum_)) - 1) {
				nextReadOrder_ = 0;
			}
			else {
				++nextReadOrder_;
			}
		}
		nextReadStep_ = 0;
	}
	else {
		++nextReadStep_;
	}

	isFindNextStep_ = true;
}

/// Register update order: volume -> instrument -> effect -> key on
void PlaybackManager::readStep()
{
	bool isNextSet = false;

	clearNoteDelayCounts();
	updateDelayEventCounts();

	auto& song = mod_.lock()->getSong(curSongNum_);
	for (auto& attrib : songStyle_.trackAttribs) {
		auto& step = song.getTrack(attrib.number)
					 .getPatternFromOrderNumber(playOrderNum_).getStep(playStepNum_);
		switch (attrib.source) {
		case SoundSource::FM:	isNextSet |= readFMStep(step, attrib.channelInSource);		break;
		case SoundSource::SSG:	isNextSet |= readSSGStep(step, attrib.channelInSource);		break;
		case SoundSource::Drum:	isNextSet |= readDrumStep(step, attrib.channelInSource);	break;
		}
	}
	opnaCtrl_->updateKeyOnOffStatusDrum();

	isFindNextStep_ = isNextSet;
}

bool PlaybackManager::readFMStep(Step& step, int ch)
{
	bool isNoteDelay = false;

	keyOnBasedEffsFM_.at(static_cast<size_t>(ch)).clear();

	// Set effects to queue
	for (int i = 0; i < 4; ++i) {
		Effect&& eff = Effect::makeEffectData(SoundSource::FM, step.getEffectID(i), step.getEffectValue(i));
		isNoteDelay |= setEffectToQueueFM(ch, std::move(eff));
	}

	bool changedNextStep;
	if (isNoteDelay) {
		// Set effect
		changedNextStep = readFMEffectFromQueue(ch);
		readTickFMForNoteDelay(step, ch);
		opnaCtrl_->tickEvent(SoundSource::FM, ch);
	}
	else {
		changedNextStep = readFMEventsInStep(step, ch);
	}

	return changedNextStep;
}

bool PlaybackManager::readFMEventsInStep(Step& step, int ch, bool calledByNoteDelay)
{
	int noteNum = step.getNoteNumber();
	if (!calledByNoteDelay && noteNum != -1) clearFMDelayBeyondStepCounts(ch);	// Except no key event

	// Set volume
	int vol = step.getVolume();
	if (0 <= vol && vol < 0x80) {
		opnaCtrl_->setVolumeFM(ch, vol);
	}

	// Set instrument
	if (step.getInstrumentNumber() != -1) {
		if (auto inst = std::dynamic_pointer_cast<InstrumentFM>(
					instMan_.lock()->getInstrumentSharedPtr(step.getInstrumentNumber())))
			opnaCtrl_->setInstrumentFM(ch, inst);
	}

	// Set effect
	bool changedNextStep = readFMEffectFromQueue(ch);

	// Set key
	switch (noteNum) {
	case -1:	// None
		if (!calledByNoteDelay) {	// When this is called by note delay, tick event will be updated in readTick
			checkFMDelayEventsInTick(step, ch);
			opnaCtrl_->tickEvent(SoundSource::FM, ch);
		}
		break;
	case -2:	// Key off
		opnaCtrl_->keyOffFM(ch);
		break;
	case -3:	// Echo 0
		opnaCtrl_->keyOnFM(ch, 0);
		break;
	case -4:	// Echo 1
		opnaCtrl_->keyOnFM(ch, 1);
		break;
	case -5:	// Echo 2
		opnaCtrl_->keyOnFM(ch, 2);
		break;
	case -6:	// Echo 3
		opnaCtrl_->keyOnFM(ch, 3);
		break;
	default:	// Key on
	{
		std::pair<int, Note> octNote = noteNumberToOctaveAndNote(step.getNoteNumber());
		opnaCtrl_->keyOnFM(ch, octNote.second, octNote.first, 0);
		break;
	}
	}

	return changedNextStep;
}

bool PlaybackManager::readSSGStep(Step& step, int ch)
{
	bool isNoteDelay = false;

	keyOnBasedEffsSSG_.at(static_cast<size_t>(ch)).clear();

	// Set effects to queue
	for (int i = 0; i < 4; ++i) {
		Effect&& eff = Effect::makeEffectData(SoundSource::SSG, step.getEffectID(i), step.getEffectValue(i));
		isNoteDelay |= setEffectToQueueSSG(ch, std::move(eff));
	}

	bool changedNextStep;
	if (isNoteDelay) {
		// Set effect
		changedNextStep = readSSGEffectFromQueue(ch);
		opnaCtrl_->tickEvent(SoundSource::SSG, ch);
	}
	else {
		changedNextStep = readSSGEventsInStep(step, ch);
	}

	return changedNextStep;
}

bool PlaybackManager::readSSGEventsInStep(Step& step, int ch, bool calledByNoteDelay)
{
	int noteNum = step.getNoteNumber();
	if (!calledByNoteDelay && noteNum != -1) clearSSGDelayBeyondStepCounts(ch);	// Except no key event

	// Set volume
	int vol = step.getVolume();
	if (0 <= vol && vol < 0x10) {
		opnaCtrl_->setVolumeSSG(ch, vol);
	}

	// Set instrument
	if (step.getInstrumentNumber() != -1) {
		if (auto inst = std::dynamic_pointer_cast<InstrumentSSG>(
					instMan_.lock()->getInstrumentSharedPtr(step.getInstrumentNumber())))
			opnaCtrl_->setInstrumentSSG(ch, inst);
	}

	// Set effect
	bool changedNextStep = readSSGEffectFromQueue(ch);

	// Set key
	switch (noteNum) {
	case -1:	// None
		if (!calledByNoteDelay) {	// When this is called by note delay, tick event will be updated in readTick
			checkSSGDelayEventsInTick(step, ch);
			opnaCtrl_->tickEvent(SoundSource::SSG, ch);
		}
		break;
	case -2:	// Key off
		opnaCtrl_->keyOffSSG(ch);
		break;
	case -3:	// Echo 0
		opnaCtrl_->keyOnSSG(ch, 0);
		break;
	case -4:	// Echo 1
		opnaCtrl_->keyOnSSG(ch, 1);
		break;
	case -5:	// Echo 2
		opnaCtrl_->keyOnSSG(ch, 2);
		break;
	case -6:	// Echo 3
		opnaCtrl_->keyOnSSG(ch, 3);
		break;
	default:	// Key on
	{
		std::pair<int, Note> octNote = noteNumberToOctaveAndNote(step.getNoteNumber());
		opnaCtrl_->keyOnSSG(ch, octNote.second, octNote.first, 0);
		break;
	}
	}

	return changedNextStep;
}

bool PlaybackManager::readDrumStep(Step& step, int ch)
{
	bool isNoteDelay = false;

	keyOnBasedEffsDrum_.at(static_cast<size_t>(ch)).clear();

	// Set effects to queue
	for (int i = 0; i < 4; ++i) {
		Effect&& eff = Effect::makeEffectData(SoundSource::Drum, step.getEffectID(i), step.getEffectValue(i));
		isNoteDelay |= setEffectToQueueDrum(ch, std::move(eff));
	}

	bool changedNextStep;
	if (isNoteDelay) {
		// Set effect
		changedNextStep = readDrumEffectFromQueue(ch);
		opnaCtrl_->tickEvent(SoundSource::Drum, ch);
	}
	else {
		changedNextStep = readDrumEventsInStep(step, ch);
	}

	return changedNextStep;
}

bool PlaybackManager::readDrumEventsInStep(Step& step, int ch, bool calledByNoteDelay)
{
	int noteNum = step.getNoteNumber();
	if (!calledByNoteDelay && noteNum != -1) clearDrumDelayBeyondStepCounts(ch);	// Except no key event

	// Set volume
	int vol = step.getVolume();
	if (0 <= vol && vol < 0x20) {
		opnaCtrl_->setVolumeDrum(ch, vol);
	}

	// Set effect
	bool changedNextStep = readDrumEffectFromQueue(ch);

	// Set key
	switch (noteNum) {
	case -1:	// None
		if (!calledByNoteDelay) {	// When this is called by note delay, tick event will be updated in readTick
			checkDrumDelayEventsInTick(step, ch);
			opnaCtrl_->tickEvent(SoundSource::Drum, ch);
		}
		break;
	case -2:	// Key off
		opnaCtrl_->setKeyOffFlagDrum(ch);
		break;
	default:	// Key on & Echo
		opnaCtrl_->setKeyOnFlagDrum(ch);
		break;
	}

	return changedNextStep;
}

bool PlaybackManager::setEffectToQueueFM(int ch, Effect eff)
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
	case EffectType::NoteSlideUp:
	case EffectType::NoteSlideDown:
	case EffectType::NoteCut:
	case EffectType::TransposeDelay:
	case EffectType::VolumeDelay:
		keyOnBasedEffsFM_.at(static_cast<size_t>(ch)).push_back(std::move(eff));
		break;
	case EffectType::SpeedTempoChange:
	case EffectType::Groove:
		stepBeginBasedEffsFM_.at(static_cast<size_t>(ch)).push_back(std::move(eff));
		break;
	case EffectType::NoteDelay:
		if (eff.value < tickCounter_.lock()->getSpeed()) {
			stepBeginBasedEffsFM_.at(static_cast<size_t>(ch)).push_back(std::move(eff));
			return true;
		}
		return false;
	case EffectType::PositionJump:
	case EffectType::SongEnd:
	case EffectType::PatternBreak:
		stepEndBasedEffsFM_.at(static_cast<size_t>(ch)).push_back(std::move(eff));
		break;
	default:
		break;
	}

	return false;
}

bool PlaybackManager::readFMEffectFromQueue(int ch)
{
	size_t uch = static_cast<size_t>(ch);
	bool isNoteDelay = false;
	bool changedNextPos = false;

	// Read step end based effects
	for (const Effect& eff : stepEndBasedEffsFM_.at(uch)) {
		switch (eff.type) {
		case EffectType::PositionJump:
			changedNextPos |= effPositionJump(eff.value);
			break;
		case EffectType::SongEnd:
			effTrackEnd();
			changedNextPos = true;
			break;
		case EffectType::PatternBreak:
			changedNextPos |= effPatternBreak(eff.value);
			break;
		default:
			break;
		}
	}
	stepEndBasedEffsFM_.at(uch).clear();

	// Read step beginning based effects
	for (const Effect& eff : stepBeginBasedEffsFM_.at(uch)) {	// Check speed/tempo change
		if (eff.type == EffectType::SpeedTempoChange) {
			if (eff.value < 0x20) effSpeedChange(eff.value);
			else effTempoChange(eff.value);
		}
	}
	for (const Effect& eff : stepBeginBasedEffsFM_.at(uch)) {
		switch (eff.type) {
		case EffectType::NoteDelay:
			ntDlyCntFM_.at(uch) = eff.value;
			isNoteDelay = true;
			break;
		case EffectType::Groove:
			if (eff.value < static_cast<int>(mod_.lock()->getGrooveCount()))
				effGrooveChange(eff.value);
			break;
		default:
			break;
		}
	}
	stepBeginBasedEffsFM_.at(uch).clear();

	// Read note on and step beginning based effects
	if (!isNoteDelay) {
		for (const Effect& eff : keyOnBasedEffsFM_.at(uch)) {
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
			case EffectType::NoteSlideUp:
				opnaCtrl_->setNoteSlideFM(ch, eff.value >> 4, eff.value & 0x0f);
				break;
			case EffectType::NoteSlideDown:
				opnaCtrl_->setNoteSlideFM(ch, eff.value >> 4, -(eff.value & 0x0f));
				break;
			case EffectType::NoteCut:
				ntCutDlyCntFM_.at(uch) = eff.value;
				break;
			case EffectType::TransposeDelay:
				tposeDlyCntFM_.at(uch) = (eff.value & 0x70) >> 4;
				tposeDlyValueFM_.at(uch) = ((eff.value & 0x80) ? -1 : 1) * (eff.value & 0x0f);
				break;
			case EffectType::VolumeDelay:
			{
				int count = eff.value >> 8;
				if (count > 0) {
					volDlyCntFM_.at(uch) = count;
					volDlyValueFM_.at(uch) = eff.value;
				}
				break;
			}
			default:
				break;
			}
		}
		keyOnBasedEffsFM_.at(uch).clear();
	}

	return changedNextPos;
}

bool PlaybackManager::setEffectToQueueSSG(int ch, Effect eff)
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
	case EffectType::NoteSlideUp:
	case EffectType::NoteSlideDown:
	case EffectType::NoteCut:
	case EffectType::TransposeDelay:
	case EffectType::ToneNoiseMix:
	case EffectType::NoisePitch:
	case EffectType::VolumeDelay:
	case EffectType::HardEnvHighPeriod:
	case EffectType::HardEnvLowPeriod:
	case EffectType::AutoEnvelope:
		keyOnBasedEffsSSG_.at(static_cast<size_t>(ch)).push_back(std::move(eff));
		break;
	case EffectType::SpeedTempoChange:
	case EffectType::Groove:
		stepBeginBasedEffsSSG_.at(static_cast<size_t>(ch)).push_back(std::move(eff));
		break;
	case EffectType::NoteDelay:
		if (eff.value < tickCounter_.lock()->getSpeed()) {
			stepBeginBasedEffsSSG_.at(static_cast<size_t>(ch)).push_back(std::move(eff));
			return true;
		}
		return false;
	case EffectType::PositionJump:
	case EffectType::SongEnd:
	case EffectType::PatternBreak:
		stepEndBasedEffsSSG_.at(static_cast<size_t>(ch)).push_back(std::move(eff));
		break;
	default:
		break;
	}

	return false;
}

bool PlaybackManager::readSSGEffectFromQueue(int ch)
{
	size_t uch = static_cast<size_t>(ch);
	bool isNoteDelay = false;
	bool changedNextPos = false;

	// Read step end based effects
	for (const Effect& eff : stepEndBasedEffsSSG_.at(uch)) {
		switch (eff.type) {
		case EffectType::PositionJump:
			changedNextPos |= effPositionJump(eff.value);
			break;
		case EffectType::SongEnd:
			effTrackEnd();
			changedNextPos = true;
			break;
		case EffectType::PatternBreak:
			changedNextPos |= effPatternBreak(eff.value);
			break;
		default:
			break;
		}
	}
	stepEndBasedEffsSSG_.at(uch).clear();

	// Read step beginning based effects
	for (const Effect& eff : stepBeginBasedEffsSSG_.at(uch)) {	// Check speed/tempo change
		if (eff.type == EffectType::SpeedTempoChange) {
			if (eff.value < 0x20) effSpeedChange(eff.value);
			else effTempoChange(eff.value);
		}
	}
	for (const Effect& eff : stepBeginBasedEffsSSG_.at(uch)) {
		switch (eff.type) {
		case EffectType::NoteDelay:
			ntDlyCntSSG_.at(uch) = eff.value;
			isNoteDelay = true;
			break;
		case EffectType::Groove:
			if (eff.value < static_cast<int>(mod_.lock()->getGrooveCount()))
				effGrooveChange(eff.value);
			break;
		default:
			break;
		}
	}
	stepBeginBasedEffsSSG_.at(uch).clear();

	// Read note on and step beginning based effects
	if (!isNoteDelay) {
		for (const Effect& eff : keyOnBasedEffsSSG_.at(uch)) {
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
			case EffectType::NoteSlideUp:
				opnaCtrl_->setNoteSlideSSG(ch, eff.value >> 4, eff.value & 0x0f);
				break;
			case EffectType::NoteSlideDown:
				opnaCtrl_->setNoteSlideSSG(ch, eff.value >> 4, -(eff.value & 0x0f));
				break;
			case EffectType::NoteCut:
				ntCutDlyCntSSG_.at(uch) = eff.value;
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
			{
				int count = eff.value >> 8;
				if (count > 0) {
					volDlyCntSSG_.at(uch) = count;
					volDlyValueSSG_.at(uch) = eff.value;
				}
				break;
			}
			case EffectType::AutoEnvelope:
				opnaCtrl_->setAutoEnvelopeSSG(ch, (eff.value >> 4) - 8, eff.value & 0x0f);
				break;
			default:
				break;
			}
		}
		keyOnBasedEffsSSG_.at(uch).clear();
	}

	return changedNextPos;
}

bool PlaybackManager::setEffectToQueueDrum(int ch, Effect eff)
{
	switch (eff.type) {
	case EffectType::Pan:
	case EffectType::NoteCut:
	case EffectType::MasterVolume:
	case EffectType::VolumeDelay:
		keyOnBasedEffsDrum_.at(static_cast<size_t>(ch)).push_back(std::move(eff));
		break;
	case EffectType::SpeedTempoChange:
	case EffectType::Groove:
		stepBeginBasedEffsDrum_.at(static_cast<size_t>(ch)).push_back(std::move(eff));
		break;
	case EffectType::NoteDelay:
		if (eff.value < tickCounter_.lock()->getSpeed()) {
			stepBeginBasedEffsDrum_.at(static_cast<size_t>(ch)).push_back(std::move(eff));
			return true;
		}
		return false;
	case EffectType::PositionJump:
	case EffectType::SongEnd:
	case EffectType::PatternBreak:
		stepEndBasedEffsDrum_.at(static_cast<size_t>(ch)).push_back(std::move(eff));
		break;
	default:
		break;
	}

	return false;
}

bool PlaybackManager::readDrumEffectFromQueue(int ch)
{
	size_t uch = static_cast<size_t>(ch);
	bool isNoteDelay = false;
	bool changedNextPos = false;

	// Read step end based effects
	for (const Effect& eff : stepEndBasedEffsDrum_.at(uch)) {
		switch (eff.type) {
		case EffectType::PositionJump:
			changedNextPos |= effPositionJump(eff.value);
			break;
		case EffectType::SongEnd:
			effTrackEnd();
			changedNextPos = true;
			break;
		case EffectType::PatternBreak:
			changedNextPos |= effPatternBreak(eff.value);
			break;
		default:
			break;
		}
	}
	stepEndBasedEffsDrum_.at(uch).clear();

	// Read step beginning based effects
	for (const Effect& eff : stepBeginBasedEffsDrum_.at(uch)) {	// Check speed/tempo change
		if (eff.type == EffectType::SpeedTempoChange) {
			if (eff.value < 0x20) effSpeedChange(eff.value);
			else effTempoChange(eff.value);
		}
	}
	for (const Effect& eff : stepBeginBasedEffsDrum_.at(uch)) {
		switch (eff.type) {
		case EffectType::NoteDelay:
			ntDlyCntDrum_.at(uch) = eff.value;
			isNoteDelay = true;
			break;
		case EffectType::Groove:
			if (eff.value < static_cast<int>(mod_.lock()->getGrooveCount()))
				effGrooveChange(eff.value);
			break;
		default:
			break;
		}
	}
	stepBeginBasedEffsDrum_.at(uch).clear();

	// Read key on  and step beginning based effects
	if (!isNoteDelay) {
		for (const Effect& eff : keyOnBasedEffsDrum_.at(uch)) {
			switch (eff.type) {
			case EffectType::Pan:
				if (-1 < eff.value && eff.value < 4) opnaCtrl_->setPanDrum(ch, eff.value);
				break;
			case EffectType::NoteCut:
				ntCutDlyCntDrum_.at(uch) = eff.value;
				break;
			case EffectType::MasterVolume:
				if (-1 < eff.value && eff.value < 64) opnaCtrl_->setMasterVolumeDrum(eff.value);
				break;
			case EffectType::VolumeDelay:
			{
				int count = eff.value >> 8;
				if (count > 0) {
					volDlyCntDrum_.at(uch) = count;
					volDlyValueDrum_.at(uch) = eff.value;
				}
				break;
			}
			default:
				break;
			}
		}
		keyOnBasedEffsDrum_.at(uch).clear();
	}

	return changedNextPos;
}

bool PlaybackManager::effPositionJump(int nextOrder)
{
	if (nextOrder < static_cast<int>(getOrderSize(curSongNum_))) {
		nextReadOrder_ = nextOrder;
		nextReadStep_ = 0;
		return true;
	}
	return false;
}

void PlaybackManager::effTrackEnd()
{
	nextReadOrder_ = -1;
	nextReadStep_ = -1;
}

bool PlaybackManager::effPatternBreak(int nextStep)
{
	if (playOrderNum_ == static_cast<int>(getOrderSize(curSongNum_)) - 1
			&& nextStep < static_cast<int>(getPatternSizeFromOrderNumber(curSongNum_, 0))) {
		nextReadOrder_ = 0;
		nextReadStep_ = nextStep;
		return true;
	}
	else if (nextStep < static_cast<int>(getPatternSizeFromOrderNumber(curSongNum_, playOrderNum_ + 1))) {
		nextReadOrder_ = playOrderNum_ + 1;
		nextReadStep_ = nextStep;
		return true;
	}
	return false;
}

void PlaybackManager::effSpeedChange(int speed)
{
	tickCounter_.lock()->setSpeed(speed ? speed : 1);
	tickCounter_.lock()->setGrooveEnebled(false);
}

void PlaybackManager::effTempoChange(int tempo)
{
	tickCounter_.lock()->setTempo(tempo);
	tickCounter_.lock()->setGrooveEnebled(false);
}

void PlaybackManager::effGrooveChange(int num)
{
	tickCounter_.lock()->setGroove(mod_.lock()->getGroove(num).getSequence());
	tickCounter_.lock()->setGrooveEnebled(true);
}

void PlaybackManager::readTick(int rest)
{
	if (!(playState_ & 0x02)) return;	// When it has not read first step

	updateDelayEventCounts();

	auto& song = mod_.lock()->getSong(curSongNum_);
	for (auto& attrib : songStyle_.trackAttribs) {
		auto& curStep = song.getTrack(attrib.number)
						.getPatternFromOrderNumber(playOrderNum_).getStep(playStepNum_);
		int ch = attrib.channelInSource;
		switch (attrib.source) {
		case SoundSource::FM:	checkFMDelayEventsInTick(curStep, ch);		break;
		case SoundSource::SSG:	checkSSGDelayEventsInTick(curStep, ch);		break;
		case SoundSource::Drum:	checkDrumDelayEventsInTick(curStep, ch);	break;
		}

		if (rest == 1 && nextReadOrder_ != -1 && attrib.source == SoundSource::FM) {
			// Channel envelope reset before next key on
			auto& step = song.getTrack(attrib.number)
						 .getPatternFromOrderNumber(nextReadOrder_).getStep(nextReadStep_);
			for (int i = 0; i < 4; ++i) {
				auto&& eff = Effect::makeEffectData(attrib.source, step.getEffectID(i), step.getEffectValue(i));
				if (eff.type == EffectType::NoteDelay && eff.value > 0) {	// Note delay check
					opnaCtrl_->tickEvent(attrib.source, ch);
					return;
				}
			}
			envelopeResetEffectFM(step, ch);
		}
		else {
			opnaCtrl_->tickEvent(attrib.source, ch);
		}
	}
	opnaCtrl_->updateKeyOnOffStatusDrum();
}

void PlaybackManager::checkFMDelayEventsInTick(Step& step, int ch)
{
	size_t uch = static_cast<size_t>(ch);
	// Check volume delay
	if (!volDlyCntFM_.at(uch))
		opnaCtrl_->setTemporaryVolumeFM(ch, volDlyValueFM_.at(uch));
	// Check note cut
	if (!ntCutDlyCntFM_.at(uch))
		opnaCtrl_->keyOffFM(ch);
	// Check transpose delay
	if (!tposeDlyCntFM_.at(uch))
		opnaCtrl_->setTransposeEffectFM(ch, tposeDlyValueFM_.at(uch));
	// Check note delay and envelope reset
	readTickFMForNoteDelay(step, ch);
}

void PlaybackManager::readTickFMForNoteDelay(Step& step, int ch)
{
	int cnt = ntDlyCntFM_.at(static_cast<size_t>(ch));
	if (!cnt) {
		readFMEventsInStep(step, ch, true);
	}
	else if (cnt == 1) {
		// Channel envelope reset before next key on
		envelopeResetEffectFM(step, ch);
	}
}

void PlaybackManager::envelopeResetEffectFM(Step& step, int ch)
{
	int n = step.getNoteNumber();
	if ((n >= 0 || n < -2)
			&& opnaCtrl_->enableFMEnvelopeReset(ch)) {	// Key on or echo buffer access
		for (int i = 0; i < 4; ++i) {
			auto&& eff = Effect::makeEffectData(	// "SoundSource::FM" is dummy
							 SoundSource::FM, step.getEffectID(i), step.getEffectValue(i));
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

void PlaybackManager::checkSSGDelayEventsInTick(Step& step, int ch)
{
	size_t uch = static_cast<size_t>(ch);
	// Check volume delay
	if (!volDlyCntSSG_.at(uch))
		opnaCtrl_->setTemporaryVolumeSSG(ch, volDlyValueSSG_.at(uch));
	// Check note cut
	if (!ntCutDlyCntSSG_.at(uch))
		opnaCtrl_->keyOffSSG(ch);
	// Check transpose delay
	if (!tposeDlyCntSSG_.at(uch))
		opnaCtrl_->setTransposeEffectSSG(ch, tposeDlyValueSSG_.at(uch));
	// Check note delay
	if (!ntDlyCntSSG_.at(uch))
		readSSGEventsInStep(step, ch, true);
}

void PlaybackManager::checkDrumDelayEventsInTick(Step& step, int ch)
{
	size_t uch = static_cast<size_t>(ch);
	// Check volume delay
	if (!volDlyCntDrum_.at(uch))
		opnaCtrl_->setTemporaryVolumeDrum(ch, volDlyValueDrum_.at(uch));
	// Check note cut
	if (!ntCutDlyCntDrum_.at(uch))
		opnaCtrl_->setKeyOnFlagDrum(ch);
	// Check note delay
	if (!ntDlyCntDrum_.at(uch))
		readDrumEventsInStep(step, ch, true);
}

void PlaybackManager::clearEffectQueues()
{
	for (auto& queue : keyOnBasedEffsFM_) queue.clear();
	for (auto& queue : stepBeginBasedEffsFM_) queue.clear();
	for (auto& queue : stepEndBasedEffsFM_) queue.clear();

	for (auto& queue : keyOnBasedEffsSSG_) queue.clear();
	for (auto& queue : stepBeginBasedEffsSSG_) queue.clear();
	for (auto& queue : stepEndBasedEffsSSG_) queue.clear();

	for (auto& queue : keyOnBasedEffsDrum_) queue.clear();
	for (auto& queue : stepBeginBasedEffsDrum_) queue.clear();
	for (auto& queue : stepEndBasedEffsDrum_) queue.clear();
}

void PlaybackManager::clearNoteDelayCounts()
{
	std::fill(ntDlyCntFM_.begin(), ntDlyCntFM_.end(), -1);
	std::fill(ntDlyCntSSG_.begin(), ntDlyCntSSG_.end(), -1);
	std::fill(ntDlyCntDrum_.begin(), ntDlyCntDrum_.end(), -1);
}

void PlaybackManager::clearDelayBeyondStepCounts()
{
	std::fill(ntCutDlyCntFM_.begin(), ntCutDlyCntFM_.end(), -1);
	std::fill(volDlyCntFM_.begin(), volDlyCntFM_.end(), -1);
	std::fill(volDlyValueFM_.begin(), volDlyValueFM_.end(), -1);
	std::fill(tposeDlyCntFM_.begin(), tposeDlyCntFM_.end(), -1);
	std::fill(tposeDlyValueFM_.begin(), tposeDlyValueFM_.end(), 0);

	std::fill(ntCutDlyCntSSG_.begin(), ntCutDlyCntSSG_.end(), -1);
	std::fill(volDlyCntSSG_.begin(), volDlyCntSSG_.end(), -1);
	std::fill(volDlyValueSSG_.begin(), volDlyValueSSG_.end(), -1);
	std::fill(tposeDlyCntSSG_.begin(), tposeDlyCntSSG_.end(), -1);
	std::fill(tposeDlyValueSSG_.begin(), tposeDlyValueSSG_.end(), 0);

	std::fill(ntCutDlyCntDrum_.begin(), ntCutDlyCntDrum_.end(), -1);
	std::fill(volDlyCntDrum_.begin(), volDlyCntDrum_.end(), -1);
	std::fill(volDlyValueDrum_.begin(), volDlyValueDrum_.end(), -1);
}

void PlaybackManager::clearFMDelayBeyondStepCounts(int ch)
{
	size_t uch = static_cast<size_t>(ch);
	ntCutDlyCntFM_.at(uch) = -1;
	volDlyCntFM_.at(uch) = -1;
	volDlyValueFM_.at(uch) = -1;
	tposeDlyCntFM_.at(uch) = -1;
	tposeDlyValueFM_.at(uch) = 0;
}

void PlaybackManager::clearSSGDelayBeyondStepCounts(int ch)
{
	size_t uch = static_cast<size_t>(ch);
	ntCutDlyCntSSG_.at(uch) = -1;
	volDlyCntSSG_.at(uch) = -1;
	volDlyValueSSG_.at(uch) = -1;
	tposeDlyCntSSG_.at(uch) = -1;
	tposeDlyValueSSG_.at(uch) = 0;
}

void PlaybackManager::clearDrumDelayBeyondStepCounts(int ch)
{
	size_t uch = static_cast<size_t>(ch);
	ntCutDlyCntDrum_.at(uch) = -1;
	volDlyCntDrum_.at(uch) = -1;
	volDlyValueDrum_.at(uch) = -1;
}

void PlaybackManager::updateDelayEventCounts()
{
	auto f = [](int x) { return (x == -1) ? x : --x; };
	std::transform(ntDlyCntFM_.begin(), ntDlyCntFM_.end(), ntDlyCntFM_.begin(), f);
	std::transform(ntDlyCntSSG_.begin(), ntDlyCntSSG_.end(), ntDlyCntSSG_.begin(), f);
	std::transform(ntDlyCntDrum_.begin(), ntDlyCntDrum_.end(), ntDlyCntDrum_.begin(), f);
	std::transform(ntCutDlyCntFM_.begin(), ntCutDlyCntFM_.end(), ntCutDlyCntFM_.begin(), f);
	std::transform(ntCutDlyCntSSG_.begin(), ntCutDlyCntSSG_.end(), ntCutDlyCntSSG_.begin(), f);
	std::transform(ntCutDlyCntDrum_.begin(), ntCutDlyCntDrum_.end(), ntCutDlyCntDrum_.begin(), f);
	std::transform(volDlyCntFM_.begin(), volDlyCntFM_.end(), volDlyCntFM_.begin(), f);
	std::transform(volDlyCntSSG_.begin(), volDlyCntSSG_.end(), volDlyCntSSG_.begin(), f);
	std::transform(volDlyCntDrum_.begin(), volDlyCntDrum_.end(), volDlyCntDrum_.begin(), f);
	std::transform(tposeDlyCntFM_.begin(), tposeDlyCntFM_.end(), tposeDlyCntFM_.begin(), f);
	std::transform(tposeDlyCntSSG_.begin(), tposeDlyCntSSG_.end(), tposeDlyCntSSG_.begin(), f);
}

void PlaybackManager::checkPlayPosition(int maxStepSize)
{
	if (isPlaySong() && playStepNum_ >= maxStepSize) {
		playStepNum_ = maxStepSize - 1;
		findNextStep();
	}
}

void PlaybackManager::setChannelRetrieving(bool enabled)
{
	isRetrieveChannel_ = enabled;
}

void PlaybackManager::retrieveChannelStates()
{
	size_t fmch = getFMChannelCount(songStyle_.type);

	std::vector<int> tonesCntFM(fmch, 0), tonesCntSSG(3, 0);
	std::vector<std::vector<int>> toneFM(fmch), toneSSG(3);
	for (size_t i = 0; i < fmch; ++i) {
		toneFM.at(i) = std::vector<int>(3, -1);
	}
	for (size_t i = 0; i < 3; ++i) {
		toneSSG.at(i) = std::vector<int>(3, -1);
	}
	std::vector<bool> isSetInstFM(fmch, false), isSetVolFM(fmch, false), isSetArpFM(fmch, false);
	std::vector<bool> isSetPrtFM(fmch, false), isSetVibFM(fmch, false), isSetTreFM(fmch, false);
	std::vector<bool> isSetPanFM(fmch, false), isSetVolSldFM(fmch, false), isSetDtnFM(fmch, false);
	std::vector<bool> isSetInstSSG(3, false), isSetVolSSG(3, false), isSetArpSSG(3, false), isSetPrtSSG(3, false);
	std::vector<bool> isSetVibSSG(3, false), isSetTreSSG(3, false), isSetVolSldSSG(3, false), isSetDtnSSG(3, false);
	std::vector<bool> isSetTNMixSSG(3, false);
	std::vector<bool> isSetVolDrum(6, false), isSetPanDrum(6, false);
	bool isSetMVolDrum = false;
	bool isSetNoisePitchSSG = false;
	bool isSetHardEnvPeriodHighSSG = false;
	bool isSetHardEnvPeriodLowSSG = false;
	bool isSetAutoEnvSSG = false;
	/// bit0: step
	/// bit1: tempo
	/// bit2: groove
	uint8_t speedStates = 0;

	int o = playOrderNum_;
	int s = playStepNum_;
	bool isPrevPos = false;
	Song& song = mod_.lock()->getSong(curSongNum_);

	while (true) {
		for (auto it = songStyle_.trackAttribs.rbegin(), e = songStyle_.trackAttribs.rend(); it != e; ++it) {
			Step& step = song.getTrack(it->number).getPatternFromOrderNumber(o).getStep(s);
			int ch = it->channelInSource;
			size_t uch = static_cast<size_t>(ch);

			switch (it->source) {
			case SoundSource::FM:
			{
				// Effects
				for (int i = 3; i > -1; --i) {
					Effect eff = Effect::makeEffectData(SoundSource::FM, step.getEffectID(i), step.getEffectValue(i));
					switch (eff.type) {
					case EffectType::Arpeggio:
						if (!isSetArpFM.at(uch)) {
							isSetArpFM.at(uch) = true;
							if (isPrevPos) opnaCtrl_->setArpeggioEffectFM(ch, eff.value >> 4, eff.value & 0x0f);
						}
						break;
					case EffectType::PortamentoUp:
						if (!isSetPrtFM.at(uch)) {
							isSetPrtFM.at(uch) = true;
							if (isPrevPos) opnaCtrl_->setPortamentoEffectFM(ch, eff.value);
						}
						break;
					case EffectType::PortamentoDown:
						if (!isSetPrtFM.at(uch)) {
							isSetPrtFM.at(uch) = true;
							if (isPrevPos) opnaCtrl_->setPortamentoEffectFM(ch, -eff.value);
						}
						break;
					case EffectType::TonePortamento:
						if (!isSetPrtFM.at(uch)) {
							isSetPrtFM.at(uch) = true;
							if (isPrevPos) opnaCtrl_->setPortamentoEffectFM(ch, eff.value, true);
						}
						break;
					case EffectType::Vibrato:
						if (!isSetVibFM.at(uch)) {
							isSetVibFM.at(uch) = true;
							if (isPrevPos) opnaCtrl_->setVibratoEffectFM(ch, eff.value >> 4, eff.value & 0x0f);
						}
						break;
					case EffectType::Tremolo:
						if (!isSetTreFM.at(uch)) {
							isSetTreFM.at(uch) = true;
							if (isPrevPos) opnaCtrl_->setTremoloEffectFM(ch, eff.value >> 4, eff.value & 0x0f);
						}
						break;
					case EffectType::Pan:
						if (-1 < eff.value && eff.value < 4 && !isSetPanFM.at(uch)) {
							isSetPanFM.at(uch) = true;
							if (isPrevPos) opnaCtrl_->setPanFM(ch, eff.value);
						}
						break;
					case EffectType::VolumeSlide:
						if (!isSetVolSldFM.at(uch)) {
							isSetVolSldFM.at(uch) = true;
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
							if (eff.value < 0x20 && !(speedStates & 0x1)) {	// Speed change
								speedStates |= 0x1;
								if (isPrevPos) effSpeedChange(eff.value);
							}
							else if (!(speedStates & 0x2)) {			// Tempo change
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
						if (!isSetDtnFM.at(uch)) {
							isSetDtnFM.at(uch) = true;
							if (isPrevPos) opnaCtrl_->setDetuneFM(ch, eff.value - 0x80);
						}
						break;
					default:
						break;
					}
				}
				// Volume
				int vol = step.getVolume();
				if (!isSetVolFM.at(uch) && 0 <= vol && vol < 0x80) {
					isSetVolFM.at(uch) = true;
					if (isPrevPos)
						opnaCtrl_->setVolumeFM(ch, step.getVolume());
				}
				// Instrument
				if (!isSetInstFM.at(uch) && step.getInstrumentNumber() != -1) {
					if (auto inst = std::dynamic_pointer_cast<InstrumentFM>(
								instMan_.lock()->getInstrumentSharedPtr(step.getInstrumentNumber()))) {
						isSetInstFM.at(uch) = true;
						if (isPrevPos)
							opnaCtrl_->setInstrumentFM(ch, inst);
					}
				}
				// Tone
				int t = step.getNoteNumber();
				if (isPrevPos && t != -1 && t != -2) {
					--tonesCntFM.at(uch);
					for (auto it2 = toneFM.at(uch).rbegin();
						 it2 != toneFM.at(uch).rend(); ++it2) {
						if (*it2 == -1 || *it2 == tonesCntFM.at(uch)) {
							if (t >= 0) {
								*it2 = t;
							}
							else if (t < -2) {
								*it2 = tonesCntFM.at(uch) - t + 2;
							}
							break;
						}
					}
				}
				break;
			}
			case SoundSource::SSG:
			{
				// Effects
				for (int i = 3; i > -1; --i) {
					Effect eff = Effect::makeEffectData(SoundSource::SSG, step.getEffectID(i), step.getEffectValue(i));
					switch (eff.type) {
					case EffectType::Arpeggio:
						if (!isSetArpSSG.at(uch)) {
							isSetArpSSG.at(uch) = true;
							if (isPrevPos) opnaCtrl_->setArpeggioEffectSSG(ch, eff.value >> 4, eff.value & 0x0f);
						}
						break;
					case EffectType::PortamentoUp:
						if (!isSetPrtSSG.at(uch)) {
							isSetPrtSSG.at(uch) = true;
							if (isPrevPos) opnaCtrl_->setPortamentoEffectSSG(ch, eff.value);
						}
						break;
					case EffectType::PortamentoDown:
						if (!isSetPrtSSG.at(uch)) {
							isSetPrtSSG.at(uch) = true;
							if (isPrevPos) opnaCtrl_->setPortamentoEffectSSG(ch, -eff.value);
						}
						break;
					case EffectType::TonePortamento:
						if (!isSetPrtSSG.at(uch)) {
							isSetPrtSSG.at(uch) = true;
							if (isPrevPos) opnaCtrl_->setPortamentoEffectSSG(ch, eff.value, true);
						}
						break;
					case EffectType::Vibrato:
						if (!isSetVibSSG.at(uch)) {
							isSetVibSSG.at(uch) = true;
							if (isPrevPos) opnaCtrl_->setVibratoEffectSSG(ch, eff.value >> 4, eff.value & 0x0f);
						}
						break;
					case EffectType::Tremolo:
						if (!isSetTreSSG.at(uch)) {
							isSetTreSSG.at(uch) = true;
							if (isPrevPos) opnaCtrl_->setTremoloEffectSSG(ch, eff.value >> 4, eff.value & 0x0f);
						}
						break;
					case EffectType::VolumeSlide:
						if (!isSetVolSldSSG.at(uch)) {
							isSetVolSldSSG.at(uch) = true;
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
							if (eff.value < 0x20 && !(speedStates & 0x1)) {	// Speed change
								speedStates |= 0x1;
								if (isPrevPos) effSpeedChange(eff.value);
							}
							else if (!(speedStates & 0x2)) {			// Tempo change
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
						if (!isSetDtnSSG.at(uch)) {
							isSetDtnSSG.at(uch) = true;
							if (isPrevPos) opnaCtrl_->setDetuneSSG(ch, eff.value - 0x80);
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
					default:
						break;
					}
				}
				// Volume
				int vol = step.getVolume();
				if (!isSetVolSSG.at(uch) && 0 <= vol && vol < 0x10) {
					isSetVolSSG.at(uch) = true;
					if (isPrevPos)
						opnaCtrl_->setVolumeSSG(ch, vol);
				}
				// Instrument
				if (!isSetInstSSG.at(uch) && step.getInstrumentNumber() != -1) {
					if (auto inst = std::dynamic_pointer_cast<InstrumentSSG>(
								instMan_.lock()->getInstrumentSharedPtr(step.getInstrumentNumber()))) {
						isSetInstSSG.at(uch) = true;
						if (isPrevPos)
							opnaCtrl_->setInstrumentSSG(ch, inst);
					}
				}
				// Tone
				int t = step.getNoteNumber();
				if (isPrevPos && t != -1 && t != -2) {
					--tonesCntSSG.at(uch);
					for (auto it2 = toneSSG.at(uch).rbegin();
						 it2 != toneSSG.at(uch).rend(); ++it2) {
						if (*it2 == -1 || *it2 == tonesCntSSG.at(uch)) {
							if (t >= 0) {
								*it2 = t;
							}
							else if (t < -2) {
								*it2 = tonesCntSSG.at(uch) - t + 2;
							}
							break;
						}
					}
				}
				break;
			}
			case SoundSource::Drum:
			{
				// Effects
				for (int i = 3; i > -1; --i) {
					Effect eff = Effect::makeEffectData(SoundSource::Drum, step.getEffectID(i), step.getEffectValue(i));
					switch (eff.type) {
					case EffectType::Pan:
						if (-1 < eff.value && eff.value < 4 && !isSetPanDrum.at(uch)) {
							isSetPanDrum.at(uch) = true;
							if (isPrevPos) opnaCtrl_->setPanDrum(ch, eff.value);
						}
						break;
					case EffectType::SpeedTempoChange:
						if (!(speedStates & 0x4)) {
							if (eff.value < 0x20 && !(speedStates & 0x1)) {	// Speed change
								speedStates |= 0x1;
								if (isPrevPos) effSpeedChange(eff.value);
							}
							else if (!(speedStates & 0x2)) {			// Tempo change
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
						if (-1 < eff.value && eff.value < 64 && !isSetMVolDrum) {
							isSetMVolDrum = true;
							if (isPrevPos) opnaCtrl_->setMasterVolumeDrum(eff.value);
						}
						break;
					default:
						break;
					}
				}
				// Volume
				int vol = step.getVolume();
				if (!isSetVolDrum.at(uch) && 0 <= vol && vol < 0x20) {
					isSetVolDrum.at(uch) = true;
					if (isPrevPos)
						opnaCtrl_->setVolumeDrum(ch, vol);
				}
				break;
			}
			}
		}

		// Move position
		isPrevPos = true;
		if (--s < 0) {
			if (--o < 0) break;
			s = static_cast<int>(getPatternSizeFromOrderNumber(curSongNum_, o)) - 1;
		}
	}

	// Echo & sequence reset
	for (size_t ch = 0; ch < fmch; ++ch) {
		for (size_t i = 0; i < 3; ++i) {
			if (toneFM.at(ch).at(i) >= 0) {
				std::pair<int, Note> octNote = noteNumberToOctaveAndNote(toneFM[ch][i]);
				opnaCtrl_->updateEchoBufferFM(static_cast<int>(ch), octNote.first, octNote.second, 0);
			}
		}
		opnaCtrl_->haltSequencesFM(static_cast<int>(ch));
	}
	for (size_t ch = 0; ch < 3; ++ch) {
		for (size_t i = 0; i < 3; ++i) {
			if (toneSSG.at(ch).at(i) >= 0) {
				std::pair<int, Note> octNote = noteNumberToOctaveAndNote(toneSSG[ch][i]);
				opnaCtrl_->updateEchoBufferSSG(static_cast<int>(ch), octNote.first, octNote.second, 0);
			}
		}
		opnaCtrl_->haltSequencesSSG(static_cast<int>(ch));
	}
}

size_t PlaybackManager::getOrderSize(int songNum) const
{
	return mod_.lock()->getSong(songNum).getOrderSize();
}

size_t PlaybackManager::getPatternSizeFromOrderNumber(int songNum, int orderNum) const
{
	return mod_.lock()->getSong(songNum).getPatternSizeFromOrderNumber(orderNum);
}
