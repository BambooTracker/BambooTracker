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
		settings.endGroup();

		// Edit settings
		settings.beginGroup("Editing");
		settings.setValue("pageJumpLength", static_cast<int>(configLocked->getPageJumpLength()));
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
		settings.endGroup();

		// Edit settings
		settings.beginGroup("Editing");
		QVariant pageJumpLengthWorkaround;
		pageJumpLengthWorkaround.setValue(configLocked->getPageJumpLength());
		configLocked->setPageJumpLength(static_cast<size_t>(settings.value("pageJumpLength", pageJumpLengthWorkaround).toInt()));
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

		return true;
	} catch (...) {
		return false;
	}
}
