#ifndef JAM_LAYOUT_HPP
#define JAM_LAYOUT_HPP

#include <memory>
#include <QString>
#include <QKeySequence>
#include "configuration.hpp"
#include "jam_manager.hpp"

// Layout decipherer
inline JamKey getJamKeyFromLayoutMapping(Qt::Key key, std::weak_ptr<Configuration> config)
{
	std::shared_ptr<Configuration> configLocked = config.lock();
	Configuration::KeyboardLayout selectedLayout = configLocked->getNoteEntryLayout();
	if (configLocked->mappingLayouts.find (selectedLayout) != configLocked->mappingLayouts.end()) {
		std::unordered_map<std::string, JamKey> selectedLayoutMapping = configLocked->mappingLayouts.at (selectedLayout);
		auto it = std::find_if(selectedLayoutMapping.begin(), selectedLayoutMapping.end(),
							   [key](const std::pair<std::string, JamKey>& t) -> bool {
			return (QKeySequence(key).matches(QKeySequence(QString::fromStdString(t.first))) == QKeySequence::ExactMatch);
		});
		if (it != selectedLayoutMapping.end()) {
			return (*it).second;
		}
		else {
			throw std::invalid_argument("Unmapped key");
		}
		//something has gone wrong, current layout has no layout map
		//TODO: handle cleanly?
	} else throw std::out_of_range("Unmapped Layout");
}

#endif // JAM_LAYOUT_HPP
