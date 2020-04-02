#include "configuration.hpp"
#include "jam_manager.hpp"

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
	instADPCMH_ = 390;

	// Toolbar state
	mainTb_.setPosition(ToolbarConfiguration::TOP_POS);
	mainTb_.setNumber(0);
	mainTb_.setBreakBefore(false);
	mainTb_.setX(-1);	// Dummy
	mainTb_.setY(-1);	// Dummy
	subTb_.setPosition(ToolbarConfiguration::TOP_POS);
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
	showWaveVisual_ = true;
	fill00ToEffectValue_ = true;
	moveCursorHScroll_ = true;
	overwriteUnusedUnedited_ = false;
	writeOnlyUsedSamples_ = false;

	// Edit settings
	pageJumpLength_ = 4;
	editableStep_ = 1;
	keyRepetision_ = true;

	// Keys
	shortcuts_ = {
		{ KeyOff, u8"-" },
		{ OctaveUp, u8"Num+*" },
		{ OctaveDown, u8"Num+/" },
		{ EchoBuffer, u8"^" },
		{ PlayAndStop, u8"Return" },
		{ Play, u8"" },
		{ PlayFromStart, u8"F5" },
		{ PlayPattern, u8"F6" },
		{ PlayFromCursor, u8"F7" },
		{ PlayFromMarker, u8"Ctrl+F7" },
		{ PlayStep, u8"Ctrl+Return" },
		{ Stop, u8"F8" },
		{ FocusOnPattern, u8"F2" },
		{ FocusOnOrder, u8"F3" },
		{ FocusOnInstrument, u8"F4" },
		{ ToggleEditJam, u8"Space" },
		{ SetMarker, u8"Ctrl+B" },
		{ PasteMix, u8"Ctrl+M" },
		{ PasteOverwrite, u8"" },
		{ SelectAll, u8"Ctrl+A" },
		{ Deselect, u8"Esc" },
		{ SelectRow, u8"" },
		{ SelectColumn, u8"" },
		{ SelectPattern, u8"" },
		{ SelectOrder, u8"" },
		{ GoToStep, u8"Alt+G" },
		{ ToggleTrack, u8"Alt+F9" },
		{ SoloTrack, u8"Alt+F10" },
		{ Interpolate, u8"Ctrl+G" },
		{ Reverse, u8"Ctrl+R" },
		{ GoToPrevOrder, u8"Ctrl+Left" },
		{ GoToNextOrder, u8"Ctrl+Right" },
		{ ToggleBookmark, u8"Ctrl+K" },
		{ PrevBookmark, u8"Ctrl+PgUp" },
		{ NextBookmark, u8"Ctrl+PgDown" },
		{ DecreaseNote, u8"Ctrl+F1" },
		{ IncreaseNote, u8"Ctrl+F2" },
		{ DecreaseOctave, u8"Ctrl+F3" },
		{ IncreaseOctave, u8"Ctrl+F4" },
		{ PrevInstrument, u8"Alt+Left" },
		{ NextInstrument, u8"Alt+Right" },
		{ MaskInstrument, u8"" },
		{ MaskVolume, u8"" },
		{ EditInstrument, u8"Ctrl+I" },
		{ FollowMode, u8"ScrollLock" },
		{ DuplicateOrder, u8"Ctrl+D" },
		{ ClonePatterns, u8"Alt+D" },
		{ CloneOrder, u8"" },
		{ ReplaceInstrument, u8"Alt+S" },
		{ ExpandPattern, u8"" },
		{ ShrinkPattern, u8"" },
		{ FineDecreaseValues, u8"Shift+F1" },
		{ FineIncreaseValues, u8"Shift+F2" },
		{ CoarseDecreaseValues, u8"Shift+F3" },
		{ CoarseIncreaseValuse, u8"Shift+F4" },
		{ ExpandEffect, u8"Alt+L" },
		{ ShrinkEffect, u8"Alt+K" },
		{ PrevHighlighted, u8"Ctrl+Up" },
		{ NextHighlighted, u8"Ctrl+Down" },
	};
	noteEntryLayout_ = QWERTY;

	// Sound //
	sndAPI_ = u8"";
	sndDevice_ = u8"";
	realChip_ = RealChipInterface::NONE;
	emulator_ = 1;
	sampleRate_ = 44100;
	bufferLength_ = 40;

	// Midi //
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
	const std::unordered_map<std::string, JamKey> mappingQWERTY = {
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
	const std::unordered_map<std::string, JamKey> mappingQWERTZ = {
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
	const std::unordered_map<std::string, JamKey> mappingAZERTY = {
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
	mappingCustom = {};
	mappingLayouts = {
		{ Custom, mappingCustom },
		{ QWERTY, mappingQWERTY },
		{ QWERTZ, mappingQWERTZ },
		{ AZERTY, mappingAZERTY }
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

// Internal //
void Configuration::setFollowMode(bool enabled) { followMode_ = enabled; }

bool Configuration::getFollowMode() const { return followMode_; }

void Configuration::setWorkingDirectory(std::string path) { workDir_ = path; }

std::string Configuration::getWorkingDirectory() const { return workDir_; }

void Configuration::setInstrumentOpenFormat(int i) { instOpenFormat_ = i; }

int Configuration::getInstrumentOpenFormat() const { return instOpenFormat_; }

void Configuration::setBankOpenFormat(int i) { bankOpenFormat_ = i; }

int Configuration::getBankOpenFormat() const { return bankOpenFormat_; }

void Configuration::setInstrumentMask(bool enabled) { instMask_ = enabled; }

bool Configuration::getInstrumentMask() const { return instMask_; }

void Configuration::setVolumeMask(bool enabled) { volMask_ = enabled; }

bool Configuration::getVolumeMask() const { return volMask_; }

void Configuration::setVisibleToolbar(bool visible) { visibleToolbar_ = visible; }

bool Configuration::getVisibleToolbar() const { return visibleToolbar_; }

void Configuration::setVisibleStatusBar(bool visible) { visibleStatusBar_ = visible; }

bool Configuration::getVisibleStatusBar() const { return visibleStatusBar_; }

// Mainwindow state
void Configuration::setMainWindowWidth(int w) { mainW_ = w; }

int Configuration::getMainWindowWidth() const { return mainW_; }

void Configuration::setMainWindowHeight(int h) { mainH_ = h; }

int Configuration::getMainWindowHeight() const { return mainH_; }

void Configuration::setMainWindowMaximized(bool isMax) { mainMax_ = isMax; }

bool Configuration::getMainWindowMaximized() const { return mainMax_; }

void Configuration::setMainWindowX(int x) { mainX_ = x; }

int Configuration::getMainWindowX() const { return mainX_; }

void Configuration::setMainWindowY(int y) { mainY_ = y; }

int Configuration::getMainWindowY() const { return mainY_; }

void Configuration::setMainWindowVerticalSplit(int y) { mainVSplit_ = y; }

int Configuration::getMainWindowVerticalSplit() const { return mainVSplit_; }

// Instrument editor state
void Configuration::setInstrumentFMWindowWidth(int w) { instFMW_ = w; }

int Configuration::getInstrumentFMWindowWidth() const { return instFMW_; }

void Configuration::setInstrumentFMWindowHeight(int h) { instFMH_ = h; }

int Configuration::getInstrumentFMWindowHeight() const { return instFMH_; }

void Configuration::setInstrumentSSGWindowWidth(int w) { instSSGW_ = w; }

int Configuration::getInstrumentSSGWindowWidth() const { return instSSGW_; }

void Configuration::setInstrumentSSGWindowHeight(int h) { instSSGH_ = h; }

int Configuration::getInstrumentSSGWindowHeight() const { return instSSGH_; }

void Configuration::setInstrumentADPCMWindowWidth(int w) { instADPCMW_ = w; }

int Configuration::getInstrumentADPCMWindowWidth() const { return instADPCMW_; }

void Configuration::setInstrumentADPCMWindowHeight(int h) { instADPCMH_ = h; }

int Configuration::getInstrumentADPCMWindowHeight() const { return instADPCMH_; }

// Toolbar state
using TBConfig = Configuration::ToolbarConfiguration;
void TBConfig::setPosition(ToolbarPosition pos) { pos_ = pos; }

TBConfig::ToolbarPosition TBConfig::getPosition() const { return pos_; }

void TBConfig::setNumber(int n) { num_ = n; }

int TBConfig::getNumber() const { return num_; }

void TBConfig::setBreakBefore(bool enabled) { hasBreakBefore_ = enabled; }

bool TBConfig::hasBreakBefore() const { return hasBreakBefore_; }

void TBConfig::setX(int x) { x_ = x; }

int TBConfig::getX() const { return x_; }

void TBConfig::setY(int y) { y_ = y; }

int TBConfig::getY() const { return y_; }

TBConfig& Configuration::getMainToolbarConfiguration() { return mainTb_; }

TBConfig& Configuration::getSubToolbarConfiguration() { return subTb_; }

// General //
// General settings
void Configuration::setWarpCursor(bool enabled) { warpCursor_ = enabled; }

bool Configuration::getWarpCursor() const { return warpCursor_; }

void Configuration::setWarpAcrossOrders(bool enabled) { warpAcrossOrders_ = enabled; }

bool Configuration::getWarpAcrossOrders() const { return warpAcrossOrders_; }

void Configuration::setShowRowNumberInHex(bool enabled) { showRowNumHex_ = enabled; }

bool Configuration::getShowRowNumberInHex() const { return showRowNumHex_; }

void Configuration::setShowPreviousNextOrders(bool enabled) { showPrevNextOrders_ = enabled; }

bool Configuration::getShowPreviousNextOrders() const { return showPrevNextOrders_; }

void Configuration::setBackupModules(bool enabled) { backupModules_ = enabled; }

bool Configuration::getBackupModules() const { return backupModules_; }

void Configuration::setDontSelectOnDoubleClick(bool enabled) { dontSelectOnDoubleClick_ = enabled; }

bool Configuration::getDontSelectOnDoubleClick() const { return dontSelectOnDoubleClick_; }

void Configuration::setReverseFMVolumeOrder(bool enabled) { reverseFMVolumeOrder_= enabled; }

bool Configuration::getReverseFMVolumeOrder() const { return reverseFMVolumeOrder_; }

void Configuration::setMoveCursorToRight(bool enabled) { moveCursorToRight_ = enabled; }

bool Configuration::getMoveCursorToRight() const { return moveCursorToRight_; }

void Configuration::setRetrieveChannelState(bool enabled) { retrieveChannelState_ = enabled; }

bool Configuration::getRetrieveChannelState() const { return retrieveChannelState_; }

void Configuration::setEnableTranslation(bool enabled) { enableTranslation_ = enabled; }

bool Configuration::getEnableTranslation() const { return enableTranslation_; }

void Configuration::setShowFMDetuneAsSigned(bool enabled) { showFMDetuneSigned_ = enabled; }

bool Configuration::getShowFMDetuneAsSigned() const { return showFMDetuneSigned_; }

void Configuration::setShowWaveVisual(bool enabled) { showWaveVisual_ = enabled; }

bool Configuration::getShowWaveVisual() const { return showWaveVisual_; }

void Configuration::setFill00ToEffectValue(bool enabled) { fill00ToEffectValue_ = enabled; }

bool Configuration::getFill00ToEffectValue() const { return fill00ToEffectValue_; }

void Configuration::setMoveCursorByHorizontalScroll(bool enabled) { moveCursorHScroll_ = enabled; }

bool Configuration::getMoveCursorByHorizontalScroll() const { return moveCursorHScroll_; }

void Configuration::setOverwriteUnusedUneditedPropety(bool enabled) { overwriteUnusedUnedited_ = enabled; }

bool Configuration::getOverwriteUnusedUneditedPropety() const { return overwriteUnusedUnedited_; }

void Configuration::setWriteOnlyUsedSamples(bool enabled) { writeOnlyUsedSamples_ = enabled; }

bool Configuration::getWriteOnlyUsedSamples() const { return writeOnlyUsedSamples_; }

// Edit settings
void Configuration::setPageJumpLength(size_t length) { pageJumpLength_ = length; }

size_t Configuration::getPageJumpLength() const { return pageJumpLength_; }

void Configuration::setEditableStep(size_t step) { editableStep_ = step; }

size_t Configuration::getEditableStep() const { return editableStep_; }

void Configuration::setKeyRepetition(bool enabled) { keyRepetision_ = enabled; }

bool Configuration::getKeyRepetition() const { return keyRepetision_; }

// Keys
void Configuration::setShortcuts(std::unordered_map<ShortcutAction, std::string> shortcuts) { shortcuts_ = shortcuts; }

std::unordered_map<Configuration::ShortcutAction, std::string> Configuration::getShortcuts() const { return shortcuts_; }

void Configuration::setNoteEntryLayout(KeyboardLayout layout) { noteEntryLayout_ = layout; }

Configuration::KeyboardLayout Configuration::getNoteEntryLayout() const { return noteEntryLayout_; }

void Configuration::setCustomLayoutKeys(std::unordered_map<std::string, JamKey> mapping)
{
	mappingLayouts[KeyboardLayout::Custom] = mapping;
}

std::unordered_map<std::string, JamKey> Configuration::getCustomLayoutKeys() const
{
	return mappingLayouts.at(KeyboardLayout::Custom);
}

// Sound //
void Configuration::setSoundAPI(std::string api) { sndAPI_ = api; }

std::string Configuration::getSoundAPI() const { return sndAPI_; }

void Configuration::setSoundDevice(std::string device) { sndDevice_ = device; }

std::string Configuration::getSoundDevice() const { return sndDevice_; }

void Configuration::setRealChipInterface(RealChipInterface type) { realChip_ = type; }

RealChipInterface Configuration::getRealChipInterface() const { return realChip_; }

void Configuration::setEmulator(int emulator) { emulator_ = emulator; }

int Configuration::getEmulator() const { return emulator_; }

void Configuration::setSampleRate(uint32_t rate) { sampleRate_ = rate; }

uint32_t Configuration::getSampleRate() const { return sampleRate_; }

void Configuration::setBufferLength(size_t length) { bufferLength_ = length; }

size_t Configuration::getBufferLength() const { return bufferLength_; }

// Midi //
void Configuration::setMidiInputPort(const std::string &port) { midiInPort_ = port; }

std::string Configuration::getMidiInputPort() const { return midiInPort_; }

// Mixer //
void Configuration::setMixerVolumeMaster(int percentage) { mixerVolumeMaster_ = percentage; }

int Configuration::getMixerVolumeMaster() const { return mixerVolumeMaster_; }

void Configuration::setMixerVolumeFM(double dB) { mixerVolumeFM_ = dB; }

double Configuration::getMixerVolumeFM() const { return mixerVolumeFM_; }

void Configuration::setMixerVolumeSSG(double dB) { mixerVolumeSSG_ = dB; }

double Configuration::getMixerVolumeSSG() const { return mixerVolumeSSG_; }

// Input //
void Configuration::setFMEnvelopeTexts(std::vector<FMEnvelopeText> texts) { fmEnvelopeTexts_ = texts; }

std::vector<FMEnvelopeText> Configuration::getFMEnvelopeTexts() const { return fmEnvelopeTexts_; }

// Appearrance
void Configuration::setPatternEditorHeaderFont(std::string font) { ptnHdFont_ = font; }

std::string Configuration::getPatternEditorHeaderFont() const { return ptnHdFont_; }

void Configuration::setPatternEditorHeaderFontSize(int size) { ptnHdFontSize_ = size; }

int Configuration::getPatternEditorHeaderFontSize() const { return ptnHdFontSize_; }

void Configuration::setPatternEditorRowsFont(std::string font) { ptnRowFont_ = font; }

std::string Configuration::getPatternEditorRowsFont() const { return ptnRowFont_; }

void Configuration::setPatternEditorRowsFontSize(int size) { ptnRowFontSize_ = size; }

int Configuration::getPatternEditorRowsFontSize() const { return ptnRowFontSize_; }

void Configuration::setOrderListHeaderFont(std::string font) { odrHdFont_ = font; }

std::string Configuration::getOrderListHeaderFont() const { return odrHdFont_; }

void Configuration::setOrderListHeaderFontSize(int size) { odrHdFontSize_ = size; }

int Configuration::getOrderListHeaderFontSize() const { return odrHdFontSize_; }

void Configuration::setOrderListRowsFont(std::string font) { odrRowFont_ = font; }

std::string Configuration::getOrderListRowsFont() const { return odrRowFont_; }

void Configuration::setOrderListRowsFontSize(int size) { odrRowFontSize_ = size; }

int Configuration::getOrderListRowsFontSize() const { return odrRowFontSize_; }
