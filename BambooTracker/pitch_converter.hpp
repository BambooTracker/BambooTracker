#pragma once

#include <cstdint>
#include "misc.hpp"

class PitchConverter
{
public:
	static uint16_t getPitchFM(Note note, int octave, int pitch);
	static uint16_t getPitchSSGSquare(Note note, int octave, int pitch);
	static uint16_t getPitchSSGSquare(int n);
	static uint16_t getPitchSSGTriangle(Note note, int octave, int pitch);
	static uint16_t getPitchSSGSaw(Note note, int octave, int pitch);

private:
	static const uint16_t centTableFM_[3072];
	static const uint16_t centTableSSGSquare_[3072];
	static const uint16_t centTableSSGTriangle_[3072];
	static const uint16_t centTableSSGSaw_[3072];

	PitchConverter();

	static int calculateIndex(int octave, Note note, int pitch)
	{
		int idx = 384 * octave + static_cast<int>(note) + pitch;
		if (idx < 0) return 0;
		else if (idx > 3071) return 3071;
		else return idx;
	}
};
