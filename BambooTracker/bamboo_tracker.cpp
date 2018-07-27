#include "bamboo_tracker.hpp"
#include <vector>
#include <algorithm>
#include "commands.hpp"

#include <QDebug>

BambooTracker::BambooTracker() :
	#ifdef SINC_INTERPOLATION
	opnaCtrl_(3993600 * 2, 44100, 40, &instMan_),
	#else
	opnaCtrl_(3993600 * 2, 44100, &instMan_),
	#endif
	octave_(4),
	curChannel_(0),
	curInstNum_(-1),
	isPlaySong_(false)
{
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

/********** Current channel **********/
void BambooTracker::selectChannel(int channel)
{
	curChannel_.setNumber(channel);
}

ChannelAttribute BambooTracker::getCurrentChannel() const
{
	return curChannel_;
}

/********** Current instrument **********/
void BambooTracker::setCurrentInstrument(int n)
{
	curInstNum_ = n;
}

/********** Instrument edit **********/
void BambooTracker::addInstrument(int num, std::string name)
{
	comMan_.invoke(std::make_unique<AddInstrumentCommand>(instMan_, num, curChannel_.getSoundSource(), name));
}

void BambooTracker::removeInstrument(int num)
{
	comMan_.invoke(std::make_unique<RemoveInstrumentCommand>(instMan_, num));
}

std::unique_ptr<AbstructInstrument> BambooTracker::getInstrument(int num)
{
	return instMan_.getInstrumentCopy(num);
}

void BambooTracker::setInstrumentName(int num, std::string name)
{
	comMan_.invoke(std::make_unique<ChangeInstrumentNameCommand>(instMan_, num, name));
}

void BambooTracker::setInstrumentFMParameter(int num, FMParameter param, int value)
{
	instMan_.setFMEnvelopeParameter(num, param, value);
	opnaCtrl_.setInstrumentFMParameter(num, param);
}

void BambooTracker::setInstrumentFMOperatorEnable(int instNum, int opNum, bool enable)
{
	instMan_.setFMOperatorEnable(instNum, opNum, enable);
	opnaCtrl_.setInstrumentFMOperatorEnable(instNum, opNum);
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
											   curChannel_.getIdInSoundSource(),
											   curChannel_.getSoundSource());
	if (list.size() == 2) {	// Key off
		JamKeyData& offData = list[1];
		switch (offData.source) {
		case SoundSource::FM:	opnaCtrl_.keyOffFM(offData.chIdInSource);	break;
		case SoundSource::PSG:	opnaCtrl_.keyOffPSG(offData.chIdInSource);	break;
		}
	}

	auto tmpInst = instMan_.getInstrumentCopy(curInstNum_);
	JamKeyData& onData = list[0];
	switch (onData.source) {
	case SoundSource::FM:
		opnaCtrl_.setInstrumentFM(onData.chIdInSource, dynamic_cast<InstrumentFM*>(tmpInst.release()));
		opnaCtrl_.keyOnFM(onData.chIdInSource,
						  JamManager::jamKeyToNote(onData.key),
						  JamManager::calcOctave(octave_, onData.key),
						  0);
		break;
	case SoundSource::PSG:
		opnaCtrl_.setInstrumentPSG(onData.chIdInSource, dynamic_cast<InstrumentPSG*>(tmpInst.release()));
		opnaCtrl_.keyOnPSG(onData.chIdInSource,
						   JamManager::jamKeyToNote(onData.key),
						   JamManager::calcOctave(octave_, onData.key),
						   0);
		break;
	}
}

void BambooTracker::jamKeyOff(JamKey key)
{
	JamKeyData &&data = jamMan_.keyOff(key);

	if (data.chIdInSource > -1) {	// Key still sound
		switch (data.source) {
		case SoundSource::FM:	opnaCtrl_.keyOffFM(data.chIdInSource);	break;
		case SoundSource::PSG:	opnaCtrl_.keyOffPSG(data.chIdInSource);	break;
		}
	}
}

/********** Play song **********/
void BambooTracker::startPlaySong()
{
	opnaCtrl_.reset();
	jamMan_.polyphonic(false);
	isPlaySong_ = true;
}

void BambooTracker::stopPlaySong()
{
	opnaCtrl_.reset();
	jamMan_.polyphonic(true);
	isPlaySong_ = false;
}

void BambooTracker::readStep()
{
//	qDebug() << "step";
}

void BambooTracker::readTick()
{
//	qDebug() << "tick";
}

/********** Stream samples **********/
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
