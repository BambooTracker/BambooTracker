#include "configuration_handler.hpp"
#include <QSettings>

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
		settings.endGroup();

		// Edit settings
		settings.beginGroup("Editing");
		settings.setValue("pageJumpLength", static_cast<int>(configLocked->getPageJumpLength()));
		settings.setValue("editableStep", static_cast<int>(configLocked->getEditableStep()));
		settings.endGroup();

		// Keys
		settings.beginGroup("Keys");
		settings.setValue("keyOffKey",     QString::fromUtf8(configLocked->getKeyOffKey().c_str(),
											 configLocked->getKeyOffKey().length()));
		settings.setValue("octaveUpKey",   QString::fromUtf8(configLocked->getOctaveUpKey().c_str(),
											 configLocked->getOctaveUpKey().length()));
		settings.setValue("octaveDownKey", QString::fromUtf8(configLocked->getOctaveDownKey().c_str(),
											 configLocked->getOctaveDownKey().length()));
		settings.setValue("echoBufferKey", QString::fromUtf8(configLocked->getEchoBufferKey().c_str(),
											 configLocked->getEchoBufferKey().length()));
		settings.endGroup();

		// Sound //
		settings.beginGroup("Sound");
		settings.setValue("soundDevice",  QString::fromUtf8(configLocked->getSoundDevice().c_str(),
											   configLocked->getSoundDevice().length()));
		settings.setValue("useSCCI",		configLocked->getUseSCCI());
		settings.setValue("sampleRate",   static_cast<int>(configLocked->getSampleRate()));
		settings.setValue("bufferLength", static_cast<int>(configLocked->getBufferLength()));
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
		for (auto pair : config.lock()->getFMEnvelopeTextMap()) {
			settings.setArrayIndex(n++);
			settings.setValue("type", QString::fromUtf8(pair.first.c_str(), pair.first.length()));
			QString data;
			for (auto type : pair.second) {
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
		settings.endGroup();

		// Edit settings
		settings.beginGroup("Editing");
		QVariant pageJumpLengthWorkaround;
		pageJumpLengthWorkaround.setValue(configLocked->getPageJumpLength());
		configLocked->setPageJumpLength(static_cast<size_t>(settings.value("pageJumpLength", pageJumpLengthWorkaround).toInt()));
		QVariant editableStepWorkaround;
		editableStepWorkaround.setValue(configLocked->getEditableStep());
		configLocked->setEditableStep(static_cast<size_t>(settings.value("editableStep", editableStepWorkaround).toInt()));
		settings.endGroup();

		// Keys
		settings.beginGroup("Keys");
		configLocked->setKeyOffKey(settings.value("keyOffKey", QString::fromStdString(configLocked->getKeyOffKey())).toString().toUtf8().toStdString());
		configLocked->setOctaveUpKey(settings.value("octaveUpKey", QString::fromStdString(configLocked->getOctaveUpKey())).toString().toUtf8().toStdString());
		configLocked->setOctaveDownKey(settings.value("octaveDownKey", QString::fromStdString(configLocked->getOctaveDownKey())).toString().toUtf8().toStdString());
		configLocked->setEchoBufferKey(settings.value("echoBufferKey", QString::fromStdString(configLocked->getEchoBufferKey())).toString().toUtf8().toStdString());
		settings.endGroup();

		// Sound //
		settings.beginGroup("Sound");
		configLocked->setSoundDevice(settings.value("soundDevice", QString::fromStdString(configLocked->getSoundDevice())).toString().toUtf8().toStdString());
		configLocked->setUseSCCI(settings.value("useSCCI", configLocked->getUseSCCI()).toBool());
		QVariant sampleRateWorkaround;
		sampleRateWorkaround.setValue(configLocked->getSampleRate());
		configLocked->setSampleRate(static_cast<uint32_t>(settings.value("sampleRate", sampleRateWorkaround).toInt()));
		QVariant bufferLengthWorkaround;
		bufferLengthWorkaround.setValue(configLocked->getBufferLength());
		configLocked->setBufferLength(static_cast<size_t>(settings.value("bufferLength", bufferLengthWorkaround).toInt()));
		settings.endGroup();

		// Mixer //
		settings.beginGroup("Mixer");
		configLocked->setMixerVolumeMaster(settings.value("mixerVolumeMaster", configLocked->getMixerVolumeMaster()).toInt());
		configLocked->setMixerVolumeFM(settings.value("mixerVolumeFM", configLocked->getMixerVolumeFM()).toDouble());
		configLocked->setMixerVolumeFM(settings.value("mixerVolumeSSG", configLocked->getMixerVolumeSSG()).toDouble());
		settings.endGroup();

		// Input //
		settings.beginGroup("Input");
		int size = settings.beginReadArray("fmEnvelopeTextMap");
		std::map<std::string, std::vector<FMEnvelopeTextType>> fmEnvelopeTextMap;
		for (int i = 0; i < size; ++i) {
			settings.setArrayIndex(i);
			std::string type = settings.value("type").toString().toUtf8().toStdString();
			std::vector<FMEnvelopeTextType> data;
			for (auto d : settings.value("order").toString().split(",")) {
				data.push_back(static_cast<FMEnvelopeTextType>(d.toInt()));
			}
			fmEnvelopeTextMap.emplace(type, data);
		}
		if (!fmEnvelopeTextMap.empty()) config.lock()->setFMEnvelopeTextMap(fmEnvelopeTextMap);
		settings.endArray();
		settings.endGroup();

		return true;
	} catch (...) {
		return false;
	}
}
