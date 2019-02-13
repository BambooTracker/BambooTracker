#ifndef CONF_HPP
#define CONF_HPP

#include <memory>
#include <QString>
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
};

#endif // CONF_HPP
