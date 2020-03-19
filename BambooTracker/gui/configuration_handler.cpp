#include "configuration_handler.hpp"
#include <vector>
#include <unordered_map>
#include <QSettings>
#include "jam_manager.hpp"
#include"enum_hash.hpp"

// config path (*nix): ~/.config/<organization>/<application>.ini
const QString ConfigurationHandler::organization = "BambooTracker";
const QString ConfigurationHandler::application = "BambooTracker";

ConfigurationHandler::ConfigurationHandler() {}

bool ConfigurationHandler::saveConfiguration(std::weak_ptr<Configuration> config)
{
	try {
		QSettings settings(QSettings::IniFormat, QSettings::UserScope, ConfigurationHandler::organization, ConfigurationHandler::application);
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
		settings.setValue("followMode",		configLocked->getFollowMode());
		settings.setValue("workingDirectory",          QString::fromStdString(configLocked->getWorkingDirectory()));
		settings.setValue("instrumentOpenFormat",		configLocked->getInstrumentOpenFormat());
		settings.setValue("bankOpenFormat",				configLocked->getBankOpenFormat());
		settings.setValue("instrumentMask",				configLocked->getInstrumentMask());
		settings.setValue("volumeMask",					configLocked->getVolumeMask());
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
		settings.setValue("showWaveVisual",			configLocked->getShowWaveVisual());
		settings.setValue("fill00ToEffectValue",	configLocked->getFill00ToEffectValue());
		settings.setValue("moveCursorByHScroll",	configLocked->getMoveCursorByHorizontalScroll());
		settings.setValue("overwriteUnusedUnedited",	configLocked->getOverwriteUnusedUneditedPropety());
		settings.setValue("writeOnlyUsedSamples",	configLocked->getWriteOnlyUsedSamples());
		settings.endGroup();

		// Edit settings
		settings.beginGroup("Editing");
		settings.setValue("pageJumpLength", static_cast<int>(configLocked->getPageJumpLength()));
		settings.setValue("editableStep", static_cast<int>(configLocked->getEditableStep()));
		settings.setValue("keyRepetition", configLocked->getKeyRepetition());
		settings.endGroup();

		// Keys
		settings.beginGroup("Keys");
		settings.setValue("keyOffKey",
						  QString::fromUtf8(configLocked->getKeyOffKey().c_str(),
											static_cast<int>(configLocked->getKeyOffKey().length())));
		settings.setValue("octaveUpKey",
						  QString::fromUtf8(configLocked->getOctaveUpKey().c_str(),
											static_cast<int>(configLocked->getOctaveUpKey().length())));
		settings.setValue("octaveDownKey",
						  QString::fromUtf8(configLocked->getOctaveDownKey().c_str(),
											static_cast<int>(configLocked->getOctaveDownKey().length())));
		settings.setValue("echoBufferKey",
						  QString::fromUtf8(configLocked->getEchoBufferKey().c_str(),
											static_cast<int>(configLocked->getEchoBufferKey().length())));
		settings.setValue("noteEntryLayout",	static_cast<int>(configLocked->getNoteEntryLayout()));
		std::unordered_map<std::string, JamKey> customLayoutMapping = configLocked->getCustomLayoutKeys();
		const std::unordered_map<JamKey, std::string> keyToNameMapping = {
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
		std::unordered_map<std::string, JamKey>::const_iterator customLayoutMappingIterator = customLayoutMapping.begin();
		while (customLayoutMappingIterator != customLayoutMapping.end()) {
			settings.setValue(QString::fromStdString("customLayout_" + keyToNameMapping.at(customLayoutMappingIterator->second)),
							  QString::fromUtf8(customLayoutMappingIterator->first.c_str(), static_cast<int> (customLayoutMappingIterator->first.length())));
			customLayoutMappingIterator++;
		}
		settings.endGroup();

		// Sound //
		settings.beginGroup("Sound");
		settings.setValue("soundAPI",
						  QString::fromUtf8(configLocked->getSoundAPI().c_str(),
											static_cast<int>(configLocked->getSoundAPI().length())));
		settings.setValue("soundDevice",
						  QString::fromUtf8(configLocked->getSoundDevice().c_str(),
											static_cast<int>(configLocked->getSoundDevice().length())));
                settings.setValue("realChipInterface",	static_cast<int>(configLocked->getRealChipInterface()));
		settings.setValue("emulator",		configLocked->getEmulator());
		settings.setValue("sampleRate",   static_cast<int>(configLocked->getSampleRate()));
		settings.setValue("bufferLength", static_cast<int>(configLocked->getBufferLength()));
		settings.endGroup();

		// Midi //
		settings.beginGroup("Midi");
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
		for (auto texts : config.lock()->getFMEnvelopeTexts()) {
			settings.setArrayIndex(n++);
			settings.setValue("type", QString::fromUtf8(texts.name.c_str(), static_cast<int>(texts.name.length())));
			QStringList typeList;
			std::transform(texts.texts.begin(), texts.texts.end(), std::back_inserter(typeList),
						   [](FMEnvelopeTextType type) { return QString::number(static_cast<int>(type)); });
			settings.setValue("order", typeList.join(","));
		}
		settings.endArray();
		settings.endGroup();

		settings.beginGroup("Appearance");
		settings.setValue("patternEditorHeaderFont", QString::fromStdString(configLocked->getPatternEditorHeaderFont()));
		settings.setValue("patternEditorHeaderFontSize", configLocked->getPatternEditorHeaderFontSize());
		settings.setValue("patternEditorRowsFont", QString::fromStdString(configLocked->getPatternEditorRowsFont()));
		settings.setValue("patternEditorRowsFontSize", configLocked->getPatternEditorRowsFontSize());
		settings.setValue("orderListHeaderFont", QString::fromStdString(configLocked->getOrderListHeaderFont()));
		settings.setValue("orderListHeaderFontSize", configLocked->getOrderListHeaderFontSize());
		settings.setValue("orderListRowsFont", QString::fromStdString(configLocked->getOrderListRowsFont()));
		settings.setValue("orderListRowsFontSize", configLocked->getOrderListRowsFontSize());
		settings.endGroup();

		return true;
	} catch (...) {
		return false;
	}
}

bool ConfigurationHandler::loadConfiguration(std::weak_ptr<Configuration> config)
{
	try {
		QSettings settings(QSettings::IniFormat, QSettings::UserScope, ConfigurationHandler::organization, ConfigurationHandler::application);
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
		configLocked->setFollowMode(settings.value("followMode", configLocked->getFollowMode()).toBool());
		configLocked->setWorkingDirectory(settings.value("workingDirectory", QString::fromStdString(configLocked->getWorkingDirectory())).toString().toStdString());
		configLocked->setInstrumentOpenFormat(settings.value("instrumentOpenFormat", configLocked->getInstrumentOpenFormat()).toInt());
		configLocked->setBankOpenFormat(settings.value("bankOpenFormat", configLocked->getBankOpenFormat()).toInt());
		configLocked->setInstrumentMask(settings.value("instrumentMask", configLocked->getInstrumentMask()).toBool());
		configLocked->setVolumeMask(settings.value("volumeMask", configLocked->getVolumeMask()).toBool());
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
		configLocked->setShowWaveVisual(settings.value("showWaveVisual", configLocked->getShowWaveVisual()).toBool());
		configLocked->setFill00ToEffectValue(settings.value("fill00ToEffectValue", configLocked->getFill00ToEffectValue()).toBool());
		configLocked->setMoveCursorByHorizontalScroll(settings.value("moveCursorByHScroll", configLocked->getMoveCursorByHorizontalScroll()).toBool());
		configLocked->setOverwriteUnusedUneditedPropety(settings.value("overwriteUnusedUnedited", configLocked->getOverwriteUnusedUneditedPropety()).toBool());
		configLocked->setOverwriteUnusedUneditedPropety(settings.value("writeOnlyUsedSamples", configLocked->getWriteOnlyUsedSamples()).toBool());
		if (settings.contains("autosetInstrument")) {	// For compatibility before v0.4.0
			configLocked->setInstrumentMask(!settings.value("autosetInstrument").toBool());
			settings.remove("autosetInstrument");
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

		// Keys
		settings.beginGroup("Keys");
		configLocked->setKeyOffKey(settings.value("keyOffKey", QString::fromStdString(configLocked->getKeyOffKey())).toString().toUtf8().toStdString());
		configLocked->setOctaveUpKey(settings.value("octaveUpKey", QString::fromStdString(configLocked->getOctaveUpKey())).toString().toUtf8().toStdString());
		configLocked->setOctaveDownKey(settings.value("octaveDownKey", QString::fromStdString(configLocked->getOctaveDownKey())).toString().toUtf8().toStdString());
		configLocked->setEchoBufferKey(settings.value("echoBufferKey", QString::fromStdString(configLocked->getEchoBufferKey())).toString().toUtf8().toStdString());
		configLocked->setNoteEntryLayout(static_cast<Configuration::KeyboardLayout>(
											 settings.value("noteEntryLayout",
															static_cast<int>(configLocked->getNoteEntryLayout())).toInt()));
		std::unordered_map<std::string, JamKey> customLayoutNewKeys;
		const std::unordered_map<std::string, JamKey> nameToKeyMapping = {
			{"lowC",       JamKey::LowC},
			{"lowCS",      JamKey::LowCS},
			{"lowD",       JamKey::LowD},
			{"lowDS",      JamKey::LowDS},
			{"lowE",       JamKey::LowE},
			{"lowF",       JamKey::LowF},
			{"lowFS",      JamKey::LowFS},
			{"lowG",       JamKey::LowG},
			{"lowGS",      JamKey::LowGS},
			{"lowA",       JamKey::LowA},
			{"lowAS",      JamKey::LowAS},
			{"lowB",       JamKey::LowB},
			{"lowHighC",   JamKey::LowC2},
			{"lowHighCS",  JamKey::LowCS2},
			{"lowHighD",   JamKey::LowD2},

			{"highC",      JamKey::HighC},
			{"highCS",     JamKey::HighCS},
			{"highD",      JamKey::HighD},
			{"highDS",     JamKey::HighDS},
			{"highE",      JamKey::HighE},
			{"highF",      JamKey::HighF},
			{"highFS",     JamKey::HighFS},
			{"highG",      JamKey::HighG},
			{"highGS",     JamKey::HighGS},
			{"highA",      JamKey::HighA},
			{"highAS",     JamKey::HighAS},
			{"highB",      JamKey::HighB},
			{"highHighC",  JamKey::HighC2},
			{"highHighCS", JamKey::HighCS2},
			{"highHighD",  JamKey::HighD2}
		};
		std::unordered_map<std::string, JamKey>::const_iterator nameToKeyMappingIterator = nameToKeyMapping.begin();
		while (nameToKeyMappingIterator != nameToKeyMapping.end()) {
			JamKey currentlyWantedJamKey = nameToKeyMappingIterator->second;
			customLayoutNewKeys[
						settings.value(QString::fromStdString("customLayout_" + nameToKeyMappingIterator->first),
									   QString::fromStdString((*std::find_if (configLocked->mappingLayouts.at(Configuration::QWERTY).begin(), configLocked->mappingLayouts.at(Configuration::QWERTY).end(),
																		[currentlyWantedJamKey](const std::pair<std::string, JamKey>& t) -> bool {
																		return (t.second) == currentlyWantedJamKey;})
														 ).first)).toString().toUtf8().toStdString()]
					= currentlyWantedJamKey;
			nameToKeyMappingIterator++;
		}
		configLocked->setCustomLayoutKeys(customLayoutNewKeys);
		settings.endGroup();

		// Sound //
		settings.beginGroup("Sound");
		configLocked->setSoundAPI(settings.value("soundAPI", QString::fromStdString(configLocked->getSoundAPI())).toString().toUtf8().toStdString());
		configLocked->setSoundDevice(settings.value("soundDevice", QString::fromStdString(configLocked->getSoundDevice())).toString().toUtf8().toStdString());
                configLocked->setRealChipInterface(static_cast<RealChipInterface>(
                                                       settings.value("realChipInterface", static_cast<int>(configLocked->getRealChipInterface())).toInt()));
		configLocked->setEmulator(settings.value("emulator", configLocked->getEmulator()).toInt());
		QVariant sampleRateWorkaround;
		sampleRateWorkaround.setValue(configLocked->getSampleRate());
		configLocked->setSampleRate(static_cast<uint32_t>(settings.value("sampleRate", sampleRateWorkaround).toInt()));
		QVariant bufferLengthWorkaround;
		bufferLengthWorkaround.setValue(configLocked->getBufferLength());
		configLocked->setBufferLength(static_cast<size_t>(settings.value("bufferLength", bufferLengthWorkaround).toInt()));
		settings.endGroup();

		// Midi //
		settings.beginGroup("Midi");
		configLocked->setMidiInputPort(settings.value("inputPort").toString().toStdString());
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
			for (auto d : settings.value("order").toString().split(",")) {
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
		configLocked->setPatternEditorHeaderFontSize(settings.value("patternEditorHeaderFontSize", configLocked->getPatternEditorHeaderFontSize()).toInt());
		configLocked->setPatternEditorRowsFont(settings.value("patternEditorRowsFont", QString::fromStdString(configLocked->getPatternEditorRowsFont())).toString().toStdString());
		configLocked->setPatternEditorRowsFontSize(settings.value("patternEditorRowsFontSize", configLocked->getPatternEditorRowsFontSize()).toInt());
		configLocked->setOrderListHeaderFont(settings.value("orderListHeaderFont", QString::fromStdString(configLocked->getOrderListHeaderFont())).toString().toStdString());
		configLocked->setOrderListHeaderFontSize(settings.value("orderListHeaderFontSize", configLocked->getOrderListHeaderFontSize()).toInt());
		configLocked->setOrderListRowsFont(settings.value("orderListRowsFont", QString::fromStdString(configLocked->getOrderListRowsFont())).toString().toStdString());
		configLocked->setOrderListRowsFontSize(settings.value("orderListRowsFontSize", configLocked->getOrderListRowsFontSize()).toInt());
		settings.endGroup();

		return true;
	} catch (...) {
		return false;
	}
}
