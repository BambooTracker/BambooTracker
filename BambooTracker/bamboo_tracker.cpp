#include "bamboo_tracker.hpp"
#include <vector>
#include "pitch_converter.hpp"
#include "commands.hpp"

#include <QDebug>

BambooTracker::BambooTracker() :
	#ifdef SINC_INTERPOLATION
	chip_(3993600 * 2, 44100, 40),
	#else
	chip_(3993600 * 2, 44100),
	#endif
	instMan_(chip_),
	octave_(4),
	curChannel_(0),
	isPlaySong_(false)
{
	initChip();
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
	return instMan_.getInstrument(num);
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
		case SoundSource::FM:	keyOffFM(offData.chIdInSource);		break;
		case SoundSource::PSG:	keyOffPSG(offData.chIdInSource);	break;
		}
	}

	JamKeyData& onData = list[0];
	switch (onData.source) {
	case SoundSource::FM:
		keyOnFM(onData.chIdInSource,
				JamManager::jamKeyToNote(onData.key),
				JamManager::calcOctave(octave_, onData.key),
				0);
		break;
	case SoundSource::PSG:
		keyOnPSG(onData.chIdInSource,
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
		case SoundSource::FM:	keyOffFM(data.chIdInSource);	break;
		case SoundSource::PSG:	keyOffPSG(data.chIdInSource);	break;
		}
	}
}

/********** Play song **********/
void BambooTracker::startPlaySong()
{
	chip_.reset();
	initChip();
	jamMan_.polyphonic(false);
	isPlaySong_ = true;
}

void BambooTracker::stopPlaySong()
{
	chip_.reset();
	initChip();
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
	chip_.mix(container, nSamples);
}

/********** Stream details **********/
int BambooTracker::getStreamRate() const
{
	return chip_.getRate();
}

int BambooTracker::getStreamDuration() const
{
	return 40;	// dummy set
}

/***********************************/
void BambooTracker::initChip()
{
	chip_.setRegister(0x29, 0x80);		// Init interrupt / YM2608 mode
	mixerPSG_ = 0xff;
	chip_.setRegister(0x07, mixerPSG_);	// PSG mix
	chip_.setRegister(0x11, 0x3f);		// Drum total volume

	// Init pan
	for (int i = 0; i < 0x200; i += 0x100) {
		for (int j = 0; j < 3; ++j) {
			chip_.setRegister(0xb4+i+j, 0xc0);
		}
	}
}

/********** Key on-off **********/
void BambooTracker::keyOnFM(int id, Note note, int octave, int fine)
{
	uint16_t pitch = PitchConverter::getPitchFM(note, octave, fine);
	// UNDONE: change channel type by Effect mode
	uint8_t bank = (id < 3)? 0 : 0x100;
	uint8_t offset = bank + id % 3;
	chip_.setRegister(0xa4 + offset, pitch >> 8);
	chip_.setRegister(0xa0 + offset, pitch & 0x00ff);
}

void BambooTracker::keyOnPSG(int id, Note note, int octave, int fine)
{
	uint16_t pitch = PitchConverter::getPitchPSG(note, octave, fine);
	uint8_t offset = id << 1;
	chip_.setRegister(0x00 + offset, pitch & 0xff);
	chip_.setRegister(0x01 + offset, pitch >> 8);

	uint8_t mask = ~(1 << id);
	mixerPSG_ &= mask;
	chip_.setRegister(0x07, mixerPSG_);

	// Dummy volume
	changeVolumePSG(id, 0xf);
	//*********************
}

void BambooTracker::keyOffFM(int id)
{
	// UNDONE: change channel type by Effect mode
	uint8_t data = id;
	chip_.setRegister(0x28, data);
}

void BambooTracker::keyOffPSG(int id)
{
	uint8_t flag = 1 << id;
	mixerPSG_ |= flag;
	chip_.setRegister(0x07, mixerPSG_);
	changeVolumePSG(id, 0);
}

/********** Volume change **********/
void BambooTracker::changeVolumePSG(int id, int level)
{
	chip_.setRegister(0x08 + id, level);
}
