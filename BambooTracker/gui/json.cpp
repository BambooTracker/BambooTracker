#include "json.hpp"
#include <QFile>
#include <QJsonObject>
#include <QJsonDocument>

const QString Json::CONFIG_PATH = "./config.json";

Json::Json() {}

bool Json::saveConfiguration(std::weak_ptr<Configuration> config)
{
	try {
		QJsonObject obj;

		// Internal //
		obj["mainWindowWidth"] = config.lock()->getMainWindowWidth();
		obj["mainWindowHeight"] = config.lock()->getMainWindowHeight();
		obj["mainWindowMaximized"] = config.lock()->getMainWindowMaximized();
		obj["mainWindowX"] = config.lock()->getMainWindowX();
		obj["mainWindowY"] = config.lock()->getMainWindowY();
		obj["instrumentFMWindowWidth"] = config.lock()->getInstrumentFMWindowWidth();
		obj["instrumentFMWindowHeight"] = config.lock()->getInstrumentFMWindowHeight();
		obj["instrumentSSGWindowWidth"] = config.lock()->getInstrumentSSGWindowWidth();
		obj["instrumentSSGWindowHeight"] = config.lock()->getInstrumentSSGWindowHeight();
		obj["followMode"] = config.lock()->getFollowMode();
		obj["workingDirectory"] = QString::fromStdString(config.lock()->getWorkingDirectory());

		// General //
		// General settings
		obj["warpCursor"] = config.lock()->getWarpCursor();
		obj["warpAcrossOrders"] = config.lock()->getWarpAcrossOrders();
		obj["showRowNumberInHex"] = config.lock()->getShowRowNumberInHex();
		obj["showPreviousNextOrders"] = config.lock()->getShowPreviousNextOrders();
		obj["backupModule"] = config.lock()->getBackupModules();
		obj["dontSelectOnDoubleClick"] = config.lock()->getDontSelectOnDoubleClick();
		obj["reverseFMVolumeOrder"] = config.lock()->getReverseFMVolumeOrder();

		// Edit settings
		obj["pageJumpLength"] = static_cast<int>(config.lock()->getPageJumpLength());

		// Keys
		obj["keyOffKey"] = QString::fromUtf8(config.lock()->getKeyOffKey().c_str(),
											 config.lock()->getKeyOffKey().length());
		obj["octaveUpKey"] = QString::fromUtf8(config.lock()->getOctaveUpKey().c_str(),
											 config.lock()->getOctaveUpKey().length());
		obj["octaveDownKey"] = QString::fromUtf8(config.lock()->getOctaveDownKey().c_str(),
											 config.lock()->getOctaveDownKey().length());
		obj["echoBufferKey"] = QString::fromUtf8(config.lock()->getEchoBufferKey().c_str(),
											 config.lock()->getEchoBufferKey().length());

		// Sound //
		obj["soundDevice"] = QString::fromUtf8(config.lock()->getSoundDevice().c_str(),
											   config.lock()->getSoundDevice().length());
		obj["sampleRate"] = static_cast<int>(config.lock()->getSampleRate());
		obj["bufferLength"] = static_cast<int>(config.lock()->getBufferLength());

		QJsonDocument doc(obj);
		QFile file(CONFIG_PATH);
		if (!file.open(QIODevice::WriteOnly)) return false;
		file.write(doc.toJson());

		return true;
	} catch (...) {
		return false;
	}
}

bool Json::loadConfiguration(std::weak_ptr<Configuration> config)
{
	try {
		QFile file(CONFIG_PATH);
		if (!file.open(QIODevice::ReadOnly)) return false;

		QJsonObject obj(QJsonDocument::fromJson(file.readAll()).object());

		// Internal //
		config.lock()->setMainWindowWidth(obj["mainWindowWidth"].toInt());
		config.lock()->setMainWindowHeight(obj["mainWindowHeight"].toInt());
		config.lock()->setMainWindowMaximized(obj["mainWindowMaximized"].toBool());
		config.lock()->setMainWindowX(obj["mainWindowX"].toInt());
		config.lock()->setMainWindowY(obj["mainWindowY"].toInt());
		config.lock()->setInstrumentFMWindowWidth(obj["instrumentFMWindowWidth"].toInt());
		config.lock()->setInstrumentFMWindowHeight(obj["instrumentFMWindowHeight"].toInt());
		config.lock()->setInstrumentSSGWindowWidth(obj["instrumentSSGWindowWidth"].toInt());
		config.lock()->setInstrumentSSGWindowHeight(obj["instrumentSSGWindowHeight"].toInt());
		config.lock()->setFollowMode(obj["followMode"].toBool());
		config.lock()->setWorkingDirectory(obj["workingDirectory"].toString().toStdString());

		// General //
		// General settings
		config.lock()->setWarpCursor(obj["warpCursor"].toBool());
		config.lock()->setWarpAcrossOrders(obj["warpAcrossOrders"].toBool());
		config.lock()->setShowRowNumberInHex(obj["showRowNumberInHex"].toBool());
		config.lock()->setShowPreviousNextOrders(obj["showPreviousNextOrders"].toBool());
		config.lock()->setBackupModules(obj["backupModule"].toBool());
		config.lock()->setDontSelectOnDoubleClick(obj["dontSelectOnDoubleClick"].toBool());
		config.lock()->setReverseFMVolumeOrder(obj["reverseFMVolumeOrder"].toBool());

		// Edit settings
		config.lock()->setPageJumpLength(static_cast<size_t>(obj["pageJumpLength"].toInt()));

		// Keys
		config.lock()->setKeyOffKey(obj["keyOffKey"].toString().toUtf8().toStdString());
		config.lock()->setOctaveUpKey(obj["octaveUpKey"].toString().toUtf8().toStdString());
		config.lock()->setOctaveDownKey(obj["octaveDownKey"].toString().toUtf8().toStdString());
		config.lock()->setEchoBufferKey(obj["echoBufferKey"].toString().toUtf8().toStdString());

		// Sound //
		config.lock()->setSoundDevice(obj["soundDevice"].toString().toUtf8().toStdString());
		config.lock()->setSampleRate(static_cast<uint32_t>(obj["sampleRate"].toInt()));
		config.lock()->setBufferLength(static_cast<size_t>(obj["bufferLength"].toInt()));

		return true;
	} catch (...) {
		return false;
	}
}
