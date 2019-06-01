#include "bamboo_tracker.hpp"
#include <algorithm>
#include <utility>
#include <set>
#include <exception>
#include "commands.hpp"
#include "io_handlers.hpp"
#include "bank.hpp"

const uint32_t BambooTracker::CHIP_CLOCK = 3993600 * 2;

BambooTracker::BambooTracker(std::weak_ptr<Configuration> config)
	: instMan_(std::make_shared<InstrumentsManager>()),
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
	  isFindNextStep_(false)
{
	opnaCtrl_ = std::make_unique<OPNAController>(
					static_cast<chip::Emu>(config.lock()->getEmulator()),
					CHIP_CLOCK,
					config.lock()->getSampleRate(),
					config.lock()->getBufferLength());
	setMasterVolume(config.lock()->getMixerVolumeMaster());
	setMasterVolumeFM(config.lock()->getMixerVolumeFM());
	setMasterVolumeSSG(config.lock()->getMixerVolumeSSG());
	isRetrieveChannel_ = config.lock()->getRetrieveChannelState();

	songStyle_ = mod_->getSong(curSongNum_).getStyle();
	jamMan_ = std::make_unique<JamManager>();

	clearDelayCounts();
}

/********** Change configuration **********/
void BambooTracker::changeConfiguration(std::weak_ptr<Configuration> config)
{
	setStreamRate(static_cast<int>(config.lock()->getSampleRate()));
	setStreamDuration(static_cast<int>(config.lock()->getBufferLength()));
	setMasterVolume(config.lock()->getMixerVolumeMaster());
	setMasterVolumeFM(config.lock()->getMixerVolumeFM());
	setMasterVolumeSSG(config.lock()->getMixerVolumeSSG());
	isRetrieveChannel_ = config.lock()->getRetrieveChannelState();
}

/********** Change octave **********/
void BambooTracker::setCurrentOctave(int octave)
{
	octave_ = octave;
}

int BambooTracker::getCurrentOctave() const
{
	return octave_;
}

/********** Current track **********/
void BambooTracker::setCurrentTrack(int num)
{
	curTrackNum_ = num;
}

TrackAttribute BambooTracker::getCurrentTrackAttribute() const
{
	TrackAttribute ret = songStyle_.trackAttribs.at(static_cast<size_t>(curTrackNum_));
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
					   instMan_, num, songStyle_.trackAttribs[static_cast<size_t>(curTrackNum_)].source, name));
}

void BambooTracker::removeInstrument(int num)
{
	comMan_.invoke(std::make_unique<RemoveInstrumentCommand>(instMan_, num));
}

std::unique_ptr<AbstractInstrument> BambooTracker::getInstrument(int num)
{
	std::shared_ptr<AbstractInstrument> inst = instMan_->getInstrumentSharedPtr(num);
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

void BambooTracker::loadInstrument(std::string path, int instNum)
{
	auto inst = InstrumentIO::loadInstrument(path, instMan_, instNum);
	comMan_.invoke(std::make_unique<AddInstrumentCommand>(
					   instMan_, std::unique_ptr<AbstractInstrument>(inst)));
}

void BambooTracker::saveInstrument(std::string path, int instNum)
{
	InstrumentIO::saveInstrument(path, instMan_, instNum);
}

void BambooTracker::importInstrument(const AbstractBank &bank, size_t index, int instNum)
{
	auto inst = bank.loadInstrument(index, instMan_, instNum);
	comMan_.invoke(std::make_unique<AddInstrumentCommand>(
					   instMan_, std::unique_ptr<AbstractInstrument>(inst)));
}

int BambooTracker::findFirstFreeInstrumentNumber() const
{
	return instMan_->findFirstFreeInstrument();
}

void BambooTracker::setInstrumentName(int num, std::string name)
{
	comMan_.invoke(std::make_unique<ChangeInstrumentNameCommand>(instMan_, num, name));
}

void BambooTracker::clearAllInstrument()
{
	instMan_->clearAll();
}

std::vector<int> BambooTracker::getInstrumentIndices() const
{
	return instMan_->getInstrumentIndices();
}

std::vector<int> BambooTracker::getUnusedInstrumentIndices() const
{
	std::vector<int> unused;
	std::set<int> regdInsts = mod_->getRegisterdInstruments();
	for (auto& inst : instMan_->getInstrumentIndices()) {
		if (regdInsts.find(inst) == regdInsts.end())
			unused.push_back(inst);
	}
	return unused;
}

void BambooTracker::clearUnusedInstrumentProperties()
{
	instMan_->clearUnusedInstrumentProperties();
}

//--- FM
void BambooTracker::setEnvelopeFMParameter(int envNum, FMEnvelopeParameter param, int value)
{
	instMan_->setEnvelopeFMParameter(envNum, param, value);
	opnaCtrl_->updateInstrumentFMEnvelopeParameter(envNum, param);
}

void BambooTracker::setEnvelopeFMOperatorEnable(int envNum, int opNum, bool enable)
{
	instMan_->setEnvelopeFMOperatorEnabled(envNum, opNum, enable);
	opnaCtrl_->setInstrumentFMOperatorEnabled(envNum, opNum);
}

void BambooTracker::setInstrumentFMEnvelope(int instNum, int envNum)
{
	instMan_->setInstrumentFMEnvelope(instNum, envNum);
	opnaCtrl_->updateInstrumentFM(instNum);
}

std::vector<int> BambooTracker::getEnvelopeFMUsers(int envNum) const
{
	return instMan_->getEnvelopeFMUsers(envNum);
}

void BambooTracker::setLFOFMParameter(int lfoNum, FMLFOParameter param, int value)
{
	instMan_->setLFOFMParameter(lfoNum, param, value);
	opnaCtrl_->updateInstrumentFMLFOParameter(lfoNum, param);
}

void BambooTracker::setInstrumentFMLFOEnabled(int instNum, bool enabled)
{
	instMan_->setInstrumentFMLFOEnabled(instNum, enabled);
	opnaCtrl_->updateInstrumentFM(instNum);
}

void BambooTracker::setInstrumentFMLFO(int instNum, int lfoNum)
{
	instMan_->setInstrumentFMLFO(instNum, lfoNum);
	opnaCtrl_->updateInstrumentFM(instNum);
}

std::vector<int> BambooTracker::getLFOFMUsers(int lfoNum) const
{
	return instMan_->getLFOFMUsers(lfoNum);
}

void BambooTracker::addOperatorSequenceFMSequenceCommand(FMEnvelopeParameter param, int opSeqNum, int type, int data)
{
	instMan_->addOperatorSequenceFMSequenceCommand(param, opSeqNum, type, data);
}

void BambooTracker::removeOperatorSequenceFMSequenceCommand(FMEnvelopeParameter param, int opSeqNum)
{
	instMan_->removeOperatorSequenceFMSequenceCommand(param, opSeqNum);
}

void BambooTracker::setOperatorSequenceFMSequenceCommand(FMEnvelopeParameter param, int opSeqNum, int cnt, int type, int data)
{
	instMan_->setOperatorSequenceFMSequenceCommand(param, opSeqNum, cnt, type, data);
}

void BambooTracker::setOperatorSequenceFMLoops(FMEnvelopeParameter param, int opSeqNum, std::vector<int> begins, std::vector<int> ends, std::vector<int> times)
{
	instMan_->setOperatorSequenceFMLoops(param, opSeqNum, std::move(begins), std::move(ends), std::move(times));
}

void BambooTracker::setOperatorSequenceFMRelease(FMEnvelopeParameter param, int opSeqNum, ReleaseType type, int begin)
{
	instMan_->setOperatorSequenceFMRelease(param, opSeqNum, type, begin);
}

void BambooTracker::setInstrumentFMOperatorSequence(int instNum, FMEnvelopeParameter param, int opSeqNum)
{
	instMan_->setInstrumentFMOperatorSequence(instNum, param, opSeqNum);
	opnaCtrl_->updateInstrumentFM(instNum);
}

void BambooTracker::setInstrumentFMOperatorSequenceEnabled(int instNum, FMEnvelopeParameter param, bool enabled)
{
	instMan_->setInstrumentFMOperatorSequenceEnabled(instNum, param, enabled);
	opnaCtrl_->updateInstrumentFM(instNum);
}

std::vector<int> BambooTracker::getOperatorSequenceFMUsers(FMEnvelopeParameter param, int opSeqNum) const
{
	return instMan_->getOperatorSequenceFMUsers(param, opSeqNum);
}

void BambooTracker::setArpeggioFMType(int arpNum, int type)
{
	instMan_->setArpeggioFMType(arpNum, type);
}

void BambooTracker::addArpeggioFMSequenceCommand(int arpNum, int type, int data)
{
	instMan_->addArpeggioFMSequenceCommand(arpNum, type, data);
}

void BambooTracker::removeArpeggioFMSequenceCommand(int arpNum)
{
	instMan_->removeArpeggioFMSequenceCommand(arpNum);
}

void BambooTracker::setArpeggioFMSequenceCommand(int arpNum, int cnt, int type, int data)
{
	instMan_->setArpeggioFMSequenceCommand(arpNum, cnt, type, data);
}

void BambooTracker::setArpeggioFMLoops(int arpNum, std::vector<int> begins, std::vector<int> ends, std::vector<int> times)
{
	instMan_->setArpeggioFMLoops(arpNum, std::move(begins), std::move(ends), std::move(times));
}

void BambooTracker::setArpeggioFMRelease(int arpNum, ReleaseType type, int begin)
{
	instMan_->setArpeggioFMRelease(arpNum, type, begin);
}

void BambooTracker::setInstrumentFMArpeggio(int instNum, FMOperatorType op, int arpNum)
{
	instMan_->setInstrumentFMArpeggio(instNum, op, arpNum);
	opnaCtrl_->updateInstrumentFM(instNum);
}

void BambooTracker::setInstrumentFMArpeggioEnabled(int instNum, FMOperatorType op, bool enabled)
{
	instMan_->setInstrumentFMArpeggioEnabled(instNum, op, enabled);
	opnaCtrl_->updateInstrumentFM(instNum);
}

std::vector<int> BambooTracker::getArpeggioFMUsers(int arpNum) const
{
	return instMan_->getArpeggioFMUsers(arpNum);
}

void BambooTracker::setPitchFMType(int ptNum, int type)
{
	instMan_->setPitchFMType(ptNum, type);
}

void BambooTracker::addPitchFMSequenceCommand(int ptNum, int type, int data)
{
	instMan_->addPitchFMSequenceCommand(ptNum, type, data);
}

void BambooTracker::removePitchFMSequenceCommand(int ptNum)
{
	instMan_->removePitchFMSequenceCommand(ptNum);
}

void BambooTracker::setPitchFMSequenceCommand(int ptNum, int cnt, int type, int data)
{
	instMan_->setPitchFMSequenceCommand(ptNum, cnt, type, data);
}

void BambooTracker::setPitchFMLoops(int ptNum, std::vector<int> begins, std::vector<int> ends, std::vector<int> times)
{
	instMan_->setPitchFMLoops(ptNum, std::move(begins), std::move(ends), std::move(times));
}

void BambooTracker::setPitchFMRelease(int ptNum, ReleaseType type, int begin)
{
	instMan_->setPitchFMRelease(ptNum, type, begin);
}

void BambooTracker::setInstrumentFMPitch(int instNum, FMOperatorType op, int ptNum)
{
	instMan_->setInstrumentFMPitch(instNum, op, ptNum);
	opnaCtrl_->updateInstrumentFM(instNum);
}

void BambooTracker::setInstrumentFMPitchEnabled(int instNum, FMOperatorType op, bool enabled)
{
	instMan_->setInstrumentFMPitchEnabled(instNum, op, enabled);
	opnaCtrl_->updateInstrumentFM(instNum);
}

std::vector<int> BambooTracker::getPitchFMUsers(int ptNum) const
{
	return instMan_->getPitchFMUsers(ptNum);
}

void BambooTracker::setInstrumentFMEnvelopeResetEnabled(int instNum, FMOperatorType op, bool enabled)
{
	instMan_->setInstrumentFMEnvelopeResetEnabled(instNum, op, enabled);
	opnaCtrl_->updateInstrumentFM(instNum);
}

//--- SSG
void BambooTracker::addWaveFormSSGSequenceCommand(int wfNum, int type, int data)
{
	instMan_->addWaveFormSSGSequenceCommand(wfNum, type, data);
}

void BambooTracker::removeWaveFormSSGSequenceCommand(int wfNum)
{
	instMan_->removeWaveFormSSGSequenceCommand(wfNum);
}

void BambooTracker::setWaveFormSSGSequenceCommand(int wfNum, int cnt, int type, int data)
{
	instMan_->setWaveFormSSGSequenceCommand(wfNum, cnt, type, data);
}

void BambooTracker::setWaveFormSSGLoops(int wfNum, std::vector<int> begins, std::vector<int> ends, std::vector<int> times)
{
	instMan_->setWaveFormSSGLoops(wfNum, std::move(begins), std::move(ends), std::move(times));
}

void BambooTracker::setWaveFormSSGRelease(int wfNum, ReleaseType type, int begin)
{
	instMan_->setWaveFormSSGRelease(wfNum, type, begin);
}

void BambooTracker::setInstrumentSSGWaveForm(int instNum, int wfNum)
{
	instMan_->setInstrumentSSGWaveForm(instNum, wfNum);
	opnaCtrl_->updateInstrumentSSG(instNum);
}

void BambooTracker::setInstrumentSSGWaveFormEnabled(int instNum, bool enabled)
{
	instMan_->setInstrumentSSGWaveFormEnabled(instNum, enabled);
	opnaCtrl_->updateInstrumentSSG(instNum);
}

std::vector<int> BambooTracker::getWaveFormSSGUsers(int wfNum) const
{
	return instMan_->getWaveFormSSGUsers(wfNum);
}

void BambooTracker::addToneNoiseSSGSequenceCommand(int tnNum, int type, int data)
{
	instMan_->addToneNoiseSSGSequenceCommand(tnNum, type, data);
}

void BambooTracker::removeToneNoiseSSGSequenceCommand(int tnNum)
{
	instMan_->removeToneNoiseSSGSequenceCommand(tnNum);
}

void BambooTracker::setToneNoiseSSGSequenceCommand(int tnNum, int cnt, int type, int data)
{
	instMan_->setToneNoiseSSGSequenceCommand(tnNum, cnt, type, data);
}

void BambooTracker::setToneNoiseSSGLoops(int tnNum, std::vector<int> begins, std::vector<int> ends, std::vector<int> times)
{
	instMan_->setToneNoiseSSGLoops(tnNum, std::move(begins), std::move(ends), std::move(times));
}

void BambooTracker::setToneNoiseSSGRelease(int tnNum, ReleaseType type, int begin)
{
	instMan_->setToneNoiseSSGRelease(tnNum, type, begin);
}

void BambooTracker::setInstrumentSSGToneNoise(int instNum, int tnNum)
{
	instMan_->setInstrumentSSGToneNoise(instNum, tnNum);
	opnaCtrl_->updateInstrumentSSG(instNum);
}

void BambooTracker::setInstrumentSSGToneNoiseEnabled(int instNum, bool enabled)
{
	instMan_->setInstrumentSSGToneNoiseEnabled(instNum, enabled);
	opnaCtrl_->updateInstrumentSSG(instNum);
}

std::vector<int> BambooTracker::getToneNoiseSSGUsers(int tnNum) const
{
	return instMan_->getToneNoiseSSGUsers(tnNum);
}

void BambooTracker::addEnvelopeSSGSequenceCommand(int envNum, int type, int data)
{
	instMan_->addEnvelopeSSGSequenceCommand(envNum, type, data);
}

void BambooTracker::removeEnvelopeSSGSequenceCommand(int envNum)
{
	instMan_->removeEnvelopeSSGSequenceCommand(envNum);
}

void BambooTracker::setEnvelopeSSGSequenceCommand(int envNum, int cnt, int type, int data)
{
	instMan_->setEnvelopeSSGSequenceCommand(envNum, cnt, type, data);
}

void BambooTracker::setEnvelopeSSGLoops(int envNum, std::vector<int> begins, std::vector<int> ends, std::vector<int> times)
{
	instMan_->setEnvelopeSSGLoops(envNum, std::move(begins), std::move(ends), std::move(times));
}

void BambooTracker::setEnvelopeSSGRelease(int envNum, ReleaseType type, int begin)
{
	instMan_->setEnvelopeSSGRelease(envNum, type, begin);
}

void BambooTracker::setInstrumentSSGEnvelope(int instNum, int envNum)
{
	instMan_->setInstrumentSSGEnvelope(instNum, envNum);
	opnaCtrl_->updateInstrumentSSG(instNum);
}

void BambooTracker::setInstrumentSSGEnvelopeEnabled(int instNum, bool enabled)
{
	instMan_->setInstrumentSSGEnvelopeEnabled(instNum, enabled);
	opnaCtrl_->updateInstrumentSSG(instNum);
}

std::vector<int> BambooTracker::getEnvelopeSSGUsers(int envNum) const
{
	return instMan_->getEnvelopeSSGUsers(envNum);
}

void BambooTracker::setArpeggioSSGType(int arpNum, int type)
{
	instMan_->setArpeggioSSGType(arpNum, type);
}

void BambooTracker::addArpeggioSSGSequenceCommand(int arpNum, int type, int data)
{
	instMan_->addArpeggioSSGSequenceCommand(arpNum, type, data);
}

void BambooTracker::removeArpeggioSSGSequenceCommand(int arpNum)
{
	instMan_->removeArpeggioSSGSequenceCommand(arpNum);
}

void BambooTracker::setArpeggioSSGSequenceCommand(int arpNum, int cnt, int type, int data)
{
	instMan_->setArpeggioSSGSequenceCommand(arpNum, cnt, type, data);
}

void BambooTracker::setArpeggioSSGLoops(int arpNum, std::vector<int> begins, std::vector<int> ends, std::vector<int> times)
{
	instMan_->setArpeggioSSGLoops(arpNum, std::move(begins), std::move(ends), std::move(times));
}

void BambooTracker::setArpeggioSSGRelease(int arpNum, ReleaseType type, int begin)
{
	instMan_->setArpeggioSSGRelease(arpNum, type, begin);
}

void BambooTracker::setInstrumentSSGArpeggio(int instNum, int arpNum)
{
	instMan_->setInstrumentSSGArpeggio(instNum, arpNum);
	opnaCtrl_->updateInstrumentSSG(instNum);
}

void BambooTracker::setInstrumentSSGArpeggioEnabled(int instNum, bool enabled)
{
	instMan_->setInstrumentSSGArpeggioEnabled(instNum, enabled);
	opnaCtrl_->updateInstrumentSSG(instNum);
}

std::vector<int> BambooTracker::getArpeggioSSGUsers(int arpNum) const
{
	return instMan_->getArpeggioSSGUsers(arpNum);
}

void BambooTracker::setPitchSSGType(int ptNum, int type)
{
	instMan_->setPitchSSGType(ptNum, type);
}

void BambooTracker::addPitchSSGSequenceCommand(int ptNum, int type, int data)
{
	instMan_->addPitchSSGSequenceCommand(ptNum, type, data);
}

void BambooTracker::removePitchSSGSequenceCommand(int ptNum)
{
	instMan_->removePitchSSGSequenceCommand(ptNum);
}

void BambooTracker::setPitchSSGSequenceCommand(int ptNum, int cnt, int type, int data)
{
	instMan_->setPitchSSGSequenceCommand(ptNum, cnt, type, data);
}

void BambooTracker::setPitchSSGLoops(int ptNum, std::vector<int> begins, std::vector<int> ends, std::vector<int> times)
{
	instMan_->setPitchSSGLoops(ptNum, std::move(begins), std::move(ends), std::move(times));
}

void BambooTracker::setPitchSSGRelease(int ptNum, ReleaseType type, int begin)
{
	instMan_->setPitchSSGRelease(ptNum, type, begin);
}

void BambooTracker::setInstrumentSSGPitch(int instNum, int ptNum)
{
	instMan_->setInstrumentSSGPitch(instNum, ptNum);
	opnaCtrl_->updateInstrumentSSG(instNum);
}

void BambooTracker::setInstrumentSSGPitchEnabled(int instNum, bool enabled)
{
	instMan_->setInstrumentSSGPitchEnabled(instNum, enabled);
	opnaCtrl_->updateInstrumentSSG(instNum);
}

std::vector<int> BambooTracker::getPitchSSGUsers(int ptNum) const
{
	return instMan_->getPitchSSGUsers(ptNum);
}

/********** Song edit **********/
int BambooTracker::getCurrentSongNumber() const
{
	return curSongNum_;
}

void BambooTracker::setCurrentSongNumber(int num)
{
	curSongNum_ = num;
	curTrackNum_ = 0;
	curOrderNum_ = 0;
	curStepNum_ = 0;

	auto& song = mod_->getSong(curSongNum_);
	songStyle_ = song.getStyle();

	/*jamMan_->clear();*/

	// Reset
	opnaCtrl_->reset();
	opnaCtrl_->setMode(songStyle_.type);
	tickCounter_.resetCount();
	tickCounter_.setTempo(song.getTempo());
	tickCounter_.setSpeed(song.getSpeed());
	tickCounter_.setGroove(mod_->getGroove(song.getGroove()).getSequence());
	tickCounter_.setGrooveEnebled(!song.isUsedTempo());

	size_t fmch = getFMChannelCount(songStyle_.type);
	muteStateFM_ = std::vector<bool>(fmch, false);
	for (int i = 0; i < static_cast<int>(fmch); ++i) opnaCtrl_->setMuteFMState(i, false);

	ntDlyCntFM_ = std::vector<int>(fmch);
	ntCutDlyCntFM_ = std::vector<int>(fmch);
	volDlyCntFM_ = std::vector<int>(fmch);
	volDlyValueFM_ = std::vector<int>(fmch, -1);
	tposeDlyCntFM_ = std::vector<int>(fmch);
	tposeDlyValueFM_ = std::vector<int>(fmch);

	muteStateSSG_ = std::vector<bool>(3, false);
	for (int i = 0; i < 3; ++i) opnaCtrl_->setMuteSSGState(i, false);
	ntDlyCntSSG_ = std::vector<int>(3);
	ntCutDlyCntSSG_ = std::vector<int>(3);
	volDlyCntSSG_ = std::vector<int>(3);
	volDlyValueSSG_ = std::vector<int>(3, -1);
	tposeDlyCntSSG_ = std::vector<int>(3);
	tposeDlyValueSSG_ = std::vector<int>(3);

	muteStateDrum_ = std::vector<bool>(6, false);
	for (int i = 0; i < 6; ++i) opnaCtrl_->setMuteDrumState(i, false);
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
	if (jamMan_->toggleJamMode() && !isPlaySong()) {
		jamMan_->polyphonic(true);
	}
	else {
		jamMan_->polyphonic(false);
	}
}

bool BambooTracker::isJamMode() const
{
	return jamMan_->isJamMode();
}

void BambooTracker::jamKeyOn(JamKey key)
{
	auto attrib = songStyle_.trackAttribs[static_cast<size_t>(curTrackNum_)];
	if (attrib.source == SoundSource::DRUM) {
		opnaCtrl_->keyOnDrum(attrib.channelInSource);
	}
	else {
		int keyNum = octaveAndNoteToNoteNumber(octave_, JamManager::jamKeyToNote(key));
		std::vector<JamKeyData>&& list = jamMan_->keyOn(key, attrib.channelInSource, attrib.source, keyNum);
		if (list.size() == 2) {	// Key off
			JamKeyData& offData = list[1];
			switch (offData.source) {
			case SoundSource::FM:
				if (songStyle_.type == SongType::FMEX && offData.channelInSource == 2) {
					opnaCtrl_->keyOffFM(2, true);
					opnaCtrl_->keyOffFM(6, true);
					opnaCtrl_->keyOffFM(7, true);
					opnaCtrl_->keyOffFM(8, true);
				}
				else {
					opnaCtrl_->keyOffFM(offData.channelInSource, true);
				}
				break;
			case SoundSource::SSG:
				opnaCtrl_->keyOffSSG(offData.channelInSource, true);
				break;
			default:
				break;
			}
		}

		std::shared_ptr<AbstractInstrument> tmpInst = instMan_->getInstrumentSharedPtr(curInstNum_);
		JamKeyData& onData = list.front();

		Note note = JamManager::jamKeyToNote(onData.key);
		int octave = JamManager::calcOctave(octave_, onData.key);
		int pitch = 0;

		switch (onData.source) {
		case SoundSource::FM:
			if (auto fm = std::dynamic_pointer_cast<InstrumentFM>(tmpInst))
				opnaCtrl_->setInstrumentFM(onData.channelInSource, fm);
			if (songStyle_.type == SongType::FMEX && onData.channelInSource == 2) {
				opnaCtrl_->keyOnFM(2, note, octave, pitch, true);
				opnaCtrl_->keyOnFM(6, note, octave, pitch, true);
				opnaCtrl_->keyOnFM(7, note, octave, pitch, true);
				opnaCtrl_->keyOnFM(8, note, octave, pitch, true);
			}
			else {
				opnaCtrl_->keyOnFM(onData.channelInSource, note, octave, pitch, true);
			}
			break;
		case SoundSource::SSG:
			if (auto ssg = std::dynamic_pointer_cast<InstrumentSSG>(tmpInst))
				opnaCtrl_->setInstrumentSSG(onData.channelInSource, ssg);
			opnaCtrl_->keyOnSSG(onData.channelInSource, note, octave, pitch, true);
			break;
		default:
			break;
		}
	}
}

void BambooTracker::jamKeyOn(int keyNum)
{
	auto attrib = songStyle_.trackAttribs[static_cast<size_t>(curTrackNum_)];
	if (attrib.source == SoundSource::DRUM) {
		opnaCtrl_->keyOnDrum(attrib.channelInSource);
	}
	else {
		std::vector<JamKeyData>&& list = jamMan_->keyOn(JamKey::MIDI_KEY, attrib.channelInSource, attrib.source, keyNum);
		if (list.size() == 2) {	// Key off
			JamKeyData& offData = list[1];
			switch (offData.source) {
			case SoundSource::FM:
				if (songStyle_.type == SongType::FMEX && offData.channelInSource == 2) {
					opnaCtrl_->keyOffFM(2, true);
					opnaCtrl_->keyOffFM(6, true);
					opnaCtrl_->keyOffFM(7, true);
					opnaCtrl_->keyOffFM(8, true);
				}
				else {
					opnaCtrl_->keyOffFM(offData.channelInSource, true);
				}
				break;
			case SoundSource::SSG:
				opnaCtrl_->keyOffSSG(offData.channelInSource, true);
				break;
			default:
				break;
			}
		}

		std::shared_ptr<AbstractInstrument> tmpInst = instMan_->getInstrumentSharedPtr(curInstNum_);
		JamKeyData& onData = list.front();

		auto octNote = noteNumberToOctaveAndNote(keyNum);
		int pitch = 0;

		switch (onData.source) {
		case SoundSource::FM:
			if (auto fm = std::dynamic_pointer_cast<InstrumentFM>(tmpInst))
				opnaCtrl_->setInstrumentFM(onData.channelInSource, fm);
			if (songStyle_.type == SongType::FMEX && onData.channelInSource == 2) {
				opnaCtrl_->keyOnFM(2, octNote.second, octNote.first, pitch, true);
				opnaCtrl_->keyOnFM(6, octNote.second, octNote.first, pitch, true);
				opnaCtrl_->keyOnFM(7, octNote.second, octNote.first, pitch, true);
				opnaCtrl_->keyOnFM(8, octNote.second, octNote.first, pitch, true);
			}
			else {
				opnaCtrl_->keyOnFM(onData.channelInSource, octNote.second, octNote.first, pitch, true);
			}
			break;
		case SoundSource::SSG:
			if (auto ssg = std::dynamic_pointer_cast<InstrumentSSG>(tmpInst))
				opnaCtrl_->setInstrumentSSG(onData.channelInSource, ssg);
			opnaCtrl_->keyOnSSG(onData.channelInSource, octNote.second, octNote.first, pitch, true);
			break;
		default:
			break;
		}
	}
}

void BambooTracker::jamKeyOff(JamKey key)
{
	auto attrib = songStyle_.trackAttribs[static_cast<size_t>(curTrackNum_)];
	if (attrib.source == SoundSource::DRUM) {
		opnaCtrl_->keyOffDrum(attrib.channelInSource);
	}
	else {
		int keyNum = octaveAndNoteToNoteNumber(octave_, JamManager::jamKeyToNote(key));
		JamKeyData&& data = jamMan_->keyOff(key, keyNum);

		if (data.channelInSource > -1) {	// Key still sound
			switch (data.source) {
			case SoundSource::FM:
				if (songStyle_.type == SongType::FMEX && data.channelInSource == 2) {
					opnaCtrl_->keyOffFM(2, true);
					opnaCtrl_->keyOffFM(6, true);
					opnaCtrl_->keyOffFM(7, true);
					opnaCtrl_->keyOffFM(8, true);
				}
				else {
					opnaCtrl_->keyOffFM(data.channelInSource, true);
				}
				break;
			case SoundSource::SSG:
				opnaCtrl_->keyOffSSG(data.channelInSource, true);
				break;
			default:
				break;
			}
		}
	}
}

void BambooTracker::jamKeyOff(int keyNum)
{
	auto attrib = songStyle_.trackAttribs[static_cast<size_t>(curTrackNum_)];
	if (attrib.source == SoundSource::DRUM) {
		opnaCtrl_->keyOffDrum(attrib.channelInSource);
	}
	else {
		JamKeyData&& data = jamMan_->keyOff(JamKey::MIDI_KEY, keyNum);

		if (data.channelInSource > -1) {	// Key still sound
			switch (data.source) {
			case SoundSource::FM:
				if (songStyle_.type == SongType::FMEX && data.channelInSource == 2) {
					opnaCtrl_->keyOffFM(2, true);
					opnaCtrl_->keyOffFM(6, true);
					opnaCtrl_->keyOffFM(7, true);
					opnaCtrl_->keyOffFM(8, true);
				}
				else {
					opnaCtrl_->keyOffFM(data.channelInSource, true);
				}
				break;
			case SoundSource::SSG:
				opnaCtrl_->keyOffSSG(data.channelInSource, true);
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
	if (isRetrieveChannel_) retrieveChannelStates();
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
	if (isRetrieveChannel_) retrieveChannelStates();
}

void BambooTracker::startPlayFromCurrentStep()
{
	startPlay();
	playState_ = 0x01;
	playOrderNum_ = curOrderNum_;
	playStepNum_ = curStepNum_;
	findNextStep();
	if (isRetrieveChannel_) retrieveChannelStates();
}

void BambooTracker::startPlay()
{
	opnaCtrl_->reset();
	jamMan_->polyphonic(false);

	Song& song = mod_->getSong(curSongNum_);
	tickCounter_.setTempo(song.getTempo());
	tickCounter_.setSpeed(song.getSpeed());
	tickCounter_.setGroove(mod_->getGroove(song.getGroove()).getSequence());
	tickCounter_.setGrooveEnebled(!song.isUsedTempo());
	tickCounter_.resetCount();
	tickCounter_.setPlayState(true);

	for (size_t i = 0; i < muteStateFM_.size(); ++i) opnaCtrl_->setMuteFMState(static_cast<int>(i), muteStateFM_[i]);
	for (size_t i = 0; i < muteStateSSG_.size(); ++i) opnaCtrl_->setMuteSSGState(static_cast<int>(i), muteStateSSG_[i]);
	for (size_t i = 0; i < muteStateDrum_.size(); ++i) opnaCtrl_->setMuteDrumState(static_cast<int>(i), muteStateDrum_[i]);

	clearDelayCounts();
}

void BambooTracker::stopPlaySong()
{
	opnaCtrl_->reset();
	jamMan_->polyphonic(true);
	tickCounter_.setPlayState(false);
	playState_ = 0;
	playOrderNum_ = -1;
	playStepNum_ = -1;

	for (size_t i = 0; i < muteStateFM_.size(); ++i) opnaCtrl_->setMuteFMState(static_cast<int>(i), false);
	for (size_t i = 0; i < muteStateSSG_.size(); ++i) opnaCtrl_->setMuteSSGState(static_cast<int>(i), false);
	for (size_t i = 0; i < muteStateDrum_.size(); ++i) opnaCtrl_->setMuteDrumState(static_cast<int>(i), false);
}

bool BambooTracker::isPlaySong() const
{
	return ((playState_ & 0x01) > 0);
}

void BambooTracker::setTrackMuteState(int trackNum, bool isMute)
{
	auto& ta = songStyle_.trackAttribs[static_cast<size_t>(trackNum)];
	int ch = ta.channelInSource;
	switch (ta.source) {
	case SoundSource::FM:
		muteStateFM_.at(static_cast<size_t>(ch)) = isMute;
		if (isPlaySong()) opnaCtrl_->setMuteFMState(ch, isMute);
		break;
	case SoundSource::SSG:
		muteStateSSG_.at(static_cast<size_t>(ch)) = isMute;
		if (isPlaySong()) opnaCtrl_->setMuteSSGState(ch, isMute);
		break;
	case SoundSource::DRUM:
		muteStateDrum_.at(static_cast<size_t>(ch)) = isMute;
		if (isPlaySong()) opnaCtrl_->setMuteDrumState(ch, isMute);
		break;
	}
}

bool BambooTracker::isMute(int trackNum)
{
	auto& ta = songStyle_.trackAttribs[static_cast<size_t>(trackNum)];
	size_t ch = static_cast<size_t>(ta.channelInSource);
	switch (ta.source) {
	case SoundSource::FM:	return muteStateFM_.at(ch);
	case SoundSource::SSG:	return muteStateSSG_.at(ch);
	case SoundSource::DRUM:	return muteStateDrum_.at(ch);
	default:	return false;	// Dummy
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

/********** Export **********/
bool BambooTracker::exportToWav(std::string file, int loopCnt, std::function<bool()> f)
{
	size_t sampCnt = static_cast<size_t>(opnaCtrl_->getRate() * opnaCtrl_->getDuration() / 1000);
	size_t intrCnt = static_cast<size_t>(opnaCtrl_->getRate()) / mod_->getTickFrequency();
	size_t intrCntRest = 0;
	std::vector<int16_t> dumbuf(sampCnt << 1);

	int endOrder = 0;
	int endStep = 0;
	checkNextPositionOfLastStep(endOrder, endStep);
	bool endFlag = false;
	bool tmpFollow = isFollowPlay_;
	isFollowPlay_ = false;
	std::shared_ptr<chip::WavExportContainer> exCntr = std::make_shared<chip::WavExportContainer>();
	opnaCtrl_->setExportContainer(exCntr);
	startPlayFromStart();

	while (true) {
		size_t sampCntRest = sampCnt;
		while (sampCntRest) {
			if (!intrCntRest) {	// Interruption
				intrCntRest = intrCnt;    // Set counts to next interruption

				if (!streamCountUp()) {
					if (f()) {	// Update lambda function
						stopPlaySong();
						isFollowPlay_ = tmpFollow;
						return false;
					}

					if ((playOrderNum_ == -1 && playStepNum_ == -1)
							|| (playOrderNum_ == endOrder && playStepNum_ == endStep && !(loopCnt--))){
						endFlag = true;
						break;
					}
				}
			}

			size_t count = std::min(intrCntRest, sampCntRest);
			sampCntRest -= count;
			intrCntRest -= count;

			opnaCtrl_->getStreamSamples(&dumbuf[0], count);
		}

		if (endFlag) break;
	}

	opnaCtrl_->setExportContainer();
	stopPlaySong();
	isFollowPlay_ = tmpFollow;

	try {
		ExportHandler::writeWave(file, exCntr->getStream(), static_cast<uint32_t>(opnaCtrl_->getRate()));
		f();
		return true;
	}
	catch (...) {
		throw;
	}
}

bool BambooTracker::exportToVgm(std::string file, int target, bool gd3TagEnabled, GD3Tag tag, std::function<bool()> f)
{
	int tmpRate = opnaCtrl_->getRate();
	opnaCtrl_->setRate(44100);
	double dblIntrCnt = 44100.0 / static_cast<double>(mod_->getTickFrequency());
	size_t intrCnt = static_cast<size_t>(dblIntrCnt);
	double intrCntDiff = dblIntrCnt - intrCnt;
	double intrCntRest = 0;
	std::vector<int16_t> dumbuf((intrCnt + 1) << 1);

	int loopOrder = 0;
	int loopStep = 0;
	checkNextPositionOfLastStep(loopOrder, loopStep);
	bool loopFlag = (loopOrder != -1);
	int endCnt = (loopOrder == -1) ? 0 : 1;
	bool tmpFollow = isFollowPlay_;
	isFollowPlay_ = false;
	uint32_t loopPoint = 0;
	uint32_t loopPointSamples = 0;
	std::shared_ptr<chip::VgmExportContainer> exCntr
			= std::make_shared<chip::VgmExportContainer>(target, mod_->getTickFrequency());
	opnaCtrl_->setExportContainer(exCntr);
	startPlayFromStart();
	exCntr->forceMoveLoopPoint();

	while (true) {
		if (!streamCountUp()) {
			if (f()) {	// Update lambda function
				stopPlaySong();
				isFollowPlay_ = tmpFollow;
				return false;
			}

			if (playOrderNum_ == loopOrder && playStepNum_ == loopStep && !(endCnt--)) break;

			if (loopFlag && loopOrder == playOrderNum_ && loopStep == playStepNum_) {
				loopPoint = exCntr->setLoopPoint();
				loopPointSamples = exCntr->getSampleLength();
			}
		}

		intrCntRest += intrCntDiff;
		size_t extraIntrCnt = static_cast<size_t>(intrCntRest);
		intrCntRest -= extraIntrCnt;
		opnaCtrl_->getStreamSamples(&dumbuf[0], intrCnt + extraIntrCnt);
	}

	opnaCtrl_->setExportContainer();
	stopPlaySong();
	isFollowPlay_ = tmpFollow;
	opnaCtrl_->setRate(tmpRate);

	try {
		ExportHandler::writeVgm(file, target, exCntr->getData(), CHIP_CLOCK, mod_->getTickFrequency(),
								loopFlag, loopPoint, exCntr->getSampleLength() - loopPointSamples,
								exCntr->getSampleLength(), gd3TagEnabled, tag);
		f();
		return true;
	} catch (...) {
		throw;
	}
}

bool BambooTracker::exportToS98(std::string file, int target, bool tagEnabled, S98Tag tag, int rate, std::function<bool()> f)
{
	int tmpRate = opnaCtrl_->getRate();
	opnaCtrl_->setRate(rate);
	double dblIntrCnt = static_cast<double>(rate) / static_cast<double>(mod_->getTickFrequency());
	size_t intrCnt = static_cast<size_t>(dblIntrCnt);
	double intrCntDiff = dblIntrCnt - intrCnt;
	double intrCntRest = 0;
	std::vector<int16_t> dumbuf((intrCnt + 1) << 1);

	int loopOrder = 0;
	int loopStep = 0;
	checkNextPositionOfLastStep(loopOrder, loopStep);
	bool loopFlag = (loopOrder != -1);
	int endCnt = (loopOrder == -1) ? 0 : 1;
	bool tmpFollow = isFollowPlay_;
	isFollowPlay_ = false;
	uint32_t loopPoint = 0;
	std::shared_ptr<chip::S98ExportContainer> exCntr = std::make_shared<chip::S98ExportContainer>(target);
	opnaCtrl_->setExportContainer(exCntr);
	startPlayFromStart();
	exCntr->forceMoveLoopPoint();

	while (true) {
		exCntr->getData();	// Set wait counts
		if (!streamCountUp()) {
			if (f()) {	// Update lambda function
				stopPlaySong();
				isFollowPlay_ = tmpFollow;
				return false;
			}

			if (playOrderNum_ == loopOrder && playStepNum_ == loopStep && !(endCnt--)) break;

			if (loopFlag && loopOrder == playOrderNum_ && loopStep == playStepNum_) {
				loopPoint = exCntr->setLoopPoint();
			}
		}

		intrCntRest += intrCntDiff;
		size_t extraIntrCnt = static_cast<size_t>(intrCntRest);
		intrCntRest -= extraIntrCnt;
		opnaCtrl_->getStreamSamples(&dumbuf[0], intrCnt + extraIntrCnt);
	}

	opnaCtrl_->setExportContainer();
	stopPlaySong();
	isFollowPlay_ = tmpFollow;
	opnaCtrl_->setRate(tmpRate);

	try {
		ExportHandler::writeS98(file, target, exCntr->getData(), CHIP_CLOCK, static_cast<uint32_t>(rate),
								loopFlag, loopPoint, tagEnabled, tag);
		f();
		return true;
	} catch (...) {
		throw;
	}
}

void BambooTracker::checkNextPositionOfLastStep(int& endOrder, int& endStep) const
{
	Song& song = mod_->getSong(curSongNum_);
	int lastOrder = static_cast<int>(song.getOrderSize()) - 1;
	int lastStep = static_cast<int>(getPatternSizeFromOrderNumber(curSongNum_, lastOrder)) - 1;
	endOrder = 0;
	endStep = 0;
	for (auto attrib : songStyle_.trackAttribs) {
		Step& step = song.getTrack(attrib.number).getPatternFromOrderNumber(lastOrder).getStep(lastStep);
		for (int i = 0; i < 4; ++i) {
			int effVal = step.getEffectValue(i);
			if (effVal != -1) {
				std::string effId = step.getEffectID(i);
				if (effId == "0B" && effVal <= lastOrder) {
					endOrder = effVal;
					endStep = 0;
				}
				else if (effId == "0C") {
					endOrder = -1;
					endStep = -1;
				}
				else if (effId == "0D" && effVal < static_cast<int>(getPatternSizeFromOrderNumber(curSongNum_, 0))) {
					endOrder = 0;
					endStep = effVal;
				}
			}
		}
	}
}

/********** Backup **********/
void BambooTracker::backupModule(std::string file)
{
	ModuleIO::backupModule(file);
}

/********** Stream type **********/
void BambooTracker::useSCCI(SoundInterfaceManager* manager)
{
	opnaCtrl_->useSCCI(manager);
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
			opnaCtrl_->tickEvent(attrib.source, attrib.channelInSource);
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
						.getPatternFromOrderNumber(playOrderNum_).getStep(playStepNum_);
		int ch = attrib.channelInSource;
		switch (attrib.source) {
		case SoundSource::FM:
		{
			// Check volume delay
			if (!volDlyCntFM_.at(ch))
				opnaCtrl_->setTemporaryVolumeFM(ch, volDlyValueFM_.at(ch));
			// Check note cut
			if (!ntCutDlyCntFM_.at(ch))
				opnaCtrl_->keyOffFM(ch);
			// Check transpose delay
			if (!tposeDlyCntFM_.at(ch))
				opnaCtrl_->setTransposeEffectFM(ch, tposeDlyValueFM_.at(ch));
			// Check note delay and envelope reset
			readTickFMForNoteDelay(curStep, ch);
			break;
		}
		case SoundSource::SSG:
		{
			// Check volume delay
			if (!volDlyCntSSG_.at(ch))
				opnaCtrl_->setTemporaryVolumeSSG(ch, volDlyValueSSG_.at(ch));
			// Check note cut
			if (!ntCutDlyCntSSG_.at(ch))
				opnaCtrl_->keyOffSSG(ch);
			// Check transpose delay
			if (!tposeDlyCntSSG_.at(ch))
				opnaCtrl_->setTransposeEffectSSG(ch, tposeDlyValueSSG_.at(ch));
			// Check note delay
			if (!ntDlyCntSSG_.at(ch))
				readSSGStep(curStep, ch, true);
			break;
		}
		case SoundSource::DRUM:
		{
			// Check volume delay
			if (!volDlyCntDrum_.at(ch))
				opnaCtrl_->setTemporaryVolumeDrum(ch, volDlyValueDrum_.at(ch));
			// Check note cut
			if (!ntCutDlyCntDrum_.at(ch))
				opnaCtrl_->keyOnDrum(ch);
			// Check note delay
			if (!ntDlyCntDrum_.at(ch))
				readDrumStep(curStep, ch, true);
			break;
		}
		}

		if (rest == 1 && nextReadOrder_ != -1 && attrib.source == SoundSource::FM) {
			// Channel envelope reset before next key on
			auto& step = song.getTrack(attrib.number)
						 .getPatternFromOrderNumber(nextReadOrder_).getStep(nextReadStep_);
			int n = step.checkEffectID("0G");
			if (n == -1 || !step.getEffectValue(n)) {
				envelopeResetEffectFM(step, ch);
			}
			else {
				opnaCtrl_->tickEvent(attrib.source, ch);
			}
		}
		else {
			opnaCtrl_->tickEvent(attrib.source, ch);
		}
	}
}

void BambooTracker::readTickFMForNoteDelay(Step& step, int ch)
{
	int cnt = ntDlyCntFM_.at(static_cast<size_t>(ch));
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
	int n = step.getNoteNumber();
	if ((n >= 0 || n < -2)
			&& opnaCtrl_->enableFMEnvelopeReset(ch)) {	// Key on or echo buffer access
		int idx = step.checkEffectID("03");
		if ((idx == -1 && !opnaCtrl_->isTonePortamentoFM(ch))
				|| (idx != -1 && !step.getEffectValue(idx))) {
			opnaCtrl_->resetFMChannelEnvelope(ch);
		}
		else {
			opnaCtrl_->tickEvent(SoundSource::FM, ch);
		}
	}
	else {
		opnaCtrl_->tickEvent(SoundSource::FM, ch);
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

void BambooTracker::retrieveChannelStates()
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
	std::vector<bool> isSetVolDrum(6, false), isSetPanDrum(6, false);
	bool isSetMVolDrum = false;
	/// bit0: step
	/// bit1: tempo
	/// bit2: groove
	uint8_t speedStates = 0;

	int o = playOrderNum_;
	int s = playStepNum_;
	bool isPrevPos = false;
	Song& song = mod_->getSong(curSongNum_);

	while (true) {
		for (auto it = songStyle_.trackAttribs.rbegin(), e = songStyle_.trackAttribs.rend(); it != e; ++it) {
			Step& step = song.getTrack(it->number).getPatternFromOrderNumber(o).getStep(s);
			int ch = it->channelInSource;

			switch (it->source) {
			case SoundSource::FM:
			{
				// Effects
				for (int i = 3; i > -1; --i) {
					std::string id = step.getEffectID(i);
					int value = step.getEffectValue(i);
					if (id == "00" && value != -1 && !isSetArpFM.at(ch)) {		// Arpeggio
						isSetArpFM.at(ch) = true;
						if (isPrevPos) opnaCtrl_->setArpeggioEffectFM(ch, value >> 4, value & 0x0f);
					}
					else if ((id == "01" || id == "02" || id == "03")
							 && value != -1 && !isSetPrtFM.at(ch)) {	// Portamento
						isSetPrtFM.at(ch) = true;
						if (isPrevPos) opnaCtrl_->setPortamentoEffectFM(ch, value);
					}
					else if (id == "04" && value != -1 && !isSetVibFM.at(ch)) {	// Vibrato
						isSetVibFM.at(ch) = true;
						if (isPrevPos) opnaCtrl_->setVibratoEffectFM(ch, value >> 4, value & 0x0f);
					}
					else if (id == "07" && value != -1 && !isSetTreFM.at(ch)) {	// Tremolo
						isSetTreFM.at(ch) = true;
						if (isPrevPos) opnaCtrl_->setTremoloEffectFM(ch, value >> 4, value & 0x0f);
					}
					else if (id == "08" && -1 < value && value < 4 && !isSetPanFM.at(ch)) {	// Pan
						isSetPanFM.at(ch) = true;
						if (isPrevPos) opnaCtrl_->setPanFM(ch, value);
					}
					else if (id == "0A" && value != -1 && !isSetVolSldFM.at(ch)) {	// Volume slide
						isSetVolSldFM.at(ch) = true;
						if (isPrevPos) {
							int hi = value >> 4;
							int low = value & 0x0f;
							if (hi && !low) opnaCtrl_->setVolumeSlideFM(ch, hi, true);	// Slide up
							else if (!hi) opnaCtrl_->setVolumeSlideFM(ch, low, false);	// Slide down
						}
					}
					else if (id == "0F" && value != -1 && !(speedStates & 0x4)) {
						if (value < 0x20 && !(speedStates & 0x1)) {	// Speed change
							speedStates |= 0x1;
							if (isPrevPos) effSpeedChange(value);
						}
						else if (!(speedStates & 0x2)) {			// Tempo change
							speedStates |= 0x2;
							if (isPrevPos) effTempoChange(value);
						}
					}
					else if (id == "0O" && -1 < value
							 && value < static_cast<int>(mod_->getGrooveCount()) && !speedStates) {	// Groove
						speedStates |= 0x4;
						if (isPrevPos) effGrooveChange(value);
					}
					else if (id == "0P"  && value != -1 && !isSetDtnFM.at(ch)) {	// Detune
						isSetDtnFM.at(ch) = true;
						if (isPrevPos) opnaCtrl_->setDetuneFM(ch, value - 0x80);
					}
				}
				// Volume
				int vol = step.getVolume();
				if (!isSetVolFM.at(ch) && 0 <= vol && vol < 0x80) {
					isSetVolFM.at(ch) = true;
					if (isPrevPos)
						opnaCtrl_->setVolumeFM(ch, step.getVolume());
				}
				// Instrument
				if (!isSetInstFM.at(ch) && step.getInstrumentNumber() != -1) {
					if (auto inst = std::dynamic_pointer_cast<InstrumentFM>(
								instMan_->getInstrumentSharedPtr(step.getInstrumentNumber()))) {
						isSetInstFM.at(ch) = true;
						if (isPrevPos)
							opnaCtrl_->setInstrumentFM(ch, inst);
					}
				}
				// Tone
				int t = step.getNoteNumber();
				if (isPrevPos && t != -1 && t != -2) {
					--tonesCntFM.at(ch);
					for (auto it2 = toneFM.at(ch).rbegin();
						 it2 != toneFM.at(ch).rend(); ++it2) {
						if (*it2 == -1 || *it2 == tonesCntFM.at(ch)) {
							if (t >= 0) {
								*it2 = t;
							}
							else if (t < -2) {
								*it2 = tonesCntFM.at(ch) - t + 2;
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
					std::string id = step.getEffectID(i);
					int value = step.getEffectValue(i);
					if (id == "00" && value != -1 && !isSetArpSSG.at(ch)) {		// Arpeggio
						isSetArpSSG.at(ch) = true;
						if (isPrevPos) opnaCtrl_->setArpeggioEffectSSG(ch, value >> 4, value & 0x0f);
					}
					else if ((id == "01" || id == "02" || id == "03")
							 && value != -1 && !isSetPrtSSG.at(ch)) {	// Portamento
						isSetPrtSSG.at(ch) = true;
						if (isPrevPos) opnaCtrl_->setPortamentoEffectSSG(ch, value);
					}
					else if (id == "04" && value != -1 && !isSetVibSSG.at(ch)) {	// Vibrato
						isSetVibSSG.at(ch) = true;
						if (isPrevPos) opnaCtrl_->setVibratoEffectSSG(ch, value >> 4, value & 0x0f);
					}
					else if (id == "07" && value != -1 && !isSetTreSSG.at(ch)) {	// Tremolo
						isSetTreSSG.at(ch) = true;
						if (isPrevPos) opnaCtrl_->setTremoloEffectSSG(ch, value >> 4, value & 0x0f);
					}
					else if (id == "0A" && value != -1 && !isSetVolSldSSG.at(ch)) {	// Volume slide
						isSetVolSldSSG.at(ch) = true;
						if (isPrevPos) {
							int hi = value >> 4;
							int low = value & 0x0f;
							if (hi && !low) opnaCtrl_->setVolumeSlideSSG(ch, hi, true);	// Slide up
							else if (!hi) opnaCtrl_->setVolumeSlideSSG(ch, low, false);	// Slide down
						}
					}
					else if (id == "0F" && value != -1 && !(speedStates & 0x4)) {
						if (value < 0x20 && !(speedStates & 0x1)) {	// Speed change
							speedStates |= 0x1;
							if (isPrevPos) effSpeedChange(value);
						}
						else if (!(speedStates & 0x2)) {			// Tempo change
							speedStates |= 0x2;
							if (isPrevPos) effTempoChange(value);
						}
					}
					else if (id == "0O" && -1 < value
							 && value < static_cast<int>(mod_->getGrooveCount()) && !speedStates) {	// Groove
						speedStates |= 0x4;
						if (isPrevPos) effGrooveChange(value);
					}
					else if (id == "0P"  && value != -1 && !isSetDtnSSG.at(ch)) {	// Detune
						isSetDtnSSG.at(ch) = true;
						if (isPrevPos) opnaCtrl_->setDetuneSSG(ch, value - 0x80);
					}
				}
				// Volume
				int vol = step.getVolume();
				if (!isSetVolSSG.at(ch) && 0 <= vol && vol < 0x10) {
					isSetVolSSG.at(ch) = true;
					if (isPrevPos)
						opnaCtrl_->setVolumeSSG(ch, vol);
				}
				// Instrument
				if (!isSetInstSSG.at(ch) && step.getInstrumentNumber() != -1) {
					if (auto inst = std::dynamic_pointer_cast<InstrumentSSG>(
								instMan_->getInstrumentSharedPtr(step.getInstrumentNumber()))) {
						isSetInstSSG.at(ch) = true;
						if (isPrevPos)
							opnaCtrl_->setInstrumentSSG(ch, inst);
					}
				}
				// Tone
				int t = step.getNoteNumber();
				if (isPrevPos && t != -1 && t != -2) {
					--tonesCntSSG.at(ch);
					for (auto it2 = toneSSG.at(ch).rbegin();
						 it2 != toneSSG.at(ch).rend(); ++it2) {
						if (*it2 == -1 || *it2 == tonesCntSSG.at(ch)) {
							if (t >= 0) {
								*it2 = t;
							}
							else if (t < -2) {
								*it2 = tonesCntSSG.at(ch) - t + 2;
							}
							break;
						}
					}
				}
				break;
			}
			case SoundSource::DRUM:
			{
				// Effects
				for (int i = 3; i > -1; --i) {
					std::string id = step.getEffectID(i);
					int value = step.getEffectValue(i);
					if (id == "08" && -1 < value && value < 4 && !isSetPanDrum.at(ch)) {	// Pan
						isSetPanDrum.at(ch) = true;
						if (isPrevPos) opnaCtrl_->setPanDrum(ch, value);
					}
					else if (id == "0F" && value != -1 && !(speedStates & 0x4)) {
						if (value < 0x20 && !(speedStates & 0x1)) {	// Speed change
							speedStates |= 0x1;
							if (isPrevPos) effSpeedChange(value);
						}
						else if (!(speedStates & 0x2)) {			// Tempo change
							speedStates |= 0x2;
							if (isPrevPos) effTempoChange(value);
						}
					}
					else if (id == "0O" && -1 < value
							 && value < static_cast<int>(mod_->getGrooveCount()) && !speedStates) {	// Groove
						speedStates |= 0x4;
						if (isPrevPos) effGrooveChange(value);
					}
					else if (id == "0V"  && -1 < value && value < 64 && !isSetMVolDrum) {	// Master volume
						isSetMVolDrum = true;
						if (isPrevPos) opnaCtrl_->setMasterVolumeDrum(value);
					}
				}
				// Volume
				int vol = step.getVolume();
				if (!isSetVolDrum.at(ch) && 0 <= vol && vol < 0x20) {
					isSetVolDrum.at(ch) = true;
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

void BambooTracker::findNextStep()
{
	// Init
	nextReadOrder_ = playOrderNum_;
	nextReadStep_ = playStepNum_;

	// Search
	if (nextReadStep_ == static_cast<int>(getPatternSizeFromOrderNumber(curSongNum_, nextReadOrder_)) - 1) {
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
				ntDlyCntFM_.at(static_cast<size_t>(attrib.channelInSource)) = step.getEffectValue(nd);
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
				ntDlyCntSSG_.at(static_cast<size_t>(attrib.channelInSource)) = step.getEffectValue(nd);
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
				ntDlyCntDrum_.at(static_cast<size_t>(attrib.channelInSource)) = step.getEffectValue(nd);
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
	int vol = step.getVolume();
	if (0 <= vol && vol < 0x80) {
		opnaCtrl_->setVolumeFM(ch, vol);
	}
	// Set instrument
	if (step.getInstrumentNumber() != -1) {
		if (auto inst = std::dynamic_pointer_cast<InstrumentFM>(
					instMan_->getInstrumentSharedPtr(step.getInstrumentNumber())))
			opnaCtrl_->setInstrumentFM(ch, inst);
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
		opnaCtrl_->tickEvent(SoundSource::FM, ch);
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

	return isNextSet;
}

bool BambooTracker::readSSGStep(Step& step, int ch, bool isSkippedSpecial)
{
	bool isNextSet = false;

	// Set volume
	int vol = step.getVolume();
	if (0 <= vol && vol < 0x10) {
		opnaCtrl_->setVolumeSSG(ch, vol);
	}
	// Set instrument
	if (step.getInstrumentNumber() != -1) {
		if (auto inst = std::dynamic_pointer_cast<InstrumentSSG>(
					instMan_->getInstrumentSharedPtr(step.getInstrumentNumber())))
			opnaCtrl_->setInstrumentSSG(ch, inst);
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
		opnaCtrl_->tickEvent(SoundSource::SSG, ch);
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

	return isNextSet;
}

bool BambooTracker::readDrumStep(Step& step, int ch, bool isSkippedSpecial)
{
	bool isNextSet = false;

	// Set volume
	int vol = step.getVolume();
	if (0 <= vol && vol < 0x20) {
		opnaCtrl_->setVolumeDrum(ch, vol);
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
		opnaCtrl_->keyOffDrum(ch);
		break;
	default:	// Key on & Echo
		opnaCtrl_->keyOnDrum(ch);
		break;
	}

	return isNextSet;
}

bool BambooTracker::readFMEffect(int ch, std::string id, int value, bool isSkippedSpecial)
{
	bool ret = false;

	if (id == "00") {		// Arpeggio
		if (value != -1) opnaCtrl_->setArpeggioEffectFM(ch, value >> 4, value & 0x0f);
	}
	else if (id == "01") {	// Portamento up
		if (value != -1) opnaCtrl_->setPortamentoEffectFM(ch, value);
	}
	else if (id == "02") {	// Portamento down
		if (value != -1) opnaCtrl_->setPortamentoEffectFM(ch, -value);
	}
	else if (id == "03") {	// Tone portamento
		if (value != -1) opnaCtrl_->setPortamentoEffectFM(ch, value, true);
	}
	else if (id == "04") {	// Vibrato
		if (value != -1) opnaCtrl_->setVibratoEffectFM(ch, value >> 4, value & 0x0f);
	}
	else if (id == "07") {	// Tremolo
		if (value != -1) opnaCtrl_->setTremoloEffectFM(ch, value >> 4, value & 0x0f);
	}
	else if (id == "08") {	// Pan
		if (-1 < value && value < 4) opnaCtrl_->setPanFM(ch, value);
	}
	else if (id == "0A") {	// Volume slide
		if (value != -1) {
			int hi = value >> 4;
			int low = value & 0x0f;
			if (hi && !low) opnaCtrl_->setVolumeSlideFM(ch, hi, true);	// Slide up
			else if (!hi) opnaCtrl_->setVolumeSlideFM(ch, low, false);	// Slide down
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
		if (-1 < value && value < static_cast<int>(mod_->getGrooveCount()))
			effGrooveChange(value);
	}
	else if (id == "0P") {	// Detune
		if (value != -1) opnaCtrl_->setDetuneFM(ch, value - 0x80);
	}
	else if (id == "0Q") {	// Note slide up
		if (value != -1) opnaCtrl_->setNoteSlideFM(ch, value >> 4, value & 0x0f);
	}
	else if (id == "0R") {	// Note slide down
		if (value != -1) opnaCtrl_->setNoteSlideFM(ch, value >> 4, -(value & 0x0f));
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
		if (value != -1) opnaCtrl_->setArpeggioEffectSSG(ch, value >> 4, value & 0x0f);
	}
	else if (id == "01") {	// Portamento up
		if (value != -1) opnaCtrl_->setPortamentoEffectSSG(ch, value);
	}
	else if (id == "02") {	// Portamento down
		if (value != -1) opnaCtrl_->setPortamentoEffectSSG(ch, -value);
	}
	else if (id == "03") {	// Tone portamento
		if (value != -1) opnaCtrl_->setPortamentoEffectSSG(ch, value, true);
	}
	else if (id == "04") {	// Vibrato
		if (value != -1) opnaCtrl_->setVibratoEffectSSG(ch, value >> 4, value & 0x0f);
	}
	else if (id == "07") {	// Tremolo
		if (value != -1) opnaCtrl_->setTremoloEffectSSG(ch, value >> 4, value & 0x0f);
	}
	else if (id == "0A") {	// Volume slide
		if (value != -1) {
			int hi = value >> 4;
			int low = value & 0x0f;
			if (hi && !low) opnaCtrl_->setVolumeSlideSSG(ch, hi, true);	// Slide up
			else if (!hi) opnaCtrl_->setVolumeSlideSSG(ch, low, false);	// Slide down
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
		if (-1 < value && value < static_cast<int>(mod_->getGrooveCount()))
			effGrooveChange(value);
	}
	else if (id == "0P") {	// Detune
		if (value != -1) opnaCtrl_->setDetuneSSG(ch, value - 0x80);
	}
	else if (id == "0Q") {	// Note slide up
		if (value != -1) opnaCtrl_->setNoteSlideSSG(ch, value >> 4, value & 0x0f);
	}
	else if (id == "0R") {	// Note slide down
		if (value != -1) opnaCtrl_->setNoteSlideSSG(ch, value >> 4, -(value & 0x0f));
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
		if (-1 < value && value < 4) opnaCtrl_->setPanDrum(ch, value);
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
		if (-1 < value && value < static_cast<int>(mod_->getGrooveCount()))
			effGrooveChange(value);
	}
	else if (id == "0V") {	// Master volume
		if (-1 < value && value < 64) opnaCtrl_->setMasterVolumeDrum(value);
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
		ntCutDlyCntFM_.at(static_cast<size_t>(ch)) = value;
	}
	else if (id == "0T") {	// Transpose delay
		size_t c = static_cast<size_t>(ch);
		tposeDlyCntFM_.at(c) = (value & 0x70) >> 4;
		tposeDlyValueFM_.at(c) = ((value & 0x80) ? -1 : 1) * (value & 0x0f);
	}
	else if (id.front() == 'M') {	// Volume delay
		int count = ctohex(*(id.begin() + 1));
		if (value != -1) {
			if (count > 0) {
				size_t c = static_cast<size_t>(ch);
				volDlyCntFM_.at(c) = count;
				volDlyValueFM_.at(c) = value;
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
		ntCutDlyCntSSG_.at(static_cast<size_t>(ch)) = value;
	}
	else if (id == "0T") {	// Transpose delay
		size_t c = static_cast<size_t>(ch);
		tposeDlyCntSSG_.at(c) = (value & 0x70) >> 4;
		tposeDlyValueSSG_.at(c) = ((value & 0x80) ? -1 : 1) * (value & 0x0f);
	}
	else if (id.front() == 'M') {	// Volume delay
		int count = ctohex(*(id.begin() + 1));
		if (0 <= value && value < 0x10) {
			if (count > 0) {
				size_t c = static_cast<size_t>(ch);
				volDlyCntSSG_.at(c) = count;
				volDlyValueSSG_.at(c) = value;
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
		ntCutDlyCntDrum_.at(static_cast<size_t>(ch)) = value;
	}
	else if (id.front() == 'M') {	// Volume delay
		int count = ctohex(*(id.begin() + 1));
		if (0 <= value && value < 0x20) {
			if (count > 0) {
				size_t c = static_cast<size_t>(ch);
				volDlyCntDrum_.at(c) = count;
				volDlyValueDrum_.at(c) = value;
			}
		}
	}

	return ret;
}

bool BambooTracker::effPositionJump(int nextOrder)
{
	if (nextOrder < static_cast<int>(getOrderSize(curSongNum_))) {
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
	opnaCtrl_->getStreamSamples(container, nSamples);
}

void BambooTracker::killSound()
{
	jamMan_->clear();
	opnaCtrl_->reset();
}

/********** Stream details **********/
int BambooTracker::getStreamRate() const
{
	return opnaCtrl_->getRate();
}

void BambooTracker::setStreamRate(int rate)
{
	opnaCtrl_->setRate(rate);
}

int BambooTracker::getStreamDuration() const
{
	return opnaCtrl_->getDuration();
}

void BambooTracker::setStreamDuration(int duration)
{
	opnaCtrl_->setDuration(duration);
}

int BambooTracker::getStreamTempo() const
{
	return tickCounter_.getTempo();
}

int BambooTracker::getStreamSpeed() const
{
	return tickCounter_.getSpeed();
}

bool BambooTracker::getStreamGrooveEnabled() const
{
	return tickCounter_.getGrooveEnabled();
}

void BambooTracker::setMasterVolume(int percentage)
{
	opnaCtrl_->setMasterVolume(percentage);
}

void BambooTracker::setMasterVolumeFM(double dB)
{
	opnaCtrl_->setMasterVolumeFM(dB);
}

void BambooTracker::setMasterVolumeSSG(double dB)
{
	opnaCtrl_->setMasterVolumeSSG(dB);
}

/********** Module details **********/
/*----- Module -----*/
void BambooTracker::makeNewModule()
{
	stopPlaySong();

	clearAllInstrument();

	opnaCtrl_->reset();

	mod_ = std::make_shared<Module>();

	tickCounter_.setInterruptRate(mod_->getTickFrequency());

	setCurrentSongNumber(0);
	curInstNum_ = -1;

	clearCommandHistory();
}

void BambooTracker::loadModule(std::string path)
{
	makeNewModule();

	std::exception_ptr ep;
	try {
		ModuleIO::loadModule(path, mod_, instMan_);
	}
	catch (...) {
		ep = std::current_exception();
	}

	tickCounter_.setInterruptRate(mod_->getTickFrequency());
	setCurrentSongNumber(0);
	clearCommandHistory();

	if (ep) std::rethrow_exception(ep);
}

void BambooTracker::saveModule(std::string path)
{
	ModuleIO::saveModule(path, mod_, instMan_);
}

void BambooTracker::setModulePath(std::string path)
{
	mod_->setFilePath(path);
}

std::string BambooTracker::getModulePath() const
{
	return mod_->getFilePath();
}

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

void BambooTracker::setModuleComment(std::string comment)
{
	mod_->setComment(comment);
}

std::string BambooTracker::getModuleComment() const
{
	return mod_->getComment();
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

void BambooTracker::setModuleStepHighlight1Distance(size_t dist)
{
	mod_->setStepHighlight1Distance(dist);
}

size_t BambooTracker::getModuleStepHighlight1Distance() const
{
	return mod_->getStepHighlight1Distance();
}

void BambooTracker::setModuleStepHighlight2Distance(size_t dist)
{
	mod_->setStepHighlight2Distance(dist);
}

size_t BambooTracker::getModuleStepHighlight2Distance() const
{
	return mod_->getStepHighlight2Distance();
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

void BambooTracker::clearUnusedPatterns()
{
	mod_->clearUnusedPatterns();
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

size_t BambooTracker::getAllStepCount(int songNum, int loopCnt) const
{
	int os = static_cast<int>(getOrderSize(songNum));
	int loopOrder = 0;
	int loopStep = 0;
	checkNextPositionOfLastStep(loopOrder, loopStep);
	if (loopOrder == -1) {
		size_t stepCnt = 0;
		for (int i = 0; i < os; ++i)
			stepCnt += getPatternSizeFromOrderNumber(songNum, i);
		return stepCnt;
	}
	else {
		size_t introStepCnt = 0;
		for (int i = 0; i < loopOrder; ++i)
			introStepCnt += getPatternSizeFromOrderNumber(curSongNum_, i);
		introStepCnt += static_cast<size_t>(loopStep);
		size_t loopStepCnt = getPatternSizeFromOrderNumber(curSongNum_, loopOrder) - static_cast<size_t>(loopStep);
		for (int i = loopOrder + 1; i < os; ++i) {
			loopStepCnt += getPatternSizeFromOrderNumber(songNum, i);
		}
		return introStepCnt + loopStepCnt * static_cast<size_t>(loopCnt);
	}
}

/*----- Track -----*/
void BambooTracker::setEffectDisplayWidth(int songNum, int trackNum, size_t w)
{
	mod_->getSong(songNum).getTrack(trackNum).setEffectDisplayWidth(w);
}

size_t BambooTracker::getEffectDisplayWidth(int songNum, int trackNum) const
{
	return mod_->getSong(songNum).getTrack(trackNum).getEffectDisplayWidth();
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
	size_t w = songStyle_.trackAttribs.size() - static_cast<size_t>(beginTrack);
	size_t h = getOrderSize(songNum) - static_cast<size_t>(beginOrder);

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

void BambooTracker::duplicateOrder(int songNum, int orderNum)
{
	comMan_.invoke(std::make_unique<DuplicateOrderCommand>(mod_, songNum, orderNum));
}

void BambooTracker::MoveOrder(int songNum, int orderNum, bool isUp)
{
	comMan_.invoke(std::make_unique<MoveOrderCommand>(mod_, songNum, orderNum, isUp));
}

void BambooTracker::clonePatterns(int songNum, int beginOrder, int beginTrack, int endOrder, int endTrack)
{
	comMan_.invoke(std::make_unique<ClonePatternsCommand>(mod_, songNum, beginOrder, beginTrack, endOrder, endTrack));
}

void BambooTracker::cloneOrder(int songNum, int orderNum)
{
	comMan_.invoke(std::make_unique<CloneOrderCommand>(mod_, songNum, orderNum));
}

size_t BambooTracker::getOrderSize(int songNum) const
{
	return mod_->getSong(songNum).getOrderSize();
}

bool BambooTracker::canAddNewOrder(int songNum) const
{
	return mod_->getSong(songNum).canAddNewOrder();
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
			&& (songStyle_.trackAttribs.at(static_cast<size_t>(trackNum)).source
				== instMan_->getInstrumentSharedPtr(curInstNum_)->getSoundSource()))
		in = curInstNum_;
	else
		in = -1;

	comMan_.invoke(std::make_unique<SetKeyOnToStepCommand>(mod_, songNum, trackNum, orderNum, stepNum, nn, in));
}

void BambooTracker::setStepKeyOff(int songNum, int trackNum, int orderNum, int stepNum)
{
	comMan_.invoke(std::make_unique<SetKeyOffToStepCommand>(mod_, songNum, trackNum, orderNum, stepNum));
}

void BambooTracker::setEchoBufferAccess(int songNum, int trackNum, int orderNum, int stepNum, int bufNum)
{
	comMan_.invoke(std::make_unique<SetEchoBufferAccessCommand>(mod_, songNum, trackNum, orderNum, stepNum, bufNum));
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

void BambooTracker::setStepVolume(int songNum, int trackNum, int orderNum, int stepNum, int volume, bool isFMReversed)
{	
	comMan_.invoke(std::make_unique<SetVolumeToStepCommand>(mod_, songNum, trackNum, orderNum, stepNum, volume, isFMReversed));
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
	return mod_->getSong(songNum).getTrack(trackNum).getPatternFromOrderNumber(orderNum)
			.getStep(stepNum).getEffectValue(n);
}

void BambooTracker::setStepEffectValue(int songNum, int trackNum, int orderNum, int stepNum, int n, int value, bool isFMReversed)
{
	comMan_.invoke(std::make_unique<SetEffectValueToStepCommand>(mod_, songNum, trackNum, orderNum, stepNum, n, value, isFMReversed));
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

void BambooTracker::pasteOverwritePatternCells(int songNum, int beginTrack, int beginColmn, int beginOrder,
											   int beginStep, std::vector<std::vector<std::string>> cells)
{
	std::vector<std::vector<std::string>> d
			= arrangePatternDataCells(songNum, beginTrack, beginColmn, beginOrder, beginStep, std::move(cells));

	comMan_.invoke(std::make_unique<PasteOverwriteCopiedDataToPatternCommand>(
					   mod_, songNum, beginTrack, beginColmn, beginOrder, beginStep, std::move(d)));
}

std::vector<std::vector<std::string>> BambooTracker::arrangePatternDataCells(int songNum, int beginTrack, int beginColmn, int beginOrder, int beginStep,
																			 std::vector<std::vector<std::string>> cells)
{
	std::vector<std::vector<std::string>> d;
	size_t w = (songStyle_.trackAttribs.size() - static_cast<size_t>(beginTrack) - 1) * 11
			   + (11 - static_cast<size_t>(beginColmn));
	size_t h = getPatternSizeFromOrderNumber(songNum, beginOrder) - static_cast<size_t>(beginStep);

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

void BambooTracker::interpolatePattern(int songNum, int beginTrack, int beginColmn, int beginOrder, int beginStep,
									   int endTrack, int endColmn, int endStep)
{
	comMan_.invoke(std::make_unique<InterpolatePatternCommand>(
					   mod_, songNum, beginTrack, beginColmn, beginOrder, beginStep, endTrack, endColmn, endStep));
}

void BambooTracker::reversePattern(int songNum, int beginTrack, int beginColmn, int beginOrder, int beginStep,
								   int endTrack, int endColmn, int endStep)
{
	comMan_.invoke(std::make_unique<ReversePatternCommand>(
					   mod_, songNum, beginTrack, beginColmn, beginOrder, beginStep, endTrack, endColmn, endStep));
}

void BambooTracker::replaceInstrumentInPattern(int songNum, int beginTrack, int beginOrder, int beginStep,
											   int endTrack, int endStep, int newInstNum)
{
	comMan_.invoke(std::make_unique<ReplaceInstrumentInPatternCommand>(
					   mod_, songNum, beginTrack, beginOrder, beginStep, endTrack, endStep, newInstNum));
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

	int s = static_cast<int>(size);
	if (isPlaySong() && playStepNum_ >= s) {
		playStepNum_ = s - 1;
		findNextStep();
	}
}

size_t BambooTracker::getDefaultPatternSize(int songNum) const
{
	return mod_->getSong(songNum).getDefaultPatternSize();
}

void BambooTracker::getOutputHistory(int16_t* container)
{
	opnaCtrl_->getOutputHistory(container);
}
