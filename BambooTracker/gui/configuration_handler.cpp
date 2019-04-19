#include "configuration_handler.hpp"
#include <vector>
#include <QSettings>
#include "jam_manager.hpp"

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
		settings.setValue("instrumentFMWindowWidth",   configLocked->getInstrumentFMWindowWidth());
		settings.setValue("instrumentFMWindowHeight",  configLocked->getInstrumentFMWindowHeight());
		settings.setValue("instrumentSSGWindowWidth",  configLocked->getInstrumentSSGWindowWidth());
		settings.setValue("instrumentSSGWindowHeight", configLocked->getInstrumentSSGWindowHeight());
		settings.setValue("followMode",		configLocked->getFollowMode());
		settings.setValue("workingDirectory",          QString::fromStdString(configLocked->getWorkingDirectory()));
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
		settings.setValue("showWaveVisual",		configLocked->getShowWaveVisual());
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
		std::map<std::string, JamKey> customLayoutMapping = configLocked->getCustomLayoutKeys();
		const std::map<JamKey, std::string> keyToNameMapping = {
			{JamKey::LOW_C,     "lowC"},
			{JamKey::LOW_CS,    "lowCS"},
			{JamKey::LOW_D,     "lowD"},
			{JamKey::LOW_DS,    "lowDS"},
			{JamKey::LOW_E,     "lowE"},
			{JamKey::LOW_F,     "lowF"},
			{JamKey::LOW_FS,    "lowFS"},
			{JamKey::LOW_G,     "lowG"},
			{JamKey::LOW_GS,    "lowGS"},
			{JamKey::LOW_A,     "lowA"},
			{JamKey::LOW_AS,    "lowAS"},
			{JamKey::LOW_B,     "lowB"},
			{JamKey::LOW_C_H,   "lowHighC"},
			{JamKey::LOW_CS_H,  "lowHighCS"},
			{JamKey::LOW_D_H,   "lowHighD"},

			{JamKey::HIGH_C,    "highC"},
			{JamKey::HIGH_CS,   "highCS"},
			{JamKey::HIGH_D,    "highD"},
			{JamKey::HIGH_DS,   "highDS"},
			{JamKey::HIGH_E,    "highE"},
			{JamKey::HIGH_F,    "highF"},
			{JamKey::HIGH_FS,   "highFS"},
			{JamKey::HIGH_G,    "highG"},
			{JamKey::HIGH_GS,   "highGS"},
			{JamKey::HIGH_A,    "highA"},
			{JamKey::HIGH_AS,   "highAS"},
			{JamKey::HIGH_B,    "highB"},
			{JamKey::HIGH_C_H,  "highHighC"},
			{JamKey::HIGH_CS_H, "highHighCS"},
			{JamKey::HIGH_D_H,  "highHighD"}
		};
		std::map<std::string, JamKey>::const_iterator customLayoutMappingIterator = customLayoutMapping.begin();
		while (customLayoutMappingIterator != customLayoutMapping.end()) {
			settings.setValue(QString::fromStdString("customLayout_" + keyToNameMapping.at(customLayoutMappingIterator->second)),
							  QString::fromUtf8(customLayoutMappingIterator->first.c_str(), static_cast<int> (customLayoutMappingIterator->first.length())));
			customLayoutMappingIterator++;
		}
		settings.endGroup();

		// Sound //
		settings.beginGroup("Sound");
		settings.setValue("soundDevice",
						  QString::fromUtf8(configLocked->getSoundDevice().c_str(),
											static_cast<int>(configLocked->getSoundDevice().length())));
		settings.setValue("useSCCI",		configLocked->getUseSCCI());
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
			QString data;
			for (auto type : texts.texts) {
				data += QString(",%1").arg(static_cast<int>(type));
			}
			if (!data.isEmpty()) data.remove(0, 1);
			settings.setValue("order", data);
		}
		settings.endArray();
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
		configLocked->setInstrumentFMWindowWidth(settings.value("instrumentFMWindowWidth", configLocked->getInstrumentFMWindowWidth()).toInt());
		configLocked->setInstrumentFMWindowHeight(settings.value("instrumentFMWindowHeight", configLocked->getInstrumentFMWindowHeight()).toInt());
		configLocked->setInstrumentSSGWindowWidth(settings.value("instrumentSSGWindowWidth", configLocked->getInstrumentSSGWindowWidth()).toInt());
		configLocked->setInstrumentSSGWindowHeight(settings.value("instrumentSSGWindowHeight", configLocked->getInstrumentSSGWindowHeight()).toInt());
		configLocked->setFollowMode(settings.value("followMode", configLocked->getFollowMode()).toBool());
		configLocked->setWorkingDirectory(settings.value("workingDirectory", QString::fromStdString(configLocked->getWorkingDirectory())).toString().toStdString());
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
		std::map<std::string, JamKey> customLayoutNewKeys = {};
		const std::map<std::string, JamKey> nameToKeyMapping = {
			{"lowC",       JamKey::LOW_C},
			{"lowCS",      JamKey::LOW_CS},
			{"lowD",       JamKey::LOW_D},
			{"lowDS",      JamKey::LOW_DS},
			{"lowE",       JamKey::LOW_E},
			{"lowF",       JamKey::LOW_F},
			{"lowFS",      JamKey::LOW_FS},
			{"lowG",       JamKey::LOW_G},
			{"lowGS",      JamKey::LOW_GS},
			{"lowA",       JamKey::LOW_A},
			{"lowAS",      JamKey::LOW_AS},
			{"lowB",       JamKey::LOW_B},
			{"lowHighC",   JamKey::LOW_C_H},
			{"lowHighCS",  JamKey::LOW_CS_H},
			{"lowHighD",   JamKey::LOW_D_H},

			{"highC",      JamKey::HIGH_C},
			{"highCS",     JamKey::HIGH_CS},
			{"highD",      JamKey::HIGH_D},
			{"highDS",     JamKey::HIGH_DS},
			{"highE",      JamKey::HIGH_E},
			{"highF",      JamKey::HIGH_F},
			{"highFS",     JamKey::HIGH_FS},
			{"highG",      JamKey::HIGH_G},
			{"highGS",     JamKey::HIGH_GS},
			{"highA",      JamKey::HIGH_A},
			{"highAS",     JamKey::HIGH_AS},
			{"highB",      JamKey::HIGH_B},
			{"highHighC",  JamKey::HIGH_C_H},
			{"highHighCS", JamKey::HIGH_CS_H},
			{"highHighD",  JamKey::HIGH_D_H}
		};
		std::map<std::string, JamKey>::const_iterator nameToKeyMappingIterator = nameToKeyMapping.begin();
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
		configLocked->setSoundDevice(settings.value("soundDevice", QString::fromStdString(configLocked->getSoundDevice())).toString().toUtf8().toStdString());
		configLocked->setUseSCCI(settings.value("useSCCI", configLocked->getUseSCCI()).toBool());
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

		return true;
	} catch (...) {
		return false;
	}
}
