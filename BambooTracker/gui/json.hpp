#ifndef JSON_HPP
#define JSON_HPP

#include <memory>
#include <QString>
#include "configuration.hpp"

class Json
{	
public:
	static bool saveConfiguration(std::weak_ptr<Configuration> config);
	static bool loadConfiguration(std::weak_ptr<Configuration> config);

private:
	Json();
	const static QString CONFIG_PATH;
};

#endif // JSON_HPP
