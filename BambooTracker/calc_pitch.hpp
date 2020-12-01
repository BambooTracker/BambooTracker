/*
 * Copyright (C) 2018-2020 Rerrah
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
#include "misc.hpp"

namespace calc_pitch
{
enum : int { SEMINOTE_PITCH = 32 };

uint16_t calculateFNumber(Note note, int octave, int calc_pitch, int finePitch);
uint16_t calculateSSGSquareTP(Note note, int octave, int calc_pitch, int finePitch);
uint16_t calculateSSGSquareTP(int n);
uint16_t calculateSSGTriangleEP(Note note, int octave, int calc_pitch, int finePitch);
uint16_t calculateSSGSawEP(Note note, int octave, int calc_pitch, int finePitch);

int calculatePitchIndex(int octave, Note note, int calc_pitch);
}	// namespace calc_pitch
