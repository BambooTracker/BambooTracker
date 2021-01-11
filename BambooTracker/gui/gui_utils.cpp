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

#include "gui_utils.hpp"
#include <algorithm>
#include "song.hpp"

namespace gui_utils
{
QString getTrackName(SongType songType, SoundSource src, int chInSrc)
{
	QString name;
	switch (src) {
	case SoundSource::FM:
		switch (songType) {
		case SongType::Standard:
			name = "FM" + QString::number(chInSrc + 1);
			break;
		case SongType::FM3chExpanded:
			switch (chInSrc) {
			case 2:
				name = "FM3-OP1";
				break;
			case 6:
				name = "FM3-OP2";
				break;
			case 7:
				name = "FM3-OP3";
				break;
			case 8:
				name = "FM3-OP4";
				break;
			default:
				name = "FM" + QString::number(chInSrc + 1);
				break;
			}
			break;
		}
		break;
	case SoundSource::SSG:
		name = "SSG" + QString::number(chInSrc + 1);
		break;
	case SoundSource::RHYTHM:
		switch (chInSrc) {
		case 0:	name = "Bass drum";		break;
		case 1:	name = "Snare drum";	break;
		case 2:	name = "Top cymbal";	break;
		case 3:	name = "Hi-hat";		break;
		case 4:	name = "Tom";			break;
		case 5:	name = "Rim shot";		break;
		}
		break;
	case SoundSource::ADPCM:
		name = "ADPCM";
		break;
	}
	return name;
}

std::vector<int> adaptVisibleTrackList(const std::vector<int> list,
									   const SongType prevType, const SongType curType)
{
	if (prevType == curType) return list;

	std::vector<int> tracks;
	if (prevType == SongType::Standard) {
		for (const int& track : list) {
			switch (track) {
			case 0:
			case 1:
				tracks.push_back(track);
				break;
			case 2:
			{
				std::vector<int> tmp { 2, 3, 4, 5 };
				std::copy(tmp.begin(), tmp.end(), std::back_inserter(tracks));
				break;
			}
			default:
				tracks.push_back(track + 3);
				break;
			}
		}
	}
	else {	// FM3chExpanded
		bool hasPushed3 = false;
		for (const int& track : list) {
			switch (track) {
			case 0:
			case 1:
				tracks.push_back(track);
				break;
			case 2:
			case 3:
			case 4:
			case 5:
				if (!hasPushed3) {
					tracks.push_back(2);
					hasPushed3 = true;
				}
				break;
			default:
				tracks.push_back(track - 3);
				break;
			}
		}
	}
	return tracks;
}
}
