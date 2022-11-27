/*
 * Copyright (C) 2018-2022 Rerrah
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#pragma once

#include <cstdint>
#include <cstddef>
#include <string>
#include <unordered_map>
#include <vector>
#include "enum_hash.hpp"

enum class JamKey : int;
enum class RealChipInterfaceType : int;

enum class FMEnvelopeTextType : int
{
	Skip, AL, FB,
	AR1, DR1, SR1, RR1, SL1, TL1, KS1, ML1, DT1,
	AR2, DR2, SR2, RR2, SL2, TL2, KS2, ML2, DT2,
	AR3, DR3, SR3, RR3, SL3, TL3, KS3, ML3, DT3,
	AR4, DR4, SR4, RR4, SL4, TL4, KS4, ML4, DT4
};

struct FMEnvelopeText
{
	std::string name;
	std::vector<FMEnvelopeTextType> texts;
};

enum class NoteNotationSystem : int
{
	ENGLISH = 0,
	GERMAN = 1
};

namespace chip
{
enum class ResamplerType : int;
}

class Configuration
{
public:
	Configuration();

	// Internal //
public:
	void setFollowMode(bool enabled) { followMode_ = enabled; }
	bool getFollowMode() const { return followMode_; }
	void setWorkingDirectory(const std::string& path) { workDir_ = path; }
	std::string getWorkingDirectory() const { return workDir_; }
	void setInstrumentOpenFormat(int i) { instOpenFormat_ = i; }
	int getInstrumentOpenFormat() const { return instOpenFormat_; }
	void setBankOpenFormat(int i) { bankOpenFormat_ = i; }
	int getBankOpenFormat() const { return bankOpenFormat_; }
	void setInstrumentMask(bool enabled) { instMask_ = enabled; }
	bool getInstrumentMask() const { return instMask_; }
	void setVolumeMask(bool enabled) { volMask_ = enabled; }
	bool getVolumeMask() const { return volMask_; }
	void setVisibleToolbar(bool visible) { visibleToolbar_ = visible; }
	bool getVisibleToolbar() const { return visibleToolbar_; }
	void setVisibleStatusBar(bool visible) { visibleStatusBar_ = visible; }
	bool getVisibleStatusBar() const { return visibleStatusBar_; }
	void setVisibleWaveView(bool visible) { visibleWaveView_ = visible; }
	bool getVisibleWaveView() const { return visibleWaveView_; }
	enum class PasteMode : int { Cursor, Selection, Fill };
	void setPasteMode(PasteMode mode) { pasteMode_ = mode; }
	PasteMode getPasteMode() const { return pasteMode_; }
private:
	bool followMode_;
	std::string workDir_;
	int instOpenFormat_, bankOpenFormat_;
	bool instMask_, volMask_;
	bool visibleToolbar_, visibleStatusBar_, visibleWaveView_;
	PasteMode pasteMode_;

	// Mainwindow state
public:
	void setMainWindowWidth(int w) { mainW_ = w; }
	int getMainWindowWidth() const { return mainW_; }
	void setMainWindowHeight(int h) { mainH_ = h; }
	int getMainWindowHeight() const { return mainH_; }
	void setMainWindowMaximized(bool isMax) { mainMax_ = isMax; }
	bool getMainWindowMaximized() const { return mainMax_; }
	void setMainWindowX(int x) { mainX_ = x; }
	int getMainWindowX() const { return mainX_; }
	void setMainWindowY(int y) { mainY_ = y; }
	int getMainWindowY() const { return mainY_; }
	void setMainWindowVerticalSplit(int y) { mainVSplit_ = y; }
	int getMainWindowVerticalSplit() const { return mainVSplit_; }
private:
	int mainW_, mainH_;
	bool mainMax_;
	int mainX_, mainY_;
	int mainVSplit_;

	// Instrument editor state
public:
	void setInstrumentFMWindowWidth(int w) { instFMW_ = w; }
	int getInstrumentFMWindowWidth() const { return instFMW_; }
	void setInstrumentFMWindowHeight(int h) { instFMH_ = h; }
	int getInstrumentFMWindowHeight() const { return instFMH_; }
	void setInstrumentSSGWindowWidth(int w) { instSSGW_ = w; }
	int getInstrumentSSGWindowWidth() const { return instSSGW_; }
	void setInstrumentSSGWindowHeight(int h) { instSSGH_ = h; }
	int getInstrumentSSGWindowHeight() const { return instSSGH_; }
	void setInstrumentADPCMWindowWidth(int w) { instADPCMW_ = w; }
	int getInstrumentADPCMWindowWidth() const { return instADPCMW_; }
	void setInstrumentADPCMWindowHeight(int h) { instADPCMH_ = h; }
	int getInstrumentADPCMWindowHeight() const { return instADPCMH_; }
	void setInstrumentDrumkitWindowWidth(int w) { instKitW_ = w; }
	int getInstrumentDrumkitWindowWidth() const { return instKitW_; }
	void setInstrumentDrumkitWindowHeight(int h) { instKitH_ = h; }
	int getInstrumentDrumkitWindowHeight() const { return instKitH_; }
	void setInstrumentDrumkitWindowHorizontalSplit(int y) { instKitHSplit_ = y; }
	int getInstrumentDrumkitWindowHorizontalSplit() const { return instKitHSplit_; }
private:
	int instFMW_, instFMH_;
	int instSSGW_, instSSGH_;
	int instADPCMW_, instADPCMH_;
	int instKitW_, instKitH_;
	int instKitHSplit_;

	// Toolbar state
public:
	class ToolbarConfiguration
	{
	public:
		enum class Position : int { TopPosition = 0, BottomPosition, LeftPosition, RightPosition, FloatPorition };
		void setPosition(Position pos) { pos_ = pos; }
		Position getPosition() const { return pos_; }
		void setNumber(int n) { num_ = n; }
		int getNumber() const { return num_; }
		void setBreakBefore(bool enabled) { hasBreakBefore_ = enabled; }
		bool hasBreakBefore() const { return hasBreakBefore_; }
		void setX(int x) { x_ = x; }
		int getX() const { return x_; }
		void setY(int y) { y_ = y; }
		int getY() const { return y_; }
	private:
		Position pos_;
		int num_;
		bool hasBreakBefore_;
		int x_, y_;
	};
	using ToolbarPosition = ToolbarConfiguration::Position;
	ToolbarConfiguration& getMainToolbarConfiguration() { return mainTb_; }
	ToolbarConfiguration& getSubToolbarConfiguration() { return subTb_; }
private:
	ToolbarConfiguration mainTb_, subTb_;

	// General //
	// General settings
public:
	void setWarpCursor(bool enabled) { warpCursor_ = enabled; }
	bool getWarpCursor() const { return warpCursor_; }
	void setWarpAcrossOrders(bool enabled) { warpAcrossOrders_ = enabled; }
	bool getWarpAcrossOrders() const { return warpAcrossOrders_; }
	void setShowRowNumberInHex(bool enabled) { showRowNumHex_ = enabled; }
	bool getShowRowNumberInHex() const { return showRowNumHex_; }
	void setShowPreviousNextOrders(bool enabled) { showPrevNextOrders_ = enabled; }
	bool getShowPreviousNextOrders() const { return showPrevNextOrders_; }
	void setBackupModules(bool enabled) { backupModules_ = enabled; }
	bool getBackupModules() const { return backupModules_; }
	void setDontSelectOnDoubleClick(bool enabled) { dontSelectOnDoubleClick_ = enabled; }
	bool getDontSelectOnDoubleClick() const { return dontSelectOnDoubleClick_; }
	void setReverseFMVolumeOrder(bool enabled) { reverseFMVolumeOrder_= enabled; }
	bool getReverseFMVolumeOrder() const { return reverseFMVolumeOrder_; }
	void setMoveCursorToRight(bool enabled) { moveCursorToRight_ = enabled; }
	bool getMoveCursorToRight() const { return moveCursorToRight_; }
	void setRetrieveChannelState(bool enabled) { retrieveChannelState_ = enabled; }
	bool getRetrieveChannelState() const { return retrieveChannelState_; }
	void setEnableTranslation(bool enabled) { enableTranslation_ = enabled; }
	bool getEnableTranslation() const { return enableTranslation_; }
	void setShowFMDetuneAsSigned(bool enabled) { showFMDetuneSigned_ = enabled; }
	bool getShowFMDetuneAsSigned() const { return showFMDetuneSigned_; }
	void setFill00ToEffectValue(bool enabled) { fill00ToEffectValue_ = enabled; }
	bool getFill00ToEffectValue() const { return fill00ToEffectValue_; }
	void setMoveCursorByHorizontalScroll(bool enabled) { moveCursorHScroll_ = enabled; }
	bool getMoveCursorByHorizontalScroll() const { return moveCursorHScroll_; }
	void setOverwriteUnusedUneditedPropety(bool enabled) { overwriteUnusedUnedited_ = enabled; }
	bool getOverwriteUnusedUneditedPropety() const { return overwriteUnusedUnedited_; }
	void setWriteOnlyUsedSamples(bool enabled) { writeOnlyUsedSamples_ = enabled; }
	bool getWriteOnlyUsedSamples() const { return writeOnlyUsedSamples_; }
	void setReflectInstrumentNumberChange(bool enabled) { reflectInstNumChange_ = enabled; }
	bool getReflectInstrumentNumberChange() const { return reflectInstNumChange_; }
	void setFixJammingVolume(bool enabled) { fixJamVol_ = enabled; }
	bool getFixJammingVolume() const { return fixJamVol_; }
	void setMuteHiddenTracks(bool enabled) { muteHiddenTracks_ = enabled; }
	bool getMuteHiddenTracks() const { return muteHiddenTracks_; }
	void setRestoreTrackVisibility(bool enabled) { restoreTrackVis_ = enabled; }
	bool getRestoreTrackVisibility() const { return restoreTrackVis_; }
private:
	bool warpCursor_, warpAcrossOrders_, showRowNumHex_, showPrevNextOrders_, backupModules_;
	bool dontSelectOnDoubleClick_, reverseFMVolumeOrder_, moveCursorToRight_, retrieveChannelState_;
	bool enableTranslation_, showFMDetuneSigned_, fill00ToEffectValue_, moveCursorHScroll_;
	bool overwriteUnusedUnedited_, writeOnlyUsedSamples_, reflectInstNumChange_, fixJamVol_;
	bool muteHiddenTracks_, restoreTrackVis_;

	// Edit settings
public:
	void setPageJumpLength(size_t length) { pageJumpLength_ = length; }
	size_t getPageJumpLength() const { return pageJumpLength_; }
	void setEditableStep(size_t step) { editableStep_ = step; }
	size_t getEditableStep() const { return editableStep_; }
	void setKeyRepetition(bool enabled) { keyRepetision_ = enabled; }
	bool getKeyRepetition() const { return keyRepetision_; }
private:
	size_t pageJumpLength_, editableStep_;
	bool keyRepetision_;

	// Wave view
public:
	void setWaveViewFrameRate(int rate) { waveViewFps_ = rate; }
	int getWaveViewFrameRate() const { return waveViewFps_; }
private:
	int waveViewFps_;

	// Note names
public:
	void setNotationSystem(NoteNotationSystem sys) { notationSys_ = sys; }
	NoteNotationSystem getNotationSystem() const { return notationSys_; }

private:
	NoteNotationSystem notationSys_;

	// Keys
public:
	enum class ShortcutAction : int
	{
		KeyOff, OctaveUp, OctaveDown, EchoBuffer, PlayAndStop, Play, PlayFromStart, PlayPattern,
		PlayFromCursor, PlayFromMarker, PlayStep, Stop, FocusOnPattern, FocusOnOrder, FocusOnInstrument,
		ToggleEditJam, SetMarker, PasteMix, PasteOverwrite, PasteInsert, SelectAll, Deselect, SelectRow, SelectColumn,
		SelectPattern, SelectOrder, GoToStep, ToggleTrack, SoloTrack, Interpolate, Reverse, GoToPrevOrder,
		GoToNextOrder, ToggleBookmark, PrevBookmark, NextBookmark, DecreaseNote, IncreaseNote,
		DecreaseOctave, IncreaseOctave, PrevInstrument, NextInstrument, MaskInstrument, MaskVolume,
		EditInstrument, FollowMode, DuplicateOrder, ClonePatterns, CloneOrder, ReplaceInstrument,
		ExpandPattern, ShrinkPattern, FineDecreaseValues, FineIncreaseValues, CoarseDecreaseValues,
		CoarseIncreaseValuse, ExpandEffect, ShrinkEffect, PrevHighlighted, NextHighlighted,
		IncreasePatternSize, DecreasePatternSize, IncreaseEditStep, DecreaseEditStep, DisplayEffectList,
		PreviousSong, NextSong, JamVolumeUp, JamVolumeDown
	};
	void setShortcuts(std::unordered_map<ShortcutAction, std::string> shortcuts) { shortcuts_ = shortcuts; }
	std::unordered_map<ShortcutAction, std::string> getShortcuts() const { return shortcuts_; }
	enum class KeyboardLayout : int
	{
		// at the top, so new layouts can easily be added in after it
		// and it's always easy to find no matter how many layouts we add
		Custom = 0,
		QWERTY,
		QWERTZ,
		AZERTY
	};
	std::unordered_map<KeyboardLayout, std::unordered_map<std::string, JamKey>> mappingLayouts;
	void setNoteEntryLayout(KeyboardLayout layout) { noteEntryLayout_ = layout; }
	KeyboardLayout getNoteEntryLayout() const { return noteEntryLayout_; }
	void setCustomLayoutKeys(const std::unordered_map<std::string, JamKey>& mapping);
	std::unordered_map<std::string, JamKey> getCustomLayoutKeys() const;

private:
	std::unordered_map<ShortcutAction, std::string> shortcuts_;
	KeyboardLayout noteEntryLayout_;
	std::unordered_map<std::string, JamKey> mappingCustom_;

	// Sound //
public:
	void setSoundAPI(const std::string& api) { sndAPI_ = api; }
	std::string getSoundAPI() const { return sndAPI_; }
	void setSoundDevice(const std::string& device) { sndDevice_ = device; }
	std::string getSoundDevice() const { return sndDevice_; }
	void setRealChipInterface(RealChipInterfaceType type) { realChip_ = type; }
	RealChipInterfaceType getRealChipInterface() const { return realChip_; }
	void setEmulator(int emulator) { emulator_ = emulator; }
	int getEmulator() const { return emulator_; }
	void setSampleRate(uint32_t rate) { sampleRate_ = rate; }
	uint32_t getSampleRate() const { return sampleRate_; }
	void setBufferLength(size_t length) { bufferLength_ = length; }
	size_t getBufferLength() const { return bufferLength_; }
	void setResamplerType(chip::ResamplerType type) { resamplerType_ = type; }
	chip::ResamplerType getResamplerType() const { return resamplerType_; }
	void setImmediateWriteModeEnabled(bool enabled) { isImmediateWriteMode_ = enabled; }
	bool getImmediateWriteModeEnabled() const { return isImmediateWriteMode_; }

private:
	std::string sndAPI_, sndDevice_;
	RealChipInterfaceType realChip_;
	int emulator_;
	uint32_t sampleRate_;
	size_t bufferLength_;
	chip::ResamplerType resamplerType_;
	bool isImmediateWriteMode_;

	// Midi //
public:
	void setMidiEnabled(const bool enabled) { midiEnabled_ = enabled; }
	bool getMidiEnabled() const { return midiEnabled_; }
	void setMidiAPI(const std::string& api) { midiAPI_ = api; }
	std::string getMidiAPI() const { return midiAPI_; }
	void setMidiInputPort(const std::string& port) { midiInPort_ = port; }
	std::string getMidiInputPort() const { return midiInPort_; }
private:
	bool midiEnabled_;
	std::string midiAPI_, midiInPort_;

	// Mixer //
public:
	void setMixerVolumeMaster(int percentage) { mixerVolumeMaster_ = percentage; }
	int getMixerVolumeMaster() const { return mixerVolumeMaster_; }
	void setMixerVolumeFM(double dB) { mixerVolumeFM_ = dB; }
	double getMixerVolumeFM() const { return mixerVolumeFM_; }
	void setMixerVolumeSSG(double dB) { mixerVolumeSSG_ = dB; }
	double getMixerVolumeSSG() const { return mixerVolumeSSG_; }
private:
	int mixerVolumeMaster_;
	double mixerVolumeFM_, mixerVolumeSSG_;

	// Input //
public:
	void setFMEnvelopeTexts(const std::vector<FMEnvelopeText>& texts) { fmEnvelopeTexts_ = texts; }
	std::vector<FMEnvelopeText> getFMEnvelopeTexts() const { return fmEnvelopeTexts_; }

	// Appearance //
public:
	void setPatternEditorHeaderFont(const std::string& font) { ptnHdFont_ = font; }
	std::string getPatternEditorHeaderFont() const { return ptnHdFont_; }
	void setPatternEditorRowsFont(const std::string& font) { ptnRowFont_ = font; }
	std::string getPatternEditorRowsFont() const { return ptnRowFont_; }
	void setOrderListHeaderFont(const std::string& font) { odrHdFont_ = font; }
	std::string getOrderListHeaderFont() const { return odrHdFont_; }
	void setOrderListRowsFont(const std::string& font) { odrRowFont_ = font; }
	std::string getOrderListRowsFont() const { return odrRowFont_; }
private:
	std::string ptnHdFont_, ptnRowFont_, odrHdFont_, odrRowFont_;

private:
	std::vector<FMEnvelopeText> fmEnvelopeTexts_;
};
