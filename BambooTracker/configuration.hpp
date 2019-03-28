#pragma once

#include <cstdint>
#include <cstddef>
#include <string>
#include <map>
#include <vector>
#include <Qt>
#include "misc.hpp"
#include "jam_manager.hpp"

struct FMEnvelopeText
{
	std::string name;
	std::vector<FMEnvelopeTextType> texts;
};

class Configuration
{
public:
	Configuration();

	// Internal //
public:
	void setMainWindowWidth(int w);
	int getMainWindowWidth() const;
	void setMainWindowHeight(int h);
	int getMainWindowHeight() const;
	void setMainWindowMaximized(bool isMax);
	bool getMainWindowMaximized() const;
	void setMainWindowX(int x);
	int getMainWindowX() const;
	void setMainWindowY(int y);
	int getMainWindowY() const;
	void setInstrumentFMWindowWidth(int w);
	int getInstrumentFMWindowWidth() const;
	void setInstrumentFMWindowHeight(int h);
	int getInstrumentFMWindowHeight() const;
	void setInstrumentSSGWindowWidth(int w);
	int getInstrumentSSGWindowWidth() const;
	void setInstrumentSSGWindowHeight(int h);
	int getInstrumentSSGWindowHeight() const;
	void setFollowMode(bool enabled);
	bool getFollowMode() const;
	void setWorkingDirectory(std::string path);
	std::string getWorkingDirectory() const;
private:
	int mainW_, mainH_;
	bool mainMax_;
	int mainX_, mainY_;
	int instFMW_, instFMH_;
	int instSSGW_, instSSGH_;
	bool followMode_;
	std::string workDir_;

	// General //
	// General settings
public:
	void setWarpCursor(bool enabled);
	bool getWarpCursor() const;
	void setWarpAcrossOrders(bool enabled);
	bool getWarpAcrossOrders() const;
	void setShowRowNumberInHex(bool enabled);
	bool getShowRowNumberInHex() const;
	void setShowPreviousNextOrders(bool enabled);
	bool getShowPreviousNextOrders() const;
	void setBackupModules(bool enabled);
	bool getBackupModules() const;
	void setDontSelectOnDoubleClick(bool enabled);
	bool getDontSelectOnDoubleClick() const;
	void setReverseFMVolumeOrder(bool enabled);
	bool getReverseFMVolumeOrder() const;
	void setMoveCursorToRight(bool enabled);
	bool getMoveCursorToRight() const;
	void setRetrieveChannelState(bool enabled);
	bool getRetrieveChannelState() const;
	void setEnableTranslation(bool enabled);
	bool getEnableTranslation() const;
	void setShowFMDetuneAsSigned(bool enabled);
	bool getShowFMDetuneAsSigned() const;
private:
	bool warpCursor_, warpAcrossOrders_;
	bool showRowNumHex_, showPrevNextOrders_;
	bool backupModules_, dontSelectOnDoubleClick_;
	bool reverseFMVolumeOrder_, moveCursorToRight_;
	bool retrieveChannelState_, enableTranslation_;
	bool showFMDetuneSigned_;

	// Edit settings
public:
	void setPageJumpLength(size_t length);
	size_t getPageJumpLength() const;
	void setEditableStep(size_t step);
	size_t getEditableStep() const;
	void setKeyRepetition(bool enabled);
	bool getKeyRepetition() const;
private:
	size_t pageJumpLength_, editableStep_;
	bool keyRepetision_;

	// Keys
public:
	void setKeyOffKey(std::string key);
	std::string getKeyOffKey() const;
	void setOctaveUpKey(std::string key);
	std::string getOctaveUpKey() const;
	void setOctaveDownKey(std::string key);
	std::string getOctaveDownKey() const;
	void setEchoBufferKey(std::string key);
	std::string getEchoBufferKey() const;
	enum KeyboardLayout : int
	{
		// at the top, so new layouts can easily be added in after it
		// and it's always easy to find no matter how many layouts we add
		Custom = 0,
		QWERTY,
		QWERTZ,
		AZERTY
    };
    std::map<Qt::Key, JamKey> mappingCustom;
    std::map<Qt::Key, JamKey> mappingQWERTY = {
        {Qt::Key_Z,         JamKey::LOW_C},
        {Qt::Key_S,         JamKey::LOW_CS},
        {Qt::Key_X,         JamKey::LOW_D},
        {Qt::Key_D,         JamKey::LOW_DS},
        {Qt::Key_C,         JamKey::LOW_E},
        {Qt::Key_V,         JamKey::LOW_F},
        {Qt::Key_G,         JamKey::LOW_FS},
        {Qt::Key_B,         JamKey::LOW_G},
        {Qt::Key_H,         JamKey::LOW_GS},
        {Qt::Key_N,         JamKey::LOW_A},
        {Qt::Key_J,         JamKey::LOW_AS},
        {Qt::Key_M,         JamKey::LOW_B},
        {Qt::Key_Comma,     JamKey::LOW_C_H},
        {Qt::Key_L,         JamKey::LOW_CS_H},
        {Qt::Key_Period,    JamKey::LOW_D_H},

        {Qt::Key_Q,         JamKey::HIGH_C},
        {Qt::Key_2,         JamKey::HIGH_CS},
        {Qt::Key_W,         JamKey::HIGH_D},
        {Qt::Key_3,         JamKey::HIGH_DS},
        {Qt::Key_E,         JamKey::HIGH_E},
        {Qt::Key_R,         JamKey::HIGH_F},
        {Qt::Key_5,         JamKey::HIGH_FS},
        {Qt::Key_T,         JamKey::HIGH_G},
        {Qt::Key_6,         JamKey::HIGH_GS},
        {Qt::Key_Y,         JamKey::HIGH_A},
        {Qt::Key_7,         JamKey::HIGH_AS},
        {Qt::Key_U,         JamKey::HIGH_B},
        {Qt::Key_I,         JamKey::HIGH_C_H},
        {Qt::Key_9,         JamKey::HIGH_CS_H},
        {Qt::Key_O,         JamKey::HIGH_D_H},
    };
    std::map<Qt::Key, JamKey> mappingQWERTZ = {
        {Qt::Key_Y,         JamKey::LOW_C},
        {Qt::Key_S,         JamKey::LOW_CS},
        {Qt::Key_X,         JamKey::LOW_D},
        {Qt::Key_D,         JamKey::LOW_DS},
        {Qt::Key_C,         JamKey::LOW_E},
        {Qt::Key_V,         JamKey::LOW_F},
        {Qt::Key_G,         JamKey::LOW_FS},
        {Qt::Key_B,         JamKey::LOW_G},
        {Qt::Key_H,         JamKey::LOW_GS},
        {Qt::Key_N,         JamKey::LOW_A},
        {Qt::Key_J,         JamKey::LOW_AS},
        {Qt::Key_M,         JamKey::LOW_B},
        {Qt::Key_Comma,     JamKey::LOW_C_H},
        {Qt::Key_L,         JamKey::LOW_CS_H},
        {Qt::Key_Period,    JamKey::LOW_D_H},

        {Qt::Key_Q,         JamKey::HIGH_C},
        {Qt::Key_2,         JamKey::HIGH_CS},
        {Qt::Key_W,         JamKey::HIGH_D},
        {Qt::Key_3,         JamKey::HIGH_DS},
        {Qt::Key_E,         JamKey::HIGH_E},
        {Qt::Key_R,         JamKey::HIGH_F},
        {Qt::Key_5,         JamKey::HIGH_FS},
        {Qt::Key_T,         JamKey::HIGH_G},
        {Qt::Key_6,         JamKey::HIGH_GS},
        {Qt::Key_Z,         JamKey::HIGH_A},
        {Qt::Key_7,         JamKey::HIGH_AS},
        {Qt::Key_U,         JamKey::HIGH_B},
        {Qt::Key_I,         JamKey::HIGH_C_H},
        {Qt::Key_9,         JamKey::HIGH_CS_H},
        {Qt::Key_O,         JamKey::HIGH_D_H},
    };
    std::map<Qt::Key, JamKey> mappingAZERTY = {
        {Qt::Key_W,         JamKey::LOW_C},
        {Qt::Key_S,         JamKey::LOW_CS},
        {Qt::Key_X,         JamKey::LOW_D},
        {Qt::Key_D,         JamKey::LOW_DS},
        {Qt::Key_C,         JamKey::LOW_E},
        {Qt::Key_V,         JamKey::LOW_F},
        {Qt::Key_G,         JamKey::LOW_FS},
        {Qt::Key_B,         JamKey::LOW_G},
        {Qt::Key_H,         JamKey::LOW_GS},
        {Qt::Key_N,         JamKey::LOW_A},
        {Qt::Key_J,         JamKey::LOW_AS},
        {Qt::Key_Comma,     JamKey::LOW_B},
        {Qt::Key_Semicolon, JamKey::LOW_C_H},
        {Qt::Key_L,         JamKey::LOW_CS_H},
        {Qt::Key_Colon,     JamKey::LOW_D_H},

        {Qt::Key_A,         JamKey::HIGH_C},
        {Qt::Key_Eacute,    JamKey::HIGH_CS},
        {Qt::Key_Z,         JamKey::HIGH_D},
        {Qt::Key_QuoteDbl,  JamKey::HIGH_DS},
        {Qt::Key_E,         JamKey::HIGH_E},
        {Qt::Key_R,         JamKey::HIGH_F},
        {Qt::Key_ParenLeft, JamKey::HIGH_FS},
        {Qt::Key_T,         JamKey::HIGH_G},
        {Qt::Key_Minus,     JamKey::HIGH_GS},
        {Qt::Key_Y,         JamKey::HIGH_A},
        {Qt::Key_Egrave,    JamKey::HIGH_AS},
        {Qt::Key_U,         JamKey::HIGH_B},
        {Qt::Key_I,         JamKey::HIGH_C_H},
        {Qt::Key_Ccedilla,  JamKey::HIGH_CS_H},
        {Qt::Key_O,         JamKey::HIGH_D_H},
    };
    std::map<KeyboardLayout, std::map<Qt::Key, JamKey>> mappingLayouts = {
        {Custom, mappingCustom},
        {QWERTY, mappingQWERTY},
        {QWERTZ, mappingQWERTZ},
        {AZERTY, mappingAZERTY}
    };
	void setNoteEntryLayout(KeyboardLayout layout);
	KeyboardLayout getNoteEntryLayout() const;

private:
	std::string keyOffKey_, octUpKey_, octDownKey_, echoKey_;
	KeyboardLayout noteEntryLayout_;

	// Sound //
public:
	void setSoundDevice(std::string device);
	std::string getSoundDevice() const;
	void setUseSCCI(bool enabled);
	bool getUseSCCI() const;
	void setSampleRate(uint32_t rate);
	uint32_t getSampleRate() const;
	void setBufferLength(size_t length);
	size_t getBufferLength() const;
private:
	std::string sndDevice_;
	bool useSCCI_;
	uint32_t sampleRate_;
	size_t bufferLength_;

	// Midi //
public:
	void setMidiInputPort(const std::string &port);
	std::string getMidiInputPort() const;
private:
	std::string midiInPort_;

	// Mixer //
public:
	void setMixerVolumeMaster(int percentage);
	int getMixerVolumeMaster() const;
	void setMixerVolumeFM(double dB);
	double getMixerVolumeFM() const;
	void setMixerVolumeSSG(double dB);
	double getMixerVolumeSSG() const;
private:
	int mixerVolumeMaster_;
	double mixerVolumeFM_, mixerVolumeSSG_;

	// Input //
public:
	void setFMEnvelopeTexts(std::vector<FMEnvelopeText> texts);
	std::vector<FMEnvelopeText> getFMEnvelopeTexts() const;

private:
	std::vector<FMEnvelopeText> fmEnvelopeTexts_;
};
