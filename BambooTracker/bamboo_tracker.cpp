#include "bamboo_tracker.hpp"
#include <algorithm>
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
}

void BambooTracker::startPlayPattern()
{
	startPlay();
	playState_ = 0x21;
	curStepNum_ = 0;
}

void BambooTracker::startPlayFromCurrentStep()
{
	startPlay();
	playState_ = 0x41;
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

void BambooTracker::readStep()
{
	if (playState_ & 0x02) {	// Foward current step
		if (curStepNum_ == getPatternSizeFromOrderNumber(curSongNum_, curOrderNum_) - 1) {
			if (!(playState_ & 0x20)) {	// Not play pattern
				if (curOrderNum_ == getOrderList(curSongNum_, 0).size() - 1) {
					curOrderNum_ = 0;
				}
				else {
					++curOrderNum_;
				}
			}
			curStepNum_ = 0;
		}
		else {
			++curStepNum_;
		}
	}
	else {	// First read
		playState_ |= 0x02;
	}

	// Read step data
	// TODO

}

void BambooTracker::readTick()
{
	// UNDONE: read tick
}

/********** Stream events **********/
int BambooTracker::streamCountUp()
{
	int state = tickCounter_.countUp();

	if (state > 0) readTick();
	else if (!state) readStep();

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
	int nn = 12 * octave;

	switch (note) {
	case Note::C:	nn += 0;	break;
	case Note::CS:	nn += 1;	break;
	case Note::D:	nn += 2;	break;
	case Note::DS:	nn += 3;	break;
	case Note::E:	nn += 4;	break;
	case Note::F:	nn += 5;	break;
	case Note::FS:	nn += 6;	break;
	case Note::G:	nn += 7;	break;
	case Note::GS:	nn += 8;	break;
	case Note::A:	nn += 9;	break;
	case Note::AS:	nn += 10;	break;
	case Note::B:	nn += 11;	break;
	}

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
