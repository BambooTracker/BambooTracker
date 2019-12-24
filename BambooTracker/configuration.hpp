#pragma once

#include <cstdint>
#include <cstddef>
#include <string>
#include <unordered_map>
#include <vector>
#include "enum_hash.hpp"
#include "misc.hpp"

enum class JamKey : int;

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
	void setMainWindowVerticalSplit(int y);
	int getMainWindowVerticalSplit() const;
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
	void setInstrumentOpenFormat(int i);
	int getInstrumentOpenFormat() const;
	void setBankOpenFormat(int i);
	int getBankOpenFormat() const;
private:
	int mainW_, mainH_;
	bool mainMax_;
	int mainX_, mainY_;
	int mainVSplit_;
	int instFMW_, instFMH_;
	int instSSGW_, instSSGH_;
	bool followMode_;
	std::string workDir_;
	int instOpenFormat_, bankOpenFormat_;

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
	void setShowWaveVisual(bool enabled);
	bool getShowWaveVisual() const;
	void setFill00ToEffectValue(bool enabled);
	bool getFill00ToEffectValue() const;
	void setAutosetInstrument(bool enabled);
	bool getAutosetInstrument() const;
private:
	bool warpCursor_, warpAcrossOrders_;
	bool showRowNumHex_, showPrevNextOrders_;
	bool backupModules_, dontSelectOnDoubleClick_;
	bool reverseFMVolumeOrder_, moveCursorToRight_;
	bool retrieveChannelState_, enableTranslation_;
	bool showFMDetuneSigned_;
	bool showWaveVisual_;
	bool fill00ToEffectValue_;
	bool autosetInstrument_;

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
	static const std::unordered_map<std::string, JamKey> mappingQWERTY, mappingQWERTZ, mappingAZERTY;
	std::unordered_map<std::string, JamKey> mappingCustom;
	std::unordered_map<KeyboardLayout, std::unordered_map<std::string, JamKey>> mappingLayouts;
	void setNoteEntryLayout(KeyboardLayout layout);
	KeyboardLayout getNoteEntryLayout() const;
	void setCustomLayoutKeys(std::unordered_map<std::string, JamKey> mapping);
	std::unordered_map<std::string, JamKey> getCustomLayoutKeys() const;

private:
	std::string keyOffKey_, octUpKey_, octDownKey_, echoKey_;
	KeyboardLayout noteEntryLayout_;

	// Sound //
public:
	void setSoundAPI(std::string api);
	std::string getSoundAPI() const;
	void setSoundDevice(std::string device);
	std::string getSoundDevice() const;
	void setUseSCCI(bool enabled);
	bool getUseSCCI() const;
	void setEmulator(int emulator);
	int getEmulator() const;
	void setSampleRate(uint32_t rate);
	uint32_t getSampleRate() const;
	void setBufferLength(size_t length);
	size_t getBufferLength() const;
private:
	std::string sndAPI_, sndDevice_;
	bool useSCCI_;
	int emulator_;
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

	// Appearance //
public:
	void setPatternEditorHeaderFont(std::string font);
	std::string getPatternEditorHeaderFont() const;
	void setPatternEditorHeaderFontSize(int size);
	int getPatternEditorHeaderFontSize() const;
	void setPatternEditorRowsFont(std::string font);
	std::string getPatternEditorRowsFont() const;
	void setPatternEditorRowsFontSize(int size);
	int getPatternEditorRowsFontSize() const;
	void setOrderListHeaderFont(std::string font);
	std::string getOrderListHeaderFont() const;
	void setOrderListHeaderFontSize(int size);
	int getOrderListHeaderFontSize() const;
	void setOrderListRowsFont(std::string font);
	std::string getOrderListRowsFont() const;
	void setOrderListRowsFontSize(int size);
	int getOrderListRowsFontSize() const;
private:
	std::string ptnHdFont_, ptnRowFont_, odrHdFont_, odrRowFont_;
	int ptnHdFontSize_, ptnRowFontSize_, odrHdFontSize_, odrRowFontSize_;

private:
	std::vector<FMEnvelopeText> fmEnvelopeTexts_;
};
