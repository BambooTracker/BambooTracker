#ifndef GUI_UTIL_HPP
#define GUI_UTIL_HPP

#include <string>
#include <vector>
#include <algorithm>
#include <QString>
#include <QKeySequence>
#include "song.hpp"
#include "misc.hpp"

inline QString getTrackName(SongType songType, SoundSource src, int chInSrc)
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
	case SoundSource::DRUM:
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

inline QString utf8ToQString(const std::string& str)
{
	return QString::fromUtf8(str.c_str(), static_cast<int>(str.length()));
}

inline QKeySequence strToKeySeq(std::string str)
{
	return QKeySequence(utf8ToQString(str));
}

inline std::vector<int> adaptVisibleTrackList(const std::vector<int> list,
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

#endif // GUI_UTIL_HPP
