/*
 * SPDX-FileCopyrightText: 2018 Rerrah
 * SPDX-License-Identifier: MIT
 */

#include "bamboo_tracker.hpp"
#include <algorithm>
#include <utility>
#include <exception>
#include <iterator>
#include <unordered_map>
#include <unordered_set>
#include "configuration.hpp"
#include "opna_controller.hpp"
#include "playback.hpp"
#include "tick_counter.hpp"
#include "command/commands.hpp"
#include "chip/register_write_logger.hpp"
#include "io/module_io.hpp"
#include "io/instrument_io.hpp"
#include "io/bank_io.hpp"
#include "bank.hpp"
#include "note.hpp"
#include "song_length_calculator.hpp"
#include "utils.hpp"

namespace
{
const uint32_t CHIP_CLOCK = 3993600 * 2;
}

BambooTracker::BambooTracker(std::weak_ptr<Configuration> config)
	: instMan_(std::make_shared<InstrumentsManager>(config.lock()->getOverwriteUnusedUneditedPropety())),
	  jamMan_(std::make_unique<JamManager>()),
	  tickCounter_(std::make_shared<TickCounter>()),
	  mod_(std::make_shared<Module>()),
	  curOctave_(Note::DEFAULT_OCTAVE),
	  curSongNum_(0),
	  curTrackNum_(0),
	  curOrderNum_(0),
	  curStepNum_(0),
	  curInstNum_(-1),
	  curVolume_(127),
	  mkOrder_(-1),
	  mkStep_(-1),
	  isFollowPlay_(true)
{
	opnaCtrl_ = std::make_shared<OPNAController>(
					static_cast<chip::OpnaEmulator>(config.lock()->getEmulator()),
					CHIP_CLOCK,
					config.lock()->getSampleRate(),
					config.lock()->getBufferLength(),
					config.lock()->getResamplerType());
	opnaCtrl_->setImmediateWriteMode(config.lock()->getImmediateWriteModeEnabled());
	setMasterVolume(config.lock()->getMixerVolumeMaster());
	setMasterVolumeFM(config.lock()->getMixerVolumeFM());
	setMasterVolumeSSG(config.lock()->getMixerVolumeSSG());

	songStyle_ = mod_->getSong(curSongNum_).getStyle();

	playback_ = std::make_unique<PlaybackManager>(
					opnaCtrl_, instMan_, tickCounter_, mod_, config.lock()->getRetrieveChannelState());

	storeOnlyUsedSamples_ = config.lock()->getWriteOnlyUsedSamples();
	volFMReversed_ = config.lock()->getReverseFMVolumeOrder();

	makeNewModule();
}

BambooTracker::~BambooTracker() = default;

/********** Change configuration **********/
void BambooTracker::changeConfiguration(std::weak_ptr<Configuration> config)
{
	setStreamRate(static_cast<int>(config.lock()->getSampleRate()));
	setStreamDuration(static_cast<int>(config.lock()->getBufferLength()));
	opnaCtrl_->setImmediateWriteMode(config.lock()->getImmediateWriteModeEnabled());
	opnaCtrl_->setResampler(config.lock()->getResamplerType());
	setMasterVolume(config.lock()->getMixerVolumeMaster());
	if (mod_->getMixerType() == MixerType::UNSPECIFIED) {
		setMasterVolumeFM(config.lock()->getMixerVolumeFM());
		setMasterVolumeSSG(config.lock()->getMixerVolumeSSG());
	}
	playback_->setChannelRetrieving(config.lock()->getRetrieveChannelState());
	instMan_->setPropertyFindMode(config.lock()->getOverwriteUnusedUneditedPropety());
	storeOnlyUsedSamples_ = config.lock()->getWriteOnlyUsedSamples();
	volFMReversed_ = config.lock()->getReverseFMVolumeOrder();
}

/********** Current octave **********/
void BambooTracker::setCurrentOctave(int octave)
{
	curOctave_ = octave;
}

int BambooTracker::getCurrentOctave() const
{
	return curOctave_;
}

/********** Current volume **********/
void BambooTracker::setCurrentVolume(int volume)
{
	curVolume_ = volume;
}

int BambooTracker::getCurrentVolume() const
{
	return curVolume_;
}

/********** Current track **********/
void BambooTracker::setCurrentTrack(int num)
{
	curTrackNum_ = num;
}

TrackAttribute BambooTracker::getCurrentTrackAttribute() const
{
	return songStyle_.trackAttribs.at(static_cast<size_t>(curTrackNum_));
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
void BambooTracker::addInstrument(int num, InstrumentType type, const std::string& name)
{
	comMan_.invoke(std::make_unique<AddInstrumentCommand>(instMan_, num, type, name));
}

void BambooTracker::removeInstrument(int num)
{
	comMan_.invoke(std::make_unique<RemoveInstrumentCommand>(instMan_, num));
}

std::unique_ptr<AbstractInstrument> BambooTracker::getInstrument(int num)
{
	std::shared_ptr<AbstractInstrument> inst = instMan_->getInstrumentSharedPtr(num);
	if (inst == nullptr) return std::unique_ptr<AbstractInstrument>();
	else return std::unique_ptr<AbstractInstrument>(inst->clone());
}

void BambooTracker::cloneInstrument(int num, int refNum)
{
	comMan_.invoke(std::make_unique<cloneInstrumentCommand>(instMan_, num, refNum));
}

void BambooTracker::deepCloneInstrument(int num, int refNum)
{
	comMan_.invoke(std::make_unique<DeepCloneInstrumentCommand>(instMan_, num, refNum));
}

void BambooTracker::swapInstruments(int a, int b, bool patternChange)
{
	comMan_.invoke(std::make_unique<SwapInstrumentsCommand>(instMan_, mod_, a, b, patternChange));
}

void BambooTracker::loadInstrument(io::BinaryContainer& container, const std::string& path, int instNum)
{
	AbstractInstrument* inst = io::InstrumentIO::getInstance().loadInstrument(container, path, instMan_, instNum);
	comMan_.invoke(std::make_unique<AddInstrumentCommand>(instMan_, std::unique_ptr<AbstractInstrument>(inst)));
}

void BambooTracker::saveInstrument(io::BinaryContainer& container, int instNum)
{
	io::InstrumentIO::getInstance().saveInstrument(container, instMan_, instNum);
}

void BambooTracker::importInstrument(const AbstractBank &bank, size_t index, int instNum)
{
	AbstractInstrument* inst = bank.loadInstrument(index, instMan_, instNum);
	comMan_.invoke(std::make_unique<AddInstrumentCommand>(
					   instMan_, std::unique_ptr<AbstractInstrument>(inst)));
}

void BambooTracker::exportInstruments(io::BinaryContainer& container, const std::vector<int>& instNums)
{
	io::BankIO::getInstance().saveBank(container, instMan_, instNums);
}

int BambooTracker::findFirstFreeInstrumentNumber() const
{
	return instMan_->findFirstFreeInstrument();
}

void BambooTracker::setInstrumentName(int num, const std::string& name)
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
	std::vector<int> instIdcs = instMan_->getInstrumentIndices();
	std::set<int> regdInsts = mod_->getRegisterdInstruments();

	std::vector<int> unused;
	std::set_difference(instIdcs.begin(), instIdcs.end(), regdInsts.begin(), regdInsts.end(), std::back_inserter(unused));
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

std::multiset<int> BambooTracker::getEnvelopeFMUsers(int envNum) const
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

std::multiset<int> BambooTracker::getLFOFMUsers(int lfoNum) const
{
	return instMan_->getLFOFMUsers(lfoNum);
}

void BambooTracker::addOperatorSequenceFMSequenceData(FMEnvelopeParameter param, int opSeqNum, int data)
{
	instMan_->addOperatorSequenceFMSequenceData(param, opSeqNum, data);
}

void BambooTracker::removeOperatorSequenceFMSequenceData(FMEnvelopeParameter param, int opSeqNum)
{
	instMan_->removeOperatorSequenceFMSequenceData(param, opSeqNum);
}

void BambooTracker::setOperatorSequenceFMSequenceData(FMEnvelopeParameter param, int opSeqNum, int cnt, int data)
{
	instMan_->setOperatorSequenceFMSequenceData(param, opSeqNum, cnt, data);
}

void BambooTracker::addOperatorSequenceFMLoop(FMEnvelopeParameter param, int opSeqNum, const InstrumentSequenceLoop& loop)
{
	instMan_->addOperatorSequenceFMLoop(param, opSeqNum, loop);
}

void BambooTracker::removeOperatorSequenceFMLoop(FMEnvelopeParameter param, int opSeqNum, int begin, int end)
{
	instMan_->removeOperatorSequenceFMLoop(param, opSeqNum, begin, end);
}

void BambooTracker::changeOperatorSequenceFMLoop(FMEnvelopeParameter param, int opSeqNum, int prevBegin, int prevEnd, const InstrumentSequenceLoop& loop)
{
	instMan_->changeOperatorSequenceFMLoop(param, opSeqNum, prevBegin, prevEnd, loop);
}

void BambooTracker::clearOperatorSequenceFMLoops(FMEnvelopeParameter param, int opSeqNum)
{
	instMan_->clearOperatorSequenceFMLoops(param, opSeqNum);
}

void BambooTracker::setOperatorSequenceFMRelease(FMEnvelopeParameter param, int opSeqNum, const InstrumentSequenceRelease& release)
{
	instMan_->setOperatorSequenceFMRelease(param, opSeqNum, release);
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

std::multiset<int> BambooTracker::getOperatorSequenceFMUsers(FMEnvelopeParameter param, int opSeqNum) const
{
	return instMan_->getOperatorSequenceFMUsers(param, opSeqNum);
}

void BambooTracker::setArpeggioFMType(int arpNum, SequenceType type)
{
	instMan_->setArpeggioFMType(arpNum, type);
}

void BambooTracker::addArpeggioFMSequenceData(int arpNum, int data)
{
	instMan_->addArpeggioFMSequenceData(arpNum, data);
}

void BambooTracker::removeArpeggioFMSequenceData(int arpNum)
{
	instMan_->removeArpeggioFMSequenceData(arpNum);
}

void BambooTracker::setArpeggioFMSequenceData(int arpNum, int cnt, int data)
{
	instMan_->setArpeggioFMSequenceData(arpNum, cnt, data);
}

void BambooTracker::addArpeggioFMLoop(int arpNum, const InstrumentSequenceLoop& loop)
{
	instMan_->addArpeggioFMLoop(arpNum, loop);
}

void BambooTracker::removeArpeggioFMLoop(int arpNum, int begin, int end)
{
	instMan_->removeArpeggioFMLoop(arpNum, begin, end);
}

void BambooTracker::changeArpeggioFMLoop(int arpNum, int prevBegin, int prevEnd, const InstrumentSequenceLoop& loop)
{
	instMan_->changeArpeggioFMLoop(arpNum, prevBegin, prevEnd, loop);
}

void BambooTracker::clearArpeggioFMLoops(int arpNum)
{
	instMan_->clearArpeggioFMLoops(arpNum);
}

void BambooTracker::setArpeggioFMRelease(int arpNum, const InstrumentSequenceRelease& release)
{
	instMan_->setArpeggioFMRelease(arpNum, release);
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

std::multiset<int> BambooTracker::getArpeggioFMUsers(int arpNum) const
{
	return instMan_->getArpeggioFMUsers(arpNum);
}

void BambooTracker::setPitchFMType(int ptNum, SequenceType type)
{
	instMan_->setPitchFMType(ptNum, type);
}

void BambooTracker::addPitchFMSequenceData(int ptNum, int data)
{
	instMan_->addPitchFMSequenceData(ptNum, data);
}

void BambooTracker::removePitchFMSequenceData(int ptNum)
{
	instMan_->removePitchFMSequenceData(ptNum);
}

void BambooTracker::setPitchFMSequenceData(int ptNum, int cnt, int data)
{
	instMan_->setPitchFMSequenceData(ptNum, cnt, data);
}

void BambooTracker::addPitchFMLoop(int ptNum, const InstrumentSequenceLoop& loop)
{
	instMan_->addPitchFMLoop(ptNum, loop);
}

void BambooTracker::removePitchFMLoop(int ptNum, int begin, int end)
{
	instMan_->removePitchFMLoop(ptNum, begin, end);
}

void BambooTracker::changePitchFMLoop(int ptNum, int prevBegin, int prevEnd, const InstrumentSequenceLoop& loop)
{
	instMan_->changePitchFMLoop(ptNum, prevBegin, prevEnd, loop);
}

void BambooTracker::clearPitchFMLoops(int ptNum)
{
	instMan_->clearPitchFMLoops(ptNum);
}

void BambooTracker::setPitchFMRelease(int ptNum, const InstrumentSequenceRelease& release)
{
	instMan_->setPitchFMRelease(ptNum, release);
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

std::multiset<int> BambooTracker::getPitchFMUsers(int ptNum) const
{
	return instMan_->getPitchFMUsers(ptNum);
}

void BambooTracker::addPanFMSequenceData(int panNum, int data)
{
	instMan_->addPanFMSequenceData(panNum, data);
}

void BambooTracker::removePanFMSequenceData(int panNum)
{
	instMan_->removePanFMSequenceData(panNum);
}

void BambooTracker::setPanFMSequenceData(int panNum, int cnt, int data)
{
	instMan_->setPanFMSequenceData(panNum, cnt, data);
}

void BambooTracker::addPanFMLoop(int panNum, const InstrumentSequenceLoop& loop)
{
	instMan_->addPanFMLoop(panNum, loop);
}

void BambooTracker::removePanFMLoop(int panNum, int begin, int end)
{
	instMan_->removePanFMLoop(panNum, begin, end);
}

void BambooTracker::changePanFMLoop(int panNum, int prevBegin, int prevEnd, const InstrumentSequenceLoop& loop)
{
	instMan_->changePanFMLoop(panNum, prevBegin, prevEnd, loop);
}

void BambooTracker::clearPanFMLoops(int panNum)
{
	instMan_->clearPanFMLoops(panNum);
}

void BambooTracker::setPanFMRelease(int panNum, const InstrumentSequenceRelease& release)
{
	instMan_->setPanFMRelease(panNum, release);
}

void BambooTracker::setInstrumentFMPan(int instNum, int panNum)
{
	instMan_->setInstrumentFMPan(instNum, panNum);
	opnaCtrl_->updateInstrumentFM(instNum);
}

void BambooTracker::setInstrumentFMPanEnabled(int instNum, bool enabled)
{
	instMan_->setInstrumentFMPanEnabled(instNum, enabled);
	opnaCtrl_->updateInstrumentFM(instNum);
}

std::multiset<int> BambooTracker::getPanFMUsers(int panNum) const
{
	return instMan_->getPanFMUsers(panNum);
}

void BambooTracker::setInstrumentFMEnvelopeResetEnabled(int instNum, FMOperatorType op, bool enabled)
{
	instMan_->setInstrumentFMEnvelopeResetEnabled(instNum, op, enabled);
	opnaCtrl_->updateInstrumentFM(instNum);
}

//--- SSG
void BambooTracker::addWaveformSSGSequenceData(int wfNum, const SSGWaveformUnit& data)
{
	instMan_->addWaveformSSGSequenceData(wfNum, data);
}

void BambooTracker::removeWaveformSSGSequenceData(int wfNum)
{
	instMan_->removeWaveformSSGSequenceData(wfNum);
}

void BambooTracker::setWaveformSSGSequenceData(int wfNum, int cnt, const SSGWaveformUnit& data)
{
	instMan_->setWaveformSSGSequenceData(wfNum, cnt, data);
}

void BambooTracker::addWaveformSSGLoop(int wfNum, const InstrumentSequenceLoop& loop)
{
	instMan_->addWaveformSSGLoop(wfNum, loop);
}

void BambooTracker::removeWaveformSSGLoop(int wfNum, int begin, int end)
{
	instMan_->removeWaveformSSGLoop(wfNum, begin, end);
}

void BambooTracker::changeWaveformSSGLoop(int wfNum, int prevBegin, int prevEnd, const InstrumentSequenceLoop& loop)
{
	instMan_->changeWaveformSSGLoop(wfNum, prevBegin, prevEnd, loop);
}

void BambooTracker::clearWaveformSSGLoops(int wfNum)
{
	instMan_->clearWaveformSSGLoops(wfNum);
}

void BambooTracker::setWaveformSSGRelease(int wfNum, const InstrumentSequenceRelease& release)
{
	instMan_->setWaveformSSGRelease(wfNum, release);
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

std::multiset<int> BambooTracker::getWaveformSSGUsers(int wfNum) const
{
	return instMan_->getWaveformSSGUsers(wfNum);
}

void BambooTracker::addToneNoiseSSGSequenceData(int tnNum, int data)
{
	instMan_->addToneNoiseSSGSequenceData(tnNum, data);
}

void BambooTracker::removeToneNoiseSSGSequenceData(int tnNum)
{
	instMan_->removeToneNoiseSSGSequenceData(tnNum);
}

void BambooTracker::setToneNoiseSSGSequenceData(int tnNum, int cnt, int data)
{
	instMan_->setToneNoiseSSGSequenceData(tnNum, cnt, data);
}

void BambooTracker::addToneNoiseSSGLoop(int tnNum, const InstrumentSequenceLoop& loop)
{
	instMan_->addToneNoiseSSGLoop(tnNum, loop);
}

void BambooTracker::removeToneNoiseSSGLoop(int tnNum, int begin, int end)
{
	instMan_->removeToneNoiseSSGLoop(tnNum, begin, end);
}

void BambooTracker::changeToneNoiseSSGLoop(int tnNum, int prevBegin, int prevEnd, const InstrumentSequenceLoop& loop)
{
	instMan_->changeToneNoiseSSGLoop(tnNum, prevBegin, prevEnd, loop);
}

void BambooTracker::clearToneNoiseSSGLoops(int tnNum)
{
	instMan_->clearToneNoiseSSGLoops(tnNum);
}

void BambooTracker::setToneNoiseSSGRelease(int tnNum, const InstrumentSequenceRelease& release)
{
	instMan_->setToneNoiseSSGRelease(tnNum, release);
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

std::multiset<int> BambooTracker::getToneNoiseSSGUsers(int tnNum) const
{
	return instMan_->getToneNoiseSSGUsers(tnNum);
}

void BambooTracker::addEnvelopeSSGSequenceData(int envNum, const SSGEnvelopeUnit& data)
{
	instMan_->addEnvelopeSSGSequenceData(envNum, data);
}

void BambooTracker::removeEnvelopeSSGSequenceData(int envNum)
{
	instMan_->removeEnvelopeSSGSequenceData(envNum);
}

void BambooTracker::setEnvelopeSSGSequenceData(int envNum, int cnt, const SSGEnvelopeUnit& data)
{
	instMan_->setEnvelopeSSGSequenceData(envNum, cnt, data);
}

void BambooTracker::addEnvelopeSSGLoop(int envNum, const InstrumentSequenceLoop& loop)
{
	instMan_->addEnvelopeSSGLoop(envNum, loop);
}

void BambooTracker::removeEnvelopeSSGLoop(int envNum, int begin, int end)
{
	instMan_->removeEnvelopeSSGLoop(envNum, begin, end);
}

void BambooTracker::changeEnvelopeSSGLoop(int envNum, int prevBegin, int prevEnd, const InstrumentSequenceLoop& loop)
{
	instMan_->changeEnvelopeSSGLoop(envNum, prevBegin, prevEnd, loop);
}

void BambooTracker::clearEnvelopeSSGLoops(int envNum)
{
	instMan_->clearEnvelopeSSGLoops(envNum);
}

void BambooTracker::setEnvelopeSSGRelease(int envNum, const InstrumentSequenceRelease& release)
{
	instMan_->setEnvelopeSSGRelease(envNum, release);
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

std::multiset<int> BambooTracker::getEnvelopeSSGUsers(int envNum) const
{
	return instMan_->getEnvelopeSSGUsers(envNum);
}

void BambooTracker::setArpeggioSSGType(int arpNum, SequenceType type)
{
	instMan_->setArpeggioSSGType(arpNum, type);
}

void BambooTracker::addArpeggioSSGSequenceData(int arpNum, int data)
{
	instMan_->addArpeggioSSGSequenceData(arpNum, data);
}

void BambooTracker::removeArpeggioSSGSequenceData(int arpNum)
{
	instMan_->removeArpeggioSSGSequenceData(arpNum);
}

void BambooTracker::setArpeggioSSGSequenceData(int arpNum, int cnt, int data)
{
	instMan_->setArpeggioSSGSequenceData(arpNum, cnt, data);
}

void BambooTracker::addArpeggioSSGLoop(int arpNum, const InstrumentSequenceLoop& loop)
{
	instMan_->addArpeggioSSGLoop(arpNum, loop);
}

void BambooTracker::removeArpeggioSSGLoop(int arpNum, int begin, int end)
{
	instMan_->removeArpeggioSSGLoop(arpNum, begin, end);
}

void BambooTracker::changeArpeggioSSGLoop(int arpNum, int prevBegin, int prevEnd, const InstrumentSequenceLoop& loop)
{
	instMan_->changeArpeggioSSGLoop(arpNum, prevBegin, prevEnd, loop);
}

void BambooTracker::clearArpeggioSSGLoops(int arpNum)
{
	instMan_->clearArpeggioSSGLoops(arpNum);
}

void BambooTracker::setArpeggioSSGRelease(int arpNum, const InstrumentSequenceRelease& release)
{
	instMan_->setArpeggioSSGRelease(arpNum, release);
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

std::multiset<int> BambooTracker::getArpeggioSSGUsers(int arpNum) const
{
	return instMan_->getArpeggioSSGUsers(arpNum);
}

void BambooTracker::setPitchSSGType(int ptNum, SequenceType type)
{
	instMan_->setPitchSSGType(ptNum, type);
}

void BambooTracker::addPitchSSGSequenceData(int ptNum, int data)
{
	instMan_->addPitchSSGSequenceData(ptNum, data);
}

void BambooTracker::removePitchSSGSequenceData(int ptNum)
{
	instMan_->removePitchSSGSequenceData(ptNum);
}

void BambooTracker::setPitchSSGSequenceData(int ptNum, int cnt, int data)
{
	instMan_->setPitchSSGSequenceData(ptNum, cnt, data);
}

void BambooTracker::addPitchSSGLoop(int ptNum, const InstrumentSequenceLoop& loop)
{
	instMan_->addPitchSSGLoop(ptNum, loop);
}

void BambooTracker::removePitchSSGLoop(int ptNum, int begin, int end)
{
	instMan_->removePitchSSGLoop(ptNum, begin, end);
}

void BambooTracker::changePitchSSGLoop(int ptNum, int prevBegin, int prevEnd, const InstrumentSequenceLoop& loop)
{
	instMan_->changePitchSSGLoop(ptNum, prevBegin, prevEnd, loop);
}

void BambooTracker::clearPitchSSGLoops(int ptNum)
{
	instMan_->clearPitchSSGLoops(ptNum);
}

void BambooTracker::setPitchSSGRelease(int ptNum, const InstrumentSequenceRelease& release)
{
	instMan_->setPitchSSGRelease(ptNum, release);
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

std::multiset<int> BambooTracker::getPitchSSGUsers(int ptNum) const
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

void BambooTracker::setSampleADPCMRootKeyNumber(int sampNum, int n)
{
	instMan_->setSampleADPCMRootKeyNumber(sampNum, n);
	// opnaCtrl is changed through refInstADPCM (shared_ptr)
}

int BambooTracker::getSampleADPCMRootKeyNumber(int sampNum) const
{
	return instMan_->getSampleADPCMRootKeyNumber(sampNum);
}

void BambooTracker::setSampleADPCMRootDeltaN(int sampNum, int dn)
{
	instMan_->setSampleADPCMRootDeltaN(sampNum, dn);
	// opnaCtrl is changed through refInstADPCM (shared_ptr)
}

int BambooTracker::getSampleADPCMRootDeltaN(int sampNum) const
{
	return instMan_->getSampleADPCMRootDeltaN(sampNum);
}

void BambooTracker::setSampleADPCMRepeatEnabled(int sampNum, bool enabled)
{
	instMan_->setSampleADPCMRepeatEnabled(sampNum, enabled);
	// opnaCtrl is changed through refInstADPCM (shared_ptr)
}

bool BambooTracker::getSampleADPCMRepeatEnabled(int sampNum) const
{
	return instMan_->isSampleADPCMRepeatable(sampNum);
}

bool BambooTracker::setSampleADPCMRepeatRange(int sampNum, const SampleRepeatRange& range)
{
	return instMan_->setSampleADPCMRepeatrange(sampNum, range);
}

SampleRepeatRange BambooTracker::getSampleADPCMRepeatRange(int sampNum) const
{
	return instMan_->getSampleADPCMRepeatRange(sampNum);
}

void BambooTracker::storeSampleADPCMRawSample(int sampNum, const std::vector<uint8_t>& sample)
{
	instMan_->storeSampleADPCMRawSample(sampNum, sample);
}

void BambooTracker::storeSampleADPCMRawSample(int sampNum, std::vector<uint8_t>&& sample)
{
	instMan_->storeSampleADPCMRawSample(sampNum, std::move(sample));
}

std::vector<uint8_t> BambooTracker::getSampleADPCMRawSample(int sampNum) const
{
	return instMan_->getSampleADPCMRawSample(sampNum);
}

void BambooTracker::clearSampleADPCMRawSample(int sampNum)
{
	instMan_->clearSampleADPCMRawSample(sampNum);
}

bool BambooTracker::assignSampleADPCMRawSamples()
{
	opnaCtrl_->clearSamplesADPCM();
	std::vector<int> idcs = storeOnlyUsedSamples_ ? instMan_->getSampleADPCMValidIndices()
												  : instMan_->getSampleADPCMEntriedIndices();
	bool storedAll = true;
	for (auto sampNum : idcs) {
		size_t startAddr, stopAddr;
		if (opnaCtrl_->storeSampleADPCM(instMan_->getSampleADPCMRawSample(sampNum), startAddr, stopAddr)) {
			instMan_->setSampleADPCMStartAddress(sampNum, startAddr);
			instMan_->setSampleADPCMStopAddress(sampNum, stopAddr);
		}
		else {
			storedAll = false;
		}
	}
	return storedAll;
}

size_t BambooTracker::getSampleADPCMStartAddress(int sampNum) const
{
	return instMan_->getSampleADPCMStartAddress(sampNum);
}

size_t BambooTracker::getSampleADPCMStopAddress(int sampNum) const
{
	return instMan_->getSampleADPCMStopAddress(sampNum);
}

void BambooTracker::setInstrumentADPCMSample(int instNum, int sampNum)
{
	instMan_->setInstrumentADPCMSample(instNum, sampNum);
	opnaCtrl_->updateInstrumentADPCM(instNum);
}

std::multiset<int> BambooTracker::getSampleADPCMUsers(int sampNum) const
{
	return instMan_->getSampleADPCMUsers(sampNum);
}

void BambooTracker::addEnvelopeADPCMSequenceData(int envNum, int data)
{
	instMan_->addEnvelopeADPCMSequenceData(envNum, data);
}

void BambooTracker::removeEnvelopeADPCMSequenceData(int envNum)
{
	instMan_->removeEnvelopeADPCMSequenceData(envNum);
}

void BambooTracker::setEnvelopeADPCMSequenceData(int envNum, int cnt, int data)
{
	instMan_->setEnvelopeADPCMSequenceData(envNum, cnt, data);
}

void BambooTracker::addEnvelopeADPCMLoop(int arpNum, const InstrumentSequenceLoop& loop)
{
	instMan_->addEnvelopeADPCMLoop(arpNum, loop);
}

void BambooTracker::removeEnvelopeADPCMLoop(int envNum, int begin, int end)
{
	instMan_->removeEnvelopeADPCMLoop(envNum, begin, end);
}

void BambooTracker::changeEnvelopeADPCMLoop(int envNum, int prevBegin, int prevEnd, const InstrumentSequenceLoop& loop)
{
	instMan_->changeEnvelopeADPCMLoop(envNum, prevBegin, prevEnd, loop);
}

void BambooTracker::clearEnvelopeADPCMLoops(int envNum)
{
	instMan_->clearEnvelopeADPCMLoops(envNum);
}

void BambooTracker::setEnvelopeADPCMRelease(int arpNum, const InstrumentSequenceRelease& release)
{
	instMan_->setEnvelopeADPCMRelease(arpNum, release);
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

std::multiset<int> BambooTracker::getEnvelopeADPCMUsers(int envNum) const
{
	return instMan_->getEnvelopeADPCMUsers(envNum);
}

void BambooTracker::setArpeggioADPCMType(int arpNum, SequenceType type)
{
	instMan_->setArpeggioADPCMType(arpNum, type);
}

void BambooTracker::addArpeggioADPCMSequenceData(int arpNum, int data)
{
	instMan_->addArpeggioADPCMSequenceData(arpNum, data);
}

void BambooTracker::removeArpeggioADPCMSequenceData(int arpNum)
{
	instMan_->removeArpeggioADPCMSequenceData(arpNum);
}

void BambooTracker::setArpeggioADPCMSequenceData(int arpNum, int cnt, int data)
{
	instMan_->setArpeggioADPCMSequenceData(arpNum, cnt, data);
}

void BambooTracker::addArpeggioADPCMLoop(int arpNum, const InstrumentSequenceLoop& loop)
{
	instMan_->addArpeggioADPCMLoop(arpNum, loop);
}

void BambooTracker::removeArpeggioADPCMLoop(int arpNum, int begin, int end)
{
	instMan_->removeArpeggioADPCMLoop(arpNum, begin, end);
}

void BambooTracker::changeArpeggioADPCMLoop(int arpNum, int prevBegin, int prevEnd, const InstrumentSequenceLoop& loop)
{
	instMan_->changeArpeggioADPCMLoop(arpNum, prevBegin, prevEnd, loop);
}

void BambooTracker::clearArpeggioADPCMLoops(int arpNum)
{
	instMan_->clearArpeggioADPCMLoops(arpNum);
}

void BambooTracker::setArpeggioADPCMRelease(int arpNum, const InstrumentSequenceRelease& release)
{
	instMan_->setArpeggioADPCMRelease(arpNum, release);
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

std::multiset<int> BambooTracker::getArpeggioADPCMUsers(int arpNum) const
{
	return instMan_->getArpeggioADPCMUsers(arpNum);
}

void BambooTracker::setPitchADPCMType(int ptNum, SequenceType type)
{
	instMan_->setPitchADPCMType(ptNum, type);
}

void BambooTracker::addPitchADPCMSequenceData(int ptNum, int data)
{
	instMan_->addPitchADPCMSequenceData(ptNum, data);
}

void BambooTracker::removePitchADPCMSequenceData(int ptNum)
{
	instMan_->removePitchADPCMSequenceData(ptNum);
}

void BambooTracker::setPitchADPCMSequenceData(int ptNum, int cnt, int data)
{
	instMan_->setPitchADPCMSequenceData(ptNum, cnt, data);
}

void BambooTracker::addPitchADPCMLoop(int ptNum, const InstrumentSequenceLoop& loop)
{
	instMan_->addPitchADPCMLoop(ptNum, loop);
}

void BambooTracker::removePitchADPCMLoop(int ptNum, int begin, int end)
{
	instMan_->removePitchADPCMLoop(ptNum, begin, end);
}

void BambooTracker::changePitchADPCMLoop(int ptNum, int prevBegin, int prevEnd, const InstrumentSequenceLoop& loop)
{
	instMan_->changePitchADPCMLoop(ptNum, prevBegin, prevEnd, loop);
}

void BambooTracker::clearPitchADPCMLoops(int ptNum)
{
	instMan_->clearPitchADPCMLoops(ptNum);
}

void BambooTracker::setPitchADPCMRelease(int ptNum, const InstrumentSequenceRelease& release)
{
	instMan_->setPitchADPCMRelease(ptNum, release);
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

std::multiset<int> BambooTracker::getPitchADPCMUsers(int ptNum) const
{
	return instMan_->getPitchADPCMUsers(ptNum);
}

void BambooTracker::addPanADPCMSequenceData(int panNum, int data)
{
	instMan_->addPanADPCMSequenceData(panNum, data);
}

void BambooTracker::removePanADPCMSequenceData(int panNum)
{
	instMan_->removePanADPCMSequenceData(panNum);
}

void BambooTracker::setPanADPCMSequenceData(int panNum, int cnt, int data)
{
	instMan_->setPanADPCMSequenceData(panNum, cnt, data);
}

void BambooTracker::addPanADPCMLoop(int panNum, const InstrumentSequenceLoop& loop)
{
	instMan_->addPanADPCMLoop(panNum, loop);
}

void BambooTracker::removePanADPCMLoop(int panNum, int begin, int end)
{
	instMan_->removePanADPCMLoop(panNum, begin, end);
}

void BambooTracker::changePanADPCMLoop(int panNum, int prevBegin, int prevEnd, const InstrumentSequenceLoop& loop)
{
	instMan_->changePanADPCMLoop(panNum, prevBegin, prevEnd, loop);
}

void BambooTracker::clearPanADPCMLoops(int panNum)
{
	instMan_->clearPanADPCMLoops(panNum);
}

void BambooTracker::setPanADPCMRelease(int panNum, const InstrumentSequenceRelease& release)
{
	instMan_->setPanADPCMRelease(panNum, release);
}

void BambooTracker::setInstrumentADPCMPan(int instNum, int panNum)
{
	instMan_->setInstrumentADPCMPan(instNum, panNum);
	opnaCtrl_->updateInstrumentADPCM(instNum);
}

void BambooTracker::setInstrumentADPCMPanEnabled(int instNum, bool enabled)
{
	instMan_->setInstrumentADPCMPanEnabled(instNum, enabled);
	opnaCtrl_->updateInstrumentADPCM(instNum);
}

std::multiset<int> BambooTracker::getPanADPCMUsers(int panNum) const
{
	return instMan_->getPanADPCMUsers(panNum);
}

//--- Drumkit
void BambooTracker::setInstrumentDrumkitSample(int instNum, int key, int sampNum)
{
	instMan_->setInstrumentDrumkitSamples(instNum, key, sampNum);
	opnaCtrl_->updateInstrumentDrumkit(instNum, key);
}

void BambooTracker::setInstrumentDrumkitSampleEnabled(int instNum, int key, bool enabled)
{
	instMan_->setInstrumentDrumkitSamplesEnabled(instNum, key, enabled);
	opnaCtrl_->updateInstrumentADPCM(instNum);
}

void BambooTracker::setInstrumentDrumkitPitch(int instNum, int key, int pitch)
{
	instMan_->setInstrumentDrumkitPitch(instNum, key, pitch);
	opnaCtrl_->updateInstrumentDrumkit(instNum, key);
}

void BambooTracker::setInstrumentDrumkitPan(int instNum, int key, int pan)
{
	instMan_->setInstrumentDrumkitPan(instNum, key, pan);
	opnaCtrl_->updateInstrumentDrumkit(instNum, key);
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
	mkOrder_ = -1;
	mkStep_ = -1;

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
	tickCounter_->setGroove(mod_->getGroove(song.getGroove()));
	tickCounter_->setGrooveState(song.isUsedTempo() ? GrooveState::Invalid
													: GrooveState::ValidByGlobal);

	std::unordered_map<SoundSource, int> pairs = {
		{ SoundSource::FM, Song::getFMChannelCount(songStyle_.type) },
		{ SoundSource::SSG, 3 },
		{ SoundSource::RHYTHM, 6 },
		{ SoundSource::ADPCM, 1 },
	};
	for (auto& pair : pairs) {
		muteState_[pair.first] = std::vector<bool>(pair.second, false);
		for (int ch = 0; ch < pair.second; ++ch) opnaCtrl_->setMuteState(pair.first, ch, false);
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
bool BambooTracker::undo()
{
	return comMan_.undo();
}

bool BambooTracker::redo()
{
	return comMan_.redo();
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

void BambooTracker::jamKeyOn(JamKey key, bool volumeSet)
{
	int keyNum = jam_utils::makeNote(curOctave_, key).getNoteNumber();
	const TrackAttribute& attrib = songStyle_.trackAttribs[static_cast<size_t>(curTrackNum_)];
	funcJamKeyOn(key, keyNum, attrib, volumeSet);
}

void BambooTracker::jamKeyOn(int keyNum, bool volumeSet)
{
	const TrackAttribute& attrib = songStyle_.trackAttribs[static_cast<size_t>(curTrackNum_)];
	funcJamKeyOn(JamKey::MidiKey, keyNum, attrib, volumeSet);
}

void BambooTracker::jamKeyOnForced(JamKey key, SoundSource src, bool volumeSet, std::shared_ptr<AbstractInstrument> inst)
{
	int keyNum = jam_utils::makeNote(curOctave_, key).getNoteNumber();
	const TrackAttribute& attrib = songStyle_.trackAttribs[static_cast<size_t>(curTrackNum_)];
	if (attrib.source == src) {
		funcJamKeyOn(key, keyNum, attrib, volumeSet, inst);
	}
	else {
		auto it = utils::findIf(songStyle_.trackAttribs, [src](const TrackAttribute& attrib) { return attrib.source == src; });
		funcJamKeyOn(key, keyNum, *it, volumeSet, inst);
	}
}

void BambooTracker::jamKeyOnForced(int keyNum, SoundSource src, bool volumeSet, std::shared_ptr<AbstractInstrument> inst)
{
	const TrackAttribute& attrib = songStyle_.trackAttribs[static_cast<size_t>(curTrackNum_)];
	if (attrib.source == src) {
		funcJamKeyOn(JamKey::MidiKey, keyNum, attrib, volumeSet, inst);
	}
	else {
		auto it = utils::findIf(songStyle_.trackAttribs, [src](const TrackAttribute& attrib) { return attrib.source == src; });
		funcJamKeyOn(JamKey::MidiKey, keyNum, *it, volumeSet, inst);
	}
}

void BambooTracker::funcJamKeyOn(JamKey key, int keyNum, const TrackAttribute& attrib, bool volumeSet,
								 std::shared_ptr<AbstractInstrument> inst)
{
	if (playback_->isPlayingStep()) playback_->stopPlaySong();	// Reset

	if (attrib.source == SoundSource::RHYTHM) {
		if (volumeSet)
			opnaCtrl_->setVolumeRhythm(attrib.channelInSource, std::min(curVolume_, bt_defs::NSTEP_RHYTHM_VOLUME - 1));
		opnaCtrl_->setKeyOnFlagRhythm(attrib.channelInSource);
		opnaCtrl_->updateKeyOnOffStatusRhythm(true);
	}
	else {
		std::vector<JamKeyInfo>&& list = jamMan_->keyOn(key, attrib.channelInSource, attrib.source, keyNum);
		if (list.size() == 2) {	// Key off
			JamKeyInfo& offInfo = list[1];
			switch (offInfo.source) {
			case SoundSource::FM:
				if (songStyle_.type == SongType::FM3chExpanded && offInfo.channelInSource == 2) {
					opnaCtrl_->keyOffFM(2, true);
					opnaCtrl_->keyOffFM(6, true);
					opnaCtrl_->keyOffFM(7, true);
					opnaCtrl_->keyOffFM(8, true);
				}
				else {
					opnaCtrl_->keyOffFM(offInfo.channelInSource, true);
				}
				break;
			case SoundSource::SSG:
				opnaCtrl_->keyOffSSG(offInfo.channelInSource, true);
				break;
			case SoundSource::ADPCM:
				opnaCtrl_->keyOffADPCM(true);
				break;
			default:
				break;
			}
		}

		if (!inst) {	// Use current instrument if not specified
			inst = instMan_->getInstrumentSharedPtr(curInstNum_);
		}
		JamKeyInfo& onInfo = list.front();

		Note&& note = jam_utils::makeNote(onInfo, curOctave_);

		switch (onInfo.source) {
		case SoundSource::FM:
			if (auto fm = std::dynamic_pointer_cast<InstrumentFM>(inst))
				opnaCtrl_->setInstrumentFM(onInfo.channelInSource, fm);
			if (volumeSet) {
				int vol;
				if (volFMReversed_) vol = effect_utils::reverseFmVolume(curVolume_, true);
				else vol = std::min(curVolume_, bt_defs::NSTEP_FM_VOLUME - 1);
				opnaCtrl_->setVolumeFM(onInfo.channelInSource, vol);
			}
			if (songStyle_.type == SongType::FM3chExpanded && onInfo.channelInSource == 2) {
				opnaCtrl_->keyOnFM(2, note, true);
				opnaCtrl_->keyOnFM(6, note, true);
				opnaCtrl_->keyOnFM(7, note, true);
				opnaCtrl_->keyOnFM(8, note, true);
			}
			else {
				opnaCtrl_->keyOnFM(onInfo.channelInSource, note, true);
			}
			break;
		case SoundSource::SSG:
			if (auto ssg = std::dynamic_pointer_cast<InstrumentSSG>(inst))
				opnaCtrl_->setInstrumentSSG(onInfo.channelInSource, ssg);
			if (volumeSet)
				opnaCtrl_->setVolumeSSG(onInfo.channelInSource, std::min(curVolume_, 0xf));
			opnaCtrl_->keyOnSSG(onInfo.channelInSource, note, true);
			break;
		case SoundSource::ADPCM:
			if (auto adpcm = std::dynamic_pointer_cast<InstrumentADPCM>(inst))
				opnaCtrl_->setInstrumentADPCM(adpcm);
			else if (auto kit = std::dynamic_pointer_cast<InstrumentDrumkit>(inst))
				opnaCtrl_->setInstrumentDrumkit(kit);
			if (volumeSet) opnaCtrl_->setVolumeADPCM(curVolume_);
			opnaCtrl_->keyOnADPCM(note, true);
			break;
		default:
			break;
		}
	}
}

void BambooTracker::jamKeyOff(JamKey key)
{
	int keyNum = jam_utils::makeNote(curOctave_, key).getNoteNumber();
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
	int keyNum = jam_utils::makeNote(curOctave_, key).getNoteNumber();
	const TrackAttribute& attrib = songStyle_.trackAttribs[static_cast<size_t>(curTrackNum_)];
	if (attrib.source == src) {
		funcJamKeyOff(key, keyNum, attrib);
	}
	else {
		auto it = utils::findIf(songStyle_.trackAttribs, [src](const TrackAttribute& attrib) { return attrib.source == src; });
		funcJamKeyOff(key, keyNum, *it);
	}
}

void BambooTracker::jamKeyOffForced(int keyNum, SoundSource src)
{
	const TrackAttribute& attrib = songStyle_.trackAttribs[static_cast<size_t>(curTrackNum_)];
	if (attrib.source == src) {
		funcJamKeyOff(JamKey::MidiKey, keyNum, attrib);
	}
	else {
		auto it = utils::findIf(songStyle_.trackAttribs, [src](const TrackAttribute& attrib) { return attrib.source == src; });
		funcJamKeyOff(JamKey::MidiKey, keyNum, *it);
	}
}

void BambooTracker::funcJamKeyOff(JamKey key, int keyNum, const TrackAttribute& attrib)
{
	if (attrib.source == SoundSource::RHYTHM) {
		opnaCtrl_->setKeyOffFlagRhythm(attrib.channelInSource);
		opnaCtrl_->updateKeyOnOffStatusRhythm(true);
	}
	else {
		JamKeyInfo&& info = jamMan_->keyOff(key, keyNum);

		if (info.channelInSource > -1) {	// Key still sound
			switch (info.source) {
			case SoundSource::FM:
				if (songStyle_.type == SongType::FM3chExpanded && info.channelInSource == 2) {
					opnaCtrl_->keyOffFM(2, true);
					opnaCtrl_->keyOffFM(6, true);
					opnaCtrl_->keyOffFM(7, true);
					opnaCtrl_->keyOffFM(8, true);
				}
				else {
					opnaCtrl_->keyOffFM(info.channelInSource, true);
				}
				break;
			case SoundSource::SSG:
				opnaCtrl_->keyOffSSG(info.channelInSource, true);
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

void BambooTracker::jamkeyOffAll()
{
	std::vector<JamKeyInfo>&& onList = jamMan_->reset();
	for (auto& info : onList) {
		if (info.channelInSource > -1) {	// Key still sound
			switch (info.source) {
			case SoundSource::FM:
				if (songStyle_.type == SongType::FM3chExpanded && info.channelInSource == 2) {
					opnaCtrl_->keyOffFM(2, true);
					opnaCtrl_->keyOffFM(6, true);
					opnaCtrl_->keyOffFM(7, true);
					opnaCtrl_->keyOffFM(8, true);
				}
				else {
					opnaCtrl_->keyOffFM(info.channelInSource, true);
				}
				break;
			case SoundSource::SSG:
				opnaCtrl_->keyOffSSG(info.channelInSource, true);
				break;
			case SoundSource::ADPCM:
				opnaCtrl_->keyOffADPCM(true);
				break;
			default:
				break;
			}
		}
	}

	for (int i = 0; i < 6; ++i) opnaCtrl_->setKeyOffFlagRhythm(i);
	opnaCtrl_->updateRegisterStates();
}

bool BambooTracker::assignADPCMBeforeForcedJamKeyOn(
		std::shared_ptr<AbstractInstrument> inst, std::unordered_map<int, std::array<size_t, 2>>& sampAddrs)
{
	size_t start, stop;
	bool isAssignedAll = false;
	switch (inst->getType()) {
	case InstrumentType::ADPCM:
	{
		opnaCtrl_->clearSamplesADPCM();
		if (opnaCtrl_->storeSampleADPCM(
					std::dynamic_pointer_cast<InstrumentADPCM>(inst)->getRawSample(), start, stop)) {
			sampAddrs[0] = {{ start, stop }};
			isAssignedAll = true;
		}
		break;
	}
	case InstrumentType::Drumkit:
	{
		opnaCtrl_->clearSamplesADPCM();
		std::vector<std::vector<size_t>> addrs;

		auto kit = std::dynamic_pointer_cast<InstrumentDrumkit>(inst);
		for (const int& key : kit->getAssignedKeys()) {
			int n = kit->getSampleNumber(key);
			if (!sampAddrs.count(n)) {
				bool assigned = opnaCtrl_->storeSampleADPCM(kit->getRawSample(key), start, stop);
				if (assigned) sampAddrs[n] = {{ start, stop }};
				isAssignedAll &= assigned;
			}
		}
		break;
	}
	default:
		break;
	}
	return isAssignedAll;
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
	playback_->startPlayFromPosition(curOrderNum_, curStepNum_);
	startPlay();
}

bool BambooTracker::startPlayFromMarker()
{
	Song& song = mod_->getSong(curSongNum_);
	if (mkOrder_ != -1 && mkOrder_ < static_cast<int>(song.getOrderSize())
			&& mkStep_ != -1 && mkStep_ < static_cast<int>(song.getPatternSizeFromOrderNumber(mkOrder_))) {
		playback_->startPlayFromPosition(mkOrder_, mkStep_);
		startPlay();
		return true;
	}
	return false;
}

void BambooTracker::playStep()
{
	playback_->playStep(curOrderNum_, curStepNum_);
	for (auto& pair : muteState_) {
		for (size_t i = 0; i < pair.second.size(); ++i) {
			opnaCtrl_->setMuteState(pair.first, static_cast<int>(i), pair.second[i]);
		}
	}
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

void BambooTracker::setMarker(int order, int step)
{
	if (mkOrder_ == order && mkStep_ == step) {
		mkOrder_ = -1;
		mkStep_ = -1;
	}
	else {
		mkOrder_ = order;
		mkStep_ = step;
	}
}

int BambooTracker::getMarkerOrder() const
{
	return mkOrder_;
}

int BambooTracker::getMarkerStep() const
{
	return mkStep_;
}

/********** Export **********/
namespace
{
void checkNextPositionOfLastStepAndStepSize(Song& song, int& endOrder, int& endStep)
{
	endOrder = 0;
	endStep = 0;

	std::vector<TrackAttribute> attribs = song.getTrackAttributes();
	std::unordered_set<int> orderStepMap;
	int lastOrder = static_cast<int>(song.getOrderSize()) - 1;

	for (int curOrder = 0; !orderStepMap.count(curOrder); curOrder = endOrder) {
		orderStepMap.insert(curOrder);	// Arrived flag
		// Default next order
		endOrder = (endOrder + 1) % (lastOrder + 1);
		endStep = 0;

		// Check jump effect
		for (auto attrib : attribs) {
			Step& step = song.getTrack(attrib.number).getPatternFromOrderNumber(curOrder)
						 .getStep(static_cast<int>(song.getPatternSizeFromOrderNumber(curOrder)) - 1);
			for (int i = 0; i < Step::N_EFFECT; ++i) {
				Effect&& eff = effect_utils::validateEffect(attrib.source, step.getEffect(i));
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
					return;
				case EffectType::PatternBreak:
					if (curOrder == lastOrder
							&& eff.value < static_cast<int>(song.getPatternSizeFromOrderNumber(0))) {
						endOrder = 0;
						endStep = eff.value;
					}
					else if (eff.value < static_cast<int>(song.getPatternSizeFromOrderNumber(curOrder + 1))) {
						endOrder = curOrder + 1;
						endStep = eff.value;
					}
					break;
				default:
					break;
				}
			}
		}
	}
}
}

bool BambooTracker::exportToWav(io::WavContainer& container, int loopCnt, ExportCancellCallback checkFunc)
{
	int tmpRate = opnaCtrl_->getRate();
	opnaCtrl_->setRate(static_cast<int>(container.getSampleRate()));
	size_t sampCnt = static_cast<size_t>(opnaCtrl_->getRate() * opnaCtrl_->getDuration() / 1000);
	size_t intrCnt = static_cast<size_t>(opnaCtrl_->getRate()) / mod_->getTickFrequency();
	size_t intrCntRest = 0;
	std::vector<int16_t> buf(sampCnt << 1);

	int endOrder, endStep;
	checkNextPositionOfLastStepAndStepSize(mod_->getSong(curSongNum_), endOrder, endStep);
	bool endFlag = false;
	bool tmpFollow = std::exchange(isFollowPlay_, false);
	startPlayFromStart();

	while (true) {
		size_t sampCntRest = sampCnt;
		while (sampCntRest) {
			if (!intrCntRest) {	// Interruption
				intrCntRest = intrCnt;    // Set counts to next interruption

				if (!streamCountUp()) {
					if (checkFunc()) {	// Update lambda function
						stopPlaySong();
						isFollowPlay_ = tmpFollow;
						opnaCtrl_->setRate(tmpRate);
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

			bool result = opnaCtrl_->getStreamSamples(buf.data(), count);
			if (!result) {
				stopPlaySong();
				isFollowPlay_ = tmpFollow;
				opnaCtrl_->setRate(tmpRate);
				return false;
			}
			container.appendSample(buf.data(), count);
		}

		if (endFlag) break;
	}

	stopPlaySong();
	isFollowPlay_ = tmpFollow;
	opnaCtrl_->setRate(tmpRate);

	return true;
}

bool BambooTracker::exportToVgm(io::BinaryContainer& container, int target, bool gd3TagEnabled,
								const io::GD3Tag& tag, bool shouldSetMix, double gain,
								ExportCancellCallback checkFunc)
{
	int tmpRate = opnaCtrl_->getRate();
	opnaCtrl_->setRate(44100);
	double dblIntrCnt = 44100.0 / static_cast<double>(mod_->getTickFrequency());
	size_t intrCnt = static_cast<size_t>(dblIntrCnt);
	double intrCntDiff = dblIntrCnt - intrCnt;
	double intrCntRest = 0;

	int loopOrder, loopStep;
	checkNextPositionOfLastStepAndStepSize(mod_->getSong(curSongNum_), loopOrder, loopStep);
	bool loopFlag = (loopOrder != -1);
	int endCnt = (loopOrder == -1) ? 0 : 1;
	bool tmpFollow = std::exchange(isFollowPlay_, false);
	uint32_t loopPoint = 0;
	uint32_t loopPointSamples = 0;

	auto exCntr = std::make_shared<chip::VgmLogger>(target, mod_->getTickFrequency());

	// Set ADPCM
	opnaCtrl_->clearSamplesADPCM();
	std::vector<uint8_t> rom;
	for (auto sampNum : instMan_->getSampleADPCMValidIndices()) {
		std::vector<uint8_t>&& sample = instMan_->getSampleADPCMRawSample(sampNum);
		size_t startAddr, stopAddr;
		if (opnaCtrl_->storeSampleADPCM(sample, startAddr, stopAddr)) {
			instMan_->setSampleADPCMStartAddress(sampNum, startAddr);
			instMan_->setSampleADPCMStopAddress(sampNum, stopAddr);
			rom.resize((stopAddr + 1) << 5);
			std::copy(sample.begin(), sample.end(), rom.begin() + static_cast<int>(startAddr << 5));
		}
	}
	exCntr->setDataBlock(std::move(rom));

	opnaCtrl_->setExportContainer(exCntr);
	startPlayFromStart();
	exCntr->forceMoveLoopPoint();

	while (true) {
		if (!streamCountUp()) {
			if (checkFunc()) {	// Update lambda function
				stopPlaySong();
				isFollowPlay_ = tmpFollow;
				opnaCtrl_->setRate(tmpRate);
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

		exCntr->elapse(intrCnt + extraIntrCnt);
	}

	opnaCtrl_->setExportContainer();
	stopPlaySong();
	isFollowPlay_ = tmpFollow;
	opnaCtrl_->setRate(tmpRate);

	const io::GD3Tag* gd3Tag = gd3TagEnabled ? &tag : nullptr;

	std::unique_ptr<io::VgmMix> mix;
	if (shouldSetMix) {
		mix = std::make_unique<io::VgmMix>(opnaCtrl_->getMasterVolumeFM(), opnaCtrl_->getMasterVolumeSSG(), gain);
	}

	try {
		io::writeVgm(container, target, exCntr->getData(), CHIP_CLOCK, mod_->getTickFrequency(),
					 loopFlag, loopPoint, exCntr->getSampleLength() - loopPointSamples,
					 exCntr->getSampleLength(), gd3Tag, mix.get());
		return true;
	} catch (...) {
		throw;
	}
}

bool BambooTracker::exportToS98(io::BinaryContainer& container, int target, bool tagEnabled,
								const io::S98Tag& tag, int rate, ExportCancellCallback checkFunc)
{
	int tmpRate = opnaCtrl_->getRate();
	opnaCtrl_->setRate(rate);
	double dblIntrCnt = static_cast<double>(rate) / static_cast<double>(mod_->getTickFrequency());
	size_t intrCnt = static_cast<size_t>(dblIntrCnt);
	double intrCntDiff = dblIntrCnt - intrCnt;
	double intrCntRest = 0;

	int loopOrder, loopStep;
	checkNextPositionOfLastStepAndStepSize(mod_->getSong(curSongNum_), loopOrder, loopStep);
	bool loopFlag = (loopOrder != -1);
	int endCnt = (loopOrder == -1) ? 0 : 1;
	bool tmpFollow = std::exchange(isFollowPlay_, false);
	uint32_t loopPoint = 0;
	auto exCntr = std::make_shared<chip::S98Logger>(target);
	opnaCtrl_->setExportContainer(exCntr);
	startPlayFromStart();
	assignSampleADPCMRawSamples();
	exCntr->forceMoveLoopPoint();

	while (true) {
		exCntr->getData();	// Set wait counts
		if (!streamCountUp()) {
			if (checkFunc()) {	// Update lambda function
				stopPlaySong();
				isFollowPlay_ = tmpFollow;
				opnaCtrl_->setRate(tmpRate);
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
		exCntr->elapse(intrCnt + extraIntrCnt);
	}

	opnaCtrl_->setExportContainer();
	stopPlaySong();
	isFollowPlay_ = tmpFollow;
	opnaCtrl_->setRate(tmpRate);

	try {
		io::writeS98(container, target, exCntr->getData(), CHIP_CLOCK, static_cast<uint32_t>(rate),
					 loopFlag, loopPoint, tagEnabled, tag);
		return true;
	} catch (...) {
		throw;
	}
}

/********** Real chip interface **********/
void BambooTracker::connectToRealChip(RealChipInterfaceType type, RealChipInterfaceGeneratorFunc* f)
{
	opnaCtrl_->connectToRealChip(type, f);
}

RealChipInterfaceType BambooTracker::getRealChipInterfaceType() const
{
	return opnaCtrl_->getRealChipInterfaceType();
}

bool BambooTracker::hasConnectedToRealChip() const
{
	return opnaCtrl_->hasConnectedToRealChip();
}

/********** Stream events **********/
int BambooTracker::streamCountUp()
{
	int state = playback_->streamCountUp();
	if (!state && isFollowPlay_ && !playback_->isPlayingStep()) {	// Step
		int odr = playback_->getPlayingOrderNumber();
		if (odr >= 0) {
			curOrderNum_ = odr;
			curStepNum_ = playback_->getPlayingStepNumber();
		}
	}
	return state;
}

bool BambooTracker::getStreamSamples(int16_t *container, size_t nSamples)
{
	return opnaCtrl_->getStreamSamples(container, nSamples);
}

void BambooTracker::killSound()
{
	jamMan_->reset();
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
	makeNewModule(true);
}

void BambooTracker::makeNewModule(bool withInstrument)
{
	stopPlaySong();

	clearAllInstrument();

	opnaCtrl_->reset();

	mod_ = std::make_shared<Module>();

	tickCounter_->setInterruptRate(mod_->getTickFrequency());

	setCurrentSongNumber(0);

	if (withInstrument) {
		addInstrument(0, InstrumentType::FM, u8"Instrument 00");
		curInstNum_ = 0;
	}
	else {
		curInstNum_ = -1;
	}

	clearCommandHistory();
}

void BambooTracker::loadModule(io::BinaryContainer& container)
{
	makeNewModule(false);

	std::exception_ptr ep;
	try {
		io::ModuleIO::getInstance().loadModule(container, mod_, instMan_);
	}
	catch (...) {
		ep = std::current_exception();
	}

	tickCounter_->setInterruptRate(mod_->getTickFrequency());
	setCurrentSongNumber(0);
	clearCommandHistory();

	if (ep) std::rethrow_exception(ep);
}

void BambooTracker::saveModule(io::BinaryContainer& container)
{
	io::ModuleIO::getInstance().saveModule(container, mod_, instMan_);
}

void BambooTracker::setModulePath(const std::string& path)
{
	mod_->setFilePath(path);
}

std::string BambooTracker::getModulePath() const
{
	return mod_->getFilePath();
}

void BambooTracker::setModuleTitle(const std::string& title)
{
	mod_->setTitle(title);
}

std::string BambooTracker::getModuleTitle() const
{
	return mod_->getTitle();
}

void BambooTracker::setModuleAuthor(const std::string& author)
{
	mod_->setAuthor(author);
}

std::string BambooTracker::getModuleAuthor() const
{
	return mod_->getAuthor();
}

void BambooTracker::setModuleCopyright(const std::string& copyright)
{
	mod_->setCopyright(copyright);
}

std::string BambooTracker::getModuleCopyright() const
{
	return mod_->getCopyright();
}

void BambooTracker::setModuleComment(const std::string& comment)
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

void BambooTracker::setGroove(int num, const std::vector<int>& seq)
{
	mod_->setGroove(num, seq);
}

void BambooTracker::setGrooves(const std::vector<std::vector<int>>& seqs)
{
	mod_->setGrooves(seqs);
}

std::vector<int> BambooTracker::getGroove(int num) const
{
	return mod_->getGroove(num);
}

void BambooTracker::clearUnusedPatterns()
{
	mod_->clearUnusedPatterns();
}

std::unordered_map<int, int> BambooTracker::replaceDuplicateInstrumentsInPatterns()
{
	std::unordered_map<int, int> map = instMan_->getDuplicateInstrumentMap();
	mod_->replaceDuplicateInstrumentsInPatterns(map);
	return map;
}

void BambooTracker::clearUnusedADPCMSamples()
{
	instMan_->clearUnusedSamplesADPCM();
}

/*----- Song -----*/
void BambooTracker::setSongTitle(int songNum, const std::string& title)
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
	tickCounter_->setGroove(mod_->getGroove(groove));
}

int BambooTracker::getSongGroove(int songNum) const
{
	return mod_->getSong(songNum).getGroove();
}

void BambooTracker::toggleTempoOrGrooveInSong(int songNum, bool isTempo)
{
	mod_->getSong(songNum).toggleTempoOrGroove(isTempo);
	tickCounter_->setGrooveState(isTempo ? GrooveState::Invalid
										 : GrooveState::ValidByGlobal);
}

bool BambooTracker::isUsedTempoInSong(int songNum) const
{
	return mod_->getSong(songNum).isUsedTempo();
}

SongStyle BambooTracker::getSongStyle(int songNum) const
{
	return mod_->getSong(songNum).getStyle();
}

void BambooTracker::changeSongType(int songNum, SongType type)
{
	mod_->getSong(songNum).changeType(type);
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

void BambooTracker::addSong(SongType songType, const std::string& title)
{
	mod_->addSong(songType, title);
}

void BambooTracker::sortSongs(const std::vector<int>& numbers)
{
	mod_->sortSongs(std::move(numbers));
}

void BambooTracker::transposeSong(int songNum, int semitones, const std::vector<int>& excludeInsts)
{
	mod_->getSong(songNum).transpose(semitones, excludeInsts);
}

void BambooTracker::swapTracks(int songNum, int track1, int track2)
{
	mod_->getSong(songNum).swapTracks(track1, track2);
}

double BambooTracker::estimateSongLength(int songNum) const
{
	SongLengthCalculator calc(*mod_.get(), songNum);
	return calc.approximateLengthBySecond();
}

size_t BambooTracker::getTotalStepCount(int songNum, size_t loopCnt) const
{
	size_t introSize, loopSize;
	SongLengthCalculator calc(*mod_.get(), songNum);
	calc.totalStepCount(introSize, loopSize);
	return introSize + loopSize * loopCnt;
}

/*----- Bookmark -----*/
void BambooTracker::addBookmark(int songNum, const std::string& name, int order, int step)
{
	mod_->getSong(songNum).addBookmark(name, order, step);
}

void BambooTracker::changeBookmark(int songNum, int i, const std::string& name, int order, int step)
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

/*----- Key signature -----*/
void BambooTracker::addKeySignature(int songNum, KeySignature::Type key, int order, int step)
{
	mod_->getSong(songNum).addKeySignature(key, order, step);
}

void BambooTracker::changeKeySignature(int songNum, int i, KeySignature::Type key, int order, int step)
{
	mod_->getSong(songNum).changeKeySignature(i, key, order, step);
}

void BambooTracker::removeKeySignature(int songNum, int i)
{
	mod_->getSong(songNum).removeKeySignature(i);
}

void BambooTracker::clearKeySignature(int songNum)
{
	mod_->getSong(songNum).clearKeySignature();
}

KeySignature BambooTracker::getKeySignature(int songNum, int i) const
{
	return mod_->getSong(songNum).getKeySignature(i);
}

size_t BambooTracker::getKeySignatureSize(int songNum) const
{
	return mod_->getSong(songNum).getKeySignatureSize();
}

KeySignature::Type BambooTracker::searchKeySignatureAt(int songNum, int order, int step) const
{
	return mod_->getSong(songNum).searchKeySignatureAt(order, step);
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

void BambooTracker::setTrackVisibility(int songNum, int trackNum, bool visible)
{
	mod_->getSong(songNum).getTrack(trackNum).setVisibility(visible);
}

bool BambooTracker::isVisibleTrack(int songNum, int trackNum)
{
	return mod_->getSong(songNum).getTrack(trackNum).isVisible();
}

/*----- Order -----*/
std::vector<OrderInfo> BambooTracker::getOrderData(int songNum, int orderNum) const
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

bool BambooTracker::pasteOrderCells(int songNum, int beginTrack, int beginOrder, const Vector2d<int>& cells)
{
	// Clip given cells to fit the size of pasted area.
	std::size_t w = std::min(cells.rowSize(), songStyle_.trackAttribs.size() - static_cast<size_t>(beginTrack));
	std::size_t h = std::min(cells.columnSize(), getOrderSize(songNum) - static_cast<size_t>(beginOrder));
	if (w == 0 || h == 0) return false;

	auto clipped = cells.clip(0, 0, h, w);
	if (!clipped.isValid() || clipped.empty()) return false;

	return comMan_.invoke(std::make_unique<PasteCopiedDataToOrderCommand>(mod_, songNum, beginTrack, beginOrder, clipped));
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

void BambooTracker::setStepNote(int songNum, int trackNum, int orderNum, int stepNum, const Note& note, bool instMask, bool volMask)
{
	int nn = Note(note).getNoteNumber();
	SoundSource src = songStyle_.trackAttribs.at(static_cast<size_t>(trackNum)).source;

	int in = -1;
	if (!instMask && curInstNum_ != -1
			&& (src == instMan_->getInstrumentSharedPtr(curInstNum_)->getSoundSource())) {
		in = curInstNum_;
	}
	bool fmReversed = (volFMReversed_ && src == SoundSource::FM);

	comMan_.invoke(std::make_unique<SetKeyOnToStepCommand>(mod_, songNum, trackNum, orderNum, stepNum, nn, instMask, in, volMask, curVolume_, fmReversed));
}

void BambooTracker::setStepKeyOff(int songNum, int trackNum, int orderNum, int stepNum)
{
	comMan_.invoke(std::make_unique<SetKeyOffToStepCommand>(mod_, songNum, trackNum, orderNum, stepNum));
}

void BambooTracker::setStepKeyCut(int songNum, int trackNum, int orderNum, int stepNum)
{
	comMan_.invoke(std::make_unique<SetKeyCutToStepCommand>(mod_, songNum, trackNum, orderNum, stepNum));
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

int BambooTracker::setStepVolumeDigit(int songNum, int trackNum, int orderNum, int stepNum, int volume, bool secondEntry)
{	
	bool fmReversed = (volFMReversed_
					   && songStyle_.trackAttribs.at(static_cast<size_t>(trackNum)).source == SoundSource::FM);
	comMan_.invoke(std::make_unique<SetVolumeToStepCommand>(mod_, songNum, trackNum, orderNum, stepNum, volume, fmReversed, secondEntry));
	curVolume_ = mod_->getSong(songNum).getTrack(trackNum).getPatternFromOrderNumber(orderNum).getStep(stepNum).getVolume();
	if (fmReversed) curVolume_ = effect_utils::reverseFmVolume(curVolume_);
	return curVolume_;
}

void BambooTracker::eraseStepVolume(int songNum, int trackNum, int orderNum, int stepNum)
{
	comMan_.invoke(std::make_unique<EraseVolumeInStepCommand>(mod_, songNum, trackNum, orderNum, stepNum));
}

std::string BambooTracker::getStepEffectID(int songNum, int trackNum, int orderNum, int stepNum, int n) const
{
	return mod_->getSong(songNum).getTrack(trackNum).getPatternFromOrderNumber(orderNum)
			.getStep(stepNum).getEffectId(n);
}

void BambooTracker::setStepEffectIDCharacter(int songNum, int trackNum, int orderNum, int stepNum, int n, const std::string& id, bool fillValue00, bool secondEntry)
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

namespace
{
Vector2d<std::string> clipCellsToFitPastedArea(
	size_t trackCnt, size_t ptnSize, int beginTrack, int beginColmn, int beginStep,
	const Vector2d<std::string>& cells, bool overflow)
{
	std::size_t w = (trackCnt - static_cast<std::size_t>(beginTrack) - 1) * Step::N_COLUMN
					+ (Step::N_COLUMN - static_cast<std::size_t>(beginColmn));
	std::size_t h = ptnSize - static_cast<std::size_t>(beginStep);

	std::size_t width = std::min(cells.columnSize(), w);
	std::size_t height = overflow ? cells.rowSize() : std::min(cells.rowSize(), h);

	return cells.clip(0, 0, height, width);
}
}

bool BambooTracker::pastePatternCells(int songNum, int beginTrack, int beginColmn, int beginOrder, int beginStep,
									  const Vector2d<std::string>& cells, bool overflow)
{
	const auto clipped = clipCellsToFitPastedArea(songStyle_.trackAttribs.size(), getPatternSizeFromOrderNumber(songNum, beginOrder),
												  beginTrack, beginColmn, beginStep, cells, overflow);
	if (!clipped.isValid() || clipped.empty()) return false;

	return comMan_.invoke(std::make_unique<PasteCopiedDataToPatternCommand>(
		mod_, songNum, beginTrack, beginColmn, beginOrder, beginStep, clipped));
}

bool BambooTracker::pasteMixPatternCells(int songNum, int beginTrack, int beginColmn, int beginOrder, int beginStep,
										 const Vector2d<std::string>& cells, bool overflow)
{
	const auto clipped = clipCellsToFitPastedArea(songStyle_.trackAttribs.size(), getPatternSizeFromOrderNumber(songNum, beginOrder),
												  beginTrack, beginColmn, beginStep, cells, overflow);
	if (!clipped.isValid() || clipped.empty()) return false;

	return comMan_.invoke(std::make_unique<PasteMixCopiedDataToPatternCommand>(
		mod_, songNum, beginTrack, beginColmn, beginOrder, beginStep, clipped));
}

bool BambooTracker::pasteOverwritePatternCells(int songNum, int beginTrack, int beginColmn, int beginOrder,
											   int beginStep, const Vector2d<std::string>& cells, bool overflow)
{
	const auto clipped = clipCellsToFitPastedArea(songStyle_.trackAttribs.size(), getPatternSizeFromOrderNumber(songNum, beginOrder),
												  beginTrack, beginColmn, beginStep, cells, overflow);
	return comMan_.invoke(std::make_unique<PasteOverwriteCopiedDataToPatternCommand>(
		mod_, songNum, beginTrack, beginColmn, beginOrder, beginStep, clipped));
}

bool BambooTracker::pasteInsertPatternCells(int songNum, int beginTrack, int beginColmn, int beginOrder,
											int beginStep, const Vector2d<std::string>& cells)
{
	const auto clipped = clipCellsToFitPastedArea(songStyle_.trackAttribs.size(), getPatternSizeFromOrderNumber(songNum, beginOrder),
												  beginTrack, beginColmn, beginStep, cells, false);
	return comMan_.invoke(std::make_unique<PasteInsertCopiedDataToPatternCommand>(
		mod_, songNum, beginTrack, beginColmn, beginOrder, beginStep, clipped));
}

bool BambooTracker::erasePatternCells(int songNum, int beginTrack, int beginColmn, int beginOrder, int beginStep,
									  int endTrack, int endColmn, int endStep)
{
	return comMan_.invoke(std::make_unique<EraseCellsInPatternCommand>(
		mod_, songNum, beginTrack, beginColmn, beginOrder, beginStep, endTrack, endColmn, endStep));
}

void BambooTracker::transposeNoteInPattern(int songNum, int beginTrack, int beginOrder, int beginStep,
										   int endTrack, int endStep, int semitone)
{
	comMan_.invoke(std::make_unique<TransposeNoteInPatternCommand>(
					   mod_, songNum, beginTrack, beginOrder, beginStep, endTrack, endStep, semitone));
}

void BambooTracker::changeValuesInPattern(int songNum, int beginTrack, int beginColumn, int beginOrder,
										  int beginStep, int endTrack, int endColumn, int endStep, int value)
{
	comMan_.invoke(std::make_unique<ChangeValuesInPatternCommand>(
					   mod_, songNum, beginTrack, beginColumn, beginOrder, beginStep,
					   endTrack, endColumn, endStep, value, volFMReversed_));
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
