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

#ifndef GUI_UTILS_HPP
#define GUI_UTILS_HPP

#include <string>
#include <vector>
#include <QString>
#include <QKeySequence>
#include "bamboo_tracker_defs.hpp"

enum class SongType;

namespace gui_utils
{
QString getTrackName(SongType songType, SoundSource src, int chInSrc);

inline QString utf8ToQString(const std::string& str)
{
	return QString::fromUtf8(str.c_str(), static_cast<int>(str.length()));
}

inline QKeySequence strToKeySeq(std::string str)
{
	return QKeySequence(utf8ToQString(str));
}

std::vector<int> adaptVisibleTrackList(const std::vector<int> list,
									   const SongType prevType, const SongType curType);
}

namespace io
{
const QString ORGANIZATION_NAME = "BambooTracker";
}

#endif // GUI_UTILS_HPP
