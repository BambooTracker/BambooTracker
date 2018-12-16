#include "configuration_handler.hpp"
#include <QFile>
#include <QSettings>

// config path (*nix): ~/.config/<organization>/<application>.ini
const QString ConfigurationHandler::organization = "BambooTracker";
const QString ConfigurationHandler::application = "BambooTracker";

ConfigurationHandler::ConfigurationHandler() {}

bool ConfigurationHandler::saveConfiguration(std::weak_ptr<Configuration> config)
{
	try {
		QSettings settings(QSettings::IniFormat, QSettings::UserScope, ConfigurationHandler::organization, ConfigurationHandler::application);

		// Internal //
		settings.beginGroup("Internal");
		settings.setValue("mainWindowWidth",           config.lock()->getMainWindowWidth());
		settings.setValue("mainWindowHeight",          config.lock()->getMainWindowHeight());
		settings.setValue("mainWindowMaximized",       config.lock()->getMainWindowMaximized());
		settings.setValue("mainWindowX",               config.lock()->getMainWindowX());
		settings.setValue("mainWindowY",               config.lock()->getMainWindowY());
		settings.setValue("instrumentFMWindowWidth",   config.lock()->getInstrumentFMWindowWidth());
		settings.setValue("instrumentFMWindowHeight",  config.lock()->getInstrumentFMWindowHeight());
		settings.setValue("instrumentSSGWindowWidth",  config.lock()->getInstrumentSSGWindowWidth());
		settings.setValue("instrumentSSGWindowHeight", config.lock()->getInstrumentSSGWindowHeight());
		settings.setValue("followMode",		config.lock()->getFollowMode());
		settings.setValue("workingDirectory",          QString::fromStdString(config.lock()->getWorkingDirectory()));
		settings.endGroup();

		// General //
		// General settings
		settings.beginGroup("General");
		settings.setValue("warpCursor",              config.lock()->getWarpCursor());
		settings.setValue("warpAcrossOrders",        config.lock()->getWarpAcrossOrders());
		settings.setValue("showRowNumberInHex",      config.lock()->getShowRowNumberInHex());
		settings.setValue("showPreviousNextOrders",  config.lock()->getShowPreviousNextOrders());
		settings.setValue("backupModule",            config.lock()->getBackupModules());
		settings.setValue("dontSelectOnDoubleClick", config.lock()->getDontSelectOnDoubleClick());
		settings.setValue("reverseFMVolumeOrder",    config.lock()->getReverseFMVolumeOrder());
		settings.endGroup();

		// Edit settings
		settings.beginGroup("Editing");
		settings.setValue("pageJumpLength", static_cast<int>(config.lock()->getPageJumpLength()));
		settings.endGroup();

		// Keys
		settings.beginGroup("Keys");
		settings.setValue("keyOffKey",     QString::fromUtf8(config.lock()->getKeyOffKey().c_str(),
											 config.lock()->getKeyOffKey().length()));
		settings.setValue("octaveUpKey",   QString::fromUtf8(config.lock()->getOctaveUpKey().c_str(),
											 config.lock()->getOctaveUpKey().length()));
		settings.setValue("octaveDownKey", QString::fromUtf8(config.lock()->getOctaveDownKey().c_str(),
											 config.lock()->getOctaveDownKey().length()));
		settings.setValue("echoBufferKey", QString::fromUtf8(config.lock()->getEchoBufferKey().c_str(),
											 config.lock()->getEchoBufferKey().length()));
		settings.endGroup();

		// Sound //
		settings.beginGroup("Sound");
		settings.setValue("soundDevice",  QString::fromUtf8(config.lock()->getSoundDevice().c_str(),
											   config.lock()->getSoundDevice().length()));
		settings.setValue("sampleRate",   static_cast<int>(config.lock()->getSampleRate()));
		settings.setValue("bufferLength", static_cast<int>(config.lock()->getBufferLength()));
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

		// Internal //
		settings.beginGroup("Internal");
		config.lock()->setMainWindowWidth(settings.value("mainWindowWidth", "930").toInt());
		config.lock()->setMainWindowHeight(settings.value("mainWindowHeight", "780").toInt());
		config.lock()->setMainWindowMaximized(settings.value("mainWindowMaximized", "false").toBool());
		config.lock()->setMainWindowX(settings.value("mainWindowX", "-1").toInt());
		config.lock()->setMainWindowY(settings.value("mainWindowY", "-1").toInt());
		config.lock()->setInstrumentFMWindowWidth(settings.value("instrumentFMWindowWidth", "570").toInt());
		config.lock()->setInstrumentFMWindowHeight(settings.value("instrumentFMWindowHeight", "680").toInt());
		config.lock()->setInstrumentSSGWindowWidth(settings.value("instrumentSSGWindowWidth", "500").toInt());
		config.lock()->setInstrumentSSGWindowHeight(settings.value("instrumentSSGWindowHeight", "390").toInt());
		config.lock()->setFollowMode(settings.value("followMode", "true").toBool());
		config.lock()->setWorkingDirectory(settings.value("workingDirectory", "").toString().toStdString());
		settings.endGroup();

		// General //
		// General settings
		settings.beginGroup("General");
		config.lock()->setWarpCursor(settings.value("warpCursor", "true").toBool());
		config.lock()->setWarpAcrossOrders(settings.value("warpAcrossOrders", "true").toBool());
		config.lock()->setShowRowNumberInHex(settings.value("showRowNumberInHex", "true").toBool());
		config.lock()->setShowPreviousNextOrders(settings.value("showPreviousNextOrders", "true").toBool());
		config.lock()->setBackupModules(settings.value("backupModule", "true").toBool());
		config.lock()->setDontSelectOnDoubleClick(settings.value("dontSelectOnDoubleClick", "false").toBool());
		config.lock()->setReverseFMVolumeOrder(settings.value("reverseFMVolumeOrder", "true").toBool());
		settings.endGroup();

		// Edit settings
		settings.beginGroup("Editing");
		config.lock()->setPageJumpLength(static_cast<size_t>(settings.value("pageJumpLength", "4").toInt()));
		settings.endGroup();

		// Keys
		settings.beginGroup("Keys");
		config.lock()->setKeyOffKey(settings.value("keyOffKey", "-").toString().toUtf8().toStdString());
		config.lock()->setOctaveUpKey(settings.value("octaveUpKey", "*").toString().toUtf8().toStdString());
		config.lock()->setOctaveDownKey(settings.value("octaveDownKey", "/").toString().toUtf8().toStdString());
		config.lock()->setEchoBufferKey(settings.value("echoBufferKey", "^").toString().toUtf8().toStdString());
		settings.endGroup();

		// Sound //
		settings.beginGroup("Sound");
		config.lock()->setSoundDevice(settings.value("soundDevice", "").toString().toUtf8().toStdString());
		config.lock()->setSampleRate(static_cast<uint32_t>(settings.value("sampleRate", "44100").toInt()));
		config.lock()->setBufferLength(static_cast<size_t>(settings.value("bufferLength", "40").toInt()));
		settings.endGroup();

		return true;
	} catch (...) {
		return false;
	}
}
