/*
 * Copyright (C) 2020-2021 Rerrah
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef JAM_LAYOUT_HPP
#define JAM_LAYOUT_HPP

#include <memory>
#include <QString>
#include <QKeySequence>
#include "configuration.hpp"
#include "jamming.hpp"
#include "utils.hpp"

// Layout decipherer
inline JamKey getJamKeyFromLayoutMapping(Qt::Key key, std::weak_ptr<Configuration> config)
{
	std::shared_ptr<Configuration> configLocked = config.lock();
	Configuration::KeyboardLayout selectedLayout = configLocked->getNoteEntryLayout();
	if (configLocked->mappingLayouts.find(selectedLayout) != configLocked->mappingLayouts.end()) {
		std::unordered_map<std::string, JamKey> selectedLayoutMapping = configLocked->mappingLayouts.at(selectedLayout);
		auto it = utils::findIf(selectedLayoutMapping,
							   [key](const std::pair<std::string, JamKey>& t) -> bool {
			return (QKeySequence(key).matches(QKeySequence(QString::fromStdString(t.first))) == QKeySequence::ExactMatch);
		});
		if (it != selectedLayoutMapping.end()) {
			return it->second;
		}
		else {
			throw std::invalid_argument("Unmapped key");
		}
		//something has gone wrong, current layout has no layout map
		//TODO: handle cleanly?
	} else throw std::out_of_range("Unmapped Layout");
}

#endif // JAM_LAYOUT_HPP
