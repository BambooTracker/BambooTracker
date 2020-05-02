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
	void setFollowMode(bool enabled);
	bool getFollowMode() const;
	void setWorkingDirectory(std::string path);
	std::string getWorkingDirectory() const;
	void setInstrumentOpenFormat(int i);
	int getInstrumentOpenFormat() const;
	void setBankOpenFormat(int i);
	int getBankOpenFormat() const;
	void setInstrumentMask(bool enabled);
	bool getInstrumentMask() const;
	void setVolumeMask(bool enabled);
	bool getVolumeMask() const;
	void setVisibleToolbar(bool visible);
	bool getVisibleToolbar() const;
	void setVisibleStatusBar(bool visible);
	bool getVisibleStatusBar() const;
	void setVisibleWaveView(bool visible);
	bool getVisibleWaveView() const;
private:
	bool followMode_;
	std::string workDir_;
	int instOpenFormat_, bankOpenFormat_;
	bool instMask_, volMask_;
	bool visibleToolbar_, visibleStatusBar_, visibleWaveView_;

	// Mainwindow state
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
private:
	int mainW_, mainH_;
	bool mainMax_;
	int mainX_, mainY_;
	int mainVSplit_;

	// Instrument editor state
public:
	void setInstrumentFMWindowWidth(int w);
	int getInstrumentFMWindowWidth() const;
	void setInstrumentFMWindowHeight(int h);
	int getInstrumentFMWindowHeight() const;
	void setInstrumentSSGWindowWidth(int w);
	int getInstrumentSSGWindowWidth() const;
	void setInstrumentSSGWindowHeight(int h);
	int getInstrumentSSGWindowHeight() const;
	void setInstrumentADPCMWindowWidth(int w);
	int getInstrumentADPCMWindowWidth() const;
	void setInstrumentADPCMWindowHeight(int h);
	int getInstrumentADPCMWindowHeight() const;
	void setInstrumentDrumkitWindowWidth(int w);
	int getInstrumentDrumkitWindowWidth() const;
	void setInstrumentDrumkitWindowHeight(int h);
	int getInstrumentDrumkitWindowHeight() const;
private:
	int instFMW_, instFMH_;
	int instSSGW_, instSSGH_;
	int instADPCMW_, instADPCMH_;
	int instKitW_, instKitH_;

	// Toolbar state
public:
	class ToolbarConfiguration
	{
	public:
		enum ToolbarPosition : int { TOP_POS = 0, BOTTOM_POS, LEFT_POS, RIGHT_POS, FLOAT_POS };
		void setPosition(ToolbarPosition pos);
		ToolbarPosition getPosition() const;
		void setNumber(int n);
		int getNumber() const;
		void setBreakBefore(bool enabled);
		bool hasBreakBefore() const;
		void setX(int x);
		int getX() const;
		void setY(int y);
		int getY() const;
	private:
		ToolbarPosition pos_;
		int num_;
		bool hasBreakBefore_;
		int x_, y_;
	};
	ToolbarConfiguration& getMainToolbarConfiguration();
	ToolbarConfiguration& getSubToolbarConfiguration();
private:
	ToolbarConfiguration mainTb_, subTb_;

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
	void setFill00ToEffectValue(bool enabled);
	bool getFill00ToEffectValue() const;
	void setMoveCursorByHorizontalScroll(bool enabled);
	bool getMoveCursorByHorizontalScroll() const;
	void setOverwriteUnusedUneditedPropety(bool enabled);
	bool getOverwriteUnusedUneditedPropety() const;
	void setWriteOnlyUsedSamples(bool enabled);
	bool getWriteOnlyUsedSamples() const;
private:
	bool warpCursor_, warpAcrossOrders_, showRowNumHex_, showPrevNextOrders_, backupModules_;
	bool dontSelectOnDoubleClick_, reverseFMVolumeOrder_, moveCursorToRight_, retrieveChannelState_;
	bool enableTranslation_, showFMDetuneSigned_, fill00ToEffectValue_, moveCursorHScroll_;
	bool overwriteUnusedUnedited_, writeOnlyUsedSamples_;

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
	enum ShortcutAction : int
	{
		KeyOff, OctaveUp, OctaveDown, EchoBuffer, PlayAndStop, Play, PlayFromStart, PlayPattern,
		PlayFromCursor, PlayFromMarker, PlayStep, Stop, FocusOnPattern, FocusOnOrder, FocusOnInstrument,
		ToggleEditJam, SetMarker, PasteMix, PasteOverwrite, SelectAll, Deselect, SelectRow, SelectColumn,
		SelectPattern, SelectOrder, GoToStep, ToggleTrack, SoloTrack, Interpolate, Reverse, GoToPrevOrder,
		GoToNextOrder, ToggleBookmark, PrevBookmark, NextBookmark, DecreaseNote, IncreaseNote,
		DecreaseOctave, IncreaseOctave, PrevInstrument, NextInstrument, MaskInstrument, MaskVolume,
		EditInstrument, FollowMode, DuplicateOrder, ClonePatterns, CloneOrder, ReplaceInstrument,
		ExpandPattern, ShrinkPattern, FineDecreaseValues, FineIncreaseValues, CoarseDecreaseValues,
		CoarseIncreaseValuse, ExpandEffect, ShrinkEffect, PrevHighlighted, NextHighlighted,
		IncreasePatternSize, DecreasePatternSize, IncreaseEditStep, DecreaseEditStep, DisplayEffectList,
		PreviousSong, NextSong
	};
	void setShortcuts(std::unordered_map<ShortcutAction, std::string> shortcuts);
	std::unordered_map<ShortcutAction, std::string> getShortcuts() const;
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
	std::unordered_map<ShortcutAction, std::string> shortcuts_;
	KeyboardLayout noteEntryLayout_;

	// Sound //
public:
	void setSoundAPI(std::string api);
	std::string getSoundAPI() const;
	void setSoundDevice(std::string device);
	std::string getSoundDevice() const;
	void setRealChipInterface(RealChipInterface type);
	RealChipInterface getRealChipInterface() const;
	void setEmulator(int emulator);
	int getEmulator() const;
	void setSampleRate(uint32_t rate);
	uint32_t getSampleRate() const;
	void setBufferLength(size_t length);
	size_t getBufferLength() const;
private:
	std::string sndAPI_, sndDevice_;
	RealChipInterface realChip_;
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
