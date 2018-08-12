#include "bamboo_tracker.hpp"
#include <algorithm>
#include "commands.hpp"

#include <QDebug>

BambooTracker::BambooTracker()
	:
	  #ifdef SINC_INTERPOLATION
	  opnaCtrl_(3993600 * 2, 44100, 40, &instMan_),
	  #else
	  opnaCtrl_(3993600 * 2, 44100, &instMan_),
	  #endif
	  mod_(std::make_unique<Module>(ModuleType::STD)),
	  octave_(4),
	  curSongNum_(0),
	  curTrackNum_(0),
	  curOrderNum_(0),
	  curStepNum_(0),
	  curInstNum_(-1),
	  streamIntrRate_(60)	// NTSC
{
	modStyle_ = mod_->getStyle();
}

/********** Change octave **********/
int BambooTracker::raiseOctave()
{
	if (octave_ != 7) ++octave_;
	return octave_;
}

int BambooTracker::lowerOctave()
{
	if (octave_ != 0) --octave_;
	return octave_;
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

void BambooTracker::insertNewOrder(int prevOdrNum)
{

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
bool BambooTracker::toggleJamMode()
{
	return jamMan_.toggleJamMode();
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
		case SoundSource::PSG:	opnaCtrl_.keyOffPSG(offData.channelInSource);	break;
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
	case SoundSource::PSG:
		opnaCtrl_.setInstrumentPSG(onData.channelInSource, std::dynamic_pointer_cast<InstrumentPSG>(tmpInst));
		opnaCtrl_.keyOnPSG(onData.channelInSource,
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
		case SoundSource::PSG:	opnaCtrl_.keyOffPSG(data.channelInSource);	break;
		}
	}
}

/********** Play song **********/
void BambooTracker::startPlaySong()
{
	opnaCtrl_.reset();
	jamMan_.polyphonic(false);
	tickCounter_.resetCount();
	tickCounter_.setPlayState(true);
}

void BambooTracker::stopPlaySong()
{
	opnaCtrl_.reset();
	jamMan_.polyphonic(true);
	tickCounter_.setPlayState(false);
}

void BambooTracker::readStep()
{
//	qDebug() << "step";
}

void BambooTracker::readTick()
{
//	qDebug() << "tick";
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

int BambooTracker::getStepInstrumentNumber(int songNum, int trackNum, int orderNum, int stepNum) const
{
	return mod_->getSong(songNum).getTrack(trackNum).getPatternFromOrderNumber(orderNum)
			.getStep(stepNum).getInstrumentNumber();
}

int BambooTracker::getStepVolume(int songNum, int trackNum, int orderNum, int stepNum) const
{
	return mod_->getSong(songNum).getTrack(trackNum).getPatternFromOrderNumber(orderNum)
			.getStep(stepNum).getVolume();
}

std::string BambooTracker::getStepEffectString(int songNum, int trackNum, int orderNum, int stepNum) const
{
	return mod_->getSong(songNum).getTrack(trackNum).getPatternFromOrderNumber(orderNum)
			.getStep(stepNum).getEffectString();
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
