#ifndef GUI_UTIL_HPP
#define GUI_UTIL_HPP

#include <string>
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

#endif // GUI_UTIL_HPP
