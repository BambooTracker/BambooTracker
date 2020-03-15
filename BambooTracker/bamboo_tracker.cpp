#include "bamboo_tracker.hpp"
#include <algorithm>
#include <utility>
#include <unordered_set>
#include <exception>
#include <unordered_map>
#include "commands.hpp"
#include "io_handlers.hpp"
#include "bank.hpp"

const uint32_t BambooTracker::CHIP_CLOCK = 3993600 * 2;

BambooTracker::BambooTracker(std::weak_ptr<Configuration> config)
	: instMan_(std::make_shared<InstrumentsManager>(config.lock()->getOverwriteUnusedUneditedPropety())),
	  tickCounter_(std::make_shared<TickCounter>()),
	  mod_(std::make_shared<Module>()),
	  octave_(4),
	  curSongNum_(0),
	  curTrackNum_(0),
	  curOrderNum_(0),
	  curStepNum_(0),
	  curInstNum_(-1),
	  isFollowPlay_(true)
{
	opnaCtrl_ = std::make_shared<OPNAController>(
					static_cast<chip::Emu>(config.lock()->getEmulator()),
					CHIP_CLOCK,
					config.lock()->getSampleRate(),
					config.lock()->getBufferLength());
	setMasterVolume(config.lock()->getMixerVolumeMaster());
	setMasterVolumeFM(config.lock()->getMixerVolumeFM());
	setMasterVolumeSSG(config.lock()->getMixerVolumeSSG());

	songStyle_ = mod_->getSong(curSongNum_).getStyle();
	jamMan_ = std::make_unique<JamManager>();

	playback_ = std::make_unique<PlaybackManager>(
					opnaCtrl_, instMan_, tickCounter_, mod_, config.lock()->getRetrieveChannelState());

	storeOnlyUsedSamples_ = config.lock()->getWriteOnlyUsedSamples();
}

/********** Change configuration **********/
void BambooTracker::changeConfiguration(std::weak_ptr<Configuration> config)
{
	setStreamRate(static_cast<int>(config.lock()->getSampleRate()));
	setStreamDuration(static_cast<int>(config.lock()->getBufferLength()));
	setMasterVolume(config.lock()->getMixerVolumeMaster());
	if (mod_->getMixerType() == MixerType::UNSPECIFIED) {
		setMasterVolumeFM(config.lock()->getMixerVolumeFM());
		setMasterVolumeSSG(config.lock()->getMixerVolumeSSG());
	}
	playback_->setChannelRetrieving(config.lock()->getRetrieveChannelState());
	instMan_->setPropertyFindMode(config.lock()->getOverwriteUnusedUneditedPropety());
	storeOnlyUsedSamples_ = config.lock()->getWriteOnlyUsedSamples();
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

void BambooTracker::loadInstrument(BinaryContainer& container, std::string path, int instNum)
{
	auto inst = InstrumentIO::loadInstrument(container, path, instMan_, instNum);
	comMan_.invoke(std::make_unique<AddInstrumentCommand>(
					   instMan_, std::unique_ptr<AbstractInstrument>(inst)));
}

void BambooTracker::saveInstrument(BinaryContainer& container, int instNum)
{
	InstrumentIO::saveInstrument(container, instMan_, instNum);
}

void BambooTracker::importInstrument(const AbstractBank &bank, size_t index, int instNum)
{
	auto inst = bank.loadInstrument(index, instMan_, instNum);
	comMan_.invoke(std::make_unique<AddInstrumentCommand>(
					   instMan_, std::unique_ptr<AbstractInstrument>(inst)));
}

void BambooTracker::exportInstruments(BinaryContainer& container, std::vector<int> instNums)
{
	BankIO::saveBank(container, instNums, instMan_);
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
	std::unordered_set<int> regdInsts = mod_->getRegisterdInstruments();
	for (auto& inst : instMan_->getInstrumentIndices()) {
		if (!regdInsts.count(inst)) unused.push_back(inst);
	}
	return unused;
}

void BambooTracker::clearUnusedInstrumentProperties()
{
	instMan_->clearUnusedInstrumentProperties();
}

std::vector<std::string> BambooTracker::getInstrumentNames() const
{
	return instMan_->getInstrumentNameList();
}

std::vector<std::vector<int>> BambooTracker::checkDuplicateInstruments() const
{
	return instMan_->checkDuplicateInstruments();
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

void BambooTracker::setArpeggioFMType(int arpNum, SequenceType type)
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

void BambooTracker::setPitchFMType(int ptNum, SequenceType type)
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
void BambooTracker::addWaveformSSGSequenceCommand(int wfNum, int type, int data)
{
	instMan_->addWaveformSSGSequenceCommand(wfNum, type, data);
}

void BambooTracker::removeWaveformSSGSequenceCommand(int wfNum)
{
	instMan_->removeWaveformSSGSequenceCommand(wfNum);
}

void BambooTracker::setWaveformSSGSequenceCommand(int wfNum, int cnt, int type, int data)
{
	instMan_->setWaveformSSGSequenceCommand(wfNum, cnt, type, data);
}

void BambooTracker::setWaveformSSGLoops(int wfNum, std::vector<int> begins, std::vector<int> ends, std::vector<int> times)
{
	instMan_->setWaveformSSGLoops(wfNum, std::move(begins), std::move(ends), std::move(times));
}

void BambooTracker::setWaveformSSGRelease(int wfNum, ReleaseType type, int begin)
{
	instMan_->setWaveformSSGRelease(wfNum, type, begin);
}

void BambooTracker::setInstrumentSSGWaveform(int instNum, int wfNum)
{
	instMan_->setInstrumentSSGWaveform(instNum, wfNum);
	opnaCtrl_->updateInstrumentSSG(instNum);
}

void BambooTracker::setInstrumentSSGWaveformEnabled(int instNum, bool enabled)
{
	instMan_->setInstrumentSSGWaveformEnabled(instNum, enabled);
	opnaCtrl_->updateInstrumentSSG(instNum);
}

std::vector<int> BambooTracker::getWaveformSSGUsers(int wfNum) const
{
	return instMan_->getWaveformSSGUsers(wfNum);
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

void BambooTracker::setArpeggioSSGType(int arpNum, SequenceType type)
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

void BambooTracker::setPitchSSGType(int ptNum, SequenceType type)
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

//--- ADPCM
size_t BambooTracker::getADPCMLimit() const
{
	return opnaCtrl_->getDRAMSize();
}


size_t BambooTracker::getADPCMStoredSize() const
{
	return opnaCtrl_->getADPCMStoredSize();
}

void BambooTracker::setWaveformADPCMRootKeyNumber(int wfNum, int n)
{
	instMan_->setWaveformADPCMRootKeyNumber(wfNum, n);
	// opnaCtrl is changed through refInstADPCM (shared_ptr)
}

void BambooTracker::setWaveformADPCMRootDeltaN(int wfNum, int dn)
{
	instMan_->setWaveformADPCMRootDeltaN(wfNum, dn);
	// opnaCtrl is changed through refInstADPCM (shared_ptr)
}

void BambooTracker::setWaveformADPCMRepeatEnabled(int wfNum, bool enabled)
{
	instMan_->setWaveformADPCMRepeatEnabled(wfNum, enabled);
	// opnaCtrl is changed through refInstADPCM (shared_ptr)
}

void BambooTracker::storeWaveformADPCMSample(int wfNum, std::vector<uint8_t> sample)
{
	instMan_->storeWaveformADPCMSample(wfNum, sample);
}

void BambooTracker::clearWaveformADPCMSample(int wfNum)
{
	instMan_->clearWaveformADPCMSample(wfNum);
}

void BambooTracker::assignWaveformADPCMSamples()
{
	opnaCtrl_->clearSamplesADPCM();
	std::vector<int> idcs = storeOnlyUsedSamples_ ? instMan_->getWaveformADPCMValidIndices()
												  : instMan_->getWaveformADPCMEntriedIndices();
	for (auto wfNum : idcs) {
		std::vector<size_t> addresses = opnaCtrl_->storeSampleADPCM(instMan_->getWaveformADPCMSamples(wfNum));
		instMan_->setWaveformADPCMStartAddress(wfNum, addresses[0]);
		instMan_->setWaveformADPCMStopAddress(wfNum, addresses[1]);
	}
}

void BambooTracker::setInstrumentADPCMWaveform(int instNum, int wfNum)
{
	instMan_->setInstrumentADPCMWaveform(instNum, wfNum);
	opnaCtrl_->updateInstrumentADPCM(instNum);
}

std::vector<int> BambooTracker::getWaveformADPCMUsers(int wfNum) const
{
	return instMan_->getWaveformADPCMUsers(wfNum);
}

void BambooTracker::addEnvelopeADPCMSequenceCommand(int envNum, int type, int data)
{
	instMan_->addEnvelopeADPCMSequenceCommand(envNum, type, data);
}

void BambooTracker::removeEnvelopeADPCMSequenceCommand(int envNum)
{
	instMan_->removeEnvelopeADPCMSequenceCommand(envNum);
}

void BambooTracker::setEnvelopeADPCMSequenceCommand(int envNum, int cnt, int type, int data)
{
	instMan_->setEnvelopeADPCMSequenceCommand(envNum, cnt, type, data);
}

void BambooTracker::setEnvelopeADPCMLoops(int envNum, std::vector<int> begins, std::vector<int> ends, std::vector<int> times)
{
	instMan_->setEnvelopeADPCMLoops(envNum, std::move(begins), std::move(ends), std::move(times));
}

void BambooTracker::setEnvelopeADPCMRelease(int envNum, ReleaseType type, int begin)
{
	instMan_->setEnvelopeADPCMRelease(envNum, type, begin);
}

void BambooTracker::setInstrumentADPCMEnvelope(int instNum, int envNum)
{
	instMan_->setInstrumentADPCMEnvelope(instNum, envNum);
	opnaCtrl_->updateInstrumentADPCM(instNum);
}

void BambooTracker::setInstrumentADPCMEnvelopeEnabled(int instNum, bool enabled)
{
	instMan_->setInstrumentADPCMEnvelopeEnabled(instNum, enabled);
	opnaCtrl_->updateInstrumentADPCM(instNum);
}

std::vector<int> BambooTracker::getEnvelopeADPCMUsers(int envNum) const
{
	return instMan_->getEnvelopeADPCMUsers(envNum);
}

void BambooTracker::setArpeggioADPCMType(int arpNum, SequenceType type)
{
	instMan_->setArpeggioADPCMType(arpNum, type);
}

void BambooTracker::addArpeggioADPCMSequenceCommand(int arpNum, int type, int data)
{
	instMan_->addArpeggioADPCMSequenceCommand(arpNum, type, data);
}

void BambooTracker::removeArpeggioADPCMSequenceCommand(int arpNum)
{
	instMan_->removeArpeggioADPCMSequenceCommand(arpNum);
}

void BambooTracker::setArpeggioADPCMSequenceCommand(int arpNum, int cnt, int type, int data)
{
	instMan_->setArpeggioADPCMSequenceCommand(arpNum, cnt, type, data);
}

void BambooTracker::setArpeggioADPCMLoops(int arpNum, std::vector<int> begins, std::vector<int> ends, std::vector<int> times)
{
	instMan_->setArpeggioADPCMLoops(arpNum, std::move(begins), std::move(ends), std::move(times));
}

void BambooTracker::setArpeggioADPCMRelease(int arpNum, ReleaseType type, int begin)
{
	instMan_->setArpeggioADPCMRelease(arpNum, type, begin);
}

void BambooTracker::setInstrumentADPCMArpeggio(int instNum, int arpNum)
{
	instMan_->setInstrumentADPCMArpeggio(instNum, arpNum);
	opnaCtrl_->updateInstrumentADPCM(instNum);
}

void BambooTracker::setInstrumentADPCMArpeggioEnabled(int instNum, bool enabled)
{
	instMan_->setInstrumentADPCMArpeggioEnabled(instNum, enabled);
	opnaCtrl_->updateInstrumentADPCM(instNum);
}

std::vector<int> BambooTracker::getArpeggioADPCMUsers(int arpNum) const
{
	return instMan_->getArpeggioADPCMUsers(arpNum);
}

void BambooTracker::setPitchADPCMType(int ptNum, SequenceType type)
{
	instMan_->setPitchADPCMType(ptNum, type);
}

void BambooTracker::addPitchADPCMSequenceCommand(int ptNum, int type, int data)
{
	instMan_->addPitchADPCMSequenceCommand(ptNum, type, data);
}

void BambooTracker::removePitchADPCMSequenceCommand(int ptNum)
{
	instMan_->removePitchADPCMSequenceCommand(ptNum);
}

void BambooTracker::setPitchADPCMSequenceCommand(int ptNum, int cnt, int type, int data)
{
	instMan_->setPitchADPCMSequenceCommand(ptNum, cnt, type, data);
}

void BambooTracker::setPitchADPCMLoops(int ptNum, std::vector<int> begins, std::vector<int> ends, std::vector<int> times)
{
	instMan_->setPitchADPCMLoops(ptNum, std::move(begins), std::move(ends), std::move(times));
}

void BambooTracker::setPitchADPCMRelease(int ptNum, ReleaseType type, int begin)
{
	instMan_->setPitchADPCMRelease(ptNum, type, begin);
}

void BambooTracker::setInstrumentADPCMPitch(int instNum, int ptNum)
{
	instMan_->setInstrumentADPCMPitch(instNum, ptNum);
	opnaCtrl_->updateInstrumentADPCM(instNum);
}

void BambooTracker::setInstrumentADPCMPitchEnabled(int instNum, bool enabled)
{
	instMan_->setInstrumentADPCMPitchEnabled(instNum, enabled);
	opnaCtrl_->updateInstrumentADPCM(instNum);
}

std::vector<int> BambooTracker::getPitchADPCMUsers(int ptNum) const
{
	return instMan_->getPitchADPCMUsers(ptNum);
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

	playback_->setSong(mod_, curSongNum_);

	/*jamMan_->clear();*/

	// Reset
	opnaCtrl_->reset();
	opnaCtrl_->setMode(songStyle_.type);
	tickCounter_->resetCount();
	tickCounter_->setTempo(song.getTempo());
	tickCounter_->setSpeed(song.getSpeed());
	tickCounter_->setGroove(mod_->getGroove(song.getGroove()).getSequence());
	tickCounter_->setGrooveTrigger(song.isUsedTempo() ? GrooveTrigger::Invalid
													  : GrooveTrigger::ValidByGlobal);

	std::unordered_map<SoundSource, int> pairs = {
		{ SoundSource::FM, getFMChannelCount(songStyle_.type) },
		{ SoundSource::SSG, 3 },
		{ SoundSource::DRUM, 6 },
		{ SoundSource::ADPCM, 1 },
	};
	for (auto& pair : pairs) {
		muteState_[pair.first] = std::vector<bool>(pair.second, false);
		for (int i = 0; i < pair.second; ++i) opnaCtrl_->setMuteState(pair.first, i, false);
	}
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
	int keyNum = octaveAndNoteToNoteNumber(octave_, JamManager::jamKeyToNote(key));
	const TrackAttribute& attrib = songStyle_.trackAttribs[static_cast<size_t>(curTrackNum_)];
	funcJamKeyOn(key, keyNum, attrib);
}

void BambooTracker::jamKeyOn(int keyNum)
{
	const TrackAttribute& attrib = songStyle_.trackAttribs[static_cast<size_t>(curTrackNum_)];
	funcJamKeyOn(JamKey::MidiKey, keyNum, attrib);
}

void BambooTracker::jamKeyOnForced(JamKey key, SoundSource src)
{
	int keyNum = octaveAndNoteToNoteNumber(octave_, JamManager::jamKeyToNote(key));
	auto it = std::find_if(songStyle_.trackAttribs.begin(), songStyle_.trackAttribs.end(),
						   [src](TrackAttribute& attrib) { return attrib.source == src; });
	funcJamKeyOn(key, keyNum, *it);
}

void BambooTracker::jamKeyOnForced(int keyNum, SoundSource src)
{
	auto it = std::find_if(songStyle_.trackAttribs.begin(), songStyle_.trackAttribs.end(),
						   [src](TrackAttribute& attrib) { return attrib.source == src; });
	funcJamKeyOn(JamKey::MidiKey, keyNum, *it);
}

void BambooTracker::funcJamKeyOn(JamKey key, int keyNum, const TrackAttribute& attrib)
{
	if (attrib.source == SoundSource::DRUM) {
		opnaCtrl_->setKeyOnFlagDrum(attrib.channelInSource);
		opnaCtrl_->updateRegisterStates();
	}
	else {
		std::vector<JamKeyData>&& list = jamMan_->keyOn(key, attrib.channelInSource, attrib.source, keyNum);
		if (list.size() == 2) {	// Key off
			JamKeyData& offData = list[1];
			switch (offData.source) {
			case SoundSource::FM:
				if (songStyle_.type == SongType::FM3chExpanded && offData.channelInSource == 2) {
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
			case SoundSource::ADPCM:
				opnaCtrl_->keyOffADPCM(true);
				break;
			default:
				break;
			}
		}

		std::shared_ptr<AbstractInstrument> tmpInst = instMan_->getInstrumentSharedPtr(curInstNum_);
		JamKeyData& onData = list.front();

		Note note;
		int octave, pitch;
		if (key == JamKey::MidiKey) {
			auto octNote = noteNumberToOctaveAndNote(onData.keyNum);
			note = octNote.second;
			octave = octNote.first;
		}
		else {
			note = JamManager::jamKeyToNote(onData.key);
			octave = JamManager::calcOctave(octave_, onData.key);
			if (octave > 7) {	// Tone range check
				octave = 7;
				note = Note::B;
			}
		}
		pitch = 0;

		switch (onData.source) {
		case SoundSource::FM:
			if (auto fm = std::dynamic_pointer_cast<InstrumentFM>(tmpInst))
				opnaCtrl_->setInstrumentFM(onData.channelInSource, fm);
			if (songStyle_.type == SongType::FM3chExpanded && onData.channelInSource == 2) {
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
		case SoundSource::ADPCM:
			if (auto adpcm = std::dynamic_pointer_cast<InstrumentADPCM>(tmpInst))
				opnaCtrl_->setInstrumentADPCM(adpcm);
			opnaCtrl_->keyOnADPCM(note, octave, pitch, true);
			break;
		default:
			break;
		}
	}
}

void BambooTracker::jamKeyOff(JamKey key)
{
	int keyNum = octaveAndNoteToNoteNumber(octave_, JamManager::jamKeyToNote(key));
	const TrackAttribute& attrib = songStyle_.trackAttribs[static_cast<size_t>(curTrackNum_)];
	funcJamKeyOff(key, keyNum, attrib);
}

void BambooTracker::jamKeyOff(int keyNum)
{
	const TrackAttribute& attrib = songStyle_.trackAttribs[static_cast<size_t>(curTrackNum_)];
	funcJamKeyOff(JamKey::MidiKey, keyNum, attrib);
}

void BambooTracker::jamKeyOffForced(JamKey key, SoundSource src)
{
	int keyNum = octaveAndNoteToNoteNumber(octave_, JamManager::jamKeyToNote(key));
	auto it = std::find_if(songStyle_.trackAttribs.begin(), songStyle_.trackAttribs.end(),
						   [src](TrackAttribute& attrib) { return attrib.source == src; });
	funcJamKeyOff(key, keyNum, *it);
}

void BambooTracker::jamKeyOffForced(int keyNum, SoundSource src)
{
	auto it = std::find_if(songStyle_.trackAttribs.begin(), songStyle_.trackAttribs.end(),
						   [src](TrackAttribute& attrib) { return attrib.source == src; });
	funcJamKeyOff(JamKey::MidiKey, keyNum, *it);
}

void BambooTracker::funcJamKeyOff(JamKey key, int keyNum, const TrackAttribute& attrib)
{
	if (attrib.source == SoundSource::DRUM) {
		opnaCtrl_->setKeyOffFlagDrum(attrib.channelInSource);
		opnaCtrl_->updateRegisterStates();
	}
	else {
		JamKeyData&& data = jamMan_->keyOff(key, keyNum);

		if (data.channelInSource > -1) {	// Key still sound
			switch (data.source) {
			case SoundSource::FM:
				if (songStyle_.type == SongType::FM3chExpanded && data.channelInSource == 2) {
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
			case SoundSource::ADPCM:
				opnaCtrl_->keyOffADPCM(true);
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
	playback_->startPlaySong(curOrderNum_);
	startPlay();

	if (isFollowPlay_) curStepNum_ = 0;
}

void BambooTracker::startPlayFromStart()
{
	playback_->startPlayFromStart();
	startPlay();

	if (isFollowPlay_) {
		curOrderNum_ = 0;
		curStepNum_ = 0;
	}
}

void BambooTracker::startPlayPattern()
{
	playback_->startPlayPattern(curOrderNum_);
	startPlay();

	if (isFollowPlay_) curStepNum_ = 0;
}

void BambooTracker::startPlayFromCurrentStep()
{
	playback_->startPlayFromCurrentStep(curOrderNum_, curStepNum_);
	startPlay();
}

void BambooTracker::startPlay()
{
	jamMan_->polyphonic(false);

	for (auto& pair : muteState_) {
		for (size_t i = 0; i < pair.second.size(); ++i) {
			opnaCtrl_->setMuteState(pair.first, static_cast<int>(i), pair.second[i]);
		}
	}
}

void BambooTracker::stopPlaySong()
{
	playback_->stopPlaySong();
	jamMan_->polyphonic(true);

	for (auto& pair : muteState_) {
		for (size_t i = 0; i < pair.second.size(); ++i) {
			opnaCtrl_->setMuteState(pair.first, static_cast<int>(i), false);
		}
	}
}

bool BambooTracker::isPlaySong() const
{
	return playback_->isPlaySong();
}

PlaybackState BambooTracker::getPlaybackState() const
{
	return playback_->getPlaybackState();
}

void BambooTracker::setTrackMuteState(int trackNum, bool isMute)
{
	auto& ta = songStyle_.trackAttribs[static_cast<size_t>(trackNum)];
	muteState_.at(ta.source).at(static_cast<size_t>(ta.channelInSource)) = isMute;
	if (isPlaySong()) opnaCtrl_->setMuteState(ta.source, ta.channelInSource, isMute);
}

bool BambooTracker::isMute(int trackNum)
{
	auto& ta = songStyle_.trackAttribs[static_cast<size_t>(trackNum)];
	return muteState_.at(ta.source).at(ta.channelInSource);
}

void BambooTracker::setFollowPlay(bool isFollowed)
{
	isFollowPlay_ = isFollowed;
	if (isFollowed) {
		int odr = playback_->getPlayingOrderNumber();
		if (odr >= 0) {
			curOrderNum_ = odr;
			curStepNum_ = playback_->getPlayingStepNumber();
		}
	}
}

bool BambooTracker::isFollowPlay() const
{
	return isFollowPlay_;
}

int BambooTracker::getPlayingOrderNumber() const
{
	return playback_->getPlayingOrderNumber();
}

int BambooTracker::getPlayingStepNumber() const
{
	return playback_->getPlayingStepNumber();
}

/********** Export **********/
bool BambooTracker::exportToWav(WavContainer& container, int loopCnt, std::function<bool()> bar)
{
	int tmpRate = opnaCtrl_->getRate();
	opnaCtrl_->setRate(static_cast<int>(container.getSampleRate()));
	size_t sampCnt = static_cast<size_t>(opnaCtrl_->getRate() * opnaCtrl_->getDuration() / 1000);
	size_t intrCnt = static_cast<size_t>(opnaCtrl_->getRate()) / mod_->getTickFrequency();
	size_t intrCntRest = 0;
	std::vector<int16_t> dumbuf(sampCnt << 1);

	int endOrder = 0;
	int endStep = 0;
	checkNextPositionOfLastStep(endOrder, endStep);
	bool endFlag = false;
	bool tmpFollow = std::exchange(isFollowPlay_, false);
	std::shared_ptr<chip::WavExportContainer> exCntr = std::make_shared<chip::WavExportContainer>();
	opnaCtrl_->setExportContainer(exCntr);
	startPlayFromStart();

	while (true) {
		size_t sampCntRest = sampCnt;
		while (sampCntRest) {
			if (!intrCntRest) {	// Interruption
				intrCntRest = intrCnt;    // Set counts to next interruption

				if (!streamCountUp()) {
					if (bar()) {	// Update lambda function
						stopPlaySong();
						isFollowPlay_ = tmpFollow;
						return false;
					}

					int playOrder = playback_->getPlayingOrderNumber();
					int playStep = playback_->getPlayingStepNumber();
					if ((playOrder == -1 && playStep == -1)
							|| (playOrder == endOrder && playStep == endStep && !(loopCnt--))){
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
	opnaCtrl_->setRate(tmpRate);

	container.storeSample(exCntr->getStream());

	return true;
}

bool BambooTracker::exportToVgm(BinaryContainer& container, int target, bool gd3TagEnabled,
								GD3Tag tag, std::function<bool()> bar)
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
	bool tmpFollow = std::exchange(isFollowPlay_, false);
	uint32_t loopPoint = 0;
	uint32_t loopPointSamples = 0;

	std::shared_ptr<chip::VgmExportContainer> exCntr
			= std::make_shared<chip::VgmExportContainer>(target, mod_->getTickFrequency());

	// Set ADPCM
	opnaCtrl_->clearSamplesADPCM();
	std::vector<uint8_t> rom;
	for (auto wfNum : instMan_->getWaveformADPCMValidIndices()) {
		std::vector<uint8_t> sample = instMan_->getWaveformADPCMSamples(wfNum);
		std::vector<size_t> addresses = opnaCtrl_->storeSampleADPCM(sample);
		instMan_->setWaveformADPCMStartAddress(wfNum, addresses[0]);
		instMan_->setWaveformADPCMStopAddress(wfNum, addresses[1]);

		rom.resize((addresses[1] + 1) << 5);
		std::copy(sample.begin(), sample.end(), rom.begin() + static_cast<int>(addresses[0] << 5));
	}
	exCntr->setDataBlock(std::move(rom));

	opnaCtrl_->setExportContainer(exCntr);
	startPlayFromStart();
	exCntr->forceMoveLoopPoint();

	while (true) {
		if (!streamCountUp()) {
			if (bar()) {	// Update lambda function
				stopPlaySong();
				isFollowPlay_ = tmpFollow;
				return false;
			}

			int playOrder = playback_->getPlayingOrderNumber();
			int playStep = playback_->getPlayingStepNumber();
			if (playOrder == loopOrder && playStep == loopStep && !(endCnt--)) break;

			if (loopFlag && loopOrder == playOrder && loopStep == playStep) {
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
		ExportHandler::writeVgm(container, target, exCntr->getData(), CHIP_CLOCK, mod_->getTickFrequency(),
								loopFlag, loopPoint, exCntr->getSampleLength() - loopPointSamples,
								exCntr->getSampleLength(), gd3TagEnabled, tag);
		return true;
	} catch (...) {
		throw;
	}
}

bool BambooTracker::exportToS98(BinaryContainer& container, int target, bool tagEnabled,
								S98Tag tag, int rate, std::function<bool()> bar)
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
	bool tmpFollow = std::exchange(isFollowPlay_, false);
	uint32_t loopPoint = 0;
	std::shared_ptr<chip::S98ExportContainer> exCntr = std::make_shared<chip::S98ExportContainer>(target);
	opnaCtrl_->setExportContainer(exCntr);
	startPlayFromStart();
	assignWaveformADPCMSamples();
	exCntr->forceMoveLoopPoint();

	while (true) {
		exCntr->getData();	// Set wait counts
		if (!streamCountUp()) {
			if (bar()) {	// Update lambda function
				stopPlaySong();
				isFollowPlay_ = tmpFollow;
				return false;
			}

			int playOrder = playback_->getPlayingOrderNumber();
			int playStep = playback_->getPlayingStepNumber();
			if (playOrder == loopOrder && playStep == loopStep && !(endCnt--)) break;

			if (loopFlag && loopOrder == playOrder && loopStep == playStep) {
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
		ExportHandler::writeS98(container, target, exCntr->getData(), CHIP_CLOCK, static_cast<uint32_t>(rate),
								loopFlag, loopPoint, tagEnabled, tag);
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
			Effect&& eff = Effect::makeEffectData(attrib.source, step.getEffectID(i), step.getEffectValue(i));
			switch (eff.type) {
			case EffectType::PositionJump:
				if (eff.value <= lastOrder) {
					endOrder = eff.value;
					endStep = 0;
				}
				break;
			case EffectType::SongEnd:
				endOrder = -1;
				endStep = -1;
				break;
			case EffectType::PatternBreak:
				if (eff.value < static_cast<int>(getPatternSizeFromOrderNumber(curSongNum_, 0))) {
					endOrder = 0;
					endStep = eff.value;
				}
				break;
			default:
				break;
			}
		}
	}
}

/********** Real chip interface **********/
void BambooTracker::useSCCI(scci::SoundInterfaceManager* manager)
{
	opnaCtrl_->useSCCI(manager);
}

void BambooTracker::useC86CTL(C86ctlBase* base)
{
	opnaCtrl_->useC86CTL(base);
}

RealChipInterface BambooTracker::getRealChipinterface() const
{
	if (opnaCtrl_->isUsedSCCI()) return RealChipInterface::SCCI;
	else if (opnaCtrl_->isUsedC86CTL()) return RealChipInterface::C86CTL;
	else return RealChipInterface::NONE;
}

/********** Stream events **********/
int BambooTracker::streamCountUp()
{
	int state = playback_->streamCountUp();
	if (!state && isFollowPlay_) {	// Step
		int odr = playback_->getPlayingOrderNumber();
		if (odr >= 0) {
			curOrderNum_ = odr;
			curStepNum_ = playback_->getPlayingStepNumber();
		}
	}
	return state;
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
	return tickCounter_->getTempo();
}

int BambooTracker::getStreamSpeed() const
{
	return tickCounter_->getSpeed();
}

bool BambooTracker::getStreamGrooveEnabled() const
{
	return tickCounter_->getGrooveEnabled();
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

	tickCounter_->setInterruptRate(mod_->getTickFrequency());

	setCurrentSongNumber(0);
	curInstNum_ = -1;

	clearCommandHistory();
}

void BambooTracker::loadModule(BinaryContainer& container)
{
	makeNewModule();

	std::exception_ptr ep;
	try {
		ModuleIO::loadModule(container, mod_, instMan_);
	}
	catch (...) {
		ep = std::current_exception();
	}

	tickCounter_->setInterruptRate(mod_->getTickFrequency());
	setCurrentSongNumber(0);
	clearCommandHistory();

	if (ep) std::rethrow_exception(ep);
}

void BambooTracker::saveModule(BinaryContainer& container)
{
	ModuleIO::saveModule(container, mod_, instMan_);
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
	tickCounter_->setInterruptRate(freq);
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

void BambooTracker::setModuleMixerType(MixerType type)
{
	mod_->setMixerType(type);
}

MixerType BambooTracker::getModuleMixerType() const
{
	return mod_->getMixerType();
}

void BambooTracker::setModuleCustomMixerFMLevel(double level)
{
	mod_->setCustomMixerFMLevel(level);
}

double BambooTracker::getModuleCustomMixerFMLevel() const
{
	return mod_->getCustomMixerFMLevel();
}

void BambooTracker::setModuleCustomMixerSSGLevel(double level)
{
	mod_->setCustomMixerSSGLevel(level);
}

double BambooTracker::getModuleCustomMixerSSGLevel() const
{
	return mod_->getCustomMixerSSGLevel();
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

void BambooTracker::replaceDuplicateInstrumentsInPatterns(std::vector<std::vector<int>> list)
{
	std::unordered_map<int, int> map;
	for (auto& group : list) {
		for (size_t i = 1; i < group.size(); ++i) {
			map.emplace(group[i], group.front());
		}
	}

	mod_->replaceDuplicateInstrumentsInPatterns(map);
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
	if (curSongNum_ == songNum) tickCounter_->setTempo(tempo);
}

int BambooTracker::getSongTempo(int songNum) const
{
	return mod_->getSong(songNum).getTempo();
}

void BambooTracker::setSongGroove(int songNum, int groove)
{
	mod_->getSong(songNum).setGroove(groove);
	tickCounter_->setGroove(mod_->getGroove(groove).getSequence());
}

int BambooTracker::getSongGroove(int songNum) const
{
	return mod_->getSong(songNum).getGroove();
}

void BambooTracker::toggleTempoOrGrooveInSong(int songNum, bool isTempo)
{
	mod_->getSong(songNum).toggleTempoOrGroove(isTempo);
	tickCounter_->setGrooveTrigger(isTempo ? GrooveTrigger::Invalid
										   : GrooveTrigger::ValidByGlobal);
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
	if (curSongNum_ == songNum) tickCounter_->setSpeed(speed);
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

/*----- Bookmark -----*/
void BambooTracker::addBookmark(int songNum, std::string name, int order, int step)
{
	mod_->getSong(songNum).addBookmark(name, order, step);
}

void BambooTracker::changeBookmark(int songNum, int i, std::string name, int order, int step)
{
	mod_->getSong(songNum).changeBookmark(i, name, order, step);
}

void BambooTracker::removeBookmark(int songNum, int i)
{
	mod_->getSong(songNum).removeBookmark(i);
}

void BambooTracker::clearBookmark(int songNum)
{
	mod_->getSong(songNum).clearBookmark();
}

void BambooTracker::swapBookmarks(int songNum, int a, int b)
{
	mod_->getSong(songNum).swapBookmarks(a, b);
}

void BambooTracker::sortBookmarkByPosition(int songNum)
{
	mod_->getSong(songNum).sortBookmarkByPosition();
}
void BambooTracker::sortBookmarkByName(int songNum)
{
	mod_->getSong(songNum).sortBookmarkByName();
}

Bookmark BambooTracker::getBookmark(int songNum, int i) const
{
	return mod_->getSong(songNum).getBookmark(i);
}

std::vector<int> BambooTracker::findBookmarks(int songNum, int order, int step)
{
	return mod_->getSong(songNum).findBookmarks(order, step);
}

Bookmark BambooTracker::getPreviousBookmark(int songNum, int order, int step)
{
	return mod_->getSong(songNum).getPreviousBookmark(order, step);
}

Bookmark BambooTracker::getNextBookmark(int songNum, int order, int step)
{
	return mod_->getSong(songNum).getNextBookmark(order, step);
}

size_t BambooTracker::getBookmarkSize(int songNum) const
{
	return mod_->getSong(songNum).getBookmarkSize();
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

void BambooTracker::setOrderPatternDigit(int songNum, int trackNum, int orderNum, int patternNum, bool secondEntry)
{
	comMan_.invoke(std::make_unique<SetPatternToOrderCommand>(mod_, songNum, trackNum, orderNum, patternNum, secondEntry));
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

void BambooTracker::setStepNote(int songNum, int trackNum, int orderNum, int stepNum, int octave, Note note, bool autosetInst)
{
	int nn = octaveAndNoteToNoteNumber(octave, note);

	int in = -1;
	if (autosetInst && curInstNum_ != -1
			&& (songStyle_.trackAttribs.at(static_cast<size_t>(trackNum)).source
				== instMan_->getInstrumentSharedPtr(curInstNum_)->getSoundSource()))
		in = curInstNum_;

	comMan_.invoke(std::make_unique<SetKeyOnToStepCommand>(mod_, songNum, trackNum, orderNum, stepNum, nn, autosetInst, in));
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

void BambooTracker::setStepInstrumentDigit(int songNum, int trackNum, int orderNum, int stepNum, int instNum, bool secondEntry)
{
	comMan_.invoke(std::make_unique<SetInstrumentToStepCommand>(mod_, songNum, trackNum, orderNum, stepNum, instNum, secondEntry));
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

void BambooTracker::setStepVolumeDigit(int songNum, int trackNum, int orderNum, int stepNum, int volume, bool isFMReversed, bool secondEntry)
{	
	comMan_.invoke(std::make_unique<SetVolumeToStepCommand>(mod_, songNum, trackNum, orderNum, stepNum, volume, isFMReversed, secondEntry));
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

void BambooTracker::setStepEffectIDCharacter(int songNum, int trackNum, int orderNum, int stepNum, int n, std::string id, bool fillValue00, bool secondEntry)
{
	comMan_.invoke(std::make_unique<SetEffectIDToStepCommand>(mod_, songNum, trackNum, orderNum, stepNum, n, id, fillValue00, secondEntry));
}

int BambooTracker::getStepEffectValue(int songNum, int trackNum, int orderNum, int stepNum, int n) const
{
	return mod_->getSong(songNum).getTrack(trackNum).getPatternFromOrderNumber(orderNum)
			.getStep(stepNum).getEffectValue(n);
}

void BambooTracker::setStepEffectValueDigit(int songNum, int trackNum, int orderNum, int stepNum, int n, int value, EffectDisplayControl ctrl, bool secondEntry)
{
	comMan_.invoke(std::make_unique<SetEffectValueToStepCommand>(mod_, songNum, trackNum, orderNum, stepNum, n, value, ctrl, secondEntry));
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

void BambooTracker::transposeNoteKeyInPattern(int songNum, int beginTrack, int beginOrder, int beginStep,
											 int endTrack, int endStep, int seminote)
{
	comMan_.invoke(std::make_unique<TransposeNoteKeyInPatternCommand>(
					   mod_, songNum, beginTrack, beginOrder, beginStep, endTrack, endStep, seminote));
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
	return mod_->getSong(songNum).getPatternSizeFromOrderNumber(orderNum);
}

void BambooTracker::setDefaultPatternSize(int songNum, size_t size)
{
	mod_->getSong(songNum).setDefaultPatternSize(size);
	playback_->checkPlayPosition(static_cast<int>(size));
}

size_t BambooTracker::getDefaultPatternSize(int songNum) const
{
	return mod_->getSong(songNum).getDefaultPatternSize();
}

void BambooTracker::getOutputHistory(int16_t* container)
{
	opnaCtrl_->getOutputHistory(container);
}
