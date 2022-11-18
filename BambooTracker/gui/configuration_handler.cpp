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

#include "configuration_handler.hpp"
#include <vector>
#include <unordered_map>
#include <QString>
#include <QSettings>
#include <QFile>
#include "configuration.hpp"
#include "jamming.hpp"
#include "enum_hash.hpp"
#include "gui/gui_utils.hpp"
#include "utils.hpp"

namespace io
{
namespace
{
// config path (*nix): ~/.config/<organization>/<application>.ini
const QString APPLICATION = "BambooTracker";

const std::unordered_map<Configuration::ShortcutAction, QString> SHORTCUTS_NAME_MAP = {
	{ Configuration::ShortcutAction::KeyOff, "keyOff" },
	{ Configuration::ShortcutAction::OctaveUp, "octaveUp" },
	{ Configuration::ShortcutAction::OctaveDown, "octaveDown" },
	{ Configuration::ShortcutAction::EchoBuffer, "echoBuffer" },
	{ Configuration::ShortcutAction::PlayAndStop, "playAndStop" },
	{ Configuration::ShortcutAction::Play, "play" },
	{ Configuration::ShortcutAction::PlayFromStart, "playFromStart" },
	{ Configuration::ShortcutAction::PlayPattern, "playPattern" },
	{ Configuration::ShortcutAction::PlayFromCursor, "playFromCursor" },
	{ Configuration::ShortcutAction::PlayFromMarker, "playFromMarker" },
	{ Configuration::ShortcutAction::PlayStep, "playStep" },
	{ Configuration::ShortcutAction::Stop, "stop" },
	{ Configuration::ShortcutAction::FocusOnPattern, "ocusOnPattern" },
	{ Configuration::ShortcutAction::FocusOnOrder, "focusOnOrder" },
	{ Configuration::ShortcutAction::FocusOnInstrument, "focusOnInstrument" },
	{ Configuration::ShortcutAction::ToggleEditJam, "toggleEditJam" },
	{ Configuration::ShortcutAction::SetMarker, "setMarker" },
	{ Configuration::ShortcutAction::PasteMix, "pasteMix" },
	{ Configuration::ShortcutAction::PasteOverwrite, "pasteOverwrite" },
	{ Configuration::ShortcutAction::PasteInsert, "pasteInsert" },
	{ Configuration::ShortcutAction::SelectAll, "selectAll" },
	{ Configuration::ShortcutAction::Deselect, "deselect" },
	{ Configuration::ShortcutAction::SelectRow, "selectRow" },
	{ Configuration::ShortcutAction::SelectColumn, "selectColumn" },
	{ Configuration::ShortcutAction::SelectPattern, "selectPattern" },
	{ Configuration::ShortcutAction::SelectOrder, "selectOrder" },
	{ Configuration::ShortcutAction::GoToStep, "goToStep" },
	{ Configuration::ShortcutAction::ToggleTrack, "toggleTrack" },
	{ Configuration::ShortcutAction::SoloTrack, "soloTrack" },
	{ Configuration::ShortcutAction::Interpolate, "interpolate" },
	{ Configuration::ShortcutAction::Reverse, "reverse" },
	{ Configuration::ShortcutAction::GoToPrevOrder, "goToPrevOrder" },
	{ Configuration::ShortcutAction::GoToNextOrder, "goToNextOrder" },
	{ Configuration::ShortcutAction::ToggleBookmark, "toggleBookmark" },
	{ Configuration::ShortcutAction::PrevBookmark, "prevBookmark" },
	{ Configuration::ShortcutAction::NextBookmark, "nextBookmark" },
	{ Configuration::ShortcutAction::DecreaseNote, "decreaseNote" },
	{ Configuration::ShortcutAction::IncreaseNote, "increaseNote" },
	{ Configuration::ShortcutAction::DecreaseOctave, "decreaseOctave" },
	{ Configuration::ShortcutAction::IncreaseOctave, "increaseOctave" },
	{ Configuration::ShortcutAction::PrevInstrument, "prevInstrument" },
	{ Configuration::ShortcutAction::NextInstrument, "nextInstrument" },
	{ Configuration::ShortcutAction::MaskInstrument, "maskInstrument" },
	{ Configuration::ShortcutAction::MaskVolume, "maskVolume" },
	{ Configuration::ShortcutAction::EditInstrument, "editInstrument" },
	{ Configuration::ShortcutAction::FollowMode, "followMode" },
	{ Configuration::ShortcutAction::DuplicateOrder, "duplicateOrder" },
	{ Configuration::ShortcutAction::ClonePatterns, "clonePatterns" },
	{ Configuration::ShortcutAction::CloneOrder, "cloneOrder" },
	{ Configuration::ShortcutAction::ReplaceInstrument, "replaceInstrument" },
	{ Configuration::ShortcutAction::ExpandPattern, "expandPattern" },
	{ Configuration::ShortcutAction::ShrinkPattern, "shrinkPattern" },
	{ Configuration::ShortcutAction::FineDecreaseValues, "fineDecreaseValues" },
	{ Configuration::ShortcutAction::FineIncreaseValues, "fineIncreaseValues" },
	{ Configuration::ShortcutAction::CoarseDecreaseValues, "coarseDecreaseValues" },
	{ Configuration::ShortcutAction::CoarseIncreaseValuse, "coarseIncreaseValuse" },
	{ Configuration::ShortcutAction::ExpandEffect, "expandEffect" },
	{ Configuration::ShortcutAction::ShrinkEffect, "shrinkEffect" },
	{ Configuration::ShortcutAction::PrevHighlighted, "prevHighlightedStep" },
	{ Configuration::ShortcutAction::NextHighlighted, "nextHighlightedStep" },
	{ Configuration::ShortcutAction::IncreasePatternSize, "incPtnSize" },
	{ Configuration::ShortcutAction::DecreasePatternSize, "decPtnSize" },
	{ Configuration::ShortcutAction::IncreaseEditStep, "incEditStep" },
	{ Configuration::ShortcutAction::DecreaseEditStep, "decEditStep" },
	{ Configuration::ShortcutAction::DisplayEffectList, "dispEffectList" },
	{ Configuration::ShortcutAction::PreviousSong, "prevSong" },
	{ Configuration::ShortcutAction::NextSong, "nextSong" },
	{ Configuration::ShortcutAction::JamVolumeUp, "jamVolumeUp" },
	{ Configuration::ShortcutAction::JamVolumeDown, "jamVolumeDown" }
};

const std::unordered_map<JamKey, QString> JAM_KEY_NAME_MAP = {
	{JamKey::LowC,     "lowC"},
	{JamKey::LowCS,    "lowCS"},
	{JamKey::LowD,     "lowD"},
	{JamKey::LowDS,    "lowDS"},
	{JamKey::LowE,     "lowE"},
	{JamKey::LowF,     "lowF"},
	{JamKey::LowFS,    "lowFS"},
	{JamKey::LowG,     "lowG"},
	{JamKey::LowGS,    "lowGS"},
	{JamKey::LowA,     "lowA"},
	{JamKey::LowAS,    "lowAS"},
	{JamKey::LowB,     "lowB"},
	{JamKey::LowC2,   "lowHighC"},
	{JamKey::LowCS2,  "lowHighCS"},
	{JamKey::LowD2,   "lowHighD"},

	{JamKey::HighC,    "highC"},
	{JamKey::HighCS,   "highCS"},
	{JamKey::HighD,    "highD"},
	{JamKey::HighDS,   "highDS"},
	{JamKey::HighE,    "highE"},
	{JamKey::HighF,    "highF"},
	{JamKey::HighFS,   "highFS"},
	{JamKey::HighG,    "highG"},
	{JamKey::HighGS,   "highGS"},
	{JamKey::HighA,    "highA"},
	{JamKey::HighAS,   "highAS"},
	{JamKey::HighB,    "highB"},
	{JamKey::HighC2,  "highHighC"},
	{JamKey::HighCS2, "highHighCS"},
	{JamKey::HighD2,  "highHighD"}
};
}

bool saveConfiguration(std::weak_ptr<Configuration> config)
{
	try {
		QSettings settings(QSettings::IniFormat, QSettings::UserScope, io::ORGANIZATION_NAME, APPLICATION);
		std::shared_ptr<Configuration> configLocked = config.lock();

		// Internal //
		settings.beginGroup("Internal");
		settings.setValue("mainWindowWidth",           configLocked->getMainWindowWidth());
		settings.setValue("mainWindowHeight",          configLocked->getMainWindowHeight());
		settings.setValue("mainWindowMaximized",       configLocked->getMainWindowMaximized());
		settings.setValue("mainWindowX",               configLocked->getMainWindowX());
		settings.setValue("mainWindowY",               configLocked->getMainWindowY());
		settings.setValue("mainWindowVerticalSplit",	configLocked->getMainWindowVerticalSplit());
		settings.setValue("instrumentFMWindowWidth",   configLocked->getInstrumentFMWindowWidth());
		settings.setValue("instrumentFMWindowHeight",  configLocked->getInstrumentFMWindowHeight());
		settings.setValue("instrumentSSGWindowWidth",  configLocked->getInstrumentSSGWindowWidth());
		settings.setValue("instrumentSSGWindowHeight", configLocked->getInstrumentSSGWindowHeight());
		settings.setValue("instrumentADPCMWindowWidth",  configLocked->getInstrumentADPCMWindowWidth());
		settings.setValue("instrumentADPCMWindowHeight", configLocked->getInstrumentADPCMWindowHeight());
		settings.setValue("instrumentDrumkitWindowWidth",  configLocked->getInstrumentDrumkitWindowWidth());
		settings.setValue("instrumentDrumkitWindowHeight", configLocked->getInstrumentDrumkitWindowHeight());
		settings.setValue("instrumentDrumkitWindowHorizontalSplit", configLocked->getInstrumentDrumkitWindowHorizontalSplit());
		settings.setValue("followMode",		configLocked->getFollowMode());
		settings.setValue("workingDirectory",          QString::fromStdString(configLocked->getWorkingDirectory()));
		settings.setValue("instrumentOpenFormat",		configLocked->getInstrumentOpenFormat());
		settings.setValue("bankOpenFormat",				configLocked->getBankOpenFormat());
		settings.setValue("instrumentMask",				configLocked->getInstrumentMask());
		settings.setValue("volumeMask",					configLocked->getVolumeMask());
		settings.setValue("visibleToolbar",				configLocked->getVisibleToolbar());
		settings.setValue("visibleStatusBar",			configLocked->getVisibleStatusBar());
		settings.setValue("visibleWaveView",			configLocked->getVisibleWaveView());
		settings.setValue("pasteMode",					static_cast<int>(configLocked->getPasteMode()));
		auto& mainTbConfig = configLocked->getMainToolbarConfiguration();
		settings.setValue("mainToolbarPosition",		static_cast<int>(mainTbConfig.getPosition()));
		settings.setValue("mainToolbarNumber",			mainTbConfig.getNumber());
		settings.setValue("hasBreakBeforeMainToolbar",	mainTbConfig.hasBreakBefore());
		settings.setValue("mainToolbarX",				mainTbConfig.getX());
		settings.setValue("mainToolbarY",				mainTbConfig.getY());
		auto& subTbConfig = configLocked->getSubToolbarConfiguration();
		settings.setValue("subToolbarPosition",			static_cast<int>(subTbConfig.getPosition()));
		settings.setValue("subToolbarNumber",			subTbConfig.getNumber());
		settings.setValue("hasBreakBeforesubToolbar",	subTbConfig.hasBreakBefore());
		settings.setValue("subToolbarX",				subTbConfig.getX());
		settings.setValue("subToolbarY",				subTbConfig.getY());
		settings.endGroup();

		// General //
		// General settings
		settings.beginGroup("General");
		settings.setValue("warpCursor",              configLocked->getWarpCursor());
		settings.setValue("warpAcrossOrders",        configLocked->getWarpAcrossOrders());
		settings.setValue("showRowNumberInHex",      configLocked->getShowRowNumberInHex());
		settings.setValue("showPreviousNextOrders",  configLocked->getShowPreviousNextOrders());
		settings.setValue("backupModule",            configLocked->getBackupModules());
		settings.setValue("dontSelectOnDoubleClick", configLocked->getDontSelectOnDoubleClick());
		settings.setValue("reverseFMVolumeOrder",    configLocked->getReverseFMVolumeOrder());
		settings.setValue("moveCursorToRight",       configLocked->getMoveCursorToRight());
		settings.setValue("retrieveChannelState",	configLocked->getRetrieveChannelState());
		settings.setValue("enableTranslation",		configLocked->getEnableTranslation());
		settings.setValue("showFMDetuneAsSigned",	configLocked->getShowFMDetuneAsSigned());
		settings.setValue("fill00ToEffectValue",	configLocked->getFill00ToEffectValue());
		settings.setValue("moveCursorByHScroll",	configLocked->getMoveCursorByHorizontalScroll());
		settings.setValue("overwriteUnusedUnedited",	configLocked->getOverwriteUnusedUneditedPropety());
		settings.setValue("writeOnlyUsedSamples",	configLocked->getWriteOnlyUsedSamples());
		settings.setValue("reflectInstNumChange",	configLocked->getReflectInstrumentNumberChange());
		settings.setValue("fixJammingVolume",		configLocked->getFixJammingVolume());
		settings.setValue("muteHiddenTracks",		configLocked->getMuteHiddenTracks());
		settings.setValue("restoreTrackVisibility",	configLocked->getRestoreTrackVisibility());
		settings.endGroup();

		// Edit settings
		settings.beginGroup("Editing");
		settings.setValue("pageJumpLength", static_cast<int>(configLocked->getPageJumpLength()));
		settings.setValue("editableStep", static_cast<int>(configLocked->getEditableStep()));
		settings.setValue("keyRepetition", configLocked->getKeyRepetition());
		settings.endGroup();

		// Wave view
		settings.beginGroup("WaveView");
		settings.setValue("frameRate", configLocked->getWaveViewFrameRate());
		settings.endGroup();

		// Note names
		settings.beginGroup("NoteNames");
		settings.setValue("notationSystem", static_cast<int>(configLocked->getNotationSystem()));
		settings.endGroup();

		// Keys
		settings.beginGroup("Keys");
		for (const auto& pair : configLocked->getShortcuts()) {
			settings.setValue("shortcut_" + SHORTCUTS_NAME_MAP.at(pair.first), gui_utils::utf8ToQString(pair.second));
		}
		settings.setValue("noteEntryLayout", static_cast<int>(configLocked->getNoteEntryLayout()));
		for (const auto& pair : configLocked->getCustomLayoutKeys()) {
			settings.setValue("customLayout_" + JAM_KEY_NAME_MAP.at(pair.second), gui_utils::utf8ToQString(pair.first));
		}
		settings.endGroup();

		// Sound //
		settings.beginGroup("Sound");
		settings.setValue("soundAPI", gui_utils::utf8ToQString(configLocked->getSoundAPI()));
		settings.setValue("soundDevice", gui_utils::utf8ToQString(configLocked->getSoundDevice()));
		settings.setValue("realChipInterface",	static_cast<int>(configLocked->getRealChipInterface()));
		settings.setValue("emulator",		configLocked->getEmulator());
		settings.setValue("sampleRate",   static_cast<int>(configLocked->getSampleRate()));
		settings.setValue("bufferLength", static_cast<int>(configLocked->getBufferLength()));
		settings.setValue("resamplerType", static_cast<int>(configLocked->getResamplerType()));
		settings.endGroup();

		// Midi //
		settings.beginGroup("Midi");
		settings.setValue("midiEnabled", configLocked->getMidiEnabled());
		settings.setValue("midiAPI", QString::fromStdString(configLocked->getMidiAPI()));
		settings.setValue("inputPort", QString::fromStdString(configLocked->getMidiInputPort()));
		settings.endGroup();

		// Mixer //
		settings.beginGroup("Mixer");
		settings.setValue("mixerVolumeMaster",	configLocked->getMixerVolumeMaster());
		settings.setValue("mixerVolumeFM",		configLocked->getMixerVolumeFM());
		settings.setValue("mixerVolumeSSG",		configLocked->getMixerVolumeSSG());
		settings.endGroup();

		// Input //
		settings.beginGroup("Input");
		settings.beginWriteArray("fmEnvelopeTextMap");
		int n = 0;
		for (const FMEnvelopeText& texts : config.lock()->getFMEnvelopeTexts()) {
			settings.setArrayIndex(n++);
			settings.setValue("type", gui_utils::utf8ToQString(texts.name));
			QStringList typeList;
			std::transform(texts.texts.begin(), texts.texts.end(), std::back_inserter(typeList),
						   [](FMEnvelopeTextType type) { return QString::number(static_cast<int>(type)); });
			settings.setValue("order", typeList.join(","));
		}
		settings.endArray();
		settings.endGroup();

		settings.beginGroup("Appearance");
		settings.setValue("patternEditorHeaderFont", QString::fromStdString(configLocked->getPatternEditorHeaderFont()));
		settings.setValue("patternEditorRowsFont", QString::fromStdString(configLocked->getPatternEditorRowsFont()));
		settings.setValue("orderListHeaderFont", QString::fromStdString(configLocked->getOrderListHeaderFont()));
		settings.setValue("orderListRowsFont", QString::fromStdString(configLocked->getOrderListRowsFont()));
		settings.endGroup();

		return true;
	} catch (...) {
		return false;
	}
}

bool loadConfiguration(std::weak_ptr<Configuration> config)
{
	try {
		QSettings settings(QSettings::IniFormat, QSettings::UserScope, io::ORGANIZATION_NAME, APPLICATION);
		if (!QFile(settings.fileName()).exists()) return false;

		std::shared_ptr<Configuration> configLocked = config.lock();

		// Internal //
		settings.beginGroup("Internal");
		configLocked->setMainWindowWidth(settings.value("mainWindowWidth", configLocked->getMainWindowWidth()).toInt());
		configLocked->setMainWindowHeight(settings.value("mainWindowHeight", configLocked->getMainWindowHeight()).toInt());
		configLocked->setMainWindowMaximized(settings.value("mainWindowMaximized", configLocked->getMainWindowMaximized()).toBool());
		configLocked->setMainWindowX(settings.value("mainWindowX", configLocked->getMainWindowX()).toInt());
		configLocked->setMainWindowY(settings.value("mainWindowY", configLocked->getMainWindowY()).toInt());
		configLocked->setMainWindowVerticalSplit(settings.value("mainWindowVerticalSplit", configLocked->getMainWindowVerticalSplit()).toInt());
		configLocked->setInstrumentFMWindowWidth(settings.value("instrumentFMWindowWidth", configLocked->getInstrumentFMWindowWidth()).toInt());
		configLocked->setInstrumentFMWindowHeight(settings.value("instrumentFMWindowHeight", configLocked->getInstrumentFMWindowHeight()).toInt());
		configLocked->setInstrumentSSGWindowWidth(settings.value("instrumentSSGWindowWidth", configLocked->getInstrumentSSGWindowWidth()).toInt());
		configLocked->setInstrumentSSGWindowHeight(settings.value("instrumentSSGWindowHeight", configLocked->getInstrumentSSGWindowHeight()).toInt());
		configLocked->setInstrumentADPCMWindowWidth(settings.value("instrumentADPCMWindowWidth", configLocked->getInstrumentADPCMWindowWidth()).toInt());
		configLocked->setInstrumentADPCMWindowHeight(settings.value("instrumentADPCMWindowHeight", configLocked->getInstrumentADPCMWindowHeight()).toInt());
		configLocked->setInstrumentDrumkitWindowWidth(settings.value("instrumentDrumkitWindowWidth", configLocked->getInstrumentDrumkitWindowWidth()).toInt());
		configLocked->setInstrumentDrumkitWindowHorizontalSplit(settings.value("instrumentDrumkitWindowHorizontalSplit", configLocked->getInstrumentDrumkitWindowHorizontalSplit()).toInt());
		configLocked->setInstrumentDrumkitWindowHeight(settings.value("instrumentDrumkitWindowHeight", configLocked->getInstrumentDrumkitWindowHeight()).toInt());
		configLocked->setFollowMode(settings.value("followMode", configLocked->getFollowMode()).toBool());
		configLocked->setWorkingDirectory(settings.value("workingDirectory", QString::fromStdString(configLocked->getWorkingDirectory())).toString().toStdString());
		configLocked->setInstrumentOpenFormat(settings.value("instrumentOpenFormat", configLocked->getInstrumentOpenFormat()).toInt());
		configLocked->setBankOpenFormat(settings.value("bankOpenFormat", configLocked->getBankOpenFormat()).toInt());
		configLocked->setInstrumentMask(settings.value("instrumentMask", configLocked->getInstrumentMask()).toBool());
		configLocked->setVolumeMask(settings.value("volumeMask", configLocked->getVolumeMask()).toBool());
		configLocked->setVisibleToolbar(settings.value("visibleToolbar", configLocked->getVisibleToolbar()).toBool());
		configLocked->setVisibleStatusBar(settings.value("visibleStatusBar", configLocked->getVisibleStatusBar()).toBool());
		configLocked->setVisibleWaveView(settings.value("visibleWaveView", configLocked->getVisibleWaveView()).toBool());
		configLocked->setPasteMode(static_cast<Configuration::PasteMode>(settings.value("pasteMode", static_cast<int>(configLocked->getPasteMode())).toInt()));
		auto& mainTbConfig = configLocked->getMainToolbarConfiguration();
		mainTbConfig.setPosition(static_cast<Configuration::ToolbarPosition>(settings.value("mainToolbarPosition", static_cast<int>(mainTbConfig.getPosition())).toInt()));
		mainTbConfig.setNumber(settings.value("mainToolbarNumber", mainTbConfig.getNumber()).toInt());
		mainTbConfig.setBreakBefore(settings.value("hasBreakBeforeMainToolbar", mainTbConfig.hasBreakBefore()).toBool());
		mainTbConfig.setX(settings.value("mainToolbarX", mainTbConfig.getX()).toInt());
		mainTbConfig.setY(settings.value("mainToolbarY", mainTbConfig.getY()).toInt());
		auto& subTbConfig = configLocked->getSubToolbarConfiguration();
		subTbConfig.setPosition(static_cast<Configuration::ToolbarPosition>(settings.value("subToolbarPosition", static_cast<int>(subTbConfig.getPosition())).toInt()));
		subTbConfig.setNumber(settings.value("subToolbarNumber", subTbConfig.getNumber()).toInt());
		subTbConfig.setBreakBefore(settings.value("hasBreakBeforesubToolbar", subTbConfig.hasBreakBefore()).toBool());
		subTbConfig.setX(settings.value("subToolbarX", subTbConfig.getX()).toInt());
		subTbConfig.setY(settings.value("subToolbarY", subTbConfig.getY()).toInt());
		settings.endGroup();

		// General //
		// General settings
		settings.beginGroup("General");
		configLocked->setWarpCursor(settings.value("warpCursor", configLocked->getWarpCursor()).toBool());
		configLocked->setWarpAcrossOrders(settings.value("warpAcrossOrders", configLocked->getWarpAcrossOrders()).toBool());
		configLocked->setShowRowNumberInHex(settings.value("showRowNumberInHex", configLocked->getShowRowNumberInHex()).toBool());
		configLocked->setShowPreviousNextOrders(settings.value("showPreviousNextOrders", configLocked->getShowPreviousNextOrders()).toBool());
		configLocked->setBackupModules(settings.value("backupModule", configLocked->getBackupModules()).toBool());
		configLocked->setDontSelectOnDoubleClick(settings.value("dontSelectOnDoubleClick", configLocked->getDontSelectOnDoubleClick()).toBool());
		configLocked->setReverseFMVolumeOrder(settings.value("reverseFMVolumeOrder", configLocked->getReverseFMVolumeOrder()).toBool());
		configLocked->setMoveCursorToRight(settings.value("moveCursorToRight", configLocked->getMoveCursorToRight()).toBool());
		configLocked->setRetrieveChannelState(settings.value("retrieveChannelState", configLocked->getRetrieveChannelState()).toBool());
		configLocked->setEnableTranslation(settings.value("enableTranslation", configLocked->getEnableTranslation()).toBool());
		configLocked->setShowFMDetuneAsSigned(settings.value("showFMDetuneAsSigned", configLocked->getShowFMDetuneAsSigned()).toBool());
		configLocked->setFill00ToEffectValue(settings.value("fill00ToEffectValue", configLocked->getFill00ToEffectValue()).toBool());
		configLocked->setMoveCursorByHorizontalScroll(settings.value("moveCursorByHScroll", configLocked->getMoveCursorByHorizontalScroll()).toBool());
		configLocked->setOverwriteUnusedUneditedPropety(settings.value("overwriteUnusedUnedited", configLocked->getOverwriteUnusedUneditedPropety()).toBool());
		configLocked->setWriteOnlyUsedSamples(settings.value("writeOnlyUsedSamples", configLocked->getWriteOnlyUsedSamples()).toBool());
		configLocked->setReflectInstrumentNumberChange(settings.value("reflectInstNumChange", configLocked->getReflectInstrumentNumberChange()).toBool());
		configLocked->setFixJammingVolume(settings.value("fixJammingVolume", configLocked->getFixJammingVolume()).toBool());
		configLocked->setMuteHiddenTracks(settings.value("muteHiddenTracks", configLocked->getMuteHiddenTracks()).toBool());
		configLocked->setRestoreTrackVisibility(settings.value("restoreTrackVisibility", configLocked->getRestoreTrackVisibility()).toBool());
		if (settings.contains("autosetInstrument")) {	// For compatibility before v0.4.0
			configLocked->setInstrumentMask(!settings.value("autosetInstrument").toBool());
			settings.remove("autosetInstrument");
		}
		if (settings.contains("showWaveVisual")) {	// For compatibility before v0.4.2
			configLocked->setVisibleWaveView(settings.value("showWaveVisual").toBool());
			settings.remove("showWaveVisual");
		}
		settings.endGroup();

		// Edit settings
		settings.beginGroup("Editing");
		QVariant pageJumpLengthWorkaround;
		pageJumpLengthWorkaround.setValue(configLocked->getPageJumpLength());
		configLocked->setPageJumpLength(static_cast<size_t>(settings.value("pageJumpLength", pageJumpLengthWorkaround).toInt()));
		QVariant editableStepWorkaround;
		editableStepWorkaround.setValue(configLocked->getEditableStep());
		configLocked->setEditableStep(static_cast<size_t>(settings.value("editableStep", editableStepWorkaround).toInt()));
		configLocked->setKeyRepetition(settings.value("keyRepetition", configLocked->getKeyRepetition()).toBool());
		settings.endGroup();

		// Wave view
		settings.beginGroup("WaveView");
		configLocked->setWaveViewFrameRate(settings.value("frameRate", configLocked->getWaveViewFrameRate()).toInt());
		settings.endGroup();

		// Note names
		settings.beginGroup("NoteNames");
		configLocked->setNotationSystem(
					static_cast<NoteNotationSystem>(settings.value("notationSystem", static_cast<int>(configLocked->getNotationSystem())).toInt()));
		settings.endGroup();

		// Keys
		settings.beginGroup("Keys");
		std::unordered_map<Configuration::ShortcutAction, std::string> shortcuts;
		for (const auto& pair : SHORTCUTS_NAME_MAP) {
			std::string def = configLocked->getShortcuts().at(pair.first);
			shortcuts[pair.first] = settings.value("shortcut_" + pair.second, gui_utils::utf8ToQString(def)).toString().toUtf8().toStdString();
		}
		configLocked->setShortcuts(shortcuts);
		configLocked->setNoteEntryLayout(static_cast<Configuration::KeyboardLayout>(
											 settings.value("noteEntryLayout",
															static_cast<int>(configLocked->getNoteEntryLayout())).toInt()));
		std::unordered_map<std::string, JamKey> customLayoutNewKeys;
		for (const auto& pair : JAM_KEY_NAME_MAP) {
			JamKey currentlyWantedJamKey = pair.first;
			customLayoutNewKeys[
					settings.value("customLayout_" + pair.second,
								   QString::fromStdString((*utils::findIf(configLocked->mappingLayouts.at(Configuration::KeyboardLayout::QWERTY),
																		  [currentlyWantedJamKey](const std::pair<std::string, JamKey>& t) -> bool {
				return (t.second) == currentlyWantedJamKey;})
														   ).first)).toString().toUtf8().toStdString()]
					= currentlyWantedJamKey;
		}
		configLocked->setCustomLayoutKeys(customLayoutNewKeys);
		settings.endGroup();

		// Sound //
		settings.beginGroup("Sound");
		configLocked->setSoundAPI(settings.value("soundAPI", QString::fromStdString(configLocked->getSoundAPI())).toString().toUtf8().toStdString());
		configLocked->setSoundDevice(settings.value("soundDevice", QString::fromStdString(configLocked->getSoundDevice())).toString().toUtf8().toStdString());
		configLocked->setRealChipInterface(static_cast<RealChipInterfaceType>(
											   settings.value("realChipInterface", static_cast<int>(configLocked->getRealChipInterface())).toInt()));
		configLocked->setEmulator(settings.value("emulator", configLocked->getEmulator()).toInt());
		QVariant sampleRateWorkaround;
		sampleRateWorkaround.setValue(configLocked->getSampleRate());
		configLocked->setSampleRate(static_cast<uint32_t>(settings.value("sampleRate", sampleRateWorkaround).toInt()));
		QVariant bufferLengthWorkaround;
		bufferLengthWorkaround.setValue(configLocked->getBufferLength());
		configLocked->setBufferLength(static_cast<size_t>(settings.value("bufferLength", bufferLengthWorkaround).toInt()));
		configLocked->setResamplerType(static_cast<chip::ResamplerType>(
										   settings.value("resamplerType", static_cast<int>(configLocked->getResamplerType())).toInt()));
		settings.endGroup();

		// Midi //
		settings.beginGroup("Midi");
		configLocked->setMidiEnabled(settings.value("midiEnabled", configLocked->getMidiEnabled()).toBool());
		configLocked->setMidiAPI(settings.value("midiAPI", QString::fromStdString(configLocked->getMidiAPI())).toString().toStdString());
		configLocked->setMidiInputPort(settings.value("inputPort", QString::fromStdString(configLocked->getMidiInputPort())).toString().toStdString());
		settings.endGroup();

		// Mixer //
		settings.beginGroup("Mixer");
		configLocked->setMixerVolumeMaster(settings.value("mixerVolumeMaster", configLocked->getMixerVolumeMaster()).toInt());
		configLocked->setMixerVolumeFM(settings.value("mixerVolumeFM", configLocked->getMixerVolumeFM()).toDouble());
		configLocked->setMixerVolumeSSG(settings.value("mixerVolumeSSG", configLocked->getMixerVolumeSSG()).toDouble());
		settings.endGroup();

		// Input //
		settings.beginGroup("Input");
		int size = settings.beginReadArray("fmEnvelopeTextMap");
		std::vector<FMEnvelopeText> fmEnvelopeTexts;
		for (int i = 0; i < size; ++i) {
			settings.setArrayIndex(i);
			std::string type = settings.value("type").toString().toUtf8().toStdString();
			std::vector<FMEnvelopeTextType> data;
			const QStringList list = settings.value("order").toString().split(",");
			for (const QString& d : list) {
				data.push_back(static_cast<FMEnvelopeTextType>(d.toInt()));
			}
			fmEnvelopeTexts.push_back({ type, data });
		}
		if (!fmEnvelopeTexts.empty()) config.lock()->setFMEnvelopeTexts(fmEnvelopeTexts);
		settings.endArray();
		settings.endGroup();

		// Appearance
		settings.beginGroup("Appearance");
		configLocked->setPatternEditorHeaderFont(settings.value("patternEditorHeaderFont", QString::fromStdString(configLocked->getPatternEditorHeaderFont())).toString().toStdString());
		configLocked->setPatternEditorRowsFont(settings.value("patternEditorRowsFont", QString::fromStdString(configLocked->getPatternEditorRowsFont())).toString().toStdString());
		configLocked->setOrderListHeaderFont(settings.value("orderListHeaderFont", QString::fromStdString(configLocked->getOrderListHeaderFont())).toString().toStdString());
		configLocked->setOrderListRowsFont(settings.value("orderListRowsFont", QString::fromStdString(configLocked->getOrderListRowsFont())).toString().toStdString());
		settings.endGroup();

		return true;
	}
	catch (...) {
		return false;
	}
}
}
