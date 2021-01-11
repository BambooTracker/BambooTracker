/*
 * Copyright (C) 2018-2021 Rerrah
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

#include "configuration.hpp"
#include "jamming.hpp"

namespace
{
const std::unordered_map<std::string, JamKey> KEY_MAP_QWERTY = {
	{u8"Z",         JamKey::LowC},
	{u8"S",         JamKey::LowCS},
	{u8"X",         JamKey::LowD},
	{u8"D",         JamKey::LowDS},
	{u8"C",         JamKey::LowE},
	{u8"V",         JamKey::LowF},
	{u8"G",         JamKey::LowFS},
	{u8"B",         JamKey::LowG},
	{u8"H",         JamKey::LowGS},
	{u8"N",         JamKey::LowA},
	{u8"J",         JamKey::LowAS},
	{u8"M",         JamKey::LowB},
	{u8",",         JamKey::LowC2},
	{u8"L",         JamKey::LowCS2},
	{u8".",         JamKey::LowD2},

	{u8"Q",         JamKey::HighC},
	{u8"2",         JamKey::HighCS},
	{u8"W",         JamKey::HighD},
	{u8"3",         JamKey::HighDS},
	{u8"E",         JamKey::HighE},
	{u8"R",         JamKey::HighF},
	{u8"5",         JamKey::HighFS},
	{u8"T",         JamKey::HighG},
	{u8"6",         JamKey::HighGS},
	{u8"Y",         JamKey::HighA},
	{u8"7",         JamKey::HighAS},
	{u8"U",         JamKey::HighB},
	{u8"I",         JamKey::HighC2},
	{u8"9",         JamKey::HighCS2},
	{u8"O",         JamKey::HighD2},
};
const std::unordered_map<std::string, JamKey> KEY_MAP_QWERTZ = {
	{u8"Y",         JamKey::LowC},
	{u8"S",         JamKey::LowCS},
	{u8"X",         JamKey::LowD},
	{u8"D",         JamKey::LowDS},
	{u8"C",         JamKey::LowE},
	{u8"V",         JamKey::LowF},
	{u8"G",         JamKey::LowFS},
	{u8"B",         JamKey::LowG},
	{u8"H",         JamKey::LowGS},
	{u8"N",         JamKey::LowA},
	{u8"J",         JamKey::LowAS},
	{u8"M",         JamKey::LowB},
	{u8",",         JamKey::LowC2},
	{u8"L",         JamKey::LowCS2},
	{u8".",         JamKey::LowD2},

	{u8"Q",         JamKey::HighC},
	{u8"2",         JamKey::HighCS},
	{u8"W",         JamKey::HighD},
	{u8"3",         JamKey::HighDS},
	{u8"E",         JamKey::HighE},
	{u8"R",         JamKey::HighF},
	{u8"5",         JamKey::HighFS},
	{u8"T",         JamKey::HighG},
	{u8"6",         JamKey::HighGS},
	{u8"Z",         JamKey::HighA},
	{u8"7",         JamKey::HighAS},
	{u8"U",         JamKey::HighB},
	{u8"I",         JamKey::HighC2},
	{u8"9",         JamKey::HighCS2},
	{u8"O",         JamKey::HighD2},
};
const std::unordered_map<std::string, JamKey> KEY_MAP_AZERTY = {
	{u8"W",         JamKey::LowC},
	{u8"S",         JamKey::LowCS},
	{u8"X",         JamKey::LowD},
	{u8"D",         JamKey::LowDS},
	{u8"C",         JamKey::LowE},
	{u8"V",         JamKey::LowF},
	{u8"G",         JamKey::LowFS},
	{u8"B",         JamKey::LowG},
	{u8"H",         JamKey::LowGS},
	{u8"N",         JamKey::LowA},
	{u8"J",         JamKey::LowAS},
	{u8",",         JamKey::LowB},
	{u8";",         JamKey::LowC2},
	{u8"L",         JamKey::LowCS2},
	{u8".",         JamKey::LowD2},

	{u8"A",         JamKey::HighC},
	{u8"É",         JamKey::HighCS},   //é - \xc9
	{u8"Z",         JamKey::HighD},
	{u8"\"",        JamKey::HighDS},
	{u8"E",         JamKey::HighE},
	{u8"R",         JamKey::HighF},
	{u8"(",         JamKey::HighFS},
	{u8"T",         JamKey::HighG},
	{u8"-",         JamKey::HighGS},
	{u8"Y",         JamKey::HighA},
	{u8"È",         JamKey::HighAS},   //è - \xc8
	{u8"U",         JamKey::HighB},
	{u8"I",         JamKey::HighC2},
	{u8"Ç",         JamKey::HighCS2}, //ç - \xc7
	{u8"O",         JamKey::HighD2},
};
}

Configuration::Configuration()
{
	// Internal //
	followMode_ = true;
	workDir_ = "";
	instOpenFormat_ = 0;
	bankOpenFormat_ = 0;
	instMask_ = false;
	volMask_ = true;
	visibleToolbar_ = true;
	visibleStatusBar_ = true;
	visibleWaveView_ = true;
	pasteMode_ = PasteMode::Cursor;

	// Mainwindow state
	mainW_ = 930;
	mainH_= 780;
	mainMax_ = false;
	mainX_ = -1;	// Dummy
	mainY_ = -1;	// Dummy
	mainVSplit_ = -1;	// Dummy

	// Instrument editor state
	instFMW_ = 570;
	instFMH_ = 750;
	instSSGW_ = 510;
	instSSGH_ = 390;
	instADPCMW_ = 510;
	instADPCMH_ = 430;
	instKitW_ = 590;
	instKitH_ = 430;

	// Toolbar state
	mainTb_.setPosition(ToolbarPosition::TopPosition);
	mainTb_.setNumber(0);
	mainTb_.setBreakBefore(false);
	mainTb_.setX(-1);	// Dummy
	mainTb_.setY(-1);	// Dummy
	subTb_.setPosition(ToolbarPosition::TopPosition);
	subTb_.setNumber(1);
	subTb_.setBreakBefore(false);
	subTb_.setX(-1);	// Dummy
	subTb_.setY(-1);	// Dummy

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
	showFMDetuneSigned_ = false;
	fill00ToEffectValue_ = true;
	moveCursorHScroll_ = true;
	overwriteUnusedUnedited_ = false;
	writeOnlyUsedSamples_ = false;
	reflectInstNumChange_ = false;
	fixJamVol_ = true;
	muteHiddenTracks_ = true;
	restoreTrackVis_ = false;

	// Edit settings
	pageJumpLength_ = 4;
	editableStep_ = 1;
	keyRepetision_ = true;

	// Wave view
	waveViewFps_ = 30;

	// Keys
	shortcuts_ = {
		{ ShortcutAction::KeyOff, u8"-" },
		{ ShortcutAction::OctaveUp, u8"Num+*" },
		{ ShortcutAction::OctaveDown, u8"Num+/" },
		{ ShortcutAction::EchoBuffer, u8"^" },
		{ ShortcutAction::PlayAndStop, u8"Return" },
		{ ShortcutAction::Play, u8"" },
		{ ShortcutAction::PlayFromStart, u8"F5" },
		{ ShortcutAction::PlayPattern, u8"F6" },
		{ ShortcutAction::PlayFromCursor, u8"F7" },
		{ ShortcutAction::PlayFromMarker, u8"Ctrl+F7" },
		{ ShortcutAction::PlayStep, u8"Ctrl+Return" },
		{ ShortcutAction::Stop, u8"F8" },
		{ ShortcutAction::FocusOnPattern, u8"F2" },
		{ ShortcutAction::FocusOnOrder, u8"F3" },
		{ ShortcutAction::FocusOnInstrument, u8"F4" },
		{ ShortcutAction::ToggleEditJam, u8"Space" },
		{ ShortcutAction::SetMarker, u8"Ctrl+B" },
		{ ShortcutAction::PasteMix, u8"Ctrl+M" },
		{ ShortcutAction::PasteOverwrite, u8"" },
		{ ShortcutAction::PasteInsert, u8"" },
		{ ShortcutAction::SelectAll, u8"Ctrl+A" },
		{ ShortcutAction::Deselect, u8"Esc" },
		{ ShortcutAction::SelectRow, u8"" },
		{ ShortcutAction::SelectColumn, u8"" },
		{ ShortcutAction::SelectPattern, u8"" },
		{ ShortcutAction::SelectOrder, u8"" },
		{ ShortcutAction::GoToStep, u8"Alt+G" },
		{ ShortcutAction::ToggleTrack, u8"Alt+F9" },
		{ ShortcutAction::SoloTrack, u8"Alt+F10" },
		{ ShortcutAction::Interpolate, u8"Ctrl+G" },
		{ ShortcutAction::Reverse, u8"Ctrl+R" },
		{ ShortcutAction::GoToPrevOrder, u8"Ctrl+Left" },
		{ ShortcutAction::GoToNextOrder, u8"Ctrl+Right" },
		{ ShortcutAction::ToggleBookmark, u8"Ctrl+K" },
		{ ShortcutAction::PrevBookmark, u8"Ctrl+PgUp" },
		{ ShortcutAction::NextBookmark, u8"Ctrl+PgDown" },
		{ ShortcutAction::DecreaseNote, u8"Ctrl+F1" },
		{ ShortcutAction::IncreaseNote, u8"Ctrl+F2" },
		{ ShortcutAction::DecreaseOctave, u8"Ctrl+F3" },
		{ ShortcutAction::IncreaseOctave, u8"Ctrl+F4" },
		{ ShortcutAction::PrevInstrument, u8"Alt+Left" },
		{ ShortcutAction::NextInstrument, u8"Alt+Right" },
		{ ShortcutAction::MaskInstrument, u8"" },
		{ ShortcutAction::MaskVolume, u8"" },
		{ ShortcutAction::EditInstrument, u8"Ctrl+I" },
		{ ShortcutAction::FollowMode, u8"ScrollLock" },
		{ ShortcutAction::DuplicateOrder, u8"Ctrl+D" },
		{ ShortcutAction::ClonePatterns, u8"Alt+D" },
		{ ShortcutAction::CloneOrder, u8"" },
		{ ShortcutAction::ReplaceInstrument, u8"Alt+S" },
		{ ShortcutAction::ExpandPattern, u8"" },
		{ ShortcutAction::ShrinkPattern, u8"" },
		{ ShortcutAction::FineDecreaseValues, u8"Shift+F1" },
		{ ShortcutAction::FineIncreaseValues, u8"Shift+F2" },
		{ ShortcutAction::CoarseDecreaseValues, u8"Shift+F3" },
		{ ShortcutAction::CoarseIncreaseValuse, u8"Shift+F4" },
		{ ShortcutAction::ExpandEffect, u8"Alt+L" },
		{ ShortcutAction::ShrinkEffect, u8"Alt+K" },
		{ ShortcutAction::PrevHighlighted, u8"Ctrl+Up" },
		{ ShortcutAction::NextHighlighted, u8"Ctrl+Down" },
		{ ShortcutAction::IncreasePatternSize, u8"" },
		{ ShortcutAction::DecreasePatternSize, u8"" },
		{ ShortcutAction::IncreaseEditStep, u8"" },
		{ ShortcutAction::DecreaseEditStep, u8"" },
		{ ShortcutAction::DisplayEffectList, u8"F1" },
		{ ShortcutAction::PreviousSong, u8"" },
		{ ShortcutAction::NextSong, u8"" },
		{ ShortcutAction::JamVolumeUp, u8"" },
		{ ShortcutAction::JamVolumeDown, u8"" }
	};
	noteEntryLayout_ = KeyboardLayout::QWERTY;

	// Sound //
	sndAPI_ = u8"";
	sndDevice_ = u8"";
	realChip_ = RealChipInterface::NONE;
	emulator_ = 1;
	sampleRate_ = 44100;
	bufferLength_ = 40;

	// Midi //
	midiEnabled_ = false;
	midiAPI_ = u8"";
	midiInPort_ = u8"";

	// Mixer //
	mixerVolumeMaster_ = 100;
	mixerVolumeFM_ = 0;
	mixerVolumeSSG_ = 0;

	// Input //
	fmEnvelopeTexts_ = {
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

	// Layouts
	mappingCustom_ = {};
	mappingLayouts = {
		{ KeyboardLayout::Custom, mappingCustom_ },
		{ KeyboardLayout::QWERTY, KEY_MAP_QWERTY },
		{ KeyboardLayout::QWERTZ, KEY_MAP_QWERTZ },
		{ KeyboardLayout::AZERTY, KEY_MAP_AZERTY }
	};

	// Appearance
	ptnHdFont_ = u8"";
	ptnHdFontSize_ = 10;
	ptnRowFont_ = u8"";
	ptnRowFontSize_ = 10;
	odrHdFont_ = u8"";
	odrHdFontSize_ = 10;
	odrRowFont_ = u8"";
	odrRowFontSize_ = 10;
}

// Keys
void Configuration::setCustomLayoutKeys(const std::unordered_map<std::string, JamKey>& mapping)
{
	mappingLayouts[KeyboardLayout::Custom] = mapping;
}

std::unordered_map<std::string, JamKey> Configuration::getCustomLayoutKeys() const
{
	return mappingLayouts.at(KeyboardLayout::Custom);
}
