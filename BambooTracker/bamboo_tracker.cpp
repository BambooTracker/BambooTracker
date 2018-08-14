#include "bamboo_tracker.hpp"
#include <algorithm>
#include <utility>
#include "commands.hpp"

BambooTracker::BambooTracker()
	:
	  #ifdef SINC_INTERPOLATION
	  opnaCtrl_(3993600 * 2, 44100, 40, &instMan_),
	  #else
	  opnaCtrl_(3993600 * 2, 44100, &instMan_),
	  #endif
	  mod_(std::make_shared<Module>(ModuleType::STD)),
	  octave_(4),
	  curSongNum_(0),
	  curTrackNum_(0),
	  curOrderNum_(0),
	  curStepNum_(0),
	  curInstNum_(-1),
	  playState_(false),
	  streamIntrRate_(60)	// NTSC
{
	modStyle_ = mod_->getStyle();
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
	TrackAttribute ret = modStyle_.trackAttribs.at(curTrackNum_);
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
					   instMan_, num, modStyle_.trackAttribs[curTrackNum_].source, name));
}

void BambooTracker::removeInstrument(int num)
{
	comMan_.invoke(std::make_unique<RemoveInstrumentCommand>(instMan_, num));
}

std::unique_ptr<AbstructInstrument> BambooTracker::getInstrument(int num)
{
	return instMan_.getInstrumentSharedPtr(num)->clone();
}

void BambooTracker::pasteInstrument(int num, int refNum)
{
	comMan_.invoke(std::make_unique<PasteInstrumentCommand>(instMan_, num, refNum));
	opnaCtrl_.updateInstrumentFM(num);
}

void BambooTracker::cloneInstrument(int num, int refNum)
{
	comMan_.invoke(std::make_unique<CloneInstrumentCommand>(instMan_, num, refNum));
}

int BambooTracker::findFirstFreeInstrumentNumber() const
{
	return instMan_.findFirstFreeInstrument();
}

void BambooTracker::setInstrumentName(int num, std::string name)
{
	comMan_.invoke(std::make_unique<ChangeInstrumentNameCommand>(instMan_, num, name));
}

void BambooTracker::setEnvelopeFMParameter(int envNum, FMParameter param, int value)
{
	instMan_.setEnvelopeFMParameter(envNum, param, value);
	opnaCtrl_.setInstrumentFMEnvelopeParameter(envNum, param);
}

void BambooTracker::setEnvelopeFMOperatorEnable(int envNum, int opNum, bool enable)
{
	instMan_.setEnvelopeFMOperatorEnable(envNum, opNum, enable);
	opnaCtrl_.setInstrumentFMOperatorEnable(envNum, opNum);
}

void BambooTracker::setInstrumentFMEnvelope(int instNum, int envNum)
{
	instMan_.setInstrumentFMEnvelope(instNum, envNum);
	opnaCtrl_.updateInstrumentFM(instNum);
}

/********** Song edit **********/
int BambooTracker::getCurrentSongNumber() const
{
	return curSongNum_;
}

void BambooTracker::setCurrentSongNumber(int num)
{
	curSongNum_ = num;
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

/********** Jam mode **********/
void BambooTracker::toggleJamMode()
{
	if (jamMan_.toggleJamMode() && !isPlaySong()) {
		jamMan_.polyphonic(true, modStyle_.type);
	}
	else {
		jamMan_.polyphonic(false, modStyle_.type);
	}
}

bool BambooTracker::isJamMode() const
{
	return jamMan_.isJamMode();
}

void BambooTracker::jamKeyOn(JamKey key)
{
	std::vector<JamKeyData> &&list = jamMan_.keyOn(key,
												   modStyle_.trackAttribs[curTrackNum_].channelInSource,
												   modStyle_.trackAttribs[curTrackNum_].source);
	if (list.size() == 2) {	// Key off
		JamKeyData& offData = list[1];
		switch (offData.source) {
		case SoundSource::FM:	opnaCtrl_.keyOffFM(offData.channelInSource);	break;
		case SoundSource::SSG:	opnaCtrl_.keyOffSSG(offData.channelInSource);	break;
		}
	}

	std::shared_ptr<AbstructInstrument> tmpInst = instMan_.getInstrumentSharedPtr(curInstNum_);
	JamKeyData& onData = list[0];
	switch (onData.source) {
	case SoundSource::FM:
		opnaCtrl_.setInstrumentFM(onData.channelInSource, std::dynamic_pointer_cast<InstrumentFM>(tmpInst));
		opnaCtrl_.keyOnFM(onData.channelInSource,
						  JamManager::jamKeyToNote(onData.key),
						  JamManager::calcOctave(octave_, onData.key),
						  0);
		break;
	case SoundSource::SSG:
		opnaCtrl_.setInstrumentSSG(onData.channelInSource, std::dynamic_pointer_cast<InstrumentSSG>(tmpInst));
		opnaCtrl_.keyOnSSG(onData.channelInSource,
						   JamManager::jamKeyToNote(onData.key),
						   JamManager::calcOctave(octave_, onData.key),
						   0);
		break;
	}
}

void BambooTracker::jamKeyOff(JamKey key)
{
	JamKeyData &&data = jamMan_.keyOff(key);

	if (data.channelInSource > -1) {	// Key still sound
		switch (data.source) {
		case SoundSource::FM:	opnaCtrl_.keyOffFM(data.channelInSource);	break;
		case SoundSource::SSG:	opnaCtrl_.keyOffSSG(data.channelInSource);	break;
		}
	}
}

/********** Play song **********/
void BambooTracker::startPlaySong()
{
	startPlay();
	playState_ = 0x11;
	curOrderNum_ = 0;
	curStepNum_ = 0;
	findNextStep();
}

void BambooTracker::startPlayPattern()
{
	startPlay();
	playState_ = 0x21;
	curStepNum_ = 0;
	findNextStep();
}

void BambooTracker::startPlayFromCurrentStep()
{
	startPlay();
	playState_ = 0x41;
	findNextStep();
}

void BambooTracker::startPlay()
{
	opnaCtrl_.reset();
	jamMan_.polyphonic(false, modStyle_.type);
	tickCounter_.resetCount();
	tickCounter_.setPlayState(true);
}

void BambooTracker::stopPlaySong()
{
	opnaCtrl_.reset();
	jamMan_.polyphonic(true, modStyle_.type);
	tickCounter_.setPlayState(false);
	playState_ = 0;
}

bool BambooTracker::isPlaySong() const
{
	return  ((playState_ & 0x01) > 0);
}

void BambooTracker::stepDown()
{
	if (playState_ & 0x02) {	// Foward current step
		curOrderNum_ = nextReadStepOrder_;
		curStepNum_ = nextReadStepStep_;
	}
	else {	// First read
		playState_ |= 0x02;
	}
}

void BambooTracker::findNextStep()
{
	// Init
	nextReadStepOrder_ = curOrderNum_;
	nextReadStepStep_ = curStepNum_;

	// Search
	if (nextReadStepStep_ == getPatternSizeFromOrderNumber(curSongNum_, nextReadStepOrder_) - 1) {
		if (!(playState_ & 0x20)) {	// Not play pattern
			if (nextReadStepOrder_ == getOrderList(curSongNum_, 0).size() - 1) {
				nextReadStepOrder_ = 0;
			}
			else {
				++nextReadStepOrder_;
			}
		}
		nextReadStepStep_ = 0;
	}
	else {
		++nextReadStepStep_;
	}
}

void BambooTracker::readStep()
{
	auto& song = mod_->getSong(curSongNum_);
	for (auto& attrib : modStyle_.trackAttribs) {
		auto& step = song.getTrack(attrib.number)
					 .getPatternFromOrderNumber(curOrderNum_).getStep(curStepNum_);
		switch (attrib.source) {
		case SoundSource::FM:
		{
			// Set instrument
			if (step.getInstrumentNumber() != -1)
				opnaCtrl_.setInstrumentFM(
							attrib.channelInSource,
							std::dynamic_pointer_cast<InstrumentFM>(instMan_.getInstrumentSharedPtr(step.getInstrumentNumber()))
							);
			// Set volume
			// TODO: volume set
			// Set effect
			// TODO: effect set
			// Set key
			switch (step.getNoteNumber()) {
			case -1:
				break;
			case -3:
			case -4:
				opnaCtrl_.keyOffFM(attrib.channelInSource);
				break;
			default:
			{
				std::pair<int, Note> octNote = noteNumberToOctaveAndNote(step.getNoteNumber());
				opnaCtrl_.keyOnFM(attrib.channelInSource, octNote.second, octNote.first, 0);
				break;
			}
			}
			break;
		}

		case SoundSource::SSG:
		{
			// Set instrument
			if (step.getInstrumentNumber() != -1)
				opnaCtrl_.setInstrumentSSG(
							attrib.channelInSource,
							std::dynamic_pointer_cast<InstrumentSSG>(instMan_.getInstrumentSharedPtr(step.getInstrumentNumber()))
							);
			// Set volume
			// TODO: volume set
			// Set effect
			// TODO: effect set
			// Set key
			switch (step.getNoteNumber()) {
			case -1:
				break;
			case -3:
			case -4:
				opnaCtrl_.keyOffSSG(attrib.channelInSource);
				break;
			default:
			{
				std::pair<int, Note> octNote = noteNumberToOctaveAndNote(step.getNoteNumber());
				opnaCtrl_.keyOnSSG(attrib.channelInSource, octNote.second, octNote.first, 0);
				break;
			}
			}
			break;
		}
		}
	}
}

void BambooTracker::readTick(int rest)
{
	if (!(playState_ & 0x02)) return;	// When it has not read first step

	if (rest == 1) {
		findNextStep();

		auto& song = mod_->getSong(curSongNum_);
		for (auto& attrib : modStyle_.trackAttribs) {
			auto& step = song.getTrack(attrib.number)
						 .getPatternFromOrderNumber(nextReadStepOrder_).getStep(nextReadStepStep_);
			switch (attrib.source) {
			case SoundSource::FM:
				// Key off before next key on
				if (step.getNoteNumber() >= 0 && opnaCtrl_.isKeyOnFM(attrib.channelInSource)) {
					opnaCtrl_.keyOffFM(attrib.channelInSource);
				}
				break;

			case SoundSource::SSG:
				// Key off before next key on
				if (step.getNoteNumber() >= 0 && opnaCtrl_.isKeyOnSSG(attrib.channelInSource))
					opnaCtrl_.keyOffSSG(attrib.channelInSource);
				break;
			}
		}
	}
}

/********** Stream events **********/
int BambooTracker::streamCountUp()
{
	int state = tickCounter_.countUp();

	if (state > 0) {
		readTick(state);
	}
	else if (!state) {
		stepDown();
		readStep();
	}

	return state;
}

void BambooTracker::getStreamSamples(int16_t *container, size_t nSamples)
{
	opnaCtrl_.getStreamSamples(container, nSamples);
}

/********** Stream details **********/
int BambooTracker::getStreamRate() const
{
	return opnaCtrl_.getRate();
}

int BambooTracker::getStreamDuration() const
{
	return opnaCtrl_.getDuration();
}

int BambooTracker::getStreamInterruptRate() const
{
	return streamIntrRate_;
}

/********** Module details **********/
ModuleStyle BambooTracker::getModuleStyle() const
{
	return mod_->getStyle();
}

std::vector<int> BambooTracker::getOrderList(int songNum, int trackNum) const
{
	return mod_->getSong(songNum).getTrack(trackNum).getOrderList();
}

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
			&& (modStyle_.trackAttribs.at(trackNum).source
				== instMan_.getInstrumentSharedPtr(curInstNum_)->getSoundSource()))
		in = curInstNum_;
	else
		in = -1;

	comMan_.invoke(std::make_unique<SetNoteToStepCommand>(mod_, songNum, trackNum, orderNum, stepNum, nn, in));
}

void BambooTracker::setStepKeyOn(int songNum, int trackNum, int orderNum, int stepNum)
{
	comMan_.invoke(std::make_unique<SetKeyOnToStepCommand>(mod_, songNum, trackNum, orderNum, stepNum));
}

void BambooTracker::setStepKeyOff(int songNum, int trackNum, int orderNum, int stepNum)
{
	comMan_.invoke(std::make_unique<SetKeyOffToStepCommand>(mod_, songNum, trackNum, orderNum, stepNum));
}

void BambooTracker::setStepKeyRelease(int songNum, int trackNum, int orderNum, int stepNum)
{
	comMan_.invoke(std::make_unique<SetKeyReleaseToStepCommand>(mod_, songNum, trackNum, orderNum, stepNum));
}

void BambooTracker::eraseStepNote(int songNum, int trackNum, int orderNum, int stepNum)
{
	comMan_.invoke(std::make_unique<EraseNoteInStepCommand>(mod_, songNum, trackNum, orderNum, stepNum));
}

int BambooTracker::getStepInstrument(int songNum, int trackNum, int orderNum, int stepNum) const
{
	return mod_->getSong(songNum).getTrack(trackNum).getPatternFromOrderNumber(orderNum)
			.getStep(stepNum).getInstrumentNumber();
}

void BambooTracker::setStepInstrument(int songNum, int trackNum, int orderNum, int stepNum, int instNum)
{
	mod_->getSong(songNum).getTrack(trackNum).getPatternFromOrderNumber(orderNum)
			.getStep(stepNum).setInstrumentNumber(instNum);
}

void BambooTracker::eraseStepInstrument(int songNum, int trackNum, int orderNum, int stepNum)
{
	mod_->getSong(songNum).getTrack(trackNum).getPatternFromOrderNumber(orderNum)
			.getStep(stepNum).setInstrumentNumber(-1);
}

int BambooTracker::getStepVolume(int songNum, int trackNum, int orderNum, int stepNum) const
{
	return mod_->getSong(songNum).getTrack(trackNum).getPatternFromOrderNumber(orderNum)
			.getStep(stepNum).getVolume();
}

void BambooTracker::setStepVolume(int songNum, int trackNum, int orderNum, int stepNum, int volume)
{
	mod_->getSong(songNum).getTrack(trackNum).getPatternFromOrderNumber(orderNum)
			.getStep(stepNum).setVolume(volume);
}

void BambooTracker::eraseStepVolume(int songNum, int trackNum, int orderNum, int stepNum)
{
	mod_->getSong(songNum).getTrack(trackNum).getPatternFromOrderNumber(orderNum)
			.getStep(stepNum).setVolume(-1);
}

std::string BambooTracker::getStepEffectString(int songNum, int trackNum, int orderNum, int stepNum) const
{
	return mod_->getSong(songNum).getTrack(trackNum).getPatternFromOrderNumber(orderNum)
			.getStep(stepNum).getEffectString();
}

void BambooTracker::setStepEffectString(int songNum, int trackNum, int orderNum, int stepNum, std::string str)
{
	mod_->getSong(songNum).getTrack(trackNum).getPatternFromOrderNumber(orderNum)
			.getStep(stepNum).setEffectString(str);
}

void BambooTracker::eraseEffectString(int songNum, int trackNum, int orderNum, int stepNum)
{
	mod_->getSong(songNum).getTrack(trackNum).getPatternFromOrderNumber(orderNum)
			.getStep(stepNum).setEffectString(u8"---");
}

size_t BambooTracker::getPatternSizeFromOrderNumber(int songNum, int orderNum) const
{
	size_t size = 0;
	for (auto& t : modStyle_.trackAttribs) {
		size = (!size)
			   ? mod_->getSong(songNum).getTrack(t.number).getPatternFromOrderNumber(orderNum).getSize()
			   : std::min(
					 size,
					 mod_->getSong(songNum).getTrack(t.number).getPatternFromOrderNumber(orderNum).getSize()
					 );
	}
	return size;
}
