#pragma once

#include <utility>

enum class SoundSource : int
{
	FM, SSG
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
	int oct = num / 12;
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
