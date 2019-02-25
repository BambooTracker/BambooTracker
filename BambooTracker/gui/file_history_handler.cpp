#include "file_history_handler.hpp"
#include <QSettings>

// config path (*nix): ~/.config/<ORGANIZATION_>/<FILE_>.ini
const QString FileHistoryHandler::ORGANIZATION_ = "BambooTracker";
const QString FileHistoryHandler::FILE_ = "FileHistory";

FileHistoryHandler::FileHistoryHandler() {}

bool FileHistoryHandler::saveFileHistory(std::weak_ptr<FileHistory> history)
{
	try {
		QSettings settings(QSettings::IniFormat, QSettings::UserScope, ORGANIZATION_, FILE_);
		settings.beginWriteArray("fileHistory");
		int n = 0;
		for (size_t i = 0; i < history.lock()->size(); ++i) {
			settings.setArrayIndex(n++);
			settings.setValue("path", history.lock()->at(i));
		}
		settings.endArray();
		return true;
	} catch (...) {
		return false;
	}
}

bool FileHistoryHandler::loadFileHistory(std::weak_ptr<FileHistory> history)
{
	try {
		QSettings settings(QSettings::IniFormat, QSettings::UserScope, ORGANIZATION_, FILE_);
		int size = settings.beginReadArray("fileHistory");
		history.lock()->clearHistory();
		for (int i = size - 1; 0 <= i; --i) {
			settings.setArrayIndex(i);
			history.lock()->addFile(settings.value("path").toString());
		}
		settings.endArray();
		return true;
	} catch (...) {
		return false;
	}
}
