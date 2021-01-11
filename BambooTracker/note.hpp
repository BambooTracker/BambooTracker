/*
 * Copyright (C) 2018-2021 Rerrah
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

#pragma once

#include <cstdint>
#include <utility>

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

namespace note_utils
{
std::pair<int, Note> noteNumberToOctaveAndNote(int num);
int octaveAndNoteToNoteNumber(int octave, Note note);

constexpr int SEMINOTE_PITCH = 32;

inline int octaveAndNoteToPitch(int octave, Note note)
{
	return octaveAndNoteToNoteNumber(octave, note) * SEMINOTE_PITCH;
}

uint16_t calculateFNumber(Note note, int octave, int note_utils, int finePitch);
uint16_t calculateSSGSquareTP(Note note, int octave, int note_utils, int finePitch);
uint16_t calculateSSGSquareTP(int n);
uint16_t calculateSSGTriangleEP(Note note, int octave, int note_utils, int finePitch);
uint16_t calculateSSGSawEP(Note note, int octave, int note_utils, int finePitch);

int calculatePitchIndex(int octave, Note note, int note_utils);
}
