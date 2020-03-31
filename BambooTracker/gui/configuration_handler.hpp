#ifndef CONF_HPP
#define CONF_HPP

#include <memory>
#include <QString>
#include <QSettings>
#include "configuration.hpp"

class ConfigurationHandler
{	
public:
	static bool saveConfiguration(std::weak_ptr<Configuration> config);
	static bool loadConfiguration(std::weak_ptr<Configuration> config);

private:
	ConfigurationHandler();
	const static QString organization;
	const static QString application;

	static inline void saveShortcut(QSettings& settings, const QString key, const std::string shortcut)
	{
		settings.setValue(key, QString::fromUtf8(shortcut.c_str(), static_cast<int>(shortcut.length())));
	}

	static inline std::string loadShortcut(const QSettings& settings, const QString key, const std::string shortcut)
	{
		return settings.value(key, QString::fromUtf8(shortcut.c_str(), static_cast<int>(shortcut.length()))
							  ).toString().toUtf8().toStdString();
	}
};

#endif // CONF_HPP
