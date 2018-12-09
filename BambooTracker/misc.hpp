#pragma once

#include <utility>
#include <stdexcept>

enum class SoundSource : int
{
	FM, SSG, DRUM
};

enum class SongType : int
{
	STD,
	FMEX
};

enum class Note : int
{
	C	= 0,
	CS	= 32,
	D	= 64,
	DS	= 96,
	E	= 128,
	F	= 160,
	FS	= 192,
	G	= 224,
	GS	= 256,
	A	= 288,
	AS	= 320,
	B	= 352
};

static std::pair<int, Note> noteNumberToOctaveAndNote(int num)
{
	if (num < 0) return std::pair<int, Note>(0, Note::C);

	int oct = num / 12;
	if (oct > 8) return std::pair<int, Note>(8, Note::B);

	Note note;
	switch (num % 12) {
	case 0:		note = Note::C;		break;
	case 1:		note = Note::CS;	break;
	case 2:		note = Note::D;		break;
	case 3:		note = Note::DS;	break;
	case 4:		note = Note::E;		break;
	case 5:		note = Note::F;		break;
	case 6:		note = Note::FS;	break;
	case 7:		note = Note::G;		break;
	case 8:		note = Note::GS;	break;
	case 9:		note = Note::A;		break;
	case 10:	note = Note::AS;	break;
	case 11:	note = Note::B;		break;
	}

	return std::pair<int, Note>(oct, note);
}

static int octaveAndNoteToNoteNumber(int octave, Note note)
{
	int ret = 12 * octave;

	switch (note) {
	case Note::C:	ret += 0;	break;
	case Note::CS:	ret += 1;	break;
	case Note::D:	ret += 2;	break;
	case Note::DS:	ret += 3;	break;
	case Note::E:	ret += 4;	break;
	case Note::F:	ret += 5;	break;
	case Note::FS:	ret += 6;	break;
	case Note::G:	ret += 7;	break;
	case Note::GS:	ret += 8;	break;
	case Note::A:	ret += 9;	break;
	case Note::AS:	ret += 10;	break;
	case Note::B:	ret += 11;	break;
	}

	return ret;
}

static int ctohex(const char c)
{
	if (c == '0')		return 0;
	else if (c == '1')	return 1;
	else if (c == '2')	return 2;
	else if (c == '3')	return 3;
	else if (c == '4')	return 4;
	else if (c == '5')	return 5;
	else if (c == '6')	return 6;
	else if (c == '7')	return 7;
	else if (c == '8')	return 8;
	else if (c == '9')	return 9;
	else if (c == 'A')	return 10;
	else if (c == 'B')	return 11;
	else if (c == 'C')	return 12;
	else if (c == 'D')	return 13;
	else if (c == 'E')	return 14;
	else if (c == 'F')	return 15;
	else				return -1;
}

static uint8_t uitobcd(const uint8_t v)
{
	if (v > 99) throw std::out_of_range("out of range");

	uint8_t high = v / 10;
	uint8_t low = v % 10;
	return (high << 4) + low;
}

static uint8_t bcdtoui(const uint8_t v)
{
	uint8_t high = v >> 4;
	uint8_t low = v & 0x0f;
	return high * 10 + low;
}
