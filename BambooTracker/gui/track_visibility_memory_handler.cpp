/*
 * Copyright (C) 2020 Rerrah
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

#include "track_visibility_memory_handler.hpp"
#include <algorithm>
#include <numeric>
#include <QSettings>
#include <QStringList>

// config path (*nix): ~/.config/<ORGANIZATION_>/<FILE_>.ini
const QString TrackVisibilityMemoryHandler::ORGANIZATION_ = "BambooTracker";
const QString TrackVisibilityMemoryHandler::FILE_ = "TrackVisibility";

TrackVisibilityMemoryHandler::TrackVisibilityMemoryHandler() {}

bool TrackVisibilityMemoryHandler::saveTrackVisibilityMemory(
		const SongType type, const std::vector<int>& visTracks)
{
	try {
		QSettings settings(QSettings::IniFormat, QSettings::UserScope, ORGANIZATION_, FILE_);

		int tid;
		switch (type) {
		case SongType::Standard:		tid = 0;	break;
		case SongType::FM3chExpanded:	tid = 1;	break;
		default:	return false;
		}
		settings.setValue("type", tid);

		QString str = std::accumulate(
						  visTracks.begin() + 1, visTracks.end(), QString::number(visTracks.front()),
						  [](QString str, int t) { return QString("%1,%2").arg(str).arg(t); });
		settings.setValue("visTracks", str);

		return true;
	} catch (...) {
		return false;
	}
}

bool TrackVisibilityMemoryHandler::loadTrackVisibilityMemory(
		SongType& type, std::vector<int>& visTracks)
{
	try {
		QSettings settings(QSettings::IniFormat, QSettings::UserScope, ORGANIZATION_, FILE_);

		if (!settings.contains("type")) return false;
		switch (settings.value("type", -1).toInt()) {
		case 0:	type = SongType::Standard;		break;
		case 1:	type = SongType::FM3chExpanded;	break;
		default:	return false;
		}

		if (!settings.contains("visTracks")) return false;
		QString tracks = settings.value("visTracks", "-1").toString();
		QStringList list = tracks.split(",");
		if ((type == SongType::Standard && 16 < list.size())) return false;
		else if ((type == SongType::FM3chExpanded && 19 < list.size())) return false;

		std::vector<int> tl(list.size());
		std::transform(list.begin(), list.end(), tl.begin(), [](const QString& s) { return s.toInt(); });
		std::sort(tl.begin(), tl.end());
		if (tl.front() < 0) return false;
		if ((type == SongType::Standard && 15 < tl.back())) return false;
		else if ((type == SongType::FM3chExpanded && 18 < tl.back())) return false;
		if (std::adjacent_find(tl.begin(), tl.end()) != tl.end()) return false;
		visTracks.swap(tl);

		return true;
	} catch (...) {
		return false;
	}
}
