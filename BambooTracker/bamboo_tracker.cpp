#include "bamboo_tracker.hpp"
#include <algorithm>
#include <utility>
#include "commands.hpp"

BambooTracker::BambooTracker()
	:
	  #ifdef SINC_INTERPOLATION
	  opnaCtrl_(3993600 * 2, 44100, 40),
	  #else
	  opnaCtrl_(3993600 * 2, 44100),
	  #endif
	  mod_(std::make_shared<Module>()),
	  octave_(4),
	  curSongNum_(0),
	  curTrackNum_(0),
	  curOrderNum_(0),
	  playOrderNum_(-1),
	  curStepNum_(0),
	  playStepNum_(-1),
	  curInstNum_(-1),
	  playState_(0),
	  isFollowPlay_(true),
	  streamIntrRate_(60),	// NTSC
	  isFindNextStep_(false)
{
	songStyle_ = mod_->getSong(curSongNum_).getStyle();

	clearDelayCounts();
}

/********** Change octave **********/
void BambooTracker::setCurrentOctave(int octave)
{
	octave_ = octave;
}

int BambooTracker::getCurrentOctave() const
{
	return  octave_;
}

/********** Current track **********/
void BambooTracker::setCurrentTrack(int num)
{
	curTrackNum_ = num;
}

TrackAttribute BambooTracker::getCurrentTrackAttribute() const
{
	TrackAttribute ret = songStyle_.trackAttribs.at(curTrackNum_);
	return ret;
}

/********** Current instrument **********/
void BambooTracker::setCurrentInstrument(int n)
{
	curInstNum_ = n;
}

int BambooTracker::getCurrentInstrumentNumber() const
{
	return curInstNum_;
}

/********** Instrument edit **********/
void BambooTracker::addInstrument(int num, std::string name)
{
	comMan_.invoke(std::make_unique<AddInstrumentCommand>(
					   instMan_, num, songStyle_.trackAttribs[curTrackNum_].source, name));
}

void BambooTracker::removeInstrument(int num)
{
	comMan_.invoke(std::make_unique<RemoveInstrumentCommand>(instMan_, num));
}

std::unique_ptr<AbstractInstrument> BambooTracker::getInstrument(int num)
{
	std::shared_ptr<AbstractInstrument> inst = instMan_.getInstrumentSharedPtr(num);
	if (inst == nullptr) return std::unique_ptr<AbstractInstrument>();
	else return inst->clone();
}

void BambooTracker::cloneInstrument(int num, int refNum)
{
	comMan_.invoke(std::make_unique<cloneInstrumentCommand>(instMan_, num, refNum));
}

void BambooTracker::deepCloneInstrument(int num, int refNum)
{
	comMan_.invoke(std::make_unique<DeepCloneInstrumentCommand>(instMan_, num, refNum));
}

int BambooTracker::findFirstFreeInstrumentNumber() const
{
	return instMan_.findFirstFreeInstrument();
}

void BambooTracker::setInstrumentName(int num, std::string name)
{
	comMan_.invoke(std::make_unique<ChangeInstrumentNameCommand>(instMan_, num, name));
}

//--- FM
void BambooTracker::setEnvelopeFMParameter(int envNum, FMEnvelopeParameter param, int value)
{
	instMan_.setEnvelopeFMParameter(envNum, param, value);
	opnaCtrl_.updateInstrumentFMEnvelopeParameter(envNum, param);
}

void BambooTracker::setEnvelopeFMOperatorEnable(int envNum, int opNum, bool enable)
{
	instMan_.setEnvelopeFMOperatorEnabled(envNum, opNum, enable);
	opnaCtrl_.setInstrumentFMOperatorEnabled(envNum, opNum);
}

void BambooTracker::setInstrumentFMEnvelope(int instNum, int envNum)
{
	instMan_.setInstrumentFMEnvelope(instNum, envNum);
	opnaCtrl_.updateInstrumentFM(instNum);
}

std::vector<int> BambooTracker::getEnvelopeFMUsers(int envNum) const
{
	return instMan_.getEnvelopeFMUsers(envNum);
}

void BambooTracker::setLFOFMParameter(int lfoNum, FMLFOParameter param, int value)
{
	instMan_.setLFOFMParameter(lfoNum, param, value);
	opnaCtrl_.updateInstrumentFMLFOParameter(lfoNum, param);
}

void BambooTracker::setInstrumentFMLFO(int instNum, int lfoNum)
{
	instMan_.setInstrumentFMLFO(instNum, lfoNum);
	opnaCtrl_.updateInstrumentFM(instNum);
}

std::vector<int> BambooTracker::getLFOFMUsers(int lfoNum) const
{
	return instMan_.getLFOFMUsers(lfoNum);
}

void BambooTracker::addOperatorSequenceFMSequenceCommand(FMEnvelopeParameter param, int opSeqNum, int type, int data)
{
	instMan_.addOperatorSequenceFMSequenceCommand(param, opSeqNum, type, data);
}

void BambooTracker::removeOperatorSequenceFMSequenceCommand(FMEnvelopeParameter param, int opSeqNum)
{
	instMan_.removeOperatorSequenceFMSequenceCommand(param, opSeqNum);
}

void BambooTracker::setOperatorSequenceFMSequenceCommand(FMEnvelopeParameter param, int opSeqNum, int cnt, int type, int data)
{
	instMan_.setOperatorSequenceFMSequenceCommand(param, opSeqNum, cnt, type, data);
}

void BambooTracker::setOperatorSequenceFMLoops(FMEnvelopeParameter param, int opSeqNum, std::vector<int> begins, std::vector<int> ends, std::vector<int> times)
{
	instMan_.setOperatorSequenceFMLoops(param, opSeqNum, std::move(begins), std::move(ends), std::move(times));
}

void BambooTracker::setOperatorSequenceFMRelease(FMEnvelopeParameter param, int opSeqNum, ReleaseType type, int begin)
{
	instMan_.setOperatorSequenceFMRelease(param, opSeqNum, type, begin);
}

void BambooTracker::setInstrumentFMOperatorSequence(int instNum, FMEnvelopeParameter param, int opSeqNum)
{
	instMan_.setInstrumentFMOperatorSequence(instNum, param, opSeqNum);
	opnaCtrl_.updateInstrumentFM(instNum);
}

std::vector<int> BambooTracker::getOperatorSequenceFMUsers(FMEnvelopeParameter param, int opSeqNum) const
{
	return instMan_.getOperatorSequenceFMUsers(param, opSeqNum);
}

void BambooTracker::setArpeggioFMType(int arpNum, int type)
{
	instMan_.setArpeggioFMType(arpNum, type);
}

void BambooTracker::addArpeggioFMSequenceCommand(int arpNum, int type, int data)
{
	instMan_.addArpeggioFMSequenceCommand(arpNum, type, data);
}

void BambooTracker::removeArpeggioFMSequenceCommand(int arpNum)
{
	instMan_.removeArpeggioFMSequenceCommand(arpNum);
}

void BambooTracker::setArpeggioFMSequenceCommand(int arpNum, int cnt, int type, int data)
{
	instMan_.setArpeggioFMSequenceCommand(arpNum, cnt, type, data);
}

void BambooTracker::setArpeggioFMLoops(int arpNum, std::vector<int> begins, std::vector<int> ends, std::vector<int> times)
{
	instMan_.setArpeggioFMLoops(arpNum, std::move(begins), std::move(ends), std::move(times));
}

void BambooTracker::setArpeggioFMRelease(int arpNum, ReleaseType type, int begin)
{
	instMan_.setArpeggioFMRelease(arpNum, type, begin);
}

void BambooTracker::setInstrumentFMArpeggio(int instNum, int arpNum)
{
	instMan_.setInstrumentFMArpeggio(instNum, arpNum);
	opnaCtrl_.updateInstrumentFM(instNum);
}

std::vector<int> BambooTracker::getArpeggioFMUsers(int arpNum) const
{
	return instMan_.getArpeggioFMUsers(arpNum);
}

void BambooTracker::setPitchFMType(int ptNum, int type)
{
	instMan_.setPitchFMType(ptNum, type);
}

void BambooTracker::addPitchFMSequenceCommand(int ptNum, int type, int data)
{
	instMan_.addPitchFMSequenceCommand(ptNum, type, data);
}

void BambooTracker::removePitchFMSequenceCommand(int ptNum)
{
	instMan_.removePitchFMSequenceCommand(ptNum);
}

void BambooTracker::setPitchFMSequenceCommand(int ptNum, int cnt, int type, int data)
{
	instMan_.setPitchFMSequenceCommand(ptNum, cnt, type, data);
}

void BambooTracker::setPitchFMLoops(int ptNum, std::vector<int> begins, std::vector<int> ends, std::vector<int> times)
{
	instMan_.setPitchFMLoops(ptNum, std::move(begins), std::move(ends), std::move(times));
}

void BambooTracker::setPitchFMRelease(int ptNum, ReleaseType type, int begin)
{
	instMan_.setPitchFMRelease(ptNum, type, begin);
}

void BambooTracker::setInstrumentFMPitch(int instNum, int ptNum)
{
	instMan_.setInstrumentFMPitch(instNum, ptNum);
	opnaCtrl_.updateInstrumentFM(instNum);
}

std::vector<int> BambooTracker::getPitchFMUsers(int ptNum) const
{
	return instMan_.getPitchFMUsers(ptNum);
}

void BambooTracker::setInstrumentFMEnvelopeResetEnabled(int instNum, bool enabled)
{
	instMan_.setInstrumentFMEnvelopeResetEnabled(instNum, enabled);
	opnaCtrl_.updateInstrumentFM(instNum);
}

//--- SSG
void BambooTracker::addWaveFormSSGSequenceCommand(int wfNum, int type, int data)
{
	instMan_.addWaveFormSSGSequenceCommand(wfNum, type, data);
}

void BambooTracker::removeWaveFormSSGSequenceCommand(int wfNum)
{
	instMan_.removeWaveFormSSGSequenceCommand(wfNum);
}

void BambooTracker::setWaveFormSSGSequenceCommand(int wfNum, int cnt, int type, int data)
{
	instMan_.setWaveFormSSGSequenceCommand(wfNum, cnt, type, data);
}

void BambooTracker::setWaveFormSSGLoops(int wfNum, std::vector<int> begins, std::vector<int> ends, std::vector<int> times)
{
	instMan_.setWaveFormSSGLoops(wfNum, std::move(begins), std::move(ends), std::move(times));
}

void BambooTracker::setWaveFormSSGRelease(int wfNum, ReleaseType type, int begin)
{
	instMan_.setWaveFormSSGRelease(wfNum, type, begin);
}

void BambooTracker::setInstrumentSSGWaveForm(int instNum, int wfNum)
{
	instMan_.setInstrumentSSGWaveForm(instNum, wfNum);
	opnaCtrl_.updateInstrumentSSG(instNum);
}

std::vector<int> BambooTracker::getWaveFormSSGUsers(int wfNum) const
{
	return instMan_.getWaveFormSSGUsers(wfNum);
}

void BambooTracker::addToneNoiseSSGSequenceCommand(int tnNum, int type, int data)
{
	instMan_.addToneNoiseSSGSequenceCommand(tnNum, type, data);
}

void BambooTracker::removeToneNoiseSSGSequenceCommand(int tnNum)
{
	instMan_.removeToneNoiseSSGSequenceCommand(tnNum);
}

void BambooTracker::setToneNoiseSSGSequenceCommand(int tnNum, int cnt, int type, int data)
{
	instMan_.setToneNoiseSSGSequenceCommand(tnNum, cnt, type, data);
}

void BambooTracker::setToneNoiseSSGLoops(int tnNum, std::vector<int> begins, std::vector<int> ends, std::vector<int> times)
{
	instMan_.setToneNoiseSSGLoops(tnNum, std::move(begins), std::move(ends), std::move(times));
}

void BambooTracker::setToneNoiseSSGRelease(int tnNum, ReleaseType type, int begin)
{
	instMan_.setToneNoiseSSGRelease(tnNum, type, begin);
}

void BambooTracker::setInstrumentSSGToneNoise(int instNum, int tnNum)
{
	instMan_.setInstrumentSSGToneNoise(instNum, tnNum);
	opnaCtrl_.updateInstrumentSSG(instNum);
}

std::vector<int> BambooTracker::getToneNoiseSSGUsers(int tnNum) const
{
	return instMan_.getToneNoiseSSGUsers(tnNum);
}

void BambooTracker::addEnvelopeSSGSequenceCommand(int envNum, int type, int data)
{
	instMan_.addEnvelopeSSGSequenceCommand(envNum, type, data);
}

void BambooTracker::removeEnvelopeSSGSequenceCommand(int envNum)
{
	instMan_.removeEnvelopeSSGSequenceCommand(envNum);
}

void BambooTracker::setEnvelopeSSGSequenceCommand(int envNum, int cnt, int type, int data)
{
	instMan_.setEnvelopeSSGSequenceCommand(envNum, cnt, type, data);
}

void BambooTracker::setEnvelopeSSGLoops(int envNum, std::vector<int> begins, std::vector<int> ends, std::vector<int> times)
{
	instMan_.setEnvelopeSSGLoops(envNum, std::move(begins), std::move(ends), std::move(times));
}

void BambooTracker::setEnvelopeSSGRelease(int envNum, ReleaseType type, int begin)
{
	instMan_.setEnvelopeSSGRelease(envNum, type, begin);
}

void BambooTracker::setInstrumentSSGEnvelope(int instNum, int envNum)
{
	instMan_.setInstrumentSSGEnvelope(instNum, envNum);
	opnaCtrl_.updateInstrumentSSG(instNum);
}

std::vector<int> BambooTracker::getEnvelopeSSGUsers(int envNum) const
{
	return instMan_.getEnvelopeSSGUsers(envNum);
}

void BambooTracker::setArpeggioSSGType(int arpNum, int type)
{
	instMan_.setArpeggioSSGType(arpNum, type);
}

void BambooTracker::addArpeggioSSGSequenceCommand(int arpNum, int type, int data)
{
	instMan_.addArpeggioSSGSequenceCommand(arpNum, type, data);
}

void BambooTracker::removeArpeggioSSGSequenceCommand(int arpNum)
{
	instMan_.removeArpeggioSSGSequenceCommand(arpNum);
}

void BambooTracker::setArpeggioSSGSequenceCommand(int arpNum, int cnt, int type, int data)
{
	instMan_.setArpeggioSSGSequenceCommand(arpNum, cnt, type, data);
}

void BambooTracker::setArpeggioSSGLoops(int arpNum, std::vector<int> begins, std::vector<int> ends, std::vector<int> times)
{
	instMan_.setArpeggioSSGLoops(arpNum, std::move(begins), std::move(ends), std::move(times));
}

void BambooTracker::setArpeggioSSGRelease(int arpNum, ReleaseType type, int begin)
{
	instMan_.setArpeggioSSGRelease(arpNum, type, begin);
}

void BambooTracker::setInstrumentSSGArpeggio(int instNum, int arpNum)
{
	instMan_.setInstrumentSSGArpeggio(instNum, arpNum);
	opnaCtrl_.updateInstrumentSSG(instNum);
}

std::vector<int> BambooTracker::getArpeggioSSGUsers(int arpNum) const
{
	return instMan_.getArpeggioSSGUsers(arpNum);
}

void BambooTracker::setPitchSSGType(int ptNum, int type)
{
	instMan_.setPitchSSGType(ptNum, type);
}

void BambooTracker::addPitchSSGSequenceCommand(int ptNum, int type, int data)
{
	instMan_.addPitchSSGSequenceCommand(ptNum, type, data);
}

void BambooTracker::removePitchSSGSequenceCommand(int ptNum)
{
	instMan_.removePitchSSGSequenceCommand(ptNum);
}

void BambooTracker::setPitchSSGSequenceCommand(int ptNum, int cnt, int type, int data)
{
	instMan_.setPitchSSGSequenceCommand(ptNum, cnt, type, data);
}

void BambooTracker::setPitchSSGLoops(int ptNum, std::vector<int> begins, std::vector<int> ends, std::vector<int> times)
{
	instMan_.setPitchSSGLoops(ptNum, std::move(begins), std::move(ends), std::move(times));
}

void BambooTracker::setPitchSSGRelease(int ptNum, ReleaseType type, int begin)
{
	instMan_.setPitchSSGRelease(ptNum, type, begin);
}

void BambooTracker::setInstrumentSSGPitch(int instNum, int ptNum)
{
	instMan_.setInstrumentSSGPitch(instNum, ptNum);
	opnaCtrl_.updateInstrumentSSG(instNum);
}

std::vector<int> BambooTracker::getPitchSSGUsers(int ptNum) const
{
	return instMan_.getPitchSSGUsers(ptNum);
}

/********** Song edit **********/
int BambooTracker::getCurrentSongNumber() const
{
	return curSongNum_;
}

void BambooTracker::setCurrentSongNumber(int num)
{
	curSongNum_ = num;
	songStyle_ = mod_->getSong(num).getStyle();

	// Reset
	opnaCtrl_.reset();
	tickCounter_.resetCount();
	tickCounter_.setTempo(getSongTempo(num));
	tickCounter_.setSpeed(getSongSpeed(num));

	switch (songStyle_.type) {
	case SongType::STD:
		ntDlyCntFM_ = std::vector<int>(6);
		ntCutDlyCntFM_ = std::vector<int>(6);
		volDlyCntFM_ = std::vector<int>(6);
		volDlyValueFM_ = std::vector<int>(6, -1);
		tposeDlyCntFM_ = std::vector<int>(6);
		tposeDlyValueFM_ = std::vector<int>(6);
		break;
	case SongType::FMEX:
		// UNDONE: extend ch4
		break;
	}

	ntDlyCntSSG_ = std::vector<int>(3);
	ntCutDlyCntSSG_ = std::vector<int>(3);
	volDlyCntSSG_ = std::vector<int>(3);
	volDlyValueSSG_ = std::vector<int>(3, -1);
	tposeDlyCntSSG_ = std::vector<int>(3);
	tposeDlyValueSSG_ = std::vector<int>(3);

	ntDlyCntDrum_ = std::vector<int>(6);
	ntCutDlyCntDrum_ = std::vector<int>(6);
	volDlyCntDrum_ = std::vector<int>(6);
	volDlyValueDrum_ = std::vector<int>(6, -1);
}

/********** Order edit **********/
int BambooTracker::getCurrentOrderNumber() const
{
	return curOrderNum_;
}

void BambooTracker::setCurrentOrderNumber(int num)
{
	curOrderNum_ = num;
}

/********** Pattern edit **********/
int BambooTracker::getCurrentStepNumber() const
{
	return curStepNum_;
}

void BambooTracker::setCurrentStepNumber(int num)
{
	curStepNum_ = num;
}

/********** Undo-Redo **********/
void BambooTracker::undo()
{
	comMan_.undo();
}

void BambooTracker::redo()
{
	comMan_.redo();
}

void BambooTracker::clearCommandHistory()
{
	comMan_.clear();
}

/********** Jam mode **********/
void BambooTracker::toggleJamMode()
{
	if (jamMan_.toggleJamMode() && !isPlaySong()) {
		jamMan_.polyphonic(true, songStyle_.type);
	}
	else {
		jamMan_.polyphonic(false, songStyle_.type);
	}
}

bool BambooTracker::isJamMode() const
{
	return jamMan_.isJamMode();
}

void BambooTracker::jamKeyOn(JamKey key)
{
	if (songStyle_.trackAttribs[curTrackNum_].source == SoundSource::DRUM) {
		opnaCtrl_.keyOnDrum(songStyle_.trackAttribs[curTrackNum_].channelInSource);
	}
	else {
		std::vector<JamKeyData>&& list = jamMan_.keyOn(key,
													   songStyle_.trackAttribs[curTrackNum_].channelInSource,
													   songStyle_.trackAttribs[curTrackNum_].source);
		if (list.size() == 2) {	// Key off
			JamKeyData& offData = list[1];
			switch (offData.source) {
			case SoundSource::FM:	opnaCtrl_.keyOffFM(offData.channelInSource, true);	break;
			case SoundSource::SSG:	opnaCtrl_.keyOffSSG(offData.channelInSource, true);	break;
			default:	break;
			}
		}

		std::shared_ptr<AbstractInstrument> tmpInst = instMan_.getInstrumentSharedPtr(curInstNum_);
		JamKeyData& onData = list.front();

		switch (onData.source) {
		case SoundSource::FM:
			opnaCtrl_.setInstrumentFM(onData.channelInSource, std::dynamic_pointer_cast<InstrumentFM>(tmpInst));
			opnaCtrl_.keyOnFM(onData.channelInSource,
							  JamManager::jamKeyToNote(onData.key),
							  JamManager::calcOctave(octave_, onData.key),
							  0,
							  true);
			break;
		case SoundSource::SSG:
			opnaCtrl_.setInstrumentSSG(onData.channelInSource, std::dynamic_pointer_cast<InstrumentSSG>(tmpInst));
			opnaCtrl_.keyOnSSG(onData.channelInSource,
							   JamManager::jamKeyToNote(onData.key),
							   JamManager::calcOctave(octave_, onData.key),
							   0,
							   true);
			break;
		default:
			break;
		}
	}
}

void BambooTracker::jamKeyOff(JamKey key)
{
	if (songStyle_.trackAttribs[curTrackNum_].source == SoundSource::DRUM) {
		opnaCtrl_.keyOffDrum(songStyle_.trackAttribs[curTrackNum_].channelInSource);
	}
	else {
		JamKeyData&& data = jamMan_.keyOff(key);

		if (data.channelInSource > -1) {	// Key still sound
			switch (data.source) {
			case SoundSource::FM:
				opnaCtrl_.keyOffFM(data.channelInSource, true);
				break;
			case SoundSource::SSG:
				opnaCtrl_.keyOffSSG(data.channelInSource, true);
				break;
			default:
				break;
			}
		}
	}
}

/********** Play song **********/
void BambooTracker::startPlaySong()
{
	startPlay();
	playState_ = 0x01;
	playStepNum_ = 0;
	playOrderNum_ = curOrderNum_;
	if (isFollowPlay_) curStepNum_ = 0;
	findNextStep();
}

void BambooTracker::startPlayFromStart()
{
	startPlay();
	playState_ = 0x01;
	playOrderNum_ = 0;
	playStepNum_ = 0;
	if (isFollowPlay_) {
		curOrderNum_ = 0;
		curStepNum_ = 0;
	}
	findNextStep();
}

void BambooTracker::startPlayPattern()
{
	startPlay();
	playState_ = 0x11;
	playStepNum_ = 0;
	playOrderNum_ = curOrderNum_;
	if (isFollowPlay_) curStepNum_ = 0;
	findNextStep();
}

void BambooTracker::startPlayFromCurrentStep()
{
	startPlay();
	playState_ = 0x01;
	playOrderNum_ = curOrderNum_;
	playStepNum_ = curStepNum_;
	findNextStep();
}

void BambooTracker::startPlay()
{
	opnaCtrl_.reset();
	jamMan_.polyphonic(false, songStyle_.type);

	Song& song = mod_->getSong(curSongNum_);
	tickCounter_.setTempo(song.getTempo());
	tickCounter_.setSpeed(song.getSpeed());
	tickCounter_.setGroove(mod_->getGroove(song.getGroove()).getSequence());
	tickCounter_.setGrooveEnebled(!song.isUsedTempo());
	tickCounter_.resetCount();
	tickCounter_.setPlayState(true);

	clearDelayCounts();
}

void BambooTracker::stopPlaySong()
{
	opnaCtrl_.reset();
	jamMan_.polyphonic(true, songStyle_.type);
	tickCounter_.setPlayState(false);
	playState_ = 0;
	playOrderNum_ = -1;
	playStepNum_ = -1;
}

bool BambooTracker::isPlaySong() const
{
	return  ((playState_ & 0x01) > 0);
}

void BambooTracker::setTrackMuteState(int trackNum, bool isMute)
{
	auto& ta = songStyle_.trackAttribs[trackNum];
	switch (ta.source) {
	case SoundSource::FM:	opnaCtrl_.setMuteFMState(ta.channelInSource, isMute);	break;
	case SoundSource::SSG:	opnaCtrl_.setMuteSSGState(ta.channelInSource, isMute);	break;
	case SoundSource::DRUM:	opnaCtrl_.setMuteDrumState(ta.channelInSource, isMute);	break;
	}
}

bool BambooTracker::isMute(int trackNum)
{
	auto& ta = songStyle_.trackAttribs[trackNum];
	switch (ta.source) {
	case SoundSource::FM:	return opnaCtrl_.isMuteFM(ta.channelInSource);
	case SoundSource::SSG:	return opnaCtrl_.isMuteSSG(ta.channelInSource);
	case SoundSource::DRUM:	return opnaCtrl_.isMuteDrum(ta.channelInSource);
	}
}

void BambooTracker::setFollowPlay(bool isFollowed)
{
	isFollowPlay_ = isFollowed;
}

bool BambooTracker::isFollowPlay() const
{
	return isFollowPlay_;
}

int BambooTracker::getPlayingOrderNumber() const
{
	return playOrderNum_;
}

int BambooTracker::getPlayingStepNumber() const
{
	return playStepNum_;
}

/********** Stream events **********/
int BambooTracker::streamCountUp()
{
	int state = tickCounter_.countUp();

	if (state > 0) {
		readTick(state);
	}
	else if (!state) {
		if (stepDown()) {
			readStep();
			if (!isFindNextStep_) findNextStep();
		}
		else {
			stopPlaySong();
		}
	}
	else {
		for (auto& attrib : songStyle_.trackAttribs) {
			opnaCtrl_.tickEvent(attrib.source, attrib.channelInSource);
		}
	}

	return state;
}

void BambooTracker::readTick(int rest)
{	
	if (!(playState_ & 0x02)) return;	// When it has not read first step

	// Delay
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

	auto& song = mod_->getSong(curSongNum_);
	for (auto& attrib : songStyle_.trackAttribs) {
		auto& curStep = song.getTrack(attrib.number)
					 .getPatternFromOrderNumber(curOrderNum_).getStep(playStepNum_);
		switch (attrib.source) {
		case SoundSource::FM:
		{
			// Check volume delay
			if (!volDlyCntFM_[attrib.channelInSource])
				opnaCtrl_.setTemporaryVolumeFM(attrib.channelInSource, volDlyValueFM_[attrib.channelInSource]);
			// Check note cut
			if (!ntCutDlyCntFM_[attrib.channelInSource])
				opnaCtrl_.keyOffFM(attrib.channelInSource);
			// Check transpose delay
			if (!tposeDlyCntFM_[attrib.channelInSource])
				opnaCtrl_.setTransposeEffectFM(attrib.channelInSource, tposeDlyValueFM_[attrib.channelInSource]);
			// Check note delay and envelope reset
			readTickFMForNoteDelay(curStep, attrib.channelInSource);
			break;
		}
		case SoundSource::SSG:
		{
			// Check volume delay
			if (!volDlyCntSSG_[attrib.channelInSource])
				opnaCtrl_.setTemporaryVolumeSSG(attrib.channelInSource, volDlyValueSSG_[attrib.channelInSource]);
			// Check note cut
			if (!ntCutDlyCntSSG_[attrib.channelInSource])
				opnaCtrl_.keyOffSSG(attrib.channelInSource);
			// Check transpose delay
			if (!tposeDlyCntSSG_[attrib.channelInSource])
				opnaCtrl_.setTransposeEffectSSG(attrib.channelInSource, tposeDlyValueSSG_[attrib.channelInSource]);
			// Check note delay
			if (!ntDlyCntSSG_[attrib.channelInSource])
				readSSGStep(curStep, attrib.channelInSource, true);
			break;
		}
		case SoundSource::DRUM:
		{
			// Check volume delay
			if (!volDlyCntDrum_[attrib.channelInSource])
				opnaCtrl_.setTemporaryVolumeDrum(attrib.channelInSource, volDlyValueDrum_[attrib.channelInSource]);
			// Check note cut
			if (!ntCutDlyCntDrum_[attrib.channelInSource])
				opnaCtrl_.keyOnDrum(attrib.channelInSource);
			// Check note delay
			if (!ntDlyCntDrum_[attrib.channelInSource])
				readDrumStep(curStep, attrib.channelInSource, true);
			break;
		}
		}

		if (rest == 1 && nextReadOrder_ != -1 && attrib.source == SoundSource::FM) {
			// Channel envelope reset before next key on
			auto& step = song.getTrack(attrib.number)
						 .getPatternFromOrderNumber(nextReadOrder_).getStep(nextReadStep_);
			int n = step.checkEffectID("0G");
			if (n == -1 || !step.getEffectValue(n)) {
				envelopeResetEffectFM(step, attrib.channelInSource);
			}
			else {
				opnaCtrl_.tickEvent(attrib.source, attrib.channelInSource);
			}
		}
		else {
			opnaCtrl_.tickEvent(attrib.source, attrib.channelInSource);
		}
	}
}

void BambooTracker::readTickFMForNoteDelay(Step& step, int ch)
{
	int cnt = ntDlyCntFM_[ch];
	if (!cnt) {
		readFMStep(step, ch, true);
	}
	else if (cnt == 1) {
		// Channel envelope reset before next key on
		envelopeResetEffectFM(step, ch);
	}
}

void BambooTracker::envelopeResetEffectFM(Step& step, int ch)
{
	if (step.getNoteNumber() >= 0
			&& opnaCtrl_.enableFMEnvelopeReset(ch)) {
		int idx = step.checkEffectID("03");
		if ((idx == -1 && !opnaCtrl_.isTonePortamentoFM(ch))
				|| (idx != -1 && !step.getEffectValue(idx))) {
			opnaCtrl_.resetFMChannelEnvelope(ch);
		}
		else {
			opnaCtrl_.tickEvent(SoundSource::FM, ch);
		}
	}
	else {
		opnaCtrl_.tickEvent(SoundSource::FM, ch);
	}
}

void BambooTracker::clearDelayCounts()
{
	std::fill(ntDlyCntFM_.begin(), ntDlyCntFM_.end(), -1);
	std::fill(ntCutDlyCntFM_.begin(), ntCutDlyCntFM_.end(), -1);
	std::fill(volDlyCntFM_.begin(), volDlyCntFM_.end(), -1);
	std::fill(volDlyValueFM_.begin(), volDlyValueFM_.end(), -1);
	std::fill(tposeDlyCntFM_.begin(), tposeDlyCntFM_.end(), -1);
	std::fill(tposeDlyValueFM_.begin(), tposeDlyValueFM_.end(), 0);
	std::fill(ntDlyCntSSG_.begin(), ntDlyCntSSG_.end(), -1);
	std::fill(ntCutDlyCntSSG_.begin(), ntCutDlyCntSSG_.end(), -1);
	std::fill(volDlyCntSSG_.begin(), volDlyCntSSG_.end(), -1);
	std::fill(volDlyValueSSG_.begin(), volDlyValueSSG_.end(), -1);
	std::fill(tposeDlyCntSSG_.begin(), tposeDlyCntSSG_.end(), -1);
	std::fill(tposeDlyValueSSG_.begin(), tposeDlyValueSSG_.end(), 0);
	std::fill(ntDlyCntDrum_.begin(), ntDlyCntDrum_.end(), -1);
	std::fill(ntCutDlyCntDrum_.begin(), ntCutDlyCntDrum_.end(), -1);
	std::fill(volDlyCntDrum_.begin(), volDlyCntDrum_.end(), -1);
	std::fill(volDlyValueDrum_.begin(), volDlyValueDrum_.end(), -1);
}

void BambooTracker::findNextStep()
{
	// Init
	nextReadOrder_ = playOrderNum_;
	nextReadStep_ = playStepNum_;

	// Search
	if (nextReadStep_ == getPatternSizeFromOrderNumber(curSongNum_, nextReadOrder_) - 1) {
		if (!(playState_ & 0x10)) {	// Not play pattern
			if (nextReadOrder_ == getOrderSize(curSongNum_) - 1) {
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

bool BambooTracker::stepDown()
{
	if (playState_ & 0x02) {	// Foward current step
		if (nextReadOrder_ == -1) {
			isFindNextStep_ = false;
			return false;
		}
		else {
			playOrderNum_ = nextReadOrder_;
			playStepNum_ = nextReadStep_;
			if (isFollowPlay_) {
				curOrderNum_ = nextReadOrder_;
				curStepNum_ = nextReadStep_;
			}
		}
	}
	else {	// First read
		playState_ |= 0x02;
	}

	return true;
}

/// Read order: volume -> instrument -> effect -> key on
void BambooTracker::readStep()
{
	bool isNextSet = false;

	clearDelayCounts();

	auto& song = mod_->getSong(curSongNum_);
	for (auto& attrib : songStyle_.trackAttribs) {
		auto& step = song.getTrack(attrib.number)
					 .getPatternFromOrderNumber(playOrderNum_).getStep(playStepNum_);
		switch (attrib.source) {
		case SoundSource::FM:
		{
			int nd = step.checkEffectID("0G");
			if (nd == -1 || !step.getEffectValue(nd)) {
				isNextSet |= readFMStep(step, attrib.channelInSource);
			}
			else {		// Note delay
				ntDlyCntFM_[attrib.channelInSource] = step.getEffectValue(nd);
				for (int i = 0; i < 4; ++i)
					isNextSet |= readFMSpecialEffect(attrib.channelInSource, step.getEffectID(i), step.getEffectValue(i));
				readTickFMForNoteDelay(step, attrib.channelInSource);
			}
			break;
		}

		case SoundSource::SSG:
		{
			int nd = step.checkEffectID("0G");
			if (nd == -1 || !step.getEffectValue(nd)) {
				isNextSet |= readSSGStep(step, attrib.channelInSource);
			}
			else {		// Note delay
				ntDlyCntSSG_[attrib.channelInSource] = step.getEffectValue(nd);
				for (int i = 0; i < 4; ++i)
					isNextSet |= readSSGSpecialEffect(attrib.channelInSource, step.getEffectID(i), step.getEffectValue(i));
			}
			break;
		}
		case SoundSource::DRUM:
		{
			int nd = step.checkEffectID("0G");
			if (nd == -1 || !step.getEffectValue(nd)) {
				isNextSet |= readDrumStep(step, attrib.channelInSource);
			}
			else {		// Note delay
				ntDlyCntDrum_[attrib.channelInSource] = step.getEffectValue(nd);
				for (int i = 0; i < 4; ++i)
					isNextSet |= readDrumSpecialEffect(attrib.channelInSource, step.getEffectID(i), step.getEffectValue(i));
			}
			break;
		}
		}
	}

	isFindNextStep_ = isNextSet;
}

bool BambooTracker::readFMStep(Step& step, int ch, bool isSkippedSpecial)
{
	bool isNextSet = false;

	// Set volume
	if (step.getVolume() != -1) {
		opnaCtrl_.setVolumeFM(ch, step.getVolume());
	}
	// Set instrument
	if (step.getInstrumentNumber() != -1) {
		if (auto inst = std::dynamic_pointer_cast<InstrumentFM>(
					instMan_.getInstrumentSharedPtr(step.getInstrumentNumber())))
			opnaCtrl_.setInstrumentFM(ch, inst);
	}
	// Set effect
	for (int i = 0; i < 4; ++i) {
		if (step.getEffectID(i) != "--" && step.getEffectValue(i) != -1) {
			isNextSet |= readFMEffect(ch, step.getEffectID(i), step.getEffectValue(i), isSkippedSpecial);
		}
	}
	// Set key
	switch (step.getNoteNumber()) {
	case -1:	// None
		opnaCtrl_.tickEvent(SoundSource::FM, ch, true);
		break;
	case -2:	// Key off
		opnaCtrl_.keyOffFM(ch);
		break;
	default:	// Key on
	{
		std::pair<int, Note> octNote = noteNumberToOctaveAndNote(step.getNoteNumber());
		opnaCtrl_.keyOnFM(ch, octNote.second, octNote.first, 0);
		break;
	}
	}

	return isNextSet;
}

bool BambooTracker::readSSGStep(Step& step, int ch, bool isSkippedSpecial)
{
	bool isNextSet = false;

	// Set volume
	int vol = step.getVolume();
	if (0 <= vol && vol < 0x10) {
		opnaCtrl_.setVolumeSSG(ch, step.getVolume());
	}
	// Set instrument
	if (step.getInstrumentNumber() != -1) {
		if (auto inst = std::dynamic_pointer_cast<InstrumentSSG>(
					instMan_.getInstrumentSharedPtr(step.getInstrumentNumber())))
			opnaCtrl_.setInstrumentSSG(ch, inst);
	}
	// Set effect
	for (int i = 0; i < 4; ++i) {
		if (step.getEffectID(i) != "--" && step.getEffectValue(i) != -1) {
			isNextSet |= readSSGEffect(ch, step.getEffectID(i), step.getEffectValue(i), isSkippedSpecial);
		}
	}
	// Set key
	switch (step.getNoteNumber()) {
	case -1:	// None
		opnaCtrl_.tickEvent(SoundSource::SSG, ch, true);
		break;
	case -2:	// Key off
		opnaCtrl_.keyOffSSG(ch);
		break;
	default:	// Key on
	{
		std::pair<int, Note> octNote = noteNumberToOctaveAndNote(step.getNoteNumber());
		opnaCtrl_.keyOnSSG(ch, octNote.second, octNote.first, 0);
		break;
	}
	}

	return isNextSet;
}

bool BambooTracker::readDrumStep(Step& step, int ch, bool isSkippedSpecial)
{
	bool isNextSet = false;

	// Set volume
	int vol = step.getVolume();
	if (0 <= vol && vol < 0x20) {
		opnaCtrl_.setVolumeDrum(ch, step.getVolume());
	}
	// Set effect
	for (int i = 0; i < 4; ++i) {
		if (step.getEffectID(i) != "--" && step.getEffectValue(i) != -1) {
			isNextSet |= readDrumEffect(ch, step.getEffectID(i), step.getEffectValue(i), isSkippedSpecial);
		}
	}
	// Set key
	switch (step.getNoteNumber()) {
	case -1:	// None
		break;
	case -2:	// Key off
		opnaCtrl_.keyOffDrum(ch);
		break;
	default:	// Key on
		opnaCtrl_.keyOnDrum(ch);
		break;
	}

	return isNextSet;
}

bool BambooTracker::readFMEffect(int ch, std::string id, int value, bool isSkippedSpecial)
{
	bool ret = false;

	if (id == "00") {		// Arpeggio
		if (value != -1) opnaCtrl_.setArpeggioEffectFM(ch, value >> 4, value & 0x0f);
	}
	else if (id == "01") {	// Portamento up
		if (value != -1) opnaCtrl_.setPortamentoEffectFM(ch, value);
	}
	else if (id == "02") {	// Portamento down
		if (value != -1) opnaCtrl_.setPortamentoEffectFM(ch, -value);
	}
	else if (id == "03") {	// Tone portamento
		if (value != -1) opnaCtrl_.setPortamentoEffectFM(ch, value, true);
	}
	else if (id == "04") {	// Vibrato
		if (value != -1) opnaCtrl_.setVibratoEffectFM(ch, value >> 4, value & 0x0f);
	}
	else if (id == "07") {	// Tremolo
		if (value != -1) opnaCtrl_.setTremoloEffectFM(ch, value >> 4, value & 0x0f);
	}
	else if (id == "08") {	// Pan
		if (-1 < value && value < 4) opnaCtrl_.setPanFM(ch, value);
	}
	else if (id == "0A") {	// Volume slide
		if (value != -1) {
			int hi = value >> 4;
			int low = value & 0x0f;
			if (hi && !low) opnaCtrl_.setVolumeSlideFM(ch, hi, true);	// Slide up
			else if (!hi) opnaCtrl_.setVolumeSlideFM(ch, low, false);	// Slide down
		}
	}
	else if (id == "0F") {
		if (value != -1) {
			if (value < 0x20) {	// Speed change
				effSpeedChange(value);
			}
			else {				// Tempo change
				effTempoChange(value);
			}
		}
	}
	else if (id == "0O") {	// Groove
		if (-1 < value && value < mod_->getGrooveCount())
			effGrooveChange(value);
	}
	else if (id == "0P") {	// Detune
		if (value != -1) opnaCtrl_.setDetuneFM(ch, value - 0x80);
	}
	else if (id == "0Q") {	// Note slide up
		if (value != -1) opnaCtrl_.setNoteSlideFM(ch, value >> 4, value & 0x0f);
	}
	else if (id == "0R") {	// Note slide down
		if (value != -1) opnaCtrl_.setNoteSlideFM(ch, value >> 4, -(value & 0x0f));
	}
	else if (!isSkippedSpecial) {
		ret = readFMSpecialEffect(ch, id, value);
	}

	return ret;
}

bool BambooTracker::readSSGEffect(int ch, std::string id, int value, bool isSkippedSpecial)
{
	bool ret = false;

	if (id == "00") {		// Arpeggio
		if (value != -1) opnaCtrl_.setArpeggioEffectSSG(ch, value >> 4, value & 0x0f);
	}
	else if (id == "01") {	// Portamento up
		if (value != -1) opnaCtrl_.setPortamentoEffectSSG(ch, value);
	}
	else if (id == "02") {	// Portamento down
		if (value != -1) opnaCtrl_.setPortamentoEffectSSG(ch, -value);
	}
	else if (id == "03") {	// Tone portamento
		if (value != -1) opnaCtrl_.setPortamentoEffectSSG(ch, value, true);
	}
	else if (id == "04") {	// Vibrato
		if (value != -1) opnaCtrl_.setVibratoEffectSSG(ch, value >> 4, value & 0x0f);
	}
	else if (id == "07") {	// Tremolo
		if (value != -1) opnaCtrl_.setTremoloEffectSSG(ch, value >> 4, value & 0x0f);
	}
	else if (id == "0A") {	// Volume slide
		if (value != -1) {
			int hi = value >> 4;
			int low = value & 0x0f;
			if (hi && !low) opnaCtrl_.setVolumeSlideSSG(ch, hi, true);	// Slide up
			else if (!hi) opnaCtrl_.setVolumeSlideSSG(ch, low, false);	// Slide down
		}
	}
	else if (id == "0F") {
		if (value != -1) {
			if (value < 0x20) {	// Speed change
				effSpeedChange(value);
			}
			else {				// Tempo change
				effTempoChange(value);
			}
		}
	}
	else if (id == "0O") {	// Groove
		if (-1 < value && value < mod_->getGrooveCount())
			effGrooveChange(value);
	}
	else if (id == "0P") {	// Detune
		if (value != -1) opnaCtrl_.setDetuneSSG(ch, value - 0x80);
	}
	else if (id == "0Q") {	// Note slide up
		if (value != -1) opnaCtrl_.setNoteSlideSSG(ch, value >> 4, value & 0x0f);
	}
	else if (id == "0R") {	// Note slide down
		if (value != -1) opnaCtrl_.setNoteSlideSSG(ch, value >> 4, -(value & 0x0f));
	}
	else if (!isSkippedSpecial) {
		ret = readSSGSpecialEffect(ch, id, value);
	}

	return ret;
}

bool BambooTracker::readDrumEffect(int ch, std::string id, int value, bool isSkippedSpecial)
{
	bool ret = false;

	if (id == "08") {		// Pan
		if (-1 < value && value < 4) opnaCtrl_.setPanDrum(ch, value);
	}
	else if (id == "0F") {
		if (value != -1) {
			if (value < 0x20) {	// Speed change
				effSpeedChange(value);
			}
			else {				// Tempo change
				effTempoChange(value);
			}
		}
	}
	else if (id == "0O") {	// Groove
		if (-1 < value && value < mod_->getGrooveCount())
			effGrooveChange(value);
	}
	else if (id == "0V") {	// Master volume
		if (-1 < value && value <64) opnaCtrl_.setMasterVolumeDrum(value);
	}
	else if (!isSkippedSpecial) {
		ret = readDrumSpecialEffect(ch, id, value);
	}

	return ret;
}

bool BambooTracker::readFMSpecialEffect(int ch, std::string id, int value)
{
	bool ret = false;

	if (id == "0B") {	// Position jump
		ret = effPositionJump(value);
	}
	else if (id == "0C") {	// Track end
		if (value != -1) {
			effTrackEnd();
			ret = true;
		}
	}
	else if (id == "0D") {	// Pattern break
		ret = effPatternBreak(value);
	}
	else if (id == "0S") {	// Note cut
		ntCutDlyCntFM_[ch] = value;
	}
	else if (id == "0T") {	// Transpose delay
		tposeDlyCntFM_[ch] = (value & 0x70) >> 4;
		tposeDlyValueFM_[ch] = ((value & 0x80) ? -1 : 1) * (value & 0x0f);
	}
	else if (id.front() == 'M') {	// Volume delay
		int count = ctohex(*(id.begin() + 1));
		if (value != -1) {
			if (count > 0) {
				volDlyCntFM_[ch] = count;
				volDlyValueFM_[ch] = value;
			}
		}
	}

	return ret;
}

bool BambooTracker::readSSGSpecialEffect(int ch, std::string id, int value)
{
	bool ret = false;

	if (id == "0B") {	// Position jump
		ret = effPositionJump(value);
	}
	else if (id == "0C") {	// Track end
		if (value != -1) {
			effTrackEnd();
			ret = true;
		}
	}
	else if (id == "0D") {	// Pattern break
		ret = effPatternBreak(value);
	}
	else if (id == "0S") {	// Note cut
		ntCutDlyCntSSG_[ch] = value;
	}
	else if (id == "0T") {	// Transpose delay
		tposeDlyCntSSG_[ch] = (value & 0x70) >> 4;
		tposeDlyValueSSG_[ch] = ((value & 0x80) ? -1 : 1) * (value & 0x0f);
	}
	else if (id.front() == 'M') {	// Volume delay
		int count = ctohex(*(id.begin() + 1));
		if (0 <= value && value < 0x10) {
			if (count > 0) {
				volDlyCntSSG_[ch] = count;
				volDlyValueSSG_[ch] = value;
			}
		}
	}

	return ret;
}

bool BambooTracker::readDrumSpecialEffect(int ch, std::string id, int value)
{
	bool ret = false;

	if (id == "0B") {	// Position jump
		ret = effPositionJump(value);
	}
	else if (id == "0C") {	// Track end
		if (value != -1) {
			effTrackEnd();
			ret = true;
		}
	}
	else if (id == "0D") {	// Pattern break
		ret = effPatternBreak(value);
	}
	else if (id == "0S") {	// Note cut
		ntCutDlyCntDrum_[ch] = value;
	}
	else if (id.front() == 'M') {	// Volume delay
		int count = ctohex(*(id.begin() + 1));
		if (0 <= value && value < 0x20) {
			if (count > 0) {
				volDlyCntDrum_[ch] = count;
				volDlyValueDrum_[ch] = value;
			}
		}
	}

	return ret;
}

bool BambooTracker::effPositionJump(int nextOrder)
{
	if (nextOrder < getOrderSize(curSongNum_)) {
		nextReadOrder_ = nextOrder;
		nextReadStep_ = 0;
		return true;
	}
	return false;
}

void BambooTracker::effTrackEnd()
{
	nextReadOrder_ = -1;
	nextReadStep_ = -1;
}

bool BambooTracker::effPatternBreak(int nextStep)
{
	if (playOrderNum_ == getOrderSize(curSongNum_) - 1) {
		return false;
	}
	else if (nextStep < getPatternSizeFromOrderNumber(curSongNum_, playOrderNum_ + 1)) {
		nextReadOrder_ = playOrderNum_ + 1;
		nextReadStep_ = nextStep;
		return true;
	}
	return false;
}

void BambooTracker::effSpeedChange(int speed)
{
	tickCounter_.setSpeed(speed ? speed : 1);
	tickCounter_.setGrooveEnebled(false);
}

void BambooTracker::effTempoChange(int tempo)
{
	tickCounter_.setTempo(tempo);
	tickCounter_.setGrooveEnebled(false);
}

void BambooTracker::effGrooveChange(int num)
{
	tickCounter_.setGroove(mod_->getGroove(num).getSequence());
	tickCounter_.setGrooveEnebled(true);
}

void BambooTracker::getStreamSamples(int16_t *container, size_t nSamples)
{
	opnaCtrl_.getStreamSamples(container, nSamples);
}

void BambooTracker::killSound()
{
	opnaCtrl_.reset();
}

/********** Stream details **********/
int BambooTracker::getStreamRate() const
{
	return opnaCtrl_.getRate();
}

void BambooTracker::setStreamRate(int rate)
{
	opnaCtrl_.setRate(rate);
}

int BambooTracker::getStreamDuration() const
{
	return opnaCtrl_.getDuration();
}

void BambooTracker::setStreamDuration(int duration)
{
	opnaCtrl_.setDuration(duration);
}

/********** Module details **********/
/*----- Module -----*/
void BambooTracker::setModuleTitle(std::string title)
{
	mod_->setTitle(title);
}

std::string BambooTracker::getModuleTitle() const
{
	return mod_->getTitle();
}

void BambooTracker::setModuleAuthor(std::string author)
{
	mod_->setAuthor(author);
}

std::string BambooTracker::getModuleAuthor() const
{
	return mod_->getAuthor();
}

void BambooTracker::setModuleCopyright(std::string copyright)
{
	mod_->setCopyright(copyright);
}

std::string BambooTracker::getModuleCopyright() const
{
	return mod_->getCopyright();
}

void BambooTracker::setModuleTickFrequency(unsigned int freq)
{
	mod_->setTickFrequency(freq);
	tickCounter_.setInterruptRate(freq);
}

unsigned int BambooTracker::getModuleTickFrequency() const
{
	return mod_->getTickFrequency();
}

size_t BambooTracker::getGrooveCount() const
{
	return mod_->getGrooveCount();
}

void BambooTracker::setGroove(int num, std::vector<int> seq)
{
	mod_->setGroove(num, std::move(seq));
}

void BambooTracker::setGrooves(std::vector<std::vector<int>> seqs)
{
	mod_->setGrooves(std::move(seqs));
}

std::vector<int> BambooTracker::getGroove(int num) const
{
	return mod_->getGroove(num).getSequence();
}

/*----- Song -----*/
void BambooTracker::setSongTitle(int songNum, std::string title)
{
	mod_->getSong(songNum).setTitle(title);
}

std::string BambooTracker::getSongTitle(int songNum) const
{
	return mod_->getSong(songNum).getTitle();
}

void BambooTracker::setSongTempo(int songNum, int tempo)
{
	mod_->getSong(songNum).setTempo(tempo);
	if (curSongNum_ == songNum) tickCounter_.setTempo(tempo);
}

int BambooTracker::getSongTempo(int songNum) const
{
	return mod_->getSong(songNum).getTempo();
}

void BambooTracker::setSongGroove(int songNum, int groove)
{
	mod_->getSong(songNum).setGroove(groove);
	tickCounter_.setGroove(mod_->getGroove(groove).getSequence());
}

int BambooTracker::getSongGroove(int songNum) const
{
	return mod_->getSong(songNum).getGroove();
}

void BambooTracker::toggleTempoOrGrooveInSong(int songNum, bool isTempo)
{
	mod_->getSong(songNum).toggleTempoOrGroove(isTempo);
	tickCounter_.setGrooveEnebled(!isTempo);
}

bool BambooTracker::isUsedTempoInSong(int songNum) const
{
	return mod_->getSong(songNum).isUsedTempo();
}

SongStyle BambooTracker::getSongStyle(int songNum) const
{
	return mod_->getSong(songNum).getStyle();
}

void BambooTracker::setSongSpeed(int songNum, int speed)
{
	mod_->getSong(songNum).setSpeed(speed);
	if (curSongNum_ == songNum) tickCounter_.setSpeed(speed);
}

int BambooTracker::getSongSpeed(int songNum) const
{
	return mod_->getSong(songNum).getSpeed();
}

size_t BambooTracker::getSongCount() const
{
	return mod_->getSongCount();
}

void BambooTracker::addSong(SongType songType, std::string title)
{
	mod_->addSong(songType, title);
}

void BambooTracker::sortSongs(std::vector<int> numbers)
{
	mod_->sortSongs(std::move(numbers));
}

/*----- Order -----*/
std::vector<OrderData> BambooTracker::getOrderData(int songNum, int orderNum) const
{
	return mod_->getSong(songNum).getOrderData(orderNum);
}

void BambooTracker::setOrderPattern(int songNum, int trackNum, int orderNum, int patternNum)
{
	comMan_.invoke(std::make_unique<SetPatternToOrderCommand>(mod_, songNum, trackNum, orderNum, patternNum));
}

void BambooTracker::insertOrderBelow(int songNum, int orderNum)
{
	comMan_.invoke(std::make_unique<InsertOrderBelowCommand>(mod_, songNum, orderNum));
}

void BambooTracker::deleteOrder(int songNum, int orderNum)
{
	comMan_.invoke(std::make_unique<DeleteOrderCommand>(mod_, songNum, orderNum));
}

void BambooTracker::pasteOrderCells(int songNum, int beginTrack, int beginOrder,
									std::vector<std::vector<std::string>> cells)
{
	// Arrange data
	std::vector<std::vector<std::string>> d;
	size_t w = songStyle_.trackAttribs.size() - beginTrack;
	size_t h = getOrderSize(songNum) - beginOrder;

	size_t width = std::min(cells.at(0).size(), w);
	size_t height = std::min(cells.size(), h);

	for (size_t i = 0; i < height; ++i) {
		d.emplace_back();
		for (size_t j = 0; j < width; ++j) {
			d.at(i).push_back(cells.at(i).at(j));
		}
	}

	comMan_.invoke(std::make_unique<PasteCopiedDataToOrderCommand>(mod_, songNum, beginTrack, beginOrder, std::move(d)));
}

size_t BambooTracker::getOrderSize(int songNum) const
{
	return  mod_->getSong(songNum).getOrderSize();
}

/*----- Pattern -----*/
int BambooTracker::getStepNoteNumber(int songNum, int trackNum, int orderNum, int stepNum) const
{
	return mod_->getSong(songNum).getTrack(trackNum).getPatternFromOrderNumber(orderNum)
			.getStep(stepNum).getNoteNumber();
}

void BambooTracker::setStepNote(int songNum, int trackNum, int orderNum, int stepNum, int octave, Note note)
{
	int nn = octaveAndNoteToNoteNumber(octave, note);

	int in;
	if (curInstNum_ != -1
			&& (songStyle_.trackAttribs.at(trackNum).source
				== instMan_.getInstrumentSharedPtr(curInstNum_)->getSoundSource()))
		in = curInstNum_;
	else
		in = -1;

	comMan_.invoke(std::make_unique<SetKeyOnToStepCommand>(mod_, songNum, trackNum, orderNum, stepNum, nn, in));
}

void BambooTracker::setStepKeyOff(int songNum, int trackNum, int orderNum, int stepNum)
{
	comMan_.invoke(std::make_unique<SetKeyOffToStepCommand>(mod_, songNum, trackNum, orderNum, stepNum));
}

void BambooTracker::eraseStepNote(int songNum, int trackNum, int orderNum, int stepNum)
{
	comMan_.invoke(std::make_unique<EraseStepCommand>(mod_, songNum, trackNum, orderNum, stepNum));
}

int BambooTracker::getStepInstrument(int songNum, int trackNum, int orderNum, int stepNum) const
{
	return mod_->getSong(songNum).getTrack(trackNum).getPatternFromOrderNumber(orderNum)
			.getStep(stepNum).getInstrumentNumber();
}

void BambooTracker::setStepInstrument(int songNum, int trackNum, int orderNum, int stepNum, int instNum)
{
	comMan_.invoke(std::make_unique<SetInstrumentToStepCommand>(mod_, songNum, trackNum, orderNum, stepNum, instNum));
}

void BambooTracker::eraseStepInstrument(int songNum, int trackNum, int orderNum, int stepNum)
{
	comMan_.invoke(std::make_unique<EraseInstrumentInStepCommand>(mod_, songNum, trackNum, orderNum, stepNum));
}

int BambooTracker::getStepVolume(int songNum, int trackNum, int orderNum, int stepNum) const
{
	return mod_->getSong(songNum).getTrack(trackNum).getPatternFromOrderNumber(orderNum)
			.getStep(stepNum).getVolume();
}

void BambooTracker::setStepVolume(int songNum, int trackNum, int orderNum, int stepNum, int volume)
{	
	comMan_.invoke(std::make_unique<SetVolumeToStepCommand>(mod_, songNum, trackNum, orderNum, stepNum, volume));
}

void BambooTracker::eraseStepVolume(int songNum, int trackNum, int orderNum, int stepNum)
{
	comMan_.invoke(std::make_unique<EraseVolumeInStepCommand>(mod_, songNum, trackNum, orderNum, stepNum));
}

std::string BambooTracker::getStepEffectID(int songNum, int trackNum, int orderNum, int stepNum, int n) const
{
	return mod_->getSong(songNum).getTrack(trackNum).getPatternFromOrderNumber(orderNum)
			.getStep(stepNum).getEffectID(n);
}

void BambooTracker::setStepEffectID(int songNum, int trackNum, int orderNum, int stepNum, int n, std::string id)
{
	comMan_.invoke(std::make_unique<SetEffectIDToStepCommand>(mod_, songNum, trackNum, orderNum, stepNum, n, id));
}

int BambooTracker::getStepEffectValue(int songNum, int trackNum, int orderNum, int stepNum, int n) const
{
	mod_->getSong(songNum).getTrack(trackNum).getPatternFromOrderNumber(orderNum)
			.getStep(stepNum).getEffectValue(n);
}

void BambooTracker::setStepEffectValue(int songNum, int trackNum, int orderNum, int stepNum, int n, int value)
{
	comMan_.invoke(std::make_unique<SetEffectValueToStepCommand>(mod_, songNum, trackNum, orderNum, stepNum, n, value));
}

void BambooTracker::eraseStepEffect(int songNum, int trackNum, int orderNum, int stepNum, int n)
{
	comMan_.invoke(std::make_unique<EraseEffectInStepCommand>(mod_, songNum, trackNum, orderNum, stepNum, n));
}

void BambooTracker::eraseStepEffectValue(int songNum, int trackNum, int orderNum, int stepNum, int n)
{
	comMan_.invoke(std::make_unique<EraseEffectValueInStepCommand>(mod_, songNum, trackNum, orderNum, stepNum, n));
}

void BambooTracker::insertStep(int songNum, int trackNum, int orderNum, int stepNum)
{
	comMan_.invoke(std::make_unique<InsertStepCommand>(mod_, songNum, trackNum, orderNum, stepNum));
}

void BambooTracker::deletePreviousStep(int songNum, int trackNum, int orderNum, int stepNum)
{
	comMan_.invoke(std::make_unique<DeletePreviousStepCommand>(mod_, songNum, trackNum, orderNum, stepNum));
}

void BambooTracker::pastePatternCells(int songNum, int beginTrack, int beginColmn, int beginOrder, int beginStep,
									  std::vector<std::vector<std::string>> cells)
{
	std::vector<std::vector<std::string>> d
			= arrangePatternDataCells(songNum, beginTrack, beginColmn, beginOrder, beginStep, std::move(cells));

	comMan_.invoke(std::make_unique<PasteCopiedDataToPatternCommand>(
					   mod_, songNum, beginTrack, beginColmn, beginOrder, beginStep, std::move(d)));
}

void BambooTracker::pasteMixPatternCells(int songNum, int beginTrack, int beginColmn, int beginOrder, int beginStep,
										 std::vector<std::vector<std::string>> cells)
{
	std::vector<std::vector<std::string>> d
			= arrangePatternDataCells(songNum, beginTrack, beginColmn, beginOrder, beginStep, std::move(cells));

	comMan_.invoke(std::make_unique<PasteMixCopiedDataToPatternCommand>(
					   mod_, songNum, beginTrack, beginColmn, beginOrder, beginStep, std::move(d)));
}

std::vector<std::vector<std::string>> BambooTracker::arrangePatternDataCells(int songNum, int beginTrack, int beginColmn, int beginOrder, int beginStep,
																			 std::vector<std::vector<std::string>> cells)
{
	std::vector<std::vector<std::string>> d;
	size_t w = (songStyle_.trackAttribs.size() - beginTrack - 1) * 11 + (11 - beginColmn);
	size_t h = getPatternSizeFromOrderNumber(songNum, beginOrder) - beginStep;

	size_t width = std::min(cells.at(0).size(), w);
	size_t height = std::min(cells.size(), h);

	for (size_t i = 0; i < height; ++i) {
		d.emplace_back();
		for (size_t j = 0; j < width; ++j) {
			d.at(i).push_back(cells.at(i).at(j));
		}
	}

	return d;
}

void BambooTracker::erasePatternCells(int songNum, int beginTrack, int beginColmn, int beginOrder, int beginStep,
									  int endTrack, int endColmn, int endStep)
{
	comMan_.invoke(std::make_unique<EraseCellsInPatternCommand>(
					   mod_, songNum, beginTrack, beginColmn, beginOrder, beginStep, endTrack, endColmn, endStep));
}

void BambooTracker::increaseNoteKeyInPattern(int songNum, int beginTrack, int beginOrder, int beginStep,
											 int endTrack, int endStep)
{
	comMan_.invoke(std::make_unique<IncreaseNoteKeyInPatternCommand>(
					   mod_, songNum, beginTrack, beginOrder, beginStep, endTrack, endStep));
}
void BambooTracker::decreaseNoteKeyInPattern(int songNum, int beginTrack, int beginOrder, int beginStep,
											 int endTrack, int endStep)
{
	comMan_.invoke(std::make_unique<DecreaseNoteKeyInPatternCommand>(
					   mod_, songNum, beginTrack, beginOrder, beginStep, endTrack, endStep));
}

void BambooTracker::increaseNoteOctaveInPattern(int songNum, int beginTrack, int beginOrder, int beginStep,
												int endTrack, int endStep)
{
	comMan_.invoke(std::make_unique<IncreaseNoteOctaveInPatternCommand>(
					   mod_, songNum, beginTrack, beginOrder, beginStep, endTrack, endStep));
}

void BambooTracker::decreaseNoteOctaveInPattern(int songNum, int beginTrack, int beginOrder, int beginStep,
												int endTrack, int endStep)
{
	comMan_.invoke(std::make_unique<DecreaseNoteOctaveInPatternCommand>(
					   mod_, songNum, beginTrack, beginOrder, beginStep, endTrack, endStep));
}

void BambooTracker::expandPattern(int songNum, int beginTrack, int beginColmn, int beginOrder, int beginStep,
								  int endTrack, int endColmn, int endStep)
{
	comMan_.invoke(std::make_unique<ExpandPatternCommand>(
					   mod_, songNum, beginTrack, beginColmn, beginOrder, beginStep, endTrack, endColmn, endStep));
}

void BambooTracker::shrinkPattern(int songNum, int beginTrack, int beginColmn, int beginOrder, int beginStep,
								  int endTrack, int endColmn, int endStep)
{
	comMan_.invoke(std::make_unique<ShrinkPatternCommand>(
					   mod_, songNum, beginTrack, beginColmn, beginOrder, beginStep, endTrack, endColmn, endStep));
}

size_t BambooTracker::getPatternSizeFromOrderNumber(int songNum, int orderNum) const
{
	size_t size = 0;
	for (auto& t : songStyle_.trackAttribs) {
		size = (!size)
			   ? mod_->getSong(songNum).getTrack(t.number).getPatternFromOrderNumber(orderNum).getSize()
			   : std::min(
					 size,
					 mod_->getSong(songNum).getTrack(t.number).getPatternFromOrderNumber(orderNum).getSize()
					 );
	}
	return size;
}

void BambooTracker::setDefaultPatternSize(int songNum, size_t size)
{
	mod_->getSong(songNum).setDefaultPatternSize(size);
}

size_t BambooTracker::getDefaultPatternSize(int songNum) const
{
	return mod_->getSong(songNum).getDefaultPatternSize();
}

int BambooTracker::ctohex(const char c) const
{
	if (c == '0')		return 0;
	else if (c == '1')	return 1;
	else if (c == '2')	return 2;
	else if (c == '3')	return 3;
	else if (c == '4')	return 4;
	else if (c == '5')	return 5;
	else if (c == '6')	return 6;
	else if (c == '7')	return 7;
	else if (c == '8')	return 8;
	else if (c == '9')	return 9;
	else if (c == 'A')	return 10;
	else if (c == 'B')	return 11;
	else if (c == 'C')	return 12;
	else if (c == 'D')	return 13;
	else if (c == 'E')	return 14;
	else if (c == 'F')	return 15;
	else				return -1;
}
