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
	std::shared_ptr<AbstructInstrument> inst = instMan_.getInstrumentSharedPtr(num);
	if (inst == nullptr) return std::unique_ptr<AbstructInstrument>();
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

void BambooTracker::setEnvelopeFMParameter(int envNum, FMParameter param, int value)
{
	instMan_.setEnvelopeFMParameter(envNum, param, value);
	opnaCtrl_.updateInstrumentFMEnvelopeParameter(envNum, param);
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

void BambooTracker::setTrackMuteState(int trackNum, bool isMute)
{
	auto& ta = modStyle_.trackAttribs[trackNum];
	switch (ta.source) {
	case SoundSource::FM:	opnaCtrl_.setMuteFMState(ta.channelInSource, isMute);	break;
	case SoundSource::SSG:	opnaCtrl_.setMuteSSGState(ta.channelInSource, isMute);	break;
	}
}

bool BambooTracker::isMute(int trackNum)
{
	auto& ta = modStyle_.trackAttribs[trackNum];
	switch (ta.source) {
	case SoundSource::FM:	return opnaCtrl_.isMuteFM(ta.channelInSource);
	case SoundSource::SSG:	return opnaCtrl_.isMuteSSG(ta.channelInSource);
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
				// Channel envelope reset before next key on
				if (step.getNoteNumber() >= 0 && opnaCtrl_.enableEnvelopeReset(attrib.channelInSource)) {
					opnaCtrl_.resetChannelEnvelope(attrib.channelInSource);
				}
				break;

			case SoundSource::SSG:
				break;
			}
		}
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
			if (nextReadStepOrder_ == getOrderSize(curSongNum_) - 1) {
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

/// Read order: volume -> instrument -> effect -> key on
void BambooTracker::readStep()
{
	auto& song = mod_->getSong(curSongNum_);
	for (auto& attrib : modStyle_.trackAttribs) {
		auto& step = song.getTrack(attrib.number)
					 .getPatternFromOrderNumber(curOrderNum_).getStep(curStepNum_);
		switch (attrib.source) {
		case SoundSource::FM:
		{
			// Set volume
			if (step.getVolume() != -1) {
				opnaCtrl_.setVolumeFM(attrib.channelInSource, step.getVolume());
			}
			// Set instrument
			if (step.getInstrumentNumber() != -1) {
				if (auto inst = std::dynamic_pointer_cast<InstrumentFM>(
							instMan_.getInstrumentSharedPtr(step.getInstrumentNumber())))
					opnaCtrl_.setInstrumentFM(attrib.channelInSource, inst);
			}
			// Set effect
			// TODO: effect set
			// Set key
			switch (step.getNoteNumber()) {
			case -1:	// None
				break;
			case -2:	// Key off
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
			// Set volume
			int vol = step.getVolume();
			if (0 <= vol && vol < 0x10) {
				opnaCtrl_.setVolumeSSG(attrib.channelInSource, step.getVolume());
			}
			// Set instrument
			if (step.getInstrumentNumber() != -1) {
				if (auto inst = std::dynamic_pointer_cast<InstrumentSSG>(
							instMan_.getInstrumentSharedPtr(step.getInstrumentNumber())))
					opnaCtrl_.setInstrumentSSG(attrib.channelInSource, inst);
			}
			// Set effect
			// TODO: effect set
			// Set key
			switch (step.getNoteNumber()) {
			case -1:	// None
				break;
			case -2:	// Key off
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

std::string BambooTracker::getStepEffectID(int songNum, int trackNum, int orderNum, int stepNum) const
{
	return mod_->getSong(songNum).getTrack(trackNum).getPatternFromOrderNumber(orderNum)
			.getStep(stepNum).getEffectID();
}

void BambooTracker::setStepEffectID(int songNum, int trackNum, int orderNum, int stepNum, std::string id)
{
	comMan_.invoke(std::make_unique<SetEffectIDToStepCommand>(mod_, songNum, trackNum, orderNum, stepNum, id));
}

int BambooTracker::getStepEffectValue(int songNum, int trackNum, int orderNum, int stepNum) const
{
	mod_->getSong(songNum).getTrack(trackNum).getPatternFromOrderNumber(orderNum)
			.getStep(stepNum).getEffectValue();
}

void BambooTracker::setStepEffectValue(int songNum, int trackNum, int orderNum, int stepNum, int value)
{
	comMan_.invoke(std::make_unique<SetEffectValueToStepCommand>(mod_, songNum, trackNum, orderNum, stepNum, value));
}

void BambooTracker::eraseStepEffect(int songNum, int trackNum, int orderNum, int stepNum)
{
	comMan_.invoke(std::make_unique<EraseEffectInStepCommand>(mod_, songNum, trackNum, orderNum, stepNum));
}

void BambooTracker::eraseStepEffectValue(int songNum, int trackNum, int orderNum, int stepNum)
{
	comMan_.invoke(std::make_unique<EraseEffectValueInStepCommand>(mod_, songNum, trackNum, orderNum, stepNum));
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
	std::vector<std::vector<std::string>> d;
	int w = (modStyle_.trackAttribs.size() - beginTrack - 1) * 5 + (5 - beginColmn);
	int h = getPatternSizeFromOrderNumber(songNum, beginOrder) - beginStep;

	int width = std::min(cells.at(0).size(), static_cast<size_t>(w));
	int height = std::min(cells.size(), static_cast<size_t>(h));

	for (int i = 0; i < height; ++i) {
		d.emplace_back();
		for (int j = 0; j < width; ++j) {
			d.at(i).push_back(cells.at(i).at(j));
		}
	}

	comMan_.invoke(std::make_unique<PasteCopiedDataToPatternCommand>(
					   mod_, songNum, beginTrack, beginColmn, beginOrder, beginStep, d));
}

void BambooTracker::erasePatternCells(int songNum, int beginTrack, int beginColmn, int beginOrder, int beginStep,
									  int endTrack, int endColmn, int endStep)
{
	comMan_.invoke(std::make_unique<EraseCellsInPatternCommand>(
					   mod_, songNum, beginTrack, beginColmn, beginOrder, beginStep, endTrack, endColmn, endStep));
}

size_t BambooTracker::getOrderSize(int songNum) const
{
	return  mod_->getSong(songNum).getOrderSize();
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
