#include "configuration.hpp"

Configuration::Configuration()
{
	// Internal //
	mainW_ = 930;
	mainH_= 780;
	mainMax_ = false;
	mainX_ = -1;	// Dummy
	mainY_ = -1;	// Dummy
	instFMW_ = 570;
	instFMH_ = 680;
	instSSGW_ = 500;
	instSSGH_ = 390;
	followMode_ = true;
	workDir_ = "";

	// General //
	// General settings
	warpCursor_ = true;
	warpAcrossOrders_ = true;
	showRowNumHex_ = true;
	showPrevNextOrders_ = true;
	backupModules_ = true;
	dontSelectOnDoubleClick_ = false;
	reverseFMVolumeOrder_ = true;
	moveCursorToRight_ = false;
	retrieveChannelState_ = false;
	enableTranslation_ = true;

	// Edit settings
	pageJumpLength_ = 4;
	editableStep_ = 1;
	keyRepetision_ = true;

	// Keys
	keyOffKey_ = u8"-";
	octUpKey_ = u8"Num+*";
	octDownKey_ = u8"Num+/";
	echoKey_ = u8"^";
	noteEntryLayout_ = QWERTY;

	// Sound //
	sndDevice_ = u8"";
	useSCCI_ = false;
	sampleRate_ = 44100;
	bufferLength_ = 40;

	// Midi //
	midiInPort_ = u8"";

	// Mixer //
	mixerVolumeMaster_ = 100;
	mixerVolumeFM_ = 0;
	mixerVolumeSSG_ = 0;

	// Input //
	fmEnvelopeTextMap_ = {
		{
			"PMD",
			std::vector<FMEnvelopeTextType>({
				FMEnvelopeTextType::Skip,
				FMEnvelopeTextType::AL,
				FMEnvelopeTextType::FB,
				FMEnvelopeTextType::AR1,
				FMEnvelopeTextType::DR1,
				FMEnvelopeTextType::SR1,
				FMEnvelopeTextType::RR1,
				FMEnvelopeTextType::SL1,
				FMEnvelopeTextType::TL1,
				FMEnvelopeTextType::KS1,
				FMEnvelopeTextType::ML1,
				FMEnvelopeTextType::DT1,
				FMEnvelopeTextType::Skip,
				FMEnvelopeTextType::AR2,
				FMEnvelopeTextType::DR2,
				FMEnvelopeTextType::SR2,
				FMEnvelopeTextType::RR2,
				FMEnvelopeTextType::SL2,
				FMEnvelopeTextType::TL2,
				FMEnvelopeTextType::KS2,
				FMEnvelopeTextType::ML2,
				FMEnvelopeTextType::DT2,
				FMEnvelopeTextType::Skip,
				FMEnvelopeTextType::AR3,
				FMEnvelopeTextType::DR3,
				FMEnvelopeTextType::SR3,
				FMEnvelopeTextType::RR3,
				FMEnvelopeTextType::SL3,
				FMEnvelopeTextType::TL3,
				FMEnvelopeTextType::KS3,
				FMEnvelopeTextType::ML3,
				FMEnvelopeTextType::DT3,
				FMEnvelopeTextType::Skip,
				FMEnvelopeTextType::AR4,
				FMEnvelopeTextType::DR4,
				FMEnvelopeTextType::SR4,
				FMEnvelopeTextType::RR4,
				FMEnvelopeTextType::SL4,
				FMEnvelopeTextType::TL4,
				FMEnvelopeTextType::KS4,
				FMEnvelopeTextType::ML4,
				FMEnvelopeTextType::DT4,
				FMEnvelopeTextType::Skip
			})
		},
		{
			"FMP",
			std::vector<FMEnvelopeTextType>({
				FMEnvelopeTextType::Skip,
				FMEnvelopeTextType::AR1,
				FMEnvelopeTextType::DR1,
				FMEnvelopeTextType::SR1,
				FMEnvelopeTextType::RR1,
				FMEnvelopeTextType::SL1,
				FMEnvelopeTextType::TL1,
				FMEnvelopeTextType::KS1,
				FMEnvelopeTextType::ML1,
				FMEnvelopeTextType::DT1,
				FMEnvelopeTextType::AR2,
				FMEnvelopeTextType::DR2,
				FMEnvelopeTextType::SR2,
				FMEnvelopeTextType::RR2,
				FMEnvelopeTextType::SL2,
				FMEnvelopeTextType::TL2,
				FMEnvelopeTextType::KS2,
				FMEnvelopeTextType::ML2,
				FMEnvelopeTextType::DT2,
				FMEnvelopeTextType::AR3,
				FMEnvelopeTextType::DR3,
				FMEnvelopeTextType::SR3,
				FMEnvelopeTextType::RR3,
				FMEnvelopeTextType::SL3,
				FMEnvelopeTextType::TL3,
				FMEnvelopeTextType::KS3,
				FMEnvelopeTextType::ML3,
				FMEnvelopeTextType::DT3,
				FMEnvelopeTextType::AR4,
				FMEnvelopeTextType::DR4,
				FMEnvelopeTextType::SR4,
				FMEnvelopeTextType::RR4,
				FMEnvelopeTextType::SL4,
				FMEnvelopeTextType::TL4,
				FMEnvelopeTextType::KS4,
				FMEnvelopeTextType::ML4,
				FMEnvelopeTextType::DT4,
				FMEnvelopeTextType::AL,
				FMEnvelopeTextType::FB
			})
		},
		{
			"FMP7",
			std::vector<FMEnvelopeTextType>({
				FMEnvelopeTextType::Skip,
				FMEnvelopeTextType::AR1,
				FMEnvelopeTextType::DR1,
				FMEnvelopeTextType::SR1,
				FMEnvelopeTextType::RR1,
				FMEnvelopeTextType::SL1,
				FMEnvelopeTextType::TL1,
				FMEnvelopeTextType::KS1,
				FMEnvelopeTextType::ML1,
				FMEnvelopeTextType::DT1,
				FMEnvelopeTextType::Skip,
				FMEnvelopeTextType::AR2,
				FMEnvelopeTextType::DR2,
				FMEnvelopeTextType::SR2,
				FMEnvelopeTextType::RR2,
				FMEnvelopeTextType::SL2,
				FMEnvelopeTextType::TL2,
				FMEnvelopeTextType::KS2,
				FMEnvelopeTextType::ML2,
				FMEnvelopeTextType::DT2,
				FMEnvelopeTextType::Skip,
				FMEnvelopeTextType::AR3,
				FMEnvelopeTextType::DR3,
				FMEnvelopeTextType::SR3,
				FMEnvelopeTextType::RR3,
				FMEnvelopeTextType::SL3,
				FMEnvelopeTextType::TL3,
				FMEnvelopeTextType::KS3,
				FMEnvelopeTextType::ML3,
				FMEnvelopeTextType::DT3,
				FMEnvelopeTextType::Skip,
				FMEnvelopeTextType::AR4,
				FMEnvelopeTextType::DR4,
				FMEnvelopeTextType::SR4,
				FMEnvelopeTextType::RR4,
				FMEnvelopeTextType::SL4,
				FMEnvelopeTextType::TL4,
				FMEnvelopeTextType::KS4,
				FMEnvelopeTextType::ML4,
				FMEnvelopeTextType::DT4,
				FMEnvelopeTextType::Skip,
				FMEnvelopeTextType::AL,
				FMEnvelopeTextType::FB
			})
		},
		{
			"VOPM",
			std::vector<FMEnvelopeTextType>({
				// Number
				FMEnvelopeTextType::Skip,
				// LFO
				FMEnvelopeTextType::Skip,
				FMEnvelopeTextType::Skip,
				FMEnvelopeTextType::Skip,
				FMEnvelopeTextType::Skip,
				FMEnvelopeTextType::Skip,
				// CH
				FMEnvelopeTextType::Skip,
				FMEnvelopeTextType::FB,
				FMEnvelopeTextType::AL,
				FMEnvelopeTextType::Skip,
				FMEnvelopeTextType::Skip,
				FMEnvelopeTextType::Skip,
				FMEnvelopeTextType::Skip,
				// Op
				FMEnvelopeTextType::Skip,
				FMEnvelopeTextType::AR1,
				FMEnvelopeTextType::DR1,
				FMEnvelopeTextType::SR1,
				FMEnvelopeTextType::RR1,
				FMEnvelopeTextType::SL1,
				FMEnvelopeTextType::TL1,
				FMEnvelopeTextType::KS1,
				FMEnvelopeTextType::ML1,
				FMEnvelopeTextType::DT1,
				FMEnvelopeTextType::Skip,
				FMEnvelopeTextType::Skip,
				FMEnvelopeTextType::Skip,
				FMEnvelopeTextType::AR2,
				FMEnvelopeTextType::DR2,
				FMEnvelopeTextType::SR2,
				FMEnvelopeTextType::RR2,
				FMEnvelopeTextType::SL2,
				FMEnvelopeTextType::TL2,
				FMEnvelopeTextType::KS2,
				FMEnvelopeTextType::ML2,
				FMEnvelopeTextType::DT2,
				FMEnvelopeTextType::Skip,
				FMEnvelopeTextType::Skip,
				FMEnvelopeTextType::Skip,
				FMEnvelopeTextType::AR3,
				FMEnvelopeTextType::DR3,
				FMEnvelopeTextType::SR3,
				FMEnvelopeTextType::RR3,
				FMEnvelopeTextType::SL3,
				FMEnvelopeTextType::TL3,
				FMEnvelopeTextType::KS3,
				FMEnvelopeTextType::ML3,
				FMEnvelopeTextType::DT3,
				FMEnvelopeTextType::Skip,
				FMEnvelopeTextType::Skip,
				FMEnvelopeTextType::Skip,
				FMEnvelopeTextType::AR4,
				FMEnvelopeTextType::DR4,
				FMEnvelopeTextType::SR4,
				FMEnvelopeTextType::RR4,
				FMEnvelopeTextType::SL4,
				FMEnvelopeTextType::TL4,
				FMEnvelopeTextType::KS4,
				FMEnvelopeTextType::ML4,
				FMEnvelopeTextType::DT4,
				FMEnvelopeTextType::Skip,
				FMEnvelopeTextType::Skip
			})
		},
		{
			"NRTDRV",	// For VOICE_MODE=0
			std::vector<FMEnvelopeTextType>({
				FMEnvelopeTextType::Skip,
				FMEnvelopeTextType::AL,
				FMEnvelopeTextType::FB,
				FMEnvelopeTextType::Skip,
				FMEnvelopeTextType::AR1,
				FMEnvelopeTextType::DR1,
				FMEnvelopeTextType::SR1,
				FMEnvelopeTextType::RR1,
				FMEnvelopeTextType::SL1,
				FMEnvelopeTextType::TL1,
				FMEnvelopeTextType::KS1,
				FMEnvelopeTextType::ML1,
				FMEnvelopeTextType::DT1,
				FMEnvelopeTextType::Skip,
				FMEnvelopeTextType::Skip,
				FMEnvelopeTextType::AR2,
				FMEnvelopeTextType::DR2,
				FMEnvelopeTextType::SR2,
				FMEnvelopeTextType::RR2,
				FMEnvelopeTextType::SL2,
				FMEnvelopeTextType::TL2,
				FMEnvelopeTextType::KS2,
				FMEnvelopeTextType::ML2,
				FMEnvelopeTextType::DT2,
				FMEnvelopeTextType::Skip,
				FMEnvelopeTextType::Skip,
				FMEnvelopeTextType::AR3,
				FMEnvelopeTextType::DR3,
				FMEnvelopeTextType::SR3,
				FMEnvelopeTextType::RR3,
				FMEnvelopeTextType::SL3,
				FMEnvelopeTextType::TL3,
				FMEnvelopeTextType::KS3,
				FMEnvelopeTextType::ML3,
				FMEnvelopeTextType::DT3,
				FMEnvelopeTextType::Skip,
				FMEnvelopeTextType::Skip,
				FMEnvelopeTextType::AR4,
				FMEnvelopeTextType::DR4,
				FMEnvelopeTextType::SR4,
				FMEnvelopeTextType::RR4,
				FMEnvelopeTextType::SL4,
				FMEnvelopeTextType::TL4,
				FMEnvelopeTextType::KS4,
				FMEnvelopeTextType::ML4,
				FMEnvelopeTextType::DT4,
				FMEnvelopeTextType::Skip,
				FMEnvelopeTextType::Skip
			})
		},
		{
			"MXDRV",
			std::vector<FMEnvelopeTextType>({
				FMEnvelopeTextType::Skip,
				FMEnvelopeTextType::AR1,
				FMEnvelopeTextType::DR1,
				FMEnvelopeTextType::SR1,
				FMEnvelopeTextType::RR1,
				FMEnvelopeTextType::SL1,
				FMEnvelopeTextType::TL1,
				FMEnvelopeTextType::KS1,
				FMEnvelopeTextType::ML1,
				FMEnvelopeTextType::DT1,
				FMEnvelopeTextType::Skip,
				FMEnvelopeTextType::Skip,
				FMEnvelopeTextType::AR2,
				FMEnvelopeTextType::DR2,
				FMEnvelopeTextType::SR2,
				FMEnvelopeTextType::RR2,
				FMEnvelopeTextType::SL2,
				FMEnvelopeTextType::TL2,
				FMEnvelopeTextType::KS2,
				FMEnvelopeTextType::ML2,
				FMEnvelopeTextType::DT2,
				FMEnvelopeTextType::Skip,
				FMEnvelopeTextType::Skip,
				FMEnvelopeTextType::AR3,
				FMEnvelopeTextType::DR3,
				FMEnvelopeTextType::SR3,
				FMEnvelopeTextType::RR3,
				FMEnvelopeTextType::SL3,
				FMEnvelopeTextType::TL3,
				FMEnvelopeTextType::KS3,
				FMEnvelopeTextType::ML3,
				FMEnvelopeTextType::DT3,
				FMEnvelopeTextType::Skip,
				FMEnvelopeTextType::Skip,
				FMEnvelopeTextType::AR4,
				FMEnvelopeTextType::DR4,
				FMEnvelopeTextType::SR4,
				FMEnvelopeTextType::RR4,
				FMEnvelopeTextType::SL4,
				FMEnvelopeTextType::TL4,
				FMEnvelopeTextType::KS4,
				FMEnvelopeTextType::ML4,
				FMEnvelopeTextType::DT4,
				FMEnvelopeTextType::Skip,
				FMEnvelopeTextType::Skip,
				FMEnvelopeTextType::AL,
				FMEnvelopeTextType::FB,
				FMEnvelopeTextType::Skip
			})
		},
		{
			"MMLDRV",
			std::vector<FMEnvelopeTextType>({
				FMEnvelopeTextType::Skip,
				FMEnvelopeTextType::AL,
				FMEnvelopeTextType::FB,
				FMEnvelopeTextType::AR1,
				FMEnvelopeTextType::DR1,
				FMEnvelopeTextType::SR1,
				FMEnvelopeTextType::RR1,
				FMEnvelopeTextType::SL1,
				FMEnvelopeTextType::TL1,
				FMEnvelopeTextType::KS1,
				FMEnvelopeTextType::ML1,
				FMEnvelopeTextType::DT1,
				FMEnvelopeTextType::Skip,
				FMEnvelopeTextType::Skip,
				FMEnvelopeTextType::AR2,
				FMEnvelopeTextType::DR2,
				FMEnvelopeTextType::SR2,
				FMEnvelopeTextType::RR2,
				FMEnvelopeTextType::SL2,
				FMEnvelopeTextType::TL2,
				FMEnvelopeTextType::KS2,
				FMEnvelopeTextType::ML2,
				FMEnvelopeTextType::DT2,
				FMEnvelopeTextType::Skip,
				FMEnvelopeTextType::Skip,
				FMEnvelopeTextType::AR3,
				FMEnvelopeTextType::DR3,
				FMEnvelopeTextType::SR3,
				FMEnvelopeTextType::RR3,
				FMEnvelopeTextType::SL3,
				FMEnvelopeTextType::TL3,
				FMEnvelopeTextType::KS3,
				FMEnvelopeTextType::ML3,
				FMEnvelopeTextType::DT3,
				FMEnvelopeTextType::Skip,
				FMEnvelopeTextType::Skip,
				FMEnvelopeTextType::AR4,
				FMEnvelopeTextType::DR4,
				FMEnvelopeTextType::SR4,
				FMEnvelopeTextType::RR4,
				FMEnvelopeTextType::SL4,
				FMEnvelopeTextType::TL4,
				FMEnvelopeTextType::KS4,
				FMEnvelopeTextType::ML4,
				FMEnvelopeTextType::DT4,
				FMEnvelopeTextType::Skip,
				FMEnvelopeTextType::Skip
			})
		},
		{
			"MUCOM88",
			std::vector<FMEnvelopeTextType>({
				FMEnvelopeTextType::Skip,
				FMEnvelopeTextType::FB,
				FMEnvelopeTextType::AL,
				FMEnvelopeTextType::AR1,
				FMEnvelopeTextType::DR1,
				FMEnvelopeTextType::SR1,
				FMEnvelopeTextType::RR1,
				FMEnvelopeTextType::SL1,
				FMEnvelopeTextType::TL1,
				FMEnvelopeTextType::KS1,
				FMEnvelopeTextType::ML1,
				FMEnvelopeTextType::DT1,
				FMEnvelopeTextType::AR2,
				FMEnvelopeTextType::DR2,
				FMEnvelopeTextType::SR2,
				FMEnvelopeTextType::RR2,
				FMEnvelopeTextType::SL2,
				FMEnvelopeTextType::TL2,
				FMEnvelopeTextType::KS2,
				FMEnvelopeTextType::ML2,
				FMEnvelopeTextType::DT2,
				FMEnvelopeTextType::AR3,
				FMEnvelopeTextType::DR3,
				FMEnvelopeTextType::SR3,
				FMEnvelopeTextType::RR3,
				FMEnvelopeTextType::SL3,
				FMEnvelopeTextType::TL3,
				FMEnvelopeTextType::KS3,
				FMEnvelopeTextType::ML3,
				FMEnvelopeTextType::DT3,
				FMEnvelopeTextType::AR4,
				FMEnvelopeTextType::DR4,
				FMEnvelopeTextType::SR4,
				FMEnvelopeTextType::RR4,
				FMEnvelopeTextType::SL4,
				FMEnvelopeTextType::TL4,
				FMEnvelopeTextType::KS4,
				FMEnvelopeTextType::ML4,
				FMEnvelopeTextType::DT4
			})
		}
	};
}

// Internal //
void Configuration::setMainWindowWidth(int w)
{
	mainW_ = w;
}

int Configuration::getMainWindowWidth() const
{
	return mainW_;
}

void Configuration::setMainWindowHeight(int h)
{
	mainH_ = h;
}

int Configuration::getMainWindowHeight() const
{
	return mainH_;
}

void Configuration::setMainWindowMaximized(bool isMax)
{
	mainMax_ = isMax;
}

bool Configuration::getMainWindowMaximized() const
{
	return mainMax_;
}

void Configuration::setMainWindowX(int x)
{
	mainX_ = x;
}

int Configuration::getMainWindowX() const
{
	return mainX_;
}

void Configuration::setMainWindowY(int y)
{
	mainY_ = y;
}

int Configuration::getMainWindowY() const
{
	return mainY_;
}

void Configuration::setInstrumentFMWindowWidth(int w)
{
	instFMW_ = w;
}

int Configuration::getInstrumentFMWindowWidth() const
{
	return instFMW_;
}

void Configuration::setInstrumentFMWindowHeight(int h)
{
	instFMH_ = h;
}

int Configuration::getInstrumentFMWindowHeight() const
{
	return instFMH_;
}

void Configuration::setInstrumentSSGWindowWidth(int w)
{
	instSSGW_ = w;
}

int Configuration::getInstrumentSSGWindowWidth() const
{
	return instSSGW_;
}

void Configuration::setInstrumentSSGWindowHeight(int h)
{
	instSSGH_ = h;
}

int Configuration::getInstrumentSSGWindowHeight() const
{
	return instSSGH_;
}

void Configuration::setFollowMode(bool enabled)
{
	followMode_ = enabled;
}

bool Configuration::getFollowMode() const
{
	return followMode_;
}

void Configuration::setWorkingDirectory(std::string path)
{
	workDir_ = path;
}

std::string Configuration::getWorkingDirectory() const
{
	return workDir_;
}

// General //
// General settings
void Configuration::setWarpCursor(bool enabled)
{
	warpCursor_ = enabled;
}

bool Configuration::getWarpCursor() const
{
	return warpCursor_;
}

void Configuration::setWarpAcrossOrders(bool enabled)
{
	warpAcrossOrders_ = enabled;
}

bool Configuration::getWarpAcrossOrders() const
{
	return warpAcrossOrders_;
}

void Configuration::setShowRowNumberInHex(bool enabled)
{
	showRowNumHex_ = enabled;
}

bool Configuration::getShowRowNumberInHex() const
{
	return showRowNumHex_;
}

void Configuration::setShowPreviousNextOrders(bool enabled)
{
	showPrevNextOrders_ = enabled;
}

bool Configuration::getShowPreviousNextOrders() const
{
	return showPrevNextOrders_;
}

void Configuration::setBackupModules(bool enabled)
{
	backupModules_ = enabled;
}

bool Configuration::getBackupModules() const
{
	return backupModules_;
}

void Configuration::setDontSelectOnDoubleClick(bool enabled)
{
	dontSelectOnDoubleClick_ = enabled;
}

bool Configuration::getDontSelectOnDoubleClick() const
{
	return dontSelectOnDoubleClick_;
}

void Configuration::setReverseFMVolumeOrder(bool enabled)
{
	reverseFMVolumeOrder_= enabled;
}

bool Configuration::getReverseFMVolumeOrder() const
{
	return reverseFMVolumeOrder_;
}

void Configuration::setMoveCursorToRight(bool enabled)
{
	moveCursorToRight_ = enabled;
}

bool Configuration::getMoveCursorToRight() const
{
	return moveCursorToRight_;
}

void Configuration::setRetrieveChannelState(bool enabled)
{
	retrieveChannelState_ = enabled;
}

bool Configuration::getRetrieveChannelState() const
{
	return retrieveChannelState_;
}

void Configuration::setEnableTranslation(bool enabled)
{
	enableTranslation_ = enabled;
}
bool Configuration::getEnableTranslation() const
{
	return enableTranslation_;
}

// Edit settings
void Configuration::setPageJumpLength(size_t length)
{
	pageJumpLength_ = length;
}

size_t Configuration::getPageJumpLength() const
{
	return pageJumpLength_;
}

void Configuration::setEditableStep(size_t step)
{
	editableStep_ = step;
}

size_t Configuration::getEditableStep() const
{
	return editableStep_;
}

void Configuration::setKeyRepetition(bool enabled)
{
	keyRepetision_ = enabled;
}

bool Configuration::getKeyRepetition() const
{
	return keyRepetision_;
}

// Keys
void Configuration::setKeyOffKey(std::string key)
{
	keyOffKey_ = key;
}

std::string Configuration::getKeyOffKey() const
{
	return keyOffKey_;
}

void Configuration::setOctaveUpKey(std::string key)
{
	octUpKey_ = key;
}

std::string Configuration::getOctaveUpKey() const
{
	return octUpKey_;
}

void Configuration::setOctaveDownKey(std::string key)
{
	octDownKey_ = key;
}

std::string Configuration::getOctaveDownKey() const
{
	return octDownKey_;
}

void Configuration::setEchoBufferKey(std::string key)
{
	echoKey_ = key;
}

std::string Configuration::getEchoBufferKey() const
{
	return echoKey_;
}

void Configuration::setNoteEntryLayout(KeyboardLayout layout)
{
	noteEntryLayout_ = layout;
}

Configuration::KeyboardLayout Configuration::getNoteEntryLayout() const
{
	return noteEntryLayout_;
}

// Sound //
void Configuration::setSoundDevice(std::string device)
{
	sndDevice_ = device;
}

std::string Configuration::getSoundDevice() const
{
	return sndDevice_;
}

void Configuration::setUseSCCI(bool enabled)
{
	useSCCI_ = enabled;
}

bool Configuration::getUseSCCI() const
{
	return useSCCI_;
}

void Configuration::setSampleRate(uint32_t rate)
{
	sampleRate_ = rate;
}

uint32_t Configuration::getSampleRate() const
{
	return sampleRate_;
}

void Configuration::setBufferLength(size_t length)
{
	bufferLength_ = length;
}

size_t Configuration::getBufferLength() const
{
	return bufferLength_;
}

// Midi //
void Configuration::setMidiInputPort(const std::string &port)
{
	midiInPort_ = port;
}

std::string Configuration::getMidiInputPort() const
{
	return midiInPort_;
}

// Mixer //
void Configuration::setMixerVolumeMaster(int percentage)
{
	mixerVolumeMaster_ = percentage;
}

int Configuration::getMixerVolumeMaster() const
{
	return mixerVolumeMaster_;
}

void Configuration::setMixerVolumeFM(double dB)
{
	mixerVolumeFM_ = dB;
}

double Configuration::getMixerVolumeFM() const
{
	return mixerVolumeFM_;
}

void Configuration::setMixerVolumeSSG(double dB)
{
	mixerVolumeSSG_ = dB;
}

double Configuration::getMixerVolumeSSG() const
{
	return mixerVolumeSSG_;
}

// Input //
void Configuration::setFMEnvelopeTextMap(std::map<std::string, std::vector<FMEnvelopeTextType>> map)
{
	fmEnvelopeTextMap_ = map;
}

std::map<std::string, std::vector<FMEnvelopeTextType>> Configuration::getFMEnvelopeTextMap() const
{
	return fmEnvelopeTextMap_;
}
