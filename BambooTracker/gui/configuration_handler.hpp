#ifndef CONF_HPP
#define CONF_HPP

#include <memory>
#include <unordered_map>
#include <QString>
#include <QSettings>
#include "configuration.hpp"

enum class JamKey : int;

class ConfigurationHandler
{	
public:
	static bool saveConfiguration(std::weak_ptr<Configuration> config);
	static bool loadConfiguration(std::weak_ptr<Configuration> config);

private:
	ConfigurationHandler();
	const static QString ORGANIZATION_;
	const static QString APPLICATION_;

	const static std::unordered_map<Configuration::ShortcutAction, QString> SHORTCUTS_NAME_MAP_;
	const static std::unordered_map<JamKey, QString> JAM_KEY_NAME_MAP_;

	static inline std::string loadShortcut(const QSettings& settings, const QString key, const std::string shortcut)
	{
		return settings.value(key, QString::fromUtf8(shortcut.c_str(), static_cast<int>(shortcut.length()))
							  ).toString().toUtf8().toStdString();
	}
};

#endif // CONF_HPP
